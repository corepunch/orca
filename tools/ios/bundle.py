#!/usr/bin/env python3
"""Package an ORCA project as a standalone universal iPhone/iPad application."""
import argparse
import fnmatch
import json
import os
from pathlib import Path
import plistlib
import re
import shutil
import subprocess
import tempfile


def resource_filter(directory, names):
    """Keep runtime assets, excluding developer credentials and server-only trees."""
    patterns = ('.git', '.DS_Store', '.env', '.env.*', '.live-*', '.appwrite',
                '__pycache__', 'node_modules', 'build', '*.mobileprovision',
                '*.xcodeproj', '*.xcworkspace', '*.xcassets', '*.xcprivacy', '*.entitlements', '*.swift', 'ci_scripts')
    ignored = [name for name in names if any(fnmatch.fnmatchcase(name, pattern) for pattern in patterns)]
    # Chat's Appwrite Function, fixtures and local account sessions never run on-device.
    backend = Path(directory) / 'backend'
    if 'backend' in names and (backend / 'appwrite/package.json').is_file():
        ignored.append('backend')
    return ignored


def references_block(text):
    """The body of a literal `ProjectReferences = { ... }` table in package.lua."""
    match = re.search(r'^ProjectReferences\s*=\s*\{', text, re.MULTILINE)
    if not match:
        return ''
    depth, start = 1, match.end()
    for index in range(start, len(text)):
        depth += {'{': 1, '}': -1}.get(text[index], 0)
        if depth == 0:
            return text[start:index]
    return ''


def external_references(project):
    """(source, relative target) for ProjectReferences outside the project, such as
    Chat's `../octopus`. The bundle keeps the project under `project/`, so each
    one is copied where its relative Path resolves from there. Followed
    recursively; references inside a project are already copied with it."""
    project, found, pending = Path(project).resolve(), {}, [Path(project).resolve()]
    while pending:
        current = pending.pop()
        package = current / 'package.lua'
        text = package.read_text() if package.is_file() else ''
        for path in re.findall(r'Path\s*=\s*"([^"]*)"', references_block(text)):
            source = (current / path).resolve()
            if source == project or project in source.parents or source in found:
                continue
            relative = Path(os.path.relpath(source, project))
            # Keep the copy beside project/ inside the bundle: one level up at most.
            if relative.parts[:1] != ('..',) or '..' in relative.parts[1:] or not source.is_dir():
                raise ValueError('Unsupported project reference: ' + path)
            found[source] = relative
            pending.append(source)
    return sorted(found.items())


def copy_project(project, target):
    """Copy a project to <target> and its external references beside it."""
    project = Path(project).resolve()
    shutil.copytree(project, target, ignore=resource_filter)
    for source, relative in external_references(project):
        destination = Path(os.path.normpath(Path(target) / relative))
        if destination.exists():
            shutil.rmtree(destination)
        shutil.copytree(source, destination, ignore=resource_filter)


# Kept identical to tools/xcode/common.py, which the Xcode generator imports.
def url_schemes(project):
    """Lowercase schemes from a literal `URLSchemes = { "a", "b" }` list in package.lua."""
    package = Path(project) / 'package.lua'
    match = re.search(r'^URLSchemes\s*=\s*\{([^}]*)\}', package.read_text() if package.is_file() else '', re.MULTILINE)
    schemes = re.findall(r'"([^"]*)"', match[1]) if match else []
    for scheme in schemes:
        if not re.fullmatch(r'[a-z][a-z0-9+.-]{0,127}', scheme):
            raise ValueError('URLSchemes entries must be lowercase URL schemes: ' + scheme)
    return schemes


# Kept identical to tools/xcode/common.py, which the Xcode generator imports.
CAPABILITIES = ('location', 'background-refresh')
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


def capability_info(project, identifier):
    """Info.plist keys for the capabilities a project asks for, and only those."""
    names, info = capabilities(project), {}
    if 'location' in names:
        info['NSLocationWhenInUseUsageDescription'] = LOCATION_USAGE
    if 'background-refresh' in names:
        # orca.notifications.scheduleRefresh submits "<bundle id>.refresh",
        # which runs the project's background.lua.
        info['UIBackgroundModes'] = ['fetch']
        info['BGTaskSchedulerPermittedIdentifiers'] = [identifier + '.refresh']
    return info


# Kept identical to tools/xcode/common.py.
LAUNCH_COLOR = 'LaunchBackground'


def launch_background(project):
    """(r, g, b) from 0 to 1 for a literal `LaunchBackground = "#rrggbb"` in package.lua, or None."""
    package = Path(project) / 'package.lua'
    match = re.search(r'^LaunchBackground\s*=\s*"([^"]*)"', package.read_text() if package.is_file() else '', re.MULTILINE)
    if not match:
        return None
    if not re.fullmatch(r'#[0-9a-fA-F]{6}', match[1]):
        raise ValueError('LaunchBackground must be a "#rrggbb" colour: ' + match[1])
    return tuple(int(match[1][i:i + 2], 16) / 255 for i in (1, 3, 5))


def launch_catalog(directory, rgb):
    """Write <directory>/Launch.xcassets with the LaunchBackground colour set; return its path."""
    catalog = Path(directory) / 'Launch.xcassets'
    info = {'info': {'author': 'xcode', 'version': 1}}
    components = {name: '%.3f' % value for name, value in zip(('red', 'green', 'blue'), rgb)}
    components['alpha'] = '1.000'
    colour = dict(info, colors=[{'idiom': 'universal', 'color': {'color-space': 'srgb', 'components': components}}])
    for path, content in ((catalog / 'Contents.json', info), (catalog / (LAUNCH_COLOR + '.colorset/Contents.json'), colour)):
        path.parent.mkdir(parents=True, exist_ok=True)
        text = json.dumps(content, indent=2, sort_keys=True) + '\n'
        if not path.is_file() or path.read_text() != text:
            path.write_text(text)
    return catalog


def launch_info(rgb):
    """The launch screen in that colour; a dark one also keeps system sheets dark, so nothing flashes white."""
    info = {'UILaunchScreen': {'UIColorName': LAUNCH_COLOR}}
    if 0.2126 * rgb[0] + 0.7152 * rgb[1] + 0.0722 * rgb[2] < 0.3:
        info['UIUserInterfaceStyle'] = 'Dark'
    return info


def editions(project):
    """{key: name} from a literal `Editions = { { Key = "k", Name = "n" }, ... }` in package.lua."""
    package = Path(project) / 'package.lua'
    match = re.search(r'^Editions\s*=\s*\{(.*?)^\}', package.read_text() if package.is_file() else '',
                      re.MULTILINE | re.DOTALL)
    return dict(re.findall(r'Key\s*=\s*"([a-z0-9-]+)"\s*,\s*Name\s*=\s*"([^"]+)"', match[1])) if match else {}


def stamp_edition(project, target, key):
    """Write <target>/build.lua naming the edition; return its display name."""
    names = editions(project)
    if key not in names:
        raise ValueError('Unknown edition ' + repr(key) + '; package.lua declares: ' + (', '.join(names) or 'none'))
    (Path(target) / 'build.lua').write_text('-- Written by tools/ios/bundle.py --edition.\nreturn { edition = "%s" }\n' % key)
    return names[key]


def url_types(project):
    """CFBundleURLTypes for links the application opens, such as sign-in callbacks."""
    schemes = url_schemes(project)
    return {'CFBundleURLTypes': [{'CFBundleURLName': 'Sign-in links', 'CFBundleURLSchemes': schemes}]} if schemes else {}


def compile_assets(project, bundle, sdk, minimum, edition=None, launch=None):
    """Compile the project's icon catalog (the edition's icon when it has one) and a launch colour
    (`launch`, rgb) during an actual Make build."""
    catalog = project / 'Assets.xcassets'
    icon = 'AppIcon'
    if edition and (catalog / f'AppIcon-{edition}.appiconset/Contents.json').is_file():
        icon = f'AppIcon-{edition}'
    has_icon = (catalog / f'{icon}.appiconset/Contents.json').is_file()
    if not has_icon and not launch:
        return {}
    with tempfile.TemporaryDirectory(prefix='orca-app-icon-') as temporary:
        partial = Path(temporary) / 'asset-info.plist'
        catalogs = ([str(catalog)] if has_icon else []) + ([str(launch_catalog(temporary, launch))] if launch else [])
        subprocess.run([
            'xcrun', '--sdk', sdk, 'actool', *catalogs, '--compile', str(bundle),
            '--platform', sdk, '--minimum-deployment-target', minimum,
            '--target-device', 'iphone', '--target-device', 'ipad', *(['--app-icon', icon] if has_icon else []),
            '--output-partial-info-plist', str(partial),
        ], check=True, stdout=subprocess.DEVNULL)
        info = plistlib.loads(partial.read_bytes()) if partial.is_file() else {}
    if launch:
        info.update(launch_info(launch))
    return info


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('binary', 'bundle', 'project'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--sdk', choices=('iphoneos', 'iphonesimulator'), required=True)
    for name in ('identifier', 'minimum', 'display-name'):
        parser.add_argument('--' + name, required=True)
    parser.add_argument('--edition', help='an `Editions` key from package.lua; its Name becomes the display name')
    parser.add_argument('--app-group', help='an App Group the app shares with its widgets (WIDGETS=1), as ORCAAppGroup')
    args = parser.parse_args()
    project = args.project.resolve()
    if not project.is_dir() or not (project / 'package.lua').is_file():
        parser.error('project must be an ORCA directory containing package.lua')
    bundle = args.bundle.resolve()
    if bundle.suffix != '.app' or bundle == Path.cwd() or bundle in Path.cwd().parents:
        parser.error('bundle must be a separate .app directory')
    binary, share = args.binary.resolve(), Path('share').resolve()
    if not binary.is_file() or not share.is_dir():
        parser.error('binary and shared runtime resources must exist')
    for source in (project, share, binary):
        if source == bundle or bundle in source.parents or (source.is_dir() and source in bundle.parents):
            parser.error('bundle must be separate from the project, binary and shared resources')
    # Repackaging invalidates signatures and must remove resources deleted from the project.
    if bundle.exists():
        shutil.rmtree(bundle)
    bundle.mkdir(parents=True, exist_ok=True)
    shutil.copy2(binary, bundle / 'Orca')
    shutil.copytree(share, bundle / 'share', ignore=resource_filter)
    copy_project(project, bundle / 'project')
    display_name = stamp_edition(project, bundle / 'project', args.edition) if args.edition else args.display_name
    if (project / 'PrivacyInfo.xcprivacy').is_file():
        shutil.copy2(project / 'PrivacyInfo.xcprivacy', bundle / 'PrivacyInfo.xcprivacy')
    orientations = ['UIInterfaceOrientationPortrait', 'UIInterfaceOrientationLandscapeLeft',
                    'UIInterfaceOrientationLandscapeRight']
    info = {
        'CFBundleIdentifier': args.identifier, 'CFBundleExecutable': 'Orca',
        'CFBundleName': display_name, 'CFBundleDisplayName': display_name,
        'CFBundleDevelopmentRegion': 'en', 'CFBundleInfoDictionaryVersion': '6.0',
        'CFBundlePackageType': 'APPL', 'CFBundleShortVersionString': '1.0', 'CFBundleVersion': '1',
        'UIDeviceFamily': [1, 2], 'MinimumOSVersion': args.minimum,
        'CFBundleSupportedPlatforms': ['iPhoneOS' if args.sdk == 'iphoneos' else 'iPhoneSimulator'],
        'LSRequiresIPhoneOS': True, 'UIRequiredDeviceCapabilities': ['opengles-3'],
        'CADisableMinimumFrameDurationOnPhone': True,  # ProMotion iPhones otherwise cap at 60 Hz
        'UILaunchScreen': {}, 'UISupportedInterfaceOrientations': orientations,
        'UISupportedInterfaceOrientations~ipad': orientations + ['UIInterfaceOrientationPortraitUpsideDown'],
        'ORCAProject': 'project', 'ITSAppUsesNonExemptEncryption': False,
        'NSCameraUsageDescription': 'Your camera takes a profile photo or a photo to share, only when you choose to.',
        'NSContactsUsageDescription': 'Find friends who use the app: only email addresses are checked, and your contacts are never stored.',
        'UIApplicationSceneManifest': {
            'UIApplicationSupportsMultipleScenes': False,
            'UISceneConfigurations': {'UIWindowSceneSessionRoleApplication': [{
                'UISceneConfigurationName': 'ORCA', 'UISceneDelegateClassName': 'OrcaSceneDelegate',
            }]},
        },
    }
    info.update(compile_assets(project, bundle, args.sdk, args.minimum, args.edition, launch_background(project)))
    info.update(url_types(project))
    info.update(capability_info(project, args.identifier))
    languages = sorted(path.stem for path in (project / 'Locales').glob('*.lua')
                       if re.fullmatch(r'[a-z]{2,3}(?:-[A-Za-z0-9]{2,8})*', path.stem))
    if languages:
        info['CFBundleLocalizations'] = languages
    if (project / 'Push.entitlements').is_file():
        info['ORCARemoteNotifications'] = True
    if args.app_group:
        info['ORCAAppGroup'] = args.app_group
    (bundle / 'Info.plist').write_bytes(plistlib.dumps(info))
    (bundle / 'PkgInfo').write_bytes(b'APPL????')
    print('Packaged ' + str(bundle))


if __name__ == '__main__':
    main()
