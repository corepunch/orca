"""Keep generation away from Apple's installer-triggering command shims."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


TOOLS = Path(__file__).resolve().parent


class LauncherTests(unittest.TestCase):
    def test_rejects_system_python_before_executing_it(self):
        env = dict(os.environ, ORCA_PYTHON='/usr/bin/python3')
        result = subprocess.run(['/bin/sh', str(TOOLS / 'python.sh'), '-c', 'raise SystemExit(99)'],
                                env=env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn('not /usr/bin/python3', result.stderr)

    @unittest.skipUnless(Path('/usr/bin/python3').exists(), 'macOS launcher not present')
    def test_rejects_a_symlink_to_the_system_launcher(self):
        with tempfile.TemporaryDirectory() as temporary:
            alias = Path(temporary) / 'python-alias'
            alias.symlink_to('/usr/bin/python3')
            result = subprocess.run(['/bin/sh', str(TOOLS / 'python.sh'), '-c', 'raise SystemExit(99)'],
                                    env=dict(os.environ, ORCA_PYTHON=str(alias), PATH=''),
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 1)
            self.assertIn('not /usr/bin/python3', result.stderr)

    def test_missing_runtime_override_stops_without_installing_or_falling_back(self):
        with tempfile.TemporaryDirectory() as temporary:
            missing = str(Path(temporary) / 'missing-python')
            result = subprocess.run(['/bin/sh', str(TOOLS / 'python.sh'), '-c', 'raise SystemExit(99)'],
                                    env=dict(os.environ, ORCA_PYTHON=missing, PATH=''),
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 1)
            self.assertIn('must name an installed Python', result.stderr)

    def test_explicit_installed_runtime_receives_arguments_with_spaces(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            runtime = root / 'installed python'
            log = root / 'arguments'
            runtime.write_text('#!/bin/sh\nprintf "%s\\n" "$@" > "$ARGUMENT_LOG"\n')
            runtime.chmod(0o755)
            env = dict(os.environ, ORCA_PYTHON=str(runtime), ARGUMENT_LOG=str(log), PATH='')
            result = subprocess.run(['/bin/sh', str(TOOLS / 'python.sh'), 'path with spaces.py', '--all'],
                                    env=env, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(log.read_text().splitlines(), ['path with spaces.py', '--all'])

    def test_generate_and_regenerate_without_any_path_tools(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            output = root / 'project with spaces'
            env = dict(os.environ, ORCA_PYTHON=sys.executable, PATH='')
            command = ['/bin/sh', str(TOOLS / 'generate.sh'), 'chat', '--output', str(output)]
            for _ in range(2):
                result = subprocess.run(command, env=env, capture_output=True, text=True)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertTrue((output / 'chat.xcodeproj/project.pbxproj').is_file())


if __name__ == '__main__':
    unittest.main()
