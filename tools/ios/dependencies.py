#!/usr/bin/env python3
"""Build pinned static iOS dependencies, without linking host Homebrew libraries."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile


SOURCES = {
    'curl-8.10.1': ('https://curl.se/download/curl-8.10.1.tar.xz',
                    '73a4b0e99596a09fa5924a4fb7e4b995a85fda0d18a2c02ab9cf134bebce04ee'),
}
# Same SHA256-verified archives, tried when the primary host is unreachable.
MIRRORS = {
    'curl-8.10.1': ('https://github.com/curl/curl/releases/download/curl-8_10_1/curl-8.10.1.tar.xz',),
}
# Lua and LZ4 come from the repository's submodules, like the desktop build, so
# a clean Xcode Cloud archive does not depend on lua.org or codeload being up.
SUBMODULES = {'lua': 'libs/lua', 'lz4': 'libs/lz4/lib'}
ROOT = Path(__file__).resolve().parents[2]


def run(*args, cwd=None, env=None):
    subprocess.run([str(arg) for arg in args], cwd=cwd, env=env, check=True)


def source_tree(name, cache):
    url, checksum = SOURCES[name]
    suffix = '.tar.xz' if url.endswith('.xz') else '.tar.gz'
    archive = cache / 'downloads' / (name + suffix)
    archive.parent.mkdir(parents=True, exist_ok=True)
    if archive.exists() and hashlib.sha256(archive.read_bytes()).hexdigest() != checksum:
        archive.unlink()
    partial = archive.with_suffix(archive.suffix + '.partial')
    for candidate in (url, *MIRRORS.get(name, ())):
        if archive.exists():
            break
        result = subprocess.run(['curl', '--fail', '--location', '--silent', '--show-error',
                                 '--connect-timeout', '30', '--retry', '3', '--retry-delay', '2',
                                 '--output', str(partial), candidate])
        if result.returncode == 0 and hashlib.sha256(partial.read_bytes()).hexdigest() == checksum:
            partial.replace(archive)
        else:
            print('Could not fetch a verified ' + name + ' from ' + candidate, flush=True)
            partial.unlink(missing_ok=True)
    if not archive.exists():
        raise SystemExit('Could not download a verified ' + name + ' archive.')
    source = cache / 'sources' / name
    if not source.exists():
        source.mkdir(parents=True)
        run('tar', '-xf', archive, '-C', source, '--strip-components=1')
    return source


def submodule(name):
    source = ROOT / SUBMODULES[name]
    marker = source / ('lua.h' if name == 'lua' else 'lz4.h')
    if not marker.is_file():
        raise SystemExit('Missing ' + str(source) + '. Run: git submodule update --init libs/lua libs/lz4')
    return source


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', choices=('iphoneos', 'iphonesimulator'), required=True)
    parser.add_argument('--arch', choices=('arm64', 'x86_64'), required=True)
    parser.add_argument('--minimum', default='16.0')
    parser.add_argument('--build-dir', type=Path, default=Path('build/ios'))
    args = parser.parse_args()
    cache = args.build_dir.resolve()
    root = cache / (args.sdk + '-' + args.arch)
    prefix = root / 'deps'
    sdk = subprocess.check_output(['xcrun', '--sdk', args.sdk, '--show-sdk-path'], text=True).strip()
    compiler = subprocess.check_output(['xcrun', '--sdk', args.sdk, '--find', 'clang'], text=True).strip()
    minimum = '-miphoneos-version-min=' if args.sdk == 'iphoneos' else '-mios-simulator-version-min='
    flags = ['-arch', args.arch, '-isysroot', sdk, minimum + args.minimum, '-O2', '-g']
    lua_config = Path(__file__).resolve().with_name('lua_config.h')
    stamp = prefix / 'settings.json'
    lua, lz4 = submodule('lua'), submodule('lz4')
    revision = hashlib.sha256(b''.join(path.read_bytes() for folder in (lua, lz4)
                                       for path in sorted(folder.glob('*.[ch]')))).hexdigest()
    settings = json.dumps({'sdk': sdk, 'arch': args.arch, 'minimum': args.minimum,
                           'sources': SOURCES, 'submodules': revision, 'recipe': hashlib.sha256(
                               Path(__file__).read_bytes() + lua_config.read_bytes()).hexdigest()}, sort_keys=True)
    if stamp.exists() and stamp.read_text() == settings and all(
            (prefix / 'lib' / name).is_file() for name in ('liblua.a', 'liblz4.a', 'libcurl.a')):
        return
    (prefix / 'lib').mkdir(parents=True, exist_ok=True)
    (prefix / 'include').mkdir(exist_ok=True)
    env = os.environ.copy()
    # Configure must not discover macOS packages from a developer's shell.
    env.update(CC=compiler, CPP=shlex.join([compiler, '-E', *flags]),
               CFLAGS=shlex.join(flags), CPPFLAGS='', LDFLAGS=shlex.join(flags),
               PKG_CONFIG='/usr/bin/false', PKG_CONFIG_PATH='', PKG_CONFIG_LIBDIR='',
               CLANG_MODULE_CACHE_PATH=str(root / 'module-cache'))
    for key in ('CPATH', 'C_INCLUDE_PATH', 'CPLUS_INCLUDE_PATH', 'LIBRARY_PATH', 'SDKROOT',
                'SDK_NAME', 'SDK_VERSION', 'IPHONEOS_DEPLOYMENT_TARGET', 'TVOS_DEPLOYMENT_TARGET',
                'WATCHOS_DEPLOYMENT_TARGET', 'XROS_DEPLOYMENT_TARGET', 'MACOSX_DEPLOYMENT_TARGET',
                'CC_BUILD', 'CCexe', 'MAKEFLAGS', 'GNUMAKEFLAGS', 'MFLAGS', 'MAKELEVEL'):
        env.pop(key, None)
    host_sdk = subprocess.check_output(['xcrun', '--sdk', 'macosx', '--show-sdk-path'],
                                       env=env, text=True).strip()
    host_compiler = subprocess.check_output(['xcrun', '--sdk', 'macosx', '--find', 'clang'],
                                            env=env, text=True).strip()
    env['CC_BUILD'] = shlex.join([host_compiler, '-arch', os.uname().machine, '-isysroot', host_sdk,
                                  '-mmacosx-version-min=12.0'])
    jobs = str(min(os.cpu_count() or 2, 8))
    sources = {name: source_tree(name, cache) for name in SOURCES}
    objects = []
    output = root / 'dependency-build' / 'lua'
    output.mkdir(parents=True, exist_ok=True)
    for path in sorted(lua.glob('*.c')):
        if path.name in ('lua.c', 'luac.c', 'onelua.c', 'ltests.c'):
            continue
        obj = output / (path.stem + '.o')
        config = ['-include', lua_config] if path.name == 'loslib.c' else []
        run(compiler, *flags, '-DLUA_USE_IOS', *config, '-c', path, '-o', obj)
        objects.append(obj)
    run('xcrun', '--sdk', args.sdk, 'ar', 'rcs', prefix / 'lib/liblua.a', *objects)
    for name in ('lua.h', 'luaconf.h', 'lauxlib.h', 'lualib.h'):
        shutil.copy2(lua / name, prefix / 'include' / name)
    objects = []
    output = root / 'dependency-build' / 'lz4'
    output.mkdir(parents=True, exist_ok=True)
    for name in ('lz4', 'lz4hc', 'lz4frame', 'xxhash'):
        obj = output / (name + '.o')
        run(compiler, *flags, '-c', lz4 / (name + '.c'), '-o', obj)
        objects.append(obj)
    run('xcrun', '--sdk', args.sdk, 'ar', 'rcs', prefix / 'lib/liblz4.a', *objects)
    for path in lz4.glob('*.h'):
        shutil.copy2(path, prefix / 'include' / path.name)
    builds = (
        ('curl-8.10.1', ['--with-secure-transport', '--without-openssl', '--without-libpsl',
                       '--without-libidn2', '--without-nghttp2', '--without-brotli', '--without-zstd',
                       '--without-ca-bundle', '--without-ca-path', '--disable-ldap', '--disable-ldaps',
                       '--disable-ftp', '--disable-rtsp', '--disable-dict', '--disable-telnet',
                       '--disable-tftp', '--disable-pop3', '--disable-imap', '--disable-smb',
                       '--disable-smtp', '--disable-gopher', '--disable-mqtt']),
    )
    for name, options in builds:
        log = root / (name + '.log')
        print('Building ' + name + '; log: ' + str(log), flush=True)
        # Upstream make/configure files do not support spaces in build or source paths.
        # Keep only their transient work in a path without spaces, then retain static outputs.
        with tempfile.TemporaryDirectory(prefix='orca-ios-', dir='/tmp') as temporary:
            scratch = Path(temporary)
            source, output, install = scratch / 'source', scratch / 'build', scratch / 'install'
            shutil.copytree(sources[name], source)
            output.mkdir()
            with log.open('w') as stream:
                commands = ([str(source / 'configure'), '--host=' + args.arch + '-apple-darwin',
                             '--prefix=' + str(install), '--disable-shared', '--enable-static', *options],
                            ['make', '-j' + jobs], ['make', 'install'])
                for command in commands:
                    result = subprocess.run(command, cwd=output, env=env, stdout=stream, stderr=subprocess.STDOUT)
                    if result.returncode:
                        raise SystemExit(name + ' failed; see ' + str(log))
            shutil.copytree(install / 'include', prefix / 'include', dirs_exist_ok=True)
            for library in (install / 'lib').glob('*.a'):
                shutil.copy2(library, prefix / 'lib' / library.name)
    stamp.write_text(settings)


if __name__ == '__main__':
    main()
