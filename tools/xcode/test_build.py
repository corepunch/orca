"""Offline resource-copy regressions for native Xcode and standalone bundles."""
import os
import importlib.util
from pathlib import Path
import plistlib
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[2]


class ResourceTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='orca-xcode-resources-')
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name) / 'repository'
        self.project = self.root / 'apps/chat'
        self.share = self.root / 'share'
        self.project.mkdir(parents=True)
        self.share.mkdir()
        (self.project / 'package.lua').write_text('Name = "Chat"\n')
        (self.project / 'screen.xml').write_text('<Screen/>')
        (self.share / 'runtime.lua').write_text('return true\n')
        self.products = Path(self.temporary.name) / 'products'
        self.bundle = self.products / 'chat.app'
        self.bundle.mkdir(parents=True)
        self.executable = self.bundle / 'chat'
        self.executable.write_bytes(b'Xcode-managed executable')
        self.info = self.bundle / 'Info.plist'
        self.info.write_bytes(plistlib.dumps({'CFBundleIdentifier': 'example.xcode.chat'}))
        self.environment = dict(os.environ, ORCA_ROOT=str(self.root), ORCA_PROJECT=str(self.project),
                                TARGET_BUILD_DIR=str(self.products),
                                UNLOCALIZED_RESOURCES_FOLDER_PATH='chat.app',
                                DERIVED_FILE_DIR=str(Path(self.temporary.name) / 'derived'))

    def resources(self, success=True, **environment):
        result = subprocess.run([sys.executable, str(ROOT / 'tools/xcode/build.py'), 'resources'],
                                cwd=self.root, env=dict(self.environment, **environment),
                                capture_output=True, text=True)
        if success:
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        else:
            self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        return result

    def standalone(self, bundle):
        result = subprocess.run([
            sys.executable, str(ROOT / 'tools/ios/bundle.py'), '--binary', str(self.executable),
            '--bundle', str(bundle), '--project', str(self.project), '--sdk', 'iphonesimulator',
            '--identifier', 'example.standalone.chat', '--minimum', '16.0', '--display-name', 'QA Chat'],
            cwd=self.root, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def add_development_files(self):
        for parent in (self.project, self.share):
            for name in ('chat.xcodeproj', 'chat.xcworkspace', 'Other.xcassets', 'ci_scripts', '.git', 'build', '__pycache__'):
                directory = parent / name
                directory.mkdir()
                (directory / 'metadata').write_text('development only')
            (parent / 'development.mobileprovision').write_text('signing profile')
        (self.project / 'PrivacyInfo.xcprivacy').write_text('privacy manifest')

    def assert_development_files_absent(self, bundle):
        for parent in (bundle / 'project', bundle / 'share'):
            for name in ('chat.xcodeproj', 'chat.xcworkspace', 'Other.xcassets', 'ci_scripts', '.git',
                         'build', '__pycache__', 'development.mobileprovision', 'PrivacyInfo.xcprivacy'):
                self.assertFalse((parent / name).exists(), str(parent / name))

    def test_copy_keeps_xcode_managed_executable_and_info(self):
        original_executable = self.executable.read_bytes()
        original_info = self.info.read_bytes()
        (self.bundle / 'Assets.car').write_bytes(b'Xcode asset catalog')
        self.resources()
        self.assertEqual((self.bundle / 'project/package.lua').read_bytes(),
                         (self.project / 'package.lua').read_bytes())
        self.assertEqual((self.bundle / 'share/runtime.lua').read_bytes(),
                         (self.share / 'runtime.lua').read_bytes())
        self.assertEqual(self.executable.read_bytes(), original_executable)
        self.assertEqual(self.info.read_bytes(), original_info)
        self.assertEqual((self.bundle / 'Assets.car').read_bytes(), b'Xcode asset catalog')

    def test_refresh_removes_deleted_assets_and_copies_updates(self):
        (self.project / 'deleted.png').write_bytes(b'old project asset')
        (self.share / 'deleted-font.ttf').write_bytes(b'old shared asset')
        self.resources()
        (self.project / 'deleted.png').unlink()
        (self.share / 'deleted-font.ttf').unlink()
        (self.project / 'screen.xml').write_text('<Screen Name="Updated"/>')
        (self.share / 'runtime.lua').write_text('return "updated"\n')
        self.resources()
        self.assertFalse((self.bundle / 'project/deleted.png').exists())
        self.assertFalse((self.bundle / 'share/deleted-font.ttf').exists())
        self.assertEqual((self.bundle / 'project/screen.xml').read_text(), '<Screen Name="Updated"/>')
        self.assertEqual((self.bundle / 'share/runtime.lua').read_text(), 'return "updated"\n')
        self.assertTrue(self.executable.is_file())
        self.assertTrue(self.info.is_file())

    def test_resource_copy_excludes_xcode_projects_workspaces_and_ci_scripts(self):
        self.add_development_files()
        self.resources()
        self.assert_development_files_absent(self.bundle)
        self.assertTrue((self.bundle / 'project/screen.xml').is_file())

    def test_overlap_is_rejected_before_mutating_source_resources(self):
        before = {p.relative_to(self.root): p.read_bytes()
                  for p in self.root.rglob('*') if p.is_file()}
        self.resources(success=False, TARGET_BUILD_DIR=str(self.project.parent),
                       UNLOCALIZED_RESOURCES_FOLDER_PATH=self.project.name)
        after = {p.relative_to(self.root): p.read_bytes()
                 for p in self.root.rglob('*') if p.is_file()}
        self.assertEqual(before, after)

    def test_resources_folder_cannot_escape_target_build_directory(self):
        for relative in ('../escaped', str(self.project)):
            with self.subTest(relative=relative):
                result = self.resources(success=False, UNLOCALIZED_RESOURCES_FOLDER_PATH=relative)
                self.assertIn('relative', result.stderr.lower())
        self.assertFalse((self.products.parent / 'escaped').exists())

    def test_standalone_packaging_also_excludes_xcode_and_ci_metadata(self):
        self.add_development_files()
        bundle = Path(self.temporary.name) / 'standalone/chat.app'
        self.standalone(bundle)
        self.assert_development_files_absent(bundle)
        self.assertEqual((bundle / 'Orca').read_bytes(), self.executable.read_bytes())
        self.assertEqual((bundle / 'PrivacyInfo.xcprivacy').read_text(), 'privacy manifest')
        with (bundle / 'Info.plist').open('rb') as stream:
            self.assertIs(plistlib.load(stream)['ITSAppUsesNonExemptEncryption'], False)

    def test_both_packagers_remove_private_accounts_and_server_resources_but_keep_runtime_dotfiles(self):
        backend = self.project / 'backend/appwrite'
        backend.mkdir(parents=True)
        (backend / 'package.json').write_text('{"name":"server-only"}')
        (backend / 'server.js').write_text('server implementation')
        (backend.parent / '.live-accounts.json').write_text('synthetic private account fixture')
        private = ('.live-accounts.json', '.live-run.lock', '.env', '.env.local', 'node_modules')
        for source in (self.project, self.share):
            assets = source / '.runtime'
            assets.mkdir()
            (assets / 'asset.bin').write_bytes(b'needed at runtime')
            ordinary = source / 'assets/backend'
            ordinary.mkdir(parents=True)
            (ordinary / 'catalog.json').write_text('{}')
            for name in private:
                (assets / name).write_text('synthetic developer data')
        standalone = Path(self.temporary.name) / 'standalone/chat.app'
        for label, bundle in (('Xcode', self.bundle), ('Make', standalone)):
            with self.subTest(packager=label):
                # Also prove that rebuilding removes credentials from an older bundle.
                stale = bundle / 'project/backend'
                stale.mkdir(parents=True)
                (stale / '.live-accounts.json').write_text('synthetic stale account fixture')
                (bundle / 'project/.live-accounts.json').write_text('synthetic stale account fixture')
                if label == 'Xcode':
                    self.resources()
                else:
                    self.standalone(bundle)
                self.assertFalse((bundle / 'project/backend').exists())
                self.assertFalse((bundle / 'project/.live-accounts.json').exists())
                for target in (bundle / 'project', bundle / 'share'):
                    self.assertEqual((target / '.runtime/asset.bin').read_bytes(), b'needed at runtime')
                    self.assertTrue((target / 'assets/backend/catalog.json').is_file())
                    for name in private:
                        self.assertFalse((target / '.runtime' / name).exists(), str(target / '.runtime' / name))

    def test_make_icon_compiler_targets_both_devices_and_returns_generated_metadata(self):
        spec = importlib.util.spec_from_file_location('orca_bundle', ROOT / 'tools/ios/bundle.py')
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        catalog = self.project / 'Assets.xcassets'
        iconset = catalog / 'AppIcon.appiconset'
        iconset.mkdir(parents=True)
        (iconset / 'Contents.json').write_text('{}')
        metadata = {'CFBundleIcons': {'CFBundlePrimaryIcon': {'CFBundleIconName': 'AppIcon'}},
                    'CFBundleIcons~ipad': {'CFBundlePrimaryIcon': {'CFBundleIconName': 'AppIcon'}}}

        def fake_compile(command, check):
            self.assertTrue(check)
            self.assertEqual(command[:5], ['xcrun', '--sdk', 'iphonesimulator', 'actool', str(catalog)])
            self.assertIn('--compile', command)
            self.assertEqual(command[command.index('--compile') + 1], str(self.bundle))
            self.assertEqual(command[command.index('--minimum-deployment-target') + 1], '16.0')
            self.assertEqual(command[command.index('--app-icon') + 1], 'AppIcon')
            devices = [command[i + 1] for i, value in enumerate(command) if value == '--target-device']
            self.assertEqual(devices, ['iphone', 'ipad'])
            Path(command[command.index('--output-partial-info-plist') + 1]).write_bytes(plistlib.dumps(metadata))

        with patch.object(module.subprocess, 'run', side_effect=fake_compile) as compiler:
            self.assertEqual(module.compile_assets(self.project, self.bundle, 'iphonesimulator', '16.0'), metadata)
            compiler.assert_called_once()
        (iconset / 'Contents.json').unlink()
        with patch.object(module.subprocess, 'run', side_effect=AssertionError('Unexpected toolchain invocation')):
            self.assertEqual(module.compile_assets(self.project, self.bundle, 'iphoneos', '16.0'), {})


if __name__ == '__main__':
    unittest.main()
