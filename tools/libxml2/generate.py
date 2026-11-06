#!/usr/bin/env python3
"""Expand libs/libxml2's CMake templates into the configuration ORCA builds.

ORCA compiles libxml2 from tools/libxml2/sources.txt with the Makefile, Xcode,
WebGL and iOS builds, so the two configuration headers are generated once and
committed here. The feature set matches the Android build in
tools/android/dependencies.py, except that zlib is off. Rerun this script after
bumping the libs/libxml2 submodule, then check the diff.
"""
from pathlib import Path
import re

HERE = Path(__file__).resolve().parent
TEMPLATES = HERE.parents[1] / 'libs' / 'libxml2'

VERSION = '2.13.5'
VERSION_NUMBER = '21305'

FEATURES = {
    'WITH_C14N': 1, 'WITH_CATALOG': 1, 'WITH_DEBUG': 1, 'WITH_FTP': 0, 'WITH_HTML': 1,
    'WITH_HTTP': 0, 'WITH_ICONV': 0, 'WITH_ICU': 0, 'WITH_ISO8859X': 1, 'WITH_LEGACY': 0,
    'WITH_LZMA': 0, 'WITH_MODULES': 0, 'WITH_OUTPUT': 1, 'WITH_PATTERN': 1, 'WITH_PUSH': 1,
    'WITH_READER': 1, 'WITH_REGEXPS': 1, 'WITH_SAX1': 1, 'WITH_SCHEMAS': 1,
    'WITH_SCHEMATRON': 1, 'WITH_THREADS': 1, 'WITH_THREAD_ALLOC': 0, 'WITH_TREE': 1,
    'WITH_VALID': 1, 'WITH_WRITER': 1, 'WITH_XINCLUDE': 1, 'WITH_XPATH': 1, 'WITH_XPTR': 1,
    'WITH_XPTR_LOCS': 0, 'WITH_ZLIB': 0,
}

HAVE = (
    'HAVE_CTYPE_H', 'HAVE_ERRNO_H', 'HAVE_FCNTL_H', 'HAVE_FLOAT_H', 'HAVE_FPRINTF',
    'HAVE_GETTIMEOFDAY', 'HAVE_INTTYPES_H', 'HAVE_LIMITS_H', 'HAVE_LOCALTIME', 'HAVE_MATH_H',
    'HAVE_PRINTF', 'HAVE_PTHREAD_H', 'HAVE_RAND', 'HAVE_SNPRINTF', 'HAVE_SPRINTF', 'HAVE_SRAND',
    'HAVE_SSCANF', 'HAVE_STAT', 'HAVE_STDARG_H', 'HAVE_STDINT_H', 'HAVE_STDLIB_H',
    'HAVE_STRFTIME', 'HAVE_STRING_H', 'HAVE_STRINGS_H', 'HAVE_SYS_STAT_H', 'HAVE_SYS_TIME_H',
    'HAVE_SYS_TYPES_H', 'HAVE_TIME', 'HAVE_TIME_H', 'HAVE_UNISTD_H', 'HAVE_VFPRINTF',
    'HAVE_VSNPRINTF', 'HAVE_VSPRINTF', 'STDC_HEADERS',
)


def xmlversion():
    text = (TEMPLATES / 'include/libxml/xmlversion.h.in').read_text()
    values = dict(FEATURES, LIBXML_VERSION_NUMBER=VERSION_NUMBER, VERSION=VERSION,
                  LIBXML_VERSION_EXTRA='', MODULE_EXTENSION='.so')
    return re.sub(r'@([A-Z_0-9]+)@', lambda m: str(values[m.group(1)]), text)


def config():
    values = {'VERSION': VERSION, 'PACKAGE': 'libxml2', 'PACKAGE_NAME': 'libxml2',
              'PACKAGE_VERSION': VERSION, 'PACKAGE_STRING': 'libxml2 ' + VERSION,
              'PACKAGE_TARNAME': 'libxml2', 'PACKAGE_BUGREPORT': '',
              'PACKAGE_URL': 'https://gitlab.gnome.org/GNOME/libxml2',
              'ICONV_CONST': '', 'LT_OBJDIR': '.libs/'}
    out = []
    for line in (TEMPLATES / 'config.h.cmake.in').read_text().splitlines():
        m = re.match(r'#cmakedefine (\w+)', line)
        if m:
            name = m.group(1)
            if name in HAVE:
                out.append('#define ' + name + ' 1')
            elif name == 'VERSION':
                out.append('#define VERSION "' + VERSION + '"')
            elif name == 'ATTRIBUTE_DESTRUCTOR':
                out.append('#define ATTRIBUTE_DESTRUCTOR __attribute__((destructor))')
            else:
                out.append('/* #undef ' + name + ' */')
            continue
        out.append(re.sub(r'@(\w+)@', lambda m: values.get(m.group(1), ''), line))
    return '\n'.join(out) + '\n'


def main():
    (HERE / 'include/libxml').mkdir(parents=True, exist_ok=True)
    (HERE / 'include/libxml/xmlversion.h').write_text(xmlversion())
    (HERE / 'config.h').write_text(config())


if __name__ == '__main__':
    main()
