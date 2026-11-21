#!/usr/bin/env python3
"""Build a project's WidgetKit extension (<project>/Widget/*.swift) into a bundled ORCA app.

Run by `make ios-sim APP=weather WIDGETS=1` after tools/ios/bundle.py and
before the app is signed. The extension is <Project>Widget.appex in the
app's PlugIns, identified as <app id>.widget, and shares the app's App
Group (ORCAAppGroup), where the app writes widget.json. Simulator builds
link their entitlements like the app and are signed ad hoc; device builds
are left unsigned for Xcode (docs/IOS.md, Widgets).
"""
import argparse
import importlib.util
from pathlib import Path
import plistlib
import shutil
import subprocess

HERE = Path(__file__).resolve().parent


def sign_module():
    spec = importlib.util.spec_from_file_location('orca_sign', HERE / 'sign.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def target(arch, sdk, minimum):
    return f'{arch}-apple-ios{minimum}' + ('-simulator' if sdk == 'iphonesimulator' else '')


def info_plist(name, identifier, app, group, sdk, minimum):
    return {
        'CFBundleIdentifier': identifier, 'CFBundleExecutable': name, 'CFBundleName': name,
        'CFBundleDisplayName': app.get('CFBundleDisplayName', name), 'CFBundlePackageType': 'XPC!',
        'CFBundleInfoDictionaryVersion': '6.0', 'CFBundleDevelopmentRegion': 'en',
        'CFBundleShortVersionString': app.get('CFBundleShortVersionString', '1.0'),
        'CFBundleVersion': app.get('CFBundleVersion', '1'), 'MinimumOSVersion': minimum,
        'CFBundleSupportedPlatforms': ['iPhoneOS' if sdk == 'iphoneos' else 'iPhoneSimulator'],
        'UIDeviceFamily': [1, 2], 'ORCAAppGroup': group,
        'NSExtension': {'NSExtensionPointIdentifier': 'com.apple.widgetkit-extension'},
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('project', 'bundle', 'build-dir'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--sdk', choices=('iphoneos', 'iphonesimulator'), required=True)
    parser.add_argument('--arch', choices=('arm64', 'x86_64'), required=True)
    parser.add_argument('--minimum', required=True)
    parser.add_argument('--app-group', required=True)
    args = parser.parse_args()
    project, bundle = args.project.resolve(), args.bundle.resolve()
    sources = sorted((project / 'Widget').glob('*.swift'))
    if not sources:
        raise SystemExit('WIDGETS=1 needs a WidgetKit extension in ' + str(project / 'Widget') + '/*.swift')
    app = plistlib.loads((bundle / 'Info.plist').read_bytes())
    if app.get('ORCAAppGroup') != args.app_group:
        raise SystemExit('The app bundle does not name the App Group ' + args.app_group + ' (bundle.py --app-group)')
    name = sources[0].stem if len(sources) == 1 else 'Widgets'
    identifier = app['CFBundleIdentifier'] + '.widget'
    appex = bundle / 'PlugIns' / (name + '.appex')
    if appex.exists():
        shutil.rmtree(appex)
    appex.mkdir(parents=True)
    work = args.build_dir.resolve() / 'widget'
    work.mkdir(parents=True, exist_ok=True)
    sdk_path = subprocess.check_output(['xcrun', '--sdk', args.sdk, '--show-sdk-path'], text=True).strip()
    command = ['xcrun', '--sdk', args.sdk, 'swiftc', '-target', target(args.arch, args.sdk, args.minimum),
               '-sdk', sdk_path, '-O', '-parse-as-library', '-application-extension', '-module-name', name,
               '-module-cache-path', str(work / 'module-cache'), *map(str, sources),
               '-framework', 'WidgetKit', '-framework', 'SwiftUI', '-o', str(appex / name)]
    simulator = args.sdk == 'iphonesimulator'
    if simulator:
        # As for the app: simulators read entitlements linked into the binary.
        entitlements = work / 'simulated.xcent'
        sign_module().write_simulated_entitlements(entitlements, identifier, args.app_group)
        for section, path in (('__entitlements', entitlements), ('__ents_der', entitlements.with_suffix('.xcent.der'))):
            command += ['-Xlinker', '-sectcreate', '-Xlinker', '__TEXT', '-Xlinker', section, '-Xlinker', str(path)]
    subprocess.run(command, check=True)
    (appex / 'Info.plist').write_bytes(plistlib.dumps(info_plist(name, identifier, app, args.app_group,
                                                                 args.sdk, args.minimum)))
    if simulator:
        subprocess.run(['codesign', '--force', '--sign', '-', str(appex)], check=True)
    print('Built ' + str(appex.relative_to(bundle.parent)) + ' (' + identifier + ', ' + args.app_group + ')'
          + ('' if simulator else '; unsigned: sign the app and widget in Xcode'))


if __name__ == '__main__':
    main()
