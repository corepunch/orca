#!/usr/bin/env python3
"""Package an ORCA project as a standalone universal iPhone/iPad application."""
import argparse
import fnmatch
from pathlib import Path
import plistlib
import shutil
import subprocess
import tempfile


def resource_filter(directory, names):
    """Keep runtime assets, excluding developer credentials and server-only trees."""
    patterns = ('.git', '.DS_Store', '.env', '.env.*', '.live-*', '.appwrite',
                '__pycache__', 'node_modules', 'build', '*.mobileprovision',
                '*.xcodeproj', '*.xcworkspace', '*.xcassets', '*.xcprivacy', '*.entitlements', 'ci_scripts')
    ignored = [name for name in names if any(fnmatch.fnmatchcase(name, pattern) for pattern in patterns)]
    # Chat's Appwrite Function, fixtures and local account sessions never run on-device.
    backend = Path(directory) / 'backend'
    if 'backend' in names and (backend / 'appwrite/package.json').is_file():
        ignored.append('backend')
    return ignored


def compile_assets(project, bundle, sdk, minimum):
    """Compile an optional native icon catalog during an actual Make build."""
    catalog = project / 'Assets.xcassets'
    if not (catalog / 'AppIcon.appiconset/Contents.json').is_file():
        return {}
    with tempfile.TemporaryDirectory(prefix='orca-app-icon-') as temporary:
        partial = Path(temporary) / 'asset-info.plist'
        subprocess.run([
            'xcrun', '--sdk', sdk, 'actool', str(catalog), '--compile', str(bundle),
            '--platform', sdk, '--minimum-deployment-target', minimum,
            '--target-device', 'iphone', '--target-device', 'ipad', '--app-icon', 'AppIcon',
            '--output-partial-info-plist', str(partial),
        ], check=True)
        return plistlib.loads(partial.read_bytes())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('binary', 'bundle', 'project'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--sdk', choices=('iphoneos', 'iphonesimulator'), required=True)
    for name in ('identifier', 'minimum', 'display-name'):
        parser.add_argument('--' + name, required=True)
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
    for name, source in (('share', share), ('project', project)):
        target = bundle / name
        shutil.copytree(source, target, ignore=resource_filter)
    if (project / 'PrivacyInfo.xcprivacy').is_file():
        shutil.copy2(project / 'PrivacyInfo.xcprivacy', bundle / 'PrivacyInfo.xcprivacy')
    orientations = ['UIInterfaceOrientationPortrait', 'UIInterfaceOrientationLandscapeLeft',
                    'UIInterfaceOrientationLandscapeRight']
    info = {
        'CFBundleIdentifier': args.identifier, 'CFBundleExecutable': 'Orca',
        'CFBundleName': args.display_name, 'CFBundleDisplayName': args.display_name,
        'CFBundleDevelopmentRegion': 'en', 'CFBundleInfoDictionaryVersion': '6.0',
        'CFBundlePackageType': 'APPL', 'CFBundleShortVersionString': '1.0', 'CFBundleVersion': '1',
        'UIDeviceFamily': [1, 2], 'MinimumOSVersion': args.minimum,
        'CFBundleSupportedPlatforms': ['iPhoneOS' if args.sdk == 'iphoneos' else 'iPhoneSimulator'],
        'LSRequiresIPhoneOS': True, 'UIRequiredDeviceCapabilities': ['opengles-3'],
        'CADisableMinimumFrameDurationOnPhone': True,  # ProMotion iPhones otherwise cap at 60 Hz
        'UILaunchScreen': {}, 'UISupportedInterfaceOrientations': orientations,
        'UISupportedInterfaceOrientations~ipad': orientations + ['UIInterfaceOrientationPortraitUpsideDown'],
        'ORCAProject': 'project', 'ITSAppUsesNonExemptEncryption': False,
        'UIApplicationSceneManifest': {
            'UIApplicationSupportsMultipleScenes': False,
            'UISceneConfigurations': {'UIWindowSceneSessionRoleApplication': [{
                'UISceneConfigurationName': 'ORCA', 'UISceneDelegateClassName': 'OrcaSceneDelegate',
            }]},
        },
    }
    info.update(compile_assets(project, bundle, args.sdk, args.minimum))
    if (project / 'Push.entitlements').is_file():
        info['ORCARemoteNotifications'] = True
    (bundle / 'Info.plist').write_bytes(plistlib.dumps(info))
    (bundle / 'PkgInfo').write_bytes(b'APPL????')
    print('Packaged ' + str(bundle))


if __name__ == '__main__':
    main()
