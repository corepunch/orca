"""Offline behavior checks for generated native iPhone/iPad Xcode projects."""
import json
import os
from pathlib import Path
import plistlib
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[2]
GENERATOR = ROOT / 'tools/xcode/generate.py'


def openstep(value):
    """Write edited project data in Xcode's property-list format."""
    if isinstance(value, dict):
        entries = (openstep(key) + ' = ' + openstep(item) + ';' for key, item in value.items())
        return '{\n' + '\n'.join(entries) + '\n}'
    if isinstance(value, list):
        return '(' + ',\n'.join(map(openstep, value)) + ')'
    return str(value) if isinstance(value, int) else json.dumps(value)


@unittest.skipUnless(shutil.which('plutil'), 'Apple plutil is required to read Xcode project files')
class GenerateProjectTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='orca-xcode-generator-')
        self.addCleanup(self.temporary.cleanup)
        self.output = Path(self.temporary.name) / 'output'

    def run_generator(self, *args, generator=GENERATOR, success=True):
        result = subprocess.run([sys.executable, str(generator), *map(str, args)],
                                cwd=ROOT, capture_output=True, text=True)
        if success:
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        else:
            self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        return result

    def generate(self, app='chat', *args, output=None):
        destination = output or self.output
        self.run_generator(app, '--output', destination, *args)
        return destination / (app + '.xcodeproj')

    def read_project(self, bundle):
        result = subprocess.run(['plutil', '-convert', 'json', '-o', '-',
                                 str(bundle / 'project.pbxproj')], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        return json.loads(result.stdout)

    def configurations(self, project, owner):
        objects = project['objects']
        refs = objects[owner['buildConfigurationList']]['buildConfigurations']
        return {objects[ref]['name']: objects[ref]['buildSettings'] for ref in refs}

    def application(self, project):
        targets = [(uid, obj) for uid, obj in project['objects'].items()
                   if obj['isa'] == 'PBXNativeTarget' and
                   obj['productType'] == 'com.apple.product-type.application']
        self.assertEqual(len(targets), 1)
        return targets[0]

    def effective_configurations(self, project):
        root = project['objects'][project['rootObject']]
        common = self.configurations(project, root)
        _, target = self.application(project)
        return {name: dict(common[name], **settings)
                for name, settings in self.configurations(project, target).items()}

    def read_info(self, bundle, settings):
        path = settings['INFOPLIST_FILE'].replace('$(PROJECT_FILE_PATH)', str(bundle))
        path = path.replace('$(SRCROOT)', str(bundle.parent)).replace('$(PROJECT_DIR)', str(bundle.parent))
        with Path(path).open('rb') as stream:
            return plistlib.load(stream)

    def customize_signing(self, bundle):
        project = self.read_project(bundle)
        _, target = self.application(project)
        expected = {}
        for name, settings in self.configurations(project, target).items():
            values = dict(DEVELOPMENT_TEAM='CUSTOM' + name.upper(),
                          PRODUCT_BUNDLE_IDENTIFIER='example.custom.' + name.lower(),
                          CODE_SIGN_STYLE='Manual', CODE_SIGN_IDENTITY='Apple Distribution',
                          PROVISIONING_PROFILE_SPECIFIER='Custom ' + name,
                          MARKETING_VERSION='2.7', CURRENT_PROJECT_VERSION='42')
            settings.update(values)
            expected[name] = values
        (bundle / 'project.pbxproj').write_text('// !$*UTF8*$!\n' + openstep(project) + '\n')
        return expected

    def test_regeneration_is_deterministic_and_does_not_rewrite_unchanged_files(self):
        bundle = self.generate()
        project = self.read_project(bundle)
        self.assertTrue(all(re.fullmatch('[A-F0-9]{24}', uid) for uid in project['objects']))
        before = {p.relative_to(bundle): p.read_bytes() for p in bundle.rglob('*') if p.is_file()}
        sentinel = 1_700_000_000_000_000_000
        for relative in before:
            os.utime(bundle / relative, ns=(sentinel, sentinel))
        self.generate()
        after = {p.relative_to(bundle): p.read_bytes() for p in bundle.rglob('*') if p.is_file()}
        self.assertEqual(before, after)
        self.assertTrue(all((bundle / relative).stat().st_mtime_ns == sentinel for relative in before))

    def test_native_target_universal_scheme_and_scene_manifest(self):
        bundle = self.generate('chat', '--display-name', 'QA Chat', '--minimum', '17.0')
        project = self.read_project(bundle)
        target_id, target = self.application(project)
        phase_types = [project['objects'][ref]['isa'] for ref in target['buildPhases']]
        self.assertIn('PBXSourcesBuildPhase', phase_types)
        self.assertEqual(set(self.effective_configurations(project)), {'Debug', 'Release'})
        for settings in self.effective_configurations(project).values():
            self.assertEqual(settings['SDKROOT'], 'iphoneos')
            self.assertEqual(set(settings['SUPPORTED_PLATFORMS'].split()), {'iphoneos', 'iphonesimulator'})
            self.assertEqual(settings['TARGETED_DEVICE_FAMILY'].replace(' ', ''), '1,2')
            self.assertEqual(settings['IPHONEOS_DEPLOYMENT_TARGET'], '17.0')
            self.assertEqual(settings['SKIP_INSTALL'], 'NO')
            info = self.read_info(bundle, settings)
            self.assertEqual(info['CFBundleDisplayName'], 'QA Chat')
            self.assertEqual(info['UIDeviceFamily'], [1, 2])
            scene = info['UIApplicationSceneManifest']['UISceneConfigurations']
            delegates = scene['UIWindowSceneSessionRoleApplication']
            self.assertTrue(any(item['UISceneDelegateClassName'] == 'OrcaSceneDelegate' for item in delegates))
        scheme = ET.parse(bundle / 'xcshareddata/xcschemes/chat-ios.xcscheme').getroot()
        archive = scheme.find('ArchiveAction')
        self.assertIsNotNone(archive)
        self.assertEqual(archive.get('buildConfiguration'), 'Release')
        entry = scheme.find('BuildAction/BuildActionEntries/BuildActionEntry')
        self.assertEqual(entry.get('buildForArchiving'), 'YES')
        self.assertEqual(entry.get('buildForRunning'), 'YES')
        for reference in scheme.iter('BuildableReference'):
            self.assertEqual(reference.get('BlueprintIdentifier'), target_id)
            self.assertEqual(reference.get('ReferencedContainer'), 'container:chat.xcodeproj')

    def test_output_outside_apps_uses_relative_repository_paths_and_no_host_artifacts(self):
        bundle = self.generate()
        project = self.read_project(bundle)
        for settings in self.effective_configurations(project).values():
            repository = settings['ORCA_ROOT'].replace('$(SRCROOT)', str(bundle.parent))
            application = settings['ORCA_PROJECT'].replace('$(SRCROOT)', str(bundle.parent))
            self.assertEqual(Path(repository).resolve(), ROOT)
            self.assertEqual(Path(application).resolve(), ROOT / 'apps/chat')
        for path in bundle.rglob('*'):
            if not path.is_file():
                continue
            with self.subTest(file=str(path.relative_to(bundle))):
                text = path.read_text()
                self.assertNotRegex(text, r'(?<=["\s])/(?:Users|home|private/var)/')
                for artifact in ('build/ios/host-deps', 'build/lib/liborca', 'build/bin/orca'):
                    self.assertNotIn(artifact, text)

    def test_regeneration_preserves_custom_signing_and_shared_scheme(self):
        bundle = self.generate()
        expected = self.customize_signing(bundle)
        scheme_path = bundle / 'xcshareddata/xcschemes/chat-ios.xcscheme'
        scheme = ET.parse(scheme_path)
        launch = scheme.getroot().find('LaunchAction')
        launch.set('enableASanStackUseAfterReturn', 'YES')
        variables = ET.SubElement(launch, 'EnvironmentVariables')
        ET.SubElement(variables, 'EnvironmentVariable', key='QA_MODE', value='offline', isEnabled='YES')
        scheme.write(scheme_path, encoding='utf-8', xml_declaration=True)
        customized = scheme_path.read_bytes()
        self.generate()
        project = self.read_project(bundle)
        actual = self.configurations(project, self.application(project)[1])
        for name, settings in expected.items():
            for key, value in settings.items():
                self.assertEqual(actual[name][key], value, name + ': ' + key)
        self.assertEqual(scheme_path.read_bytes(), customized)

    def test_explicit_identity_overrides_preserved_settings_when_copying_project(self):
        original = self.generate()
        expected = self.customize_signing(original)
        original_bytes = (original / 'project.pbxproj').read_bytes()
        copied = self.generate('chat', '--settings-from', original, '--team', 'OVERRIDETEAM',
                               '--bundle-id', 'example.override.chat', output=self.output / 'copied')
        for settings in self.effective_configurations(self.read_project(copied)).values():
            self.assertEqual(settings['DEVELOPMENT_TEAM'], 'OVERRIDETEAM')
            self.assertEqual(settings['PRODUCT_BUNDLE_IDENTIFIER'], 'example.override.chat')
            self.assertEqual(settings['CODE_SIGN_STYLE'], 'Manual')
            self.assertEqual(settings['MARKETING_VERSION'], expected['Debug']['MARKETING_VERSION'])
        self.assertEqual((original / 'project.pbxproj').read_bytes(), original_bytes)

    def test_hyphenated_application_gets_its_own_native_target(self):
        bundle = self.generate('cluster-demo')
        project = self.read_project(bundle)
        _, target = self.application(project)
        self.assertEqual(target['name'], 'cluster-demo-ios')
        product = project['objects'][target['productReference']]
        self.assertEqual(product['path'], 'cluster-demo.app')
        self.assertTrue((bundle / 'xcshareddata/xcschemes/cluster-demo-ios.xcscheme').is_file())

    def test_all_generates_projects_beside_each_application_and_ignores_non_apps(self):
        fixture = Path(self.temporary.name) / 'repository'
        shutil.copytree(ROOT / 'tools/xcode', fixture / 'tools/xcode',
                        ignore=shutil.ignore_patterns('__pycache__'))
        (fixture / 'tools/ios').mkdir()
        shutil.copy2(ROOT / 'tools/ios/Makefile', fixture / 'tools/ios/Makefile')
        for app in ('chat', 'cluster-demo'):
            app_dir = fixture / 'apps' / app
            app_dir.mkdir(parents=True)
            shutil.copy2(ROOT / 'apps' / app / 'package.lua', app_dir / 'package.lua')
        (fixture / 'apps/not-an-application').mkdir()
        self.run_generator('--all', generator=fixture / 'tools/xcode/generate.py')
        projects = {path.relative_to(fixture / 'apps').as_posix()
                    for path in (fixture / 'apps').glob('*/*.xcodeproj')}
        self.assertEqual(projects, {'chat/chat.xcodeproj', 'cluster-demo/cluster-demo.xcodeproj'})
        self.assertFalse((fixture / 'apps/not-an-application/not-an-application.xcodeproj').exists())

    def test_all_rejects_a_shared_output_or_bundle_identifier(self):
        for args in (('--output', self.output), ('--bundle-id', 'example.invalid.all')):
            with self.subTest(args=args):
                self.run_generator('--all', *args, success=False)
        self.assertFalse(self.output.exists())


if __name__ == '__main__':
    unittest.main()
