#!/usr/bin/env python3
"""Package a local macOS ORCA build as an application, ready for development signing."""
import argparse
import importlib.util
from pathlib import Path
import plistlib
import shutil
import subprocess


ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('ios_bundle', ROOT / 'tools/ios/bundle.py')
ios_bundle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ios_bundle)


def package(binary, project, bundle, identifier, display_name):
    binary, project, bundle = binary.resolve(), project.resolve(), bundle.resolve()
    build = binary.parent.parent
    plugin_dir = build / 'lib/liborca'
    sources = [binary, project, build / 'lib', plugin_dir, ROOT / 'share']
    if not binary.is_file() or not (project / 'package.lua').is_file():
        raise ValueError('A built ORCA executable and application package.lua are required')
    if bundle.suffix != '.app' or bundle.exists():
        raise ValueError('Use a new .app output path; existing bundles are never overwritten')
    if any(source == bundle or source in bundle.parents or bundle in source.parents for source in sources):
        raise ValueError('The output bundle must be separate from its source resources')
    contents = bundle / 'Contents'
    executable = contents / 'MacOS' / 'orca'
    executable.parent.mkdir(parents=True)
    shutil.copy2(binary, executable)
    for source, destination in ((project, contents / 'Resources/project'),
                                (ROOT / 'share', contents / 'Resources/share')):
        shutil.copytree(source, destination, ignore=ios_bundle.resource_filter)
    frameworks = contents / 'Frameworks'
    frameworks.mkdir()
    for source in (build / 'lib').iterdir():
        if source.is_file() and source.suffix in ('.so', '.dylib') and source.name != 'liborca_test_memory.dylib':
            shutil.copy2(source, frameworks / source.name)
    shutil.copytree(plugin_dir, frameworks / 'liborca', ignore=ios_bundle.resource_filter)
    info = dict(CFBundleIdentifier=identifier, CFBundleExecutable='orca', CFBundlePackageType='APPL',
                CFBundleName=display_name, CFBundleDisplayName=display_name,
                CFBundleShortVersionString='1.0', CFBundleVersion='1',
                CFBundleInfoDictionaryVersion='6.0', NSHighResolutionCapable=True,
                ORCAProject='project', LSMinimumSystemVersion='11.0')
    (contents / 'Info.plist').write_bytes(plistlib.dumps(info))
    (contents / 'PkgInfo').write_bytes(b'APPL????')
    libraries = sorted((contents / 'Frameworks').glob('*.so')) + sorted((contents / 'Frameworks').glob('*.dylib'))
    plugins = sorted((frameworks / 'liborca').glob('*.so'))
    names = {path.name for path in libraries}
    for path in [*libraries, *plugins, executable]:
        dependencies = subprocess.check_output(['otool', '-L', str(path)], text=True).splitlines()[1:]
        for line in dependencies:
            dependency = line.strip().split(' (', 1)[0]
            if Path(dependency).name in names:
                subprocess.run(['install_name_tool', '-change', dependency,
                                '@rpath/' + Path(dependency).name, str(path)], check=True)
        if path != executable:
            subprocess.run(['install_name_tool', '-id', '@rpath/' + path.name, str(path)], check=True)
        rpath = '@executable_path/../Frameworks' if path == executable else (
            '@loader_path/..' if path in plugins else '@loader_path')
        commands = subprocess.check_output(['otool', '-l', str(path)], text=True)
        if 'path ' + rpath + ' (' not in commands:
            subprocess.run(['install_name_tool', '-add_rpath', rpath, str(path)], check=True)
        subprocess.run(['codesign', '--force', '--sign', '-', str(path)], check=True)
    subprocess.run(['codesign', '--force', '--sign', '-', str(bundle)], check=True)
    return bundle


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/bin/orca')
    parser.add_argument('--project', type=Path, default=ROOT / 'apps/chat')
    parser.add_argument('--bundle', type=Path, required=True)
    parser.add_argument('--identifier', default='org.orca.chat')
    parser.add_argument('--display-name', default='ORCA Chat')
    args = parser.parse_args()
    try:
        output = package(args.binary, args.project, args.bundle, args.identifier, args.display_name)
    except ValueError as error:
        parser.error(str(error))
    print('Packaged local macOS application: ' + str(output))
    print('Ad hoc signing does not enable APNs. Embed a matching provisioning profile and sign with an Apple identity and push entitlements.')


if __name__ == '__main__':
    main()
