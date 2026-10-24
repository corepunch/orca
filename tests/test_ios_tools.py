"""Offline regressions for standalone iOS packaging, signing and device selection."""
import importlib.util
from pathlib import Path
import plistlib
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parent.parent


def load_tool(name):
    spec = importlib.util.spec_from_file_location('ios_' + name, ROOT / 'tools/ios' / (name + '.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


deploy, sign = load_tool('deploy'), load_tool('sign')


def physical_device(identifier, kind='iPhone', reality='physical', name=None):
    return {'identifier': identifier,
            'hardwareProperties': {'deviceType': kind, 'reality': reality, 'udid': 'hardware-' + identifier},
            'deviceProperties': {'name': name or identifier}}


class DeviceSelectionTests(unittest.TestCase):
    def test_automatic_selection_ignores_simulated_and_other_families(self):
        devices = [physical_device('tablet', 'iPad'), physical_device('sim', reality='simulated'),
                   physical_device('phone')]
        self.assertEqual(deploy.select_device(devices, 'iPhone'), 'phone')
        self.assertEqual(deploy.select_device(devices, 'iPad'), 'tablet')

    def test_automatic_selection_rejects_zero_or_multiple_physical_devices(self):
        for devices in ([], [physical_device('one'), physical_device('two')]):
            with self.subTest(count=len(devices)), self.assertRaises(ValueError):
                deploy.select_device(devices, 'iPhone')

    def test_explicit_device_accepts_coredevice_id_name_and_provisioning_udid(self):
        devices = [physical_device('one', name='My Phone'), physical_device('two')]
        for selector in ('one', 'My Phone', 'hardware-one'):
            with self.subTest(selector=selector):
                self.assertEqual(deploy.select_device(devices, 'iPhone', selector), 'one')
        with self.assertRaises(ValueError):
            deploy.select_device(devices, 'iPad', 'one')

    def test_duplicate_names_require_an_identifier(self):
        devices = [physical_device('one', name='Phone'), physical_device('two', name='Phone')]
        with self.assertRaises(ValueError):
            deploy.select_device(devices, 'iPhone', 'Phone')

    def test_simulator_uses_family_metadata_and_prefers_booted_device(self):
        shutdown = {'udid': 'one', 'name': 'QA phone', 'state': 'Shutdown', 'isAvailable': True,
                    'deviceTypeIdentifier': 'com.apple.CoreSimulator.SimDeviceType.iPhone-17'}
        booted = dict(shutdown, udid='two', name='Renamed test device', state='Booted')
        unavailable = dict(shutdown, udid='three', isAvailable=False)
        tablet = dict(shutdown, udid='four', deviceTypeIdentifier='com.apple.CoreSimulator.SimDeviceType.iPad-Pro')
        payload = {'devices': {'runtime': [shutdown, booted, unavailable, tablet]}}
        self.assertEqual(deploy.select_simulator(payload, 'iPhone')['udid'], 'two')
        self.assertEqual(deploy.select_simulator(payload, 'iPhone', 'one')['udid'], 'one')
        self.assertEqual(deploy.select_simulator(payload, 'iPad')['udid'], 'four')
        with self.assertRaises(ValueError):
            deploy.select_simulator(payload, 'iPhone', 'three')
        with self.assertRaises(ValueError):
            deploy.select_simulator({'devices': {'runtime': [shutdown, dict(shutdown, udid='duplicate')]}},
                                    'iPhone', 'QA phone')


class SigningTests(unittest.TestCase):
    def test_profile_wildcards_respect_bundle_and_team_boundaries(self):
        for pattern, identifier, expected in (
                ('TEAM.org.orca.chat', 'TEAM.org.orca.chat', True),
                ('TEAM.org.orca.*', 'TEAM.org.orca.chat', True),
                ('TEAM.*', 'TEAM.org.orca.chat', True),
                ('TEAM.org.orca.*', 'TEAM.org.orca', False),
                ('TEAM.org.orca.*', 'TEAM.org.orcachat', False),
                ('OTHER.*', 'TEAM.org.orca.chat', False),
                ('TEAM.org.other', 'TEAM.org.orca.chat', False)):
            with self.subTest(pattern=pattern, identifier=identifier):
                self.assertEqual(sign.matches(pattern, identifier), expected)

    def test_keychain_group_must_be_authorized_by_the_profile(self):
        with self.assertRaises(ValueError):
            sign.signing_entitlements('TEAM.org.orca.chat', 'TEAM', ['TEAM.org.other.*'])
        entitlements = sign.signing_entitlements('TEAM.org.orca.chat', 'TEAM', ['TEAM.*'])
        self.assertEqual(entitlements['keychain-access-groups'], ['TEAM.org.orca.chat'])
        self.assertEqual(entitlements['application-identifier'], 'TEAM.org.orca.chat')


class BuildConfigurationTests(unittest.TestCase):
    def test_invalid_sdk_and_arch_fail_before_toolchain_or_downloads(self):
        for sdk, arch, diagnostic in (('macosx', 'arm64', 'SDK must'),
                                      ('iphoneos', 'x86_64', 'require ARCH=arm64'),
                                      ('iphonesimulator', 'bad-arch', 'ARCH must')):
            with self.subTest(sdk=sdk, arch=arch):
                result = subprocess.run(['make', '-f', 'tools/ios/Makefile', '-n', 'app',
                                         'SDK=' + sdk, 'ARCH=' + arch], cwd=ROOT,
                                        capture_output=True, text=True)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(diagnostic, result.stderr)
                self.assertNotIn('xcrun', result.stdout)
                self.assertNotIn('dependencies.py', result.stdout)


class BundleTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='orca-ios-tools-')
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.project = self.root / 'apps/chat'
        self.project.mkdir(parents=True)
        (self.project / 'package.lua').write_text('Name = "Chat"\n')
        (self.project / 'screen.xml').write_text('<Screen/>')
        (self.project / '.git').mkdir()
        (self.project / '.git/config').write_text('development metadata')
        (self.root / 'share/orca').mkdir(parents=True)
        (self.root / 'share/orca/startup.lua').write_text('return true')
        self.binary = self.root / 'Orca'
        self.binary.write_bytes(b'test executable')
        self.bundle = self.root / 'output/Orca.app'

    def package(self, sdk='iphonesimulator', bundle=None, binary=None):
        return subprocess.run([
            sys.executable, str(ROOT / 'tools/ios/bundle.py'), '--binary', str(binary or self.binary),
            '--bundle', str(bundle or self.bundle), '--project', str(self.project), '--sdk', sdk,
            '--identifier', 'org.orca.test', '--minimum', '16.0', '--display-name', 'ORCA Test'],
            cwd=self.root, capture_output=True, text=True)

    def test_universal_metadata_and_runtime_resources_for_both_sdks(self):
        for sdk, platform in (('iphoneos', 'iPhoneOS'), ('iphonesimulator', 'iPhoneSimulator')):
            with self.subTest(sdk=sdk):
                result = self.package(sdk)
                self.assertEqual(result.returncode, 0, result.stderr)
                info = plistlib.loads((self.bundle / 'Info.plist').read_bytes())
                self.assertEqual(info['UIDeviceFamily'], [1, 2])
                self.assertEqual(info['CFBundleSupportedPlatforms'], [platform])
                self.assertIn('UIInterfaceOrientationPortraitUpsideDown', info['UISupportedInterfaceOrientations~ipad'])
                scene = info['UIApplicationSceneManifest']['UISceneConfigurations']['UIWindowSceneSessionRoleApplication'][0]
                self.assertEqual(scene['UISceneDelegateClassName'], 'OrcaSceneDelegate')
                self.assertEqual((self.bundle / 'Orca').read_bytes(), self.binary.read_bytes())
                self.assertTrue((self.bundle / 'project/package.lua').is_file())
                self.assertTrue((self.bundle / 'share/orca/startup.lua').is_file())
                self.assertFalse((self.bundle / 'project/.git').exists())

    def test_repackaging_removes_deleted_resources_and_obsolete_signatures(self):
        self.assertEqual(self.package().returncode, 0)
        (self.project / 'screen.xml').unlink()
        (self.bundle / 'embedded.mobileprovision').write_bytes(b'obsolete profile')
        (self.bundle / '_CodeSignature').mkdir()
        (self.bundle / '_CodeSignature/CodeResources').write_bytes(b'obsolete signature')
        result = self.package()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse((self.bundle / 'project/screen.xml').exists())
        self.assertFalse((self.bundle / 'embedded.mobileprovision').exists())
        self.assertFalse((self.bundle / '_CodeSignature').exists())

    def test_invalid_inputs_preserve_an_existing_bundle(self):
        self.bundle.mkdir(parents=True)
        sentinel = self.bundle / 'keep'
        sentinel.write_text('preserve')
        for kwargs in ({'sdk': 'macosx'}, {'binary': self.root / 'missing'}):
            with self.subTest(kwargs=kwargs):
                self.assertNotEqual(self.package(**kwargs).returncode, 0)
                self.assertEqual(sentinel.read_text(), 'preserve')

    def test_bundle_cannot_overlap_source_trees(self):
        for target in (self.project / 'Recursive.app', self.root / 'share/Recursive.app'):
            with self.subTest(target=target):
                self.assertNotEqual(self.package(bundle=target).returncode, 0)
                self.assertFalse(target.exists())
        self.assertTrue((self.project / 'package.lua').is_file())
        self.assertTrue((self.root / 'share/orca/startup.lua').is_file())


if __name__ == '__main__':
    unittest.main()
