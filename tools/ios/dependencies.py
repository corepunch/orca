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
    'lua-5.4.8': ('https://www.lua.org/ftp/lua-5.4.8.tar.gz',
                  '4f18ddae154e793e46eeab727c59ef1c0c0c2b744e7b94219710d76f530629ae'),
    'freetype-2.13.3': ('https://download.savannah.gnu.org/releases/freetype/freetype-2.13.3.tar.xz',
                       '0550350666d427c74daeb85d5ac7bb353acba5f76956395995311a9c6f063289'),
    'lz4-1.10.0': ('https://codeload.github.com/lz4/lz4/tar.gz/refs/tags/v1.10.0',
                   '537512904744b35e232912055ccf8ec66d768639ff3abe5788d90d792ec5f48b'),
    'curl-8.10.1': ('https://curl.se/download/curl-8.10.1.tar.xz',
                    '73a4b0e99596a09fa5924a4fb7e4b995a85fda0d18a2c02ab9cf134bebce04ee'),
}


def run(*args, cwd=None, env=None):
    subprocess.run([str(arg) for arg in args], cwd=cwd, env=env, check=True)


def source_tree(name, cache):
    url, checksum = SOURCES[name]
    suffix = '.tar.xz' if url.endswith('.xz') else '.tar.gz'
    archive = cache / 'downloads' / (name + suffix)
    archive.parent.mkdir(parents=True, exist_ok=True)
    if not archive.exists():
        partial = archive.with_suffix(archive.suffix + '.partial')
        run('curl', '--fail', '--location', '--retry', '3', '--output', partial, url)
        partial.replace(archive)
    if hashlib.sha256(archive.read_bytes()).hexdigest() != checksum:
        raise SystemExit('Checksum mismatch: ' + str(archive) + '. Remove the archive and retry.')
    source = cache / 'sources' / name
    if not source.exists():
        source.mkdir(parents=True)
        run('tar', '-xf', archive, '-C', source, '--strip-components=1')
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
    stamp = prefix / 'settings.json'
    settings = json.dumps({'sdk': sdk, 'arch': args.arch, 'minimum': args.minimum,
                           'sources': SOURCES, 'recipe': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}, sort_keys=True)
    if stamp.exists() and stamp.read_text() == settings and all(
            (prefix / 'lib' / name).is_file() for name in ('liblua.a', 'libfreetype.a', 'liblz4.a', 'libcurl.a')):
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
    lua = sources['lua-5.4.8'] / 'src'
    objects = []
    output = root / 'dependency-build' / 'lua'
    output.mkdir(parents=True, exist_ok=True)
    for path in sorted(lua.glob('*.c')):
        if path.name in ('lua.c', 'luac.c'):
            continue
        obj = output / (path.stem + '.o')
        run(compiler, *flags, '-DLUA_USE_IOS', '-c', path, '-o', obj)
        objects.append(obj)
    run('xcrun', '--sdk', args.sdk, 'ar', 'rcs', prefix / 'lib/liblua.a', *objects)
    for name in ('lua.h', 'luaconf.h', 'lauxlib.h', 'lualib.h', 'lua.hpp'):
        shutil.copy2(lua / name, prefix / 'include' / name)
    lz4 = sources['lz4-1.10.0'] / 'lib'
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
        ('freetype-2.13.3', ['--without-harfbuzz', '--without-bzip2', '--without-png', '--without-brotli']),
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
