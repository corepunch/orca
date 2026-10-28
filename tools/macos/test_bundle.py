"""Offline packaging regressions; system signing tools are replaced by local doubles."""
import importlib.util
from pathlib import Path
import plistlib
import tempfile
import unittest
from unittest.mock import patch


spec = importlib.util.spec_from_file_location('macos_bundle', Path(__file__).with_name('bundle.py'))
bundle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bundle)


class BundleTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='orca-macos-package-')
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name).resolve()
        self.project = self.root / 'apps/chat'
        self.binary = self.root / 'build/bin/orca'
        self.output = self.root / 'output/ORCA Chat.app'
        files = {
            'apps/chat/package.lua': 'Name="Chat"',
            'apps/chat/Push.entitlements': 'signing metadata',
            'apps/chat/.env': 'private fixture',
            'apps/chat/backend/appwrite/package.json': '{}',
            'apps/chat/backend/appwrite/private.txt': 'server fixture',
            'build/bin/orca': 'executable',
            'build/lib/liborca.so': 'runtime',
            'build/lib/libplatform.dylib': 'platform',
            'build/lib/liborca_test_memory.dylib': 'test allocator',
            'build/lib/liborca/UIKit.so': 'ui plugin',
            'build/lib/liborca/SceneKit.so': 'scene plugin',
            'build/plugins/codegen/libheader.so': 'host code generator',
            'share/orca.lua': 'return {}',
        }
        for name, data in files.items():
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(data)

    def package(self):
        with patch.object(bundle, 'ROOT', self.root), patch.object(bundle.subprocess, 'run') as run, \
                patch.object(bundle.subprocess, 'check_output', return_value='binary:\n\tbuild/lib/liborca.so (compatibility version 0)\n'):
            bundle.package(self.binary, self.project, self.output, 'example.chat', 'ORCA Chat')
            return run.call_args_list

    def test_bundle_has_runtime_plugins_and_private_files_are_excluded(self):
        calls = self.package()
        contents = self.output / 'Contents'
        self.assertEqual((contents / 'Frameworks/liborca/UIKit.so').read_text(), 'ui plugin')
        self.assertEqual((contents / 'Frameworks/liborca.so').read_text(), 'runtime')
        self.assertEqual((contents / 'Resources/share/orca.lua').read_text(), 'return {}')
        self.assertFalse((contents / 'Frameworks/liborca_test_memory.dylib').exists())
        self.assertFalse(list(contents.rglob('libheader.so')), 'Host codegen plugins are not application runtime plugins')
        for name in ('.env', 'backend', 'Push.entitlements'):
            self.assertFalse((contents / 'Resources/project' / name).exists(), name)
        info = plistlib.loads((contents / 'Info.plist').read_bytes())
        self.assertEqual(info['ORCAProject'], 'project')
        self.assertEqual(info['CFBundleIdentifier'], 'example.chat')
        self.assertEqual(info['CFBundleExecutable'], 'orca')
        commands = [call.args[0] for call in calls]
        self.assertTrue(any(cmd[:3] == ['install_name_tool', '-change', 'build/lib/liborca.so'] for cmd in commands))
        self.assertEqual(commands[-1], ['codesign', '--force', '--sign', '-', str(self.output)])

    def test_existing_bundle_is_never_overwritten(self):
        self.output.mkdir(parents=True)
        sentinel = self.output / 'user-file'
        sentinel.write_text('preserved')
        with self.assertRaises(ValueError):
            self.package()
        self.assertEqual(sentinel.read_text(), 'preserved')


if __name__ == '__main__':
    unittest.main()
