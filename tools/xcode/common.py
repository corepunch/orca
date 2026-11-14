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


def non_exempt_encryption(project):
    """True when package.lua declares `NonExemptEncryption = true` (export compliance)."""
    package = Path(project) / 'package.lua'
    text = package.read_text() if package.is_file() else ''
    return re.search(r'^NonExemptEncryption\s*=\s*true\s*$', text, re.MULTILINE) is not None


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


def unity_sources(root=ROOT):
    config = configuration(root)
    units = {}
    for folder, names in (('source', config['MODULES']), ('plugins', config['PLUGINS'])):
        for name in names:
            units['u_' + name + '.c'] = sorted((root / folder / name).rglob('*.c'))
    units['u_platform_unix.c'] = sorted((root / 'libs/platform/unix').glob('*.c'))
    # Separate units: both Monocypher files define the same private macros.
    units['u_monocypher.c'] = [root / 'libs/monocypher/src/monocypher.c']
    units['u_monocypher_ed25519.c'] = [root / 'libs/monocypher/src/optional/monocypher-ed25519.c']
    units['ios_runtime.m'] = [root / 'source/ios.m', root / 'source/ios_animation.m',
                              root / 'source/ios_scroll.m', *sorted((root / 'libs/platform/ios').glob('*.m'))]
    units['native_modules.m'] = [path for name in config['MODULES']
                                  for path in sorted((root / 'source' / name).rglob('*.m'))]
    return units


def include_sources(sources):
    return ''.join('#include ' + json.dumps(str(path)) + '\n' for path in sources)
