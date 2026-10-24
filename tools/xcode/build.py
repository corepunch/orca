#!/usr/bin/env python3
"""Prepare native Xcode sources/dependencies and copy an ORCA app's resources."""
import hashlib
import importlib.util
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys

sys.dont_write_bytecode = True
from common import configuration, include_sources, unity_sources, write


def run(*args, cwd=None, env=None):
    subprocess.run([str(argument) for argument in args], cwd=cwd, env=env, check=True)


def host_environment(output):
    env = os.environ.copy()
    for name in ('SDKROOT', 'SDK_NAME', 'SDK_VERSION', 'IPHONEOS_DEPLOYMENT_TARGET',
                 'TVOS_DEPLOYMENT_TARGET', 'WATCHOS_DEPLOYMENT_TARGET', 'XROS_DEPLOYMENT_TARGET',
                 'CPATH', 'C_INCLUDE_PATH', 'CPLUS_INCLUDE_PATH', 'LIBRARY_PATH',
                 'CFLAGS', 'CXXFLAGS', 'CPPFLAGS', 'LDFLAGS', 'ARCH_FLAGS',
                 'MAKEFLAGS', 'MFLAGS', 'GNUMAKEFLAGS', 'MAKELEVEL'):
        env.pop(name, None)
    env.update(MACOSX_DEPLOYMENT_TARGET='12.0', PYTHONDONTWRITEBYTECODE='1',
               CLANG_MODULE_CACHE_PATH=str(output / 'host-module-cache'))
    return env


def generate_api(root, output, config):
    targets = sorted(path.relative_to(root).with_suffix('').as_posix()
                     for folder, names in (('source', config['MODULES']), ('plugins', config['PLUGINS']))
                     for name in names for path in (root / folder / name).rglob('*.cgen'))
    if not targets:
        raise ValueError('No API definitions found for the configured engine modules')
    environment = host_environment(output)
    sdk = subprocess.check_output(['xcrun', '--sdk', 'macosx', '--show-sdk-path'],
                                  env=environment, text=True).strip()
    compiler = subprocess.check_output(['xcrun', '--sdk', 'macosx', '--find', 'clang'],
                                       env=environment, text=True).strip()
    generated = output / 'generated'
    previous = {path: (path.read_bytes(), path.stat()) for path in generated.rglob('*') if path.is_file()}
    # Relative output variables avoid the existing Makefile's unquoted output paths.
    # The working directory can therefore be any Xcode DerivedData path, including spaces.
    plugins = ['host-codegen/plugins/codegen/lib' + name + '.so'
               for name in ('header', 'properties', 'export')]
    run('make', '-f', root / 'tools/Makefile', 'MODULE_ROOT=' + str(root) + '/',
        'MODULES=' + ' '.join(targets), 'BUILDDIR=host-codegen', 'BINDIR=host-codegen/bin',
        'LIBDIR=host-codegen/lib', 'OBJECTDIR=host-codegen/obj', 'GENERATEDDIR=generated',
        'CODEGEN_PLUGINS=' + ' '.join(plugins), 'CODEGEN_DOCS_PLUGIN=',
        'CC=' + shlex.quote(compiler), 'ARCH_FLAGS=-arch ' + os.uname().machine,
        'HOST_XML_CFLAGS=' + shlex.join(['-isysroot', sdk, '-I' + sdk + '/usr/include/libxml2']),
        'HOST_XML_LIBS=-lxml2', *targets, cwd=output, env=environment)
    # Module rules regenerate text each time; keep unchanged headers from recompiling the app.
    for path, (content, stat) in previous.items():
        if path.is_file() and path.read_bytes() == content:
            os.utime(path, ns=(stat.st_atime_ns, stat.st_mtime_ns))


def seed_downloads(root, cache):
    spec = importlib.util.spec_from_file_location('orca_ios_dependencies', root / 'tools/ios/dependencies.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    for name, (url, checksum) in module.SOURCES.items():
        filename = name + ('.tar.xz' if url.endswith('.xz') else '.tar.gz')
        source = root / 'build/ios/downloads' / filename
        destination = cache / 'downloads' / filename
        if destination.exists() or not source.is_file():
            continue
        if hashlib.sha256(source.read_bytes()).hexdigest() != checksum:
            continue
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)


def copy_headers(source, target):
    target.mkdir(parents=True, exist_ok=True)
    for path in source.rglob('*'):
        relative = path.relative_to(source)
        destination = target / relative
        if path.is_dir():
            destination.mkdir(exist_ok=True)
        elif not destination.exists() or path.read_bytes() != destination.read_bytes():
            shutil.copy2(path, destination)
    for path in sorted(target.rglob('*'), reverse=True):
        if not (source / path.relative_to(target)).exists():
            if path.is_dir():
                path.rmdir()
            else:
                path.unlink()


def prepare(root, output):
    config = configuration(root)
    units = unity_sources(root)
    for name in shlex.split(os.environ.get('ORCA_XCODE_UNITS', '')):
        if not re.fullmatch(r'[A-Za-z0-9_-]+\.(?:c|m)', name):
            raise ValueError('ORCA_XCODE_UNITS must contain simple .c/.m filenames')
        units.setdefault(name, [])
    for name, sources in units.items():
        write(output / name, include_sources(sources))
    plugin_code = ''.join('extern int luaopen_orca_' + name + '(lua_State *);\n' for name in config['PLUGINS'])
    plugin_code += 'static const luaL_Reg plugin_modules[] = {\n'
    plugin_code += ''.join('    {"orca.' + name + '", luaopen_orca_' + name + '},\n' for name in config['PLUGINS'])
    write(output / 'include/plugins_luaopen.h', plugin_code + '    {NULL, NULL}\n};\n')
    generate_api(root, output, config)
    sdk = os.environ.get('PLATFORM_NAME', '')
    if sdk not in ('iphoneos', 'iphonesimulator'):
        raise ValueError('PLATFORM_NAME must be iphoneos or iphonesimulator')
    fallback = 'arm64' if sdk == 'iphoneos' else os.uname().machine
    architectures = list(dict.fromkeys(shlex.split(os.environ.get('ARCHS') or fallback)))
    if not architectures or any(arch not in ('arm64', 'x86_64') for arch in architectures):
        raise ValueError('ARCHS must contain arm64 and/or x86_64')
    if sdk == 'iphoneos' and architectures != ['arm64']:
        raise ValueError('Physical iPhone/iPad builds require ARCHS=arm64')
    cache = output / 'dependencies'
    seed_downloads(root, cache)
    environment = os.environ.copy()
    environment['PYTHONDONTWRITEBYTECODE'] = '1'
    for architecture in architectures:
        run(sys.executable, root / 'tools/ios/dependencies.py', '--sdk', sdk, '--arch', architecture,
            '--minimum', os.environ.get('IPHONEOS_DEPLOYMENT_TARGET', '16.0'), '--build-dir', cache,
            cwd=root, env=environment)
    copy_headers(cache / (sdk + '-' + architectures[0]) / 'deps/include', cache / 'include')


def resource_filter(directory, names):
    return [name for name in names if name.startswith('.') or name in ('build', 'ci_scripts', '__pycache__')
            or name.endswith(('.xcodeproj', '.xcworkspace', '.mobileprovision'))]


def resources(root, output):
    project = Path(os.environ['ORCA_PROJECT']).resolve()
    if not (project / 'package.lua').is_file():
        raise ValueError('ORCA_PROJECT must contain package.lua')
    relative = Path(os.environ['UNLOCALIZED_RESOURCES_FOLDER_PATH'])
    if relative.is_absolute() or '..' in relative.parts:
        raise ValueError('UNLOCALIZED_RESOURCES_FOLDER_PATH must be relative to TARGET_BUILD_DIR')
    build_directory = Path(os.environ['TARGET_BUILD_DIR']).resolve()
    destination = (build_directory / relative).resolve()
    if destination != build_directory and build_directory not in destination.parents:
        raise ValueError('Resource output must stay inside TARGET_BUILD_DIR')
    copies = [((root / 'share').resolve(), destination / 'share'), (project, destination / 'project')]
    for source, target in copies:
        if not source.is_dir():
            raise ValueError('Missing resource source: ' + str(source))
        if destination == source or source in destination.parents:
            raise ValueError('Resource output must not be inside its source directory')
        if source == target or source in target.parents or target in source.parents:
            raise ValueError('Resource output must not overlap its source directory')
    for source, target in copies:
        if target.is_symlink():
            target.unlink()
        elif target.exists():
            shutil.rmtree(target)
        shutil.copytree(source, target, ignore=resource_filter)


def main():
    if len(sys.argv) != 2 or sys.argv[1] not in ('prepare', 'resources'):
        raise ValueError('Usage: build.py prepare|resources')
    root = Path(os.environ['ORCA_ROOT']).resolve()
    output = Path(os.environ['DERIVED_FILE_DIR']).resolve()
    output.mkdir(parents=True, exist_ok=True)
    if output == root or output in root.parents:
        raise ValueError('DERIVED_FILE_DIR must be a separate build directory')
    {'prepare': prepare, 'resources': resources}[sys.argv[1]](root, output)


if __name__ == '__main__':
    try:
        main()
    except (KeyError, ValueError) as error:
        raise SystemExit('ORCA Xcode: ' + str(error))
