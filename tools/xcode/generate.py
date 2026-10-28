#!/usr/bin/env python3
"""Generate a native iPhone/iPad Xcode project beside each ORCA application."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import plistlib
import re
import xml.etree.ElementTree as ET

from common import ROOT, configuration, unity_sources, write
from pbxproj import load


def encode(value):
    if isinstance(value, dict):
        return '{\n' + ''.join(f'{encode(k)} = {encode(v)};\n' for k, v in value.items()) + '}'
    if isinstance(value, list):
        return '(\n' + ''.join(f'{encode(v)},\n' for v in value) + ')'
    if isinstance(value, int):
        return str(value)
    return json.dumps(str(value))


def read_project(path):
    if not path.exists():
        return {}
    data = load(path)
    if not isinstance(data, dict) or not isinstance(data.get('objects'), dict):
        raise ValueError('Invalid Xcode project: ' + str(path))
    return data['objects']


class Project:
    def __init__(self, existing):
        self.objects, self.existing = {}, existing

    def add(self, key, isa, **fields):
        uid = hashlib.sha256(key.encode()).hexdigest()[:24].upper()
        if uid in self.objects:
            raise ValueError('Duplicate Xcode object: ' + key)
        self.objects[uid] = dict(isa=isa, **fields)
        return uid

    def configurations(self, key, settings, overrides=None):
        refs = []
        for name in ('Debug', 'Release'):
            values = dict(settings, GCC_OPTIMIZATION_LEVEL='0' if name == 'Debug' else 's',
                          DEBUG_INFORMATION_FORMAT='dwarf' if name == 'Debug' else 'dwarf-with-dsym',
                          ONLY_ACTIVE_ARCH='YES' if name == 'Debug' else 'NO')
            if 'ORCA_APNS_ENVIRONMENT' in settings:
                values['ORCA_APNS_ENVIRONMENT'] = 'development' if name == 'Debug' else 'production'
            uid = self.add(key + '/' + name, 'XCBuildConfiguration', name=name, buildSettings=values)
            for field, value in self.existing.get(uid, {}).get('buildSettings', {}).items():
                if field.startswith(('CODE_SIGN', 'DEVELOPMENT_TEAM', 'PROVISIONING_PROFILE',
                                     'PRODUCT_BUNDLE_IDENTIFIER', 'MARKETING_VERSION',
                                     'CURRENT_PROJECT_VERSION', 'ASSETCATALOG_COMPILER_APPICON',
                                     'ORCA_APNS_ENVIRONMENT')):
                    values[field] = value
            for field in tuple(values):
                if any(field.startswith(key + '[') for key in (overrides or {})):
                    del values[field]
            values.update(overrides or {})
            refs.append(uid)
        return self.add(key + '/configurations', 'XCConfigurationList', buildConfigurations=refs,
                        defaultConfigurationIsVisible=0, defaultConfigurationName='Debug')


def scheme(path, project_name, name, target, product):
    if path.exists():
        return
    root = ET.Element('Scheme', LastUpgradeVersion='1600', version='1.3')
    action = ET.SubElement(root, 'BuildAction', parallelizeBuildables='YES', buildImplicitDependencies='YES')
    entry = ET.SubElement(ET.SubElement(action, 'BuildActionEntries'), 'BuildActionEntry',
                         **{'buildFor' + key: 'YES' for key in
                            ('Testing', 'Running', 'Profiling', 'Archiving', 'Analyzing')})
    reference = dict(BuildableIdentifier='primary', BlueprintIdentifier=target,
                     BuildableName=product, BlueprintName=name, ReferencedContainer='container:' + project_name)
    ET.SubElement(entry, 'BuildableReference', **reference)
    for tag, config in (('LaunchAction', 'Debug'), ('ProfileAction', 'Release')):
        attrs = dict(buildConfiguration=config, useCustomWorkingDirectory='NO')
        if tag == 'LaunchAction':
            attrs.update(selectedDebuggerIdentifier='Xcode.DebuggerFoundation.Debugger.LLDB',
                         selectedLauncherIdentifier='Xcode.IDEFoundation.Launcher.LLDB', launchStyle='0')
        action = ET.SubElement(root, tag, **attrs)
        runnable = ET.SubElement(action, 'BuildableProductRunnable', runnableDebuggingMode='0')
        ET.SubElement(runnable, 'BuildableReference', **reference)
    ET.SubElement(root, 'AnalyzeAction', buildConfiguration='Debug')
    ET.SubElement(root, 'ArchiveAction', buildConfiguration='Release', revealArchiveInOrganizer='YES')
    ET.indent(root)
    write(path, '<?xml version="1.0" encoding="UTF-8"?>\n' + ET.tostring(root, encoding='unicode') + '\n')


def info_plist(display_name, launch_storyboard=None):
    orientations = ['UIInterfaceOrientation' + name for name in ('Portrait', 'LandscapeLeft', 'LandscapeRight')]
    return dict(CFBundleExecutable='$(EXECUTABLE_NAME)', CFBundleIdentifier='$(PRODUCT_BUNDLE_IDENTIFIER)',
                CFBundleName=display_name, CFBundleDisplayName=display_name,
                CFBundleInfoDictionaryVersion='6.0', CFBundlePackageType='APPL',
                CFBundleShortVersionString='$(MARKETING_VERSION)', CFBundleVersion='$(CURRENT_PROJECT_VERSION)',
                CFBundleDevelopmentRegion='en', UIDeviceFamily=[1, 2], LSRequiresIPhoneOS=True,
                UIRequiredDeviceCapabilities=['opengles-3'], CADisableMinimumFrameDurationOnPhone=True,
                **({'UILaunchStoryboardName': launch_storyboard} if launch_storyboard else {'UILaunchScreen': {}}),
                UISupportedInterfaceOrientations=orientations,
                **{'UISupportedInterfaceOrientations~ipad': orientations + ['UIInterfaceOrientationPortraitUpsideDown']},
                ORCAProject='project', ITSAppUsesNonExemptEncryption=False, UIApplicationSceneManifest={
                    'UIApplicationSupportsMultipleScenes': False,
                    'UISceneConfigurations': {'UIWindowSceneSessionRoleApplication': [{
                        'UISceneConfigurationName': 'ORCA', 'UISceneDelegateClassName': 'OrcaSceneDelegate',
                    }]}})


def generate(args, app):
    app_dir = (ROOT / app if '/' in app else ROOT / 'apps' / app).resolve()
    if not app_dir.is_relative_to(ROOT) or not (app_dir / 'package.lua').is_file():
        raise ValueError('Expected an ORCA application with package.lua inside the repository: ' + app)
    name = app_dir.name
    if not re.fullmatch(r'[A-Za-z0-9_-]+', name):
        raise ValueError('App folder name must contain letters, digits, underscores or hyphens')
    output = (args.output or app_dir).resolve()
    bundle = output / (name + '.xcodeproj')
    existing = read_project((args.settings_from or bundle) / 'project.pbxproj')
    project = Project(existing)
    units = unity_sources()
    asset_catalog = app_dir / 'Assets.xcassets'
    has_app_icon = (asset_catalog / 'AppIcon.appiconset').is_dir()
    launch_screen = app_dir / 'LaunchScreen.storyboard'
    privacy_manifest = app_dir / 'PrivacyInfo.xcprivacy'
    common = {
        'ORCA_ROOT': '$(SRCROOT)/' + os.path.relpath(ROOT, output),
        'ORCA_PROJECT': '$(SRCROOT)/' + os.path.relpath(app_dir, output),
        'ORCA_XCODE_UNITS': ' '.join(units),
        'SDKROOT': 'iphoneos', 'SUPPORTED_PLATFORMS': 'iphoneos iphonesimulator',
        'IPHONEOS_DEPLOYMENT_TARGET': args.minimum, 'TARGETED_DEVICE_FAMILY': '1,2',
        'SUPPORTS_MACCATALYST': 'NO', 'SUPPORTS_MAC_DESIGNED_FOR_IPHONE_IPAD': 'NO',
        'GCC_C_LANGUAGE_STANDARD': 'gnu11', 'CLANG_ENABLE_OBJC_ARC': 'NO',
        'ENABLE_USER_SCRIPT_SANDBOXING': 'NO', 'ALWAYS_SEARCH_USER_PATHS': 'NO',
        'HEADER_SEARCH_PATHS': ['$(inherited)', '$(DERIVED_FILE_DIR)', '$(DERIVED_FILE_DIR)/include',
                                '$(DERIVED_FILE_DIR)/dependencies/include',
                                '$(ORCA_ROOT)', '$(SDKROOT)/usr/include/libxml2'],
        'LIBRARY_SEARCH_PATHS': ['$(inherited)', '$(DERIVED_FILE_DIR)/dependencies/$(PLATFORM_NAME)-$(CURRENT_ARCH)/deps/lib'],
        'GCC_PREPROCESSOR_DEFINITIONS': ['$(inherited)', 'ORCA_IOS=1', 'ORCA_STATIC=1', 'GL_SILENCE_DEPRECATION'],
        'OTHER_CFLAGS': ['$(inherited)', '-Wno-deprecated-declarations'],
        'PRODUCT_NAME': name, 'PRODUCT_BUNDLE_IDENTIFIER': 'org.orca.' + name,
        'MARKETING_VERSION': '1.0', 'CURRENT_PROJECT_VERSION': '1',
        'CODE_SIGN_STYLE': 'Automatic',
    }
    if has_app_icon:
        common['ASSETCATALOG_COMPILER_APPICON_NAME'] = 'AppIcon'
    overrides = {}
    if args.team is not None:
        overrides['DEVELOPMENT_TEAM'] = args.team
    if args.bundle_id is not None:
        overrides['PRODUCT_BUNDLE_IDENTIFIER'] = args.bundle_id
    root_configs = project.configurations('project', common, overrides)
    target_name = name + '-ios'
    frameworks = ('UIKit', 'Foundation', 'OpenGLES', 'QuartzCore', 'CoreGraphics', 'CoreText',
                  'ImageIO', 'PhotosUI', 'Security', 'SystemConfiguration', 'UniformTypeIdentifiers', 'AudioToolbox',
                  'UserNotifications')
    linker = ['$(inherited)', '-llua', '-llz4', '-lcurl', '-lxml2', '-lz', '-liconv', '-lm']
    for framework in frameworks:
        linker += ['-framework', framework]
    target_settings = {
        'INFOPLIST_FILE': '$(PROJECT_FILE_PATH)/Generated/ios-Info.plist',
        'GENERATE_INFOPLIST_FILE': 'NO', 'OTHER_LDFLAGS': linker,
        'SKIP_INSTALL': 'NO', 'INSTALL_PATH': '$(LOCAL_APPS_DIR)',
    }
    if (app_dir / 'Push.entitlements').is_file():
        target_settings.update({
            'CODE_SIGN_ENTITLEMENTS[sdk=iphoneos*]': '$(ORCA_PROJECT)/Push.entitlements',
            'ORCA_APNS_ENVIRONMENT': 'development',
        })
    target_configs = project.configurations(target_name, target_settings, overrides)
    plugin_units = {'u_' + name + '.c' for name in configuration()['PLUGINS']}
    sources, engine_references, plugin_references, native_references = [], [], [], []
    for filename in units:
        objc = filename.endswith('.m')
        ref = project.add('unity/' + filename, 'PBXFileReference', path=filename,
                          sourceTree='DERIVED_FILE_DIR', lastKnownFileType='sourcecode.c.objc' if objc else 'sourcecode.c.c')
        if objc:
            native_references.append(ref)
        elif filename in plugin_units:
            plugin_references.append(ref)
        else:
            engine_references.append(ref)
        sources.append(project.add('source/' + filename, 'PBXBuildFile', fileRef=ref, settings={
            'COMPILER_FLAGS': ('-idirafter' if objc else '-I') + ' "$(DERIVED_FILE_DIR)/generated"'}))
    prepare = project.add('prepare', 'PBXShellScriptBuildPhase', buildActionMask=2147483647,
        name='Prepare ORCA sources and dependencies', shellPath='/bin/sh',
        shellScript='set -eu\n/bin/sh "$ORCA_ROOT/tools/xcode/python.sh" "$ORCA_ROOT/tools/xcode/build.py" prepare\n',
        inputPaths=['$(ORCA_ROOT)/tools/xcode/build.py', '$(ORCA_ROOT)/tools/xcode/common.py',
                    '$(ORCA_ROOT)/tools/ios/dependencies.py'],
        outputPaths=['$(DERIVED_FILE_DIR)/' + filename for filename in units],
        runOnlyForDeploymentPostprocessing=0, alwaysOutOfDate=1)
    compile_phase = project.add('sources', 'PBXSourcesBuildPhase', buildActionMask=2147483647,
                                files=sources, runOnlyForDeploymentPostprocessing=0)
    resource_references, resource_files, resource_phase = [], [], None
    if asset_catalog.is_dir():
        resource_references.append(project.add('assets/catalog', 'PBXFileReference', name=asset_catalog.name,
            path=os.path.relpath(asset_catalog, output), sourceTree='SOURCE_ROOT',
            lastKnownFileType='folder.assetcatalog'))
        resource_files.append(project.add('assets/build', 'PBXBuildFile', fileRef=resource_references[-1]))
    if launch_screen.is_file():
        resource_references.append(project.add('launch/storyboard', 'PBXFileReference', name=launch_screen.name,
            path=os.path.relpath(launch_screen, output), sourceTree='SOURCE_ROOT',
            lastKnownFileType='file.storyboard'))
        resource_files.append(project.add('launch/build', 'PBXBuildFile', fileRef=resource_references[-1]))
    if privacy_manifest.is_file():
        resource_references.append(project.add('privacy/manifest', 'PBXFileReference', name=privacy_manifest.name,
            path=os.path.relpath(privacy_manifest, output), sourceTree='SOURCE_ROOT',
            lastKnownFileType='text.xml'))
        resource_files.append(project.add('privacy/build', 'PBXBuildFile', fileRef=resource_references[-1]))
    if resource_files:
        resource_phase = project.add('assets/resources', 'PBXResourcesBuildPhase', buildActionMask=2147483647,
            files=resource_files, runOnlyForDeploymentPostprocessing=0)
    resources = project.add('resources', 'PBXShellScriptBuildPhase', buildActionMask=2147483647,
        name='Bundle ORCA runtime and app resources', shellPath='/bin/sh',
        shellScript='set -eu\n/bin/sh "$ORCA_ROOT/tools/xcode/python.sh" "$ORCA_ROOT/tools/xcode/build.py" resources\n',
        runOnlyForDeploymentPostprocessing=0, alwaysOutOfDate=1)
    product = project.add('product', 'PBXFileReference', path=name + '.app',
                          sourceTree='BUILT_PRODUCTS_DIR', explicitFileType='wrapper.application')
    target = project.add('target/' + name, 'PBXNativeTarget', name=target_name, productName=name,
                         productReference=product, productType='com.apple.product-type.application',
                         buildConfigurationList=target_configs,
                         buildPhases=[prepare, compile_phase] + ([resource_phase] if resource_phase else []) + [resources],
                         buildRules=[], dependencies=[])
    application = project.add('browse/Application resources', 'PBXFileReference', name='Application resources',
                              path=os.path.relpath(app_dir, output), sourceTree='SOURCE_ROOT', lastKnownFileType='folder')
    engine = project.add('group/engine', 'PBXGroup', name='Engine', children=engine_references, sourceTree='<group>')
    plugins = project.add('group/plugins', 'PBXGroup', name='Plugins', children=plugin_references, sourceTree='<group>')
    native = project.add('group/native', 'PBXGroup', name='Native Sources', children=native_references, sourceTree='<group>')
    products = project.add('group/products', 'PBXGroup', name='Products', children=[product], sourceTree='<group>')
    group = project.add('group/main', 'PBXGroup',
                        children=[application, engine, plugins, native] + resource_references + [products],
                        sourceTree='<group>')
    root = project.add('project', 'PBXProject', buildConfigurationList=root_configs, compatibilityVersion='Xcode 14.0',
                       developmentRegion='en', knownRegions=['en', 'Base'], mainGroup=group,
                       productRefGroup=products, projectDirPath='', projectRoot='', targets=[target])
    attributes = existing.get(root, {}).get('attributes', {'LastUpgradeCheck': '1600'})
    project.objects[root]['attributes'] = attributes
    write(bundle / 'project.pbxproj', '// !$*UTF8*$!\n' + encode(dict(
        archiveVersion=1, classes={}, objectVersion=56, objects=project.objects, rootObject=root)) + '\n')
    display_name = args.display_name or 'ORCA ' + name.replace('-', ' ').title()
    write(bundle / 'Generated/ios-Info.plist', plistlib.dumps(info_plist(
        display_name, launch_screen.stem if launch_screen.is_file() else None)).decode())
    scheme(bundle / 'xcshareddata/xcschemes' / (target_name + '.xcscheme'), bundle.name,
           target_name, target, name + '.app')
    print(bundle)
    return bundle


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('app', nargs='?', help='App folder name under apps/, or repository-relative project path')
    parser.add_argument('--all', action='store_true', help='Generate one project in each apps/ package')
    parser.add_argument('--output', type=Path, help='Alternative project parent folder')
    parser.add_argument('--settings-from', type=Path, help='Preserve settings from an existing .xcodeproj')
    parser.add_argument('--bundle-id')
    parser.add_argument('--team')
    parser.add_argument('--display-name')
    parser.add_argument('--minimum', default='16.0')
    args = parser.parse_args()
    if bool(args.app) == args.all:
        parser.error('Choose an app or --all')
    if args.all and any((args.output, args.settings_from, args.bundle_id, args.display_name)):
        parser.error('--all cannot use per-app output, settings, bundle ID or display name')
    apps = [path.parent.relative_to(ROOT).as_posix() for path in sorted((ROOT / 'apps').glob('*/package.lua'))] if args.all else [args.app]
    try:
        for app in apps:
            generate(args, app)
    except ValueError as error:
        parser.error(str(error))


if __name__ == '__main__':
    main()
