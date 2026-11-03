#!/usr/bin/env python3
"""Build pinned static Android dependencies (mbedTLS, curl, libxml2) with the NDK."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess


# Verified archives, as for the iOS build. curl reuses the iOS pin.
SOURCES = {
    'curl-8.10.1': ('https://curl.se/download/curl-8.10.1.tar.xz',
                    '73a4b0e99596a09fa5924a4fb7e4b995a85fda0d18a2c02ab9cf134bebce04ee',
                    ('https://github.com/curl/curl/releases/download/curl-8_10_1/curl-8.10.1.tar.xz',)),
    'mbedtls-3.6.2': ('https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-3.6.2/mbedtls-3.6.2.tar.bz2',
                      '8b54fb9bcf4d5a7078028e0520acddefb7900b3e66fec7f7175bb5b7d85ccdca', ()),
}
# libxml2 publishes no GitHub archive; its tag is pinned by commit instead.
GIT = {'libxml2-2.13.5': ('https://github.com/GNOME/libxml2.git', 'v2.13.5', 'de918d45e1b2276a28a4cd32bcf556bef65284e4')}
ABIS = ('arm64-v8a', 'x86_64')


def run(*args, cwd=None):
    subprocess.run([str(arg) for arg in args], cwd=cwd, check=True)


def archive_tree(name, cache):
    url, checksum, mirrors = SOURCES[name]
    suffix = ''.join(Path(url).suffixes[-2:])
    archive = cache / 'downloads' / (name + suffix)
    archive.parent.mkdir(parents=True, exist_ok=True)
    if archive.exists() and hashlib.sha256(archive.read_bytes()).hexdigest() != checksum:
        archive.unlink()
    partial = archive.with_name(archive.name + '.partial')
    for candidate in (url, *mirrors):
        if archive.exists():
            break
        result = subprocess.run(['curl', '--fail', '--location', '--silent', '--show-error', '--connect-timeout', '30',
                                 '--retry', '3', '--retry-delay', '2', '--output', str(partial), candidate])
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


def git_tree(name, cache):
    url, tag, commit = GIT[name]
    source = cache / 'sources' / name
    if not (source / '.git').is_dir():
        shutil.rmtree(source, ignore_errors=True)
        run('git', 'clone', '--quiet', '--depth', '1', '--branch', tag, url, source)
    actual = subprocess.check_output(['git', '-C', str(source), 'rev-parse', 'HEAD'], text=True).strip()
    if actual != commit:
        raise SystemExit(f'{name} is at {actual}, expected the pinned {commit}.')
    return source


def cmake(source, build, prefix, common, *options):
    run('cmake', '-S', source, '-B', build, *common, *options)
    run('cmake', '--build', build, '--parallel', os.cpu_count() or 2)
    run('cmake', '--install', build)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ndk', type=Path, default=os.environ.get('ANDROID_NDK_HOME') or os.environ.get('ANDROID_NDK_LATEST_HOME'))
    parser.add_argument('--abi', choices=ABIS, required=True)
    parser.add_argument('--api', default='26')
    parser.add_argument('--build-dir', type=Path, default=Path('build/android'))
    args = parser.parse_args()
    if not args.ndk or not (args.ndk / 'build/cmake/android.toolchain.cmake').is_file():
        raise SystemExit('Set ANDROID_NDK_HOME (or --ndk) to an Android NDK.')
    cache = args.build_dir.resolve()
    root = cache / args.abi
    prefix = root / 'deps'
    settings = {'ndk': str(args.ndk.resolve()), 'api': args.api, 'sources': sorted([*SOURCES, *GIT])}
    stamp = prefix / 'settings.json'
    if stamp.is_file() and json.loads(stamp.read_text()) == settings and (prefix / 'lib/libcurl.a').is_file():
        return
    shutil.rmtree(prefix, ignore_errors=True)
    common = ['-DCMAKE_TOOLCHAIN_FILE=' + str(args.ndk / 'build/cmake/android.toolchain.cmake'),
              '-DANDROID_ABI=' + args.abi, '-DANDROID_PLATFORM=android-' + args.api,
              '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_SHARED_LIBS=OFF', '-DCMAKE_POSITION_INDEPENDENT_CODE=ON',
              '-DCMAKE_INSTALL_PREFIX=' + str(prefix), '-DCMAKE_PREFIX_PATH=' + str(prefix),
              '-DCMAKE_FIND_ROOT_PATH=' + str(prefix)]
    work = root / 'work'
    shutil.rmtree(work, ignore_errors=True)
    cmake(archive_tree('mbedtls-3.6.2', cache), work / 'mbedtls', prefix, common,
          '-DENABLE_TESTING=OFF', '-DENABLE_PROGRAMS=OFF', '-DMBEDTLS_FATAL_WARNINGS=OFF')
    cmake(git_tree('libxml2-2.13.5', cache), work / 'libxml2', prefix, common,
          *('-DLIBXML2_WITH_' + name + '=OFF' for name in
            ('PYTHON', 'ICONV', 'ICU', 'LZMA', 'PROGRAMS', 'TESTS', 'HTTP', 'FTP', 'MODULES')),
          '-DLIBXML2_WITH_ZLIB=ON')
    lib = prefix / 'lib'
    # Android trusts the system CA directory; mbedTLS reads every certificate in it.
    cmake(archive_tree('curl-8.10.1', cache), work / 'curl', prefix, common,
          '-DCURL_USE_MBEDTLS=ON', '-DCURL_USE_OPENSSL=OFF', '-DBUILD_CURL_EXE=OFF', '-DBUILD_TESTING=OFF',
          '-DBUILD_LIBCURL_DOCS=OFF', '-DBUILD_MISC_DOCS=OFF', '-DENABLE_CURL_MANUAL=OFF',
          '-DCURL_USE_LIBPSL=OFF', '-DCURL_USE_LIBSSH2=OFF', '-DUSE_NGHTTP2=OFF', '-DUSE_LIBIDN2=OFF',
          '-DCURL_BROTLI=OFF', '-DCURL_ZSTD=OFF', '-DCURL_DISABLE_LDAP=ON', '-DCURL_DISABLE_LDAPS=ON',
          '-DCURL_CA_PATH=/system/etc/security/cacerts', '-DCURL_CA_BUNDLE=none', '-DCURL_CA_FALLBACK=OFF',
          '-DMBEDTLS_INCLUDE_DIRS=' + str(prefix / 'include'), '-DMBEDTLS_LIBRARY=' + str(lib / 'libmbedtls.a'),
          '-DMBEDX509_LIBRARY=' + str(lib / 'libmbedx509.a'), '-DMBEDCRYPTO_LIBRARY=' + str(lib / 'libmbedcrypto.a'))
    shutil.rmtree(work, ignore_errors=True)
    stamp.write_text(json.dumps(settings))


if __name__ == '__main__':
    main()
