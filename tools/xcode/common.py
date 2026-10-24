"""Shared source layout for native ORCA Xcode projects and build phases."""
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]


def write(path, content):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists() or path.read_text() != content:
        path.write_text(content)


def configuration(root=ROOT):
    text = (root / 'tools/ios/Makefile').read_text()
    values = {}
    for name in ('MODULES', 'PLUGINS'):
        match = re.search(r'^' + name + r'\s*:=\s*(.+)$', text, re.MULTILINE)
        if not match or not re.fullmatch(r'[\w\s-]+', match[1]):
            raise ValueError('Expected a literal ' + name + ' list in tools/ios/Makefile')
        values[name] = match[1].split()
    return values


def unity_sources(root=ROOT):
    config = configuration(root)
    units = {}
    for folder, names, prefix in (('source', config['MODULES'], 'module_'),
                                  ('plugins', config['PLUGINS'], 'plugin_')):
        for name in names:
            units[prefix + name + '.c'] = sorted((root / folder / name).rglob('*.c'))
    units['platform_unix.c'] = sorted((root / 'libs/platform/unix').glob('*.c'))
    units['ios_runtime.m'] = [root / 'source/ios.m', *sorted((root / 'libs/platform/ios').glob('*.m'))]
    units['native_modules.m'] = [path for name in config['MODULES']
                                  for path in sorted((root / 'source' / name).rglob('*.m'))]
    return units


def include_sources(sources):
    return ''.join('#include ' + json.dumps(str(path)) + '\n' for path in sources)
