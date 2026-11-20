"""Shared source layout for native ORCA Xcode projects and build phases."""
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
METAL_UNIT = 'metal_renderer.m'


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


def url_schemes(project):
    """Lowercase schemes from a literal `URLSchemes = { "a", "b" }` list in package.lua."""
    package = Path(project) / 'package.lua'
    match = re.search(r'^URLSchemes\s*=\s*\{([^}]*)\}', package.read_text() if package.is_file() else '', re.MULTILINE)
    schemes = re.findall(r'"([^"]*)"', match[1]) if match else []
    for scheme in schemes:
        if not re.fullmatch(r'[a-z][a-z0-9+.-]{0,127}', scheme):
            raise ValueError('URLSchemes entries must be lowercase URL schemes: ' + scheme)
    return schemes


def url_types(project):
    """CFBundleURLTypes for links the application opens, such as sign-in callbacks."""
    schemes = url_schemes(project)
    return {'CFBundleURLTypes': [{'CFBundleURLName': 'Sign-in links', 'CFBundleURLSchemes': schemes}]} if schemes else {}


# Kept identical to tools/ios/bundle.py.
CAPABILITIES = ('location',)
LOCATION_USAGE = ('Finds where you are, only when you ask, to show the weather there. '
                  'Apple names the place; your position is not shared with anyone else.')


def capabilities(project):
    """Names from a literal `Capabilities = { "location", ... }` list in package.lua."""
    package = Path(project) / 'package.lua'
    match = re.search(r'^Capabilities\s*=\s*\{([^}]*)\}', package.read_text() if package.is_file() else '', re.MULTILINE)
    names = re.findall(r'"([^"]*)"', match[1]) if match else []
    for name in names:
        if name not in CAPABILITIES:
            raise ValueError('Unknown capability ' + repr(name) + '; known: ' + ', '.join(CAPABILITIES))
    return names


def capability_info(project):
    """Info.plist keys for the capabilities a project asks for, and only those."""
    names, info = capabilities(project), {}
    if 'location' in names:
        info['NSLocationWhenInUseUsageDescription'] = LOCATION_USAGE
    return info


def unity_sources(root=ROOT):
    config = configuration(root)
    units = {}
    for folder, names in (('source', config['MODULES']), ('plugins', config['PLUGINS'])):
        for name in names:
            units['u_' + name + '.c'] = sorted((root / folder / name).rglob('*.c'))
    units['u_platform_unix.c'] = sorted((root / 'libs/platform/unix').glob('*.c'))
    units['ios_runtime.m'] = [root / 'source/ios.m',
                              root / 'source/ios_scroll.m', *sorted((root / 'libs/platform/ios').glob('*.m'))]
    metal = root / 'source/renderer/metal'
    units['native_modules.m'] = [path for name in config['MODULES']
                                  for path in sorted((root / 'source' / name).rglob('*.m'))
                                  if metal not in path.parents]
    # The Metal renderer's OpenGL ES layer is the one unit compiled with ARC.
    units[METAL_UNIT] = sorted(metal.glob('*.m'))
    return units


def include_sources(sources):
    return ''.join('#include ' + json.dumps(str(path)) + '\n' for path in sources)
