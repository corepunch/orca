"""Pure Python regressions for reading existing Xcode project property lists."""
from pathlib import Path
import plistlib
import socket
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import pbxproj


class ProjectReaderTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='orca-pbx-reader-')
        self.addCleanup(self.temporary.cleanup)
        self.path = Path(self.temporary.name) / 'project.pbxproj'

    def load(self, content):
        self.path.write_bytes(content.encode('utf-8') if isinstance(content, str) else content)
        with patch.object(subprocess, 'Popen', side_effect=AssertionError('Reader launched a child process')), \
                patch.object(socket, 'socket', side_effect=AssertionError('Reader opened a network socket')):
            return pbxproj.load(self.path)

    def test_reads_xcode_comments_nested_objects_and_conditional_signing_settings(self):
        result = self.load('''// !$*UTF8*$!
        {
            archiveVersion = 1;
            objects = {
                ABC123 /* Debug */ = {
                    isa = XCBuildConfiguration;
                    name = Debug;
                    buildSettings = {
                        DEVELOPMENT_TEAM = QA12345678;
                        CODE_SIGN_STYLE = Manual;
                        "CODE_SIGN_IDENTITY[sdk=iphoneos*]" = "Apple Development: QA (QA12345678)";
                        PRODUCT_BUNDLE_IDENTIFIER = example.qa.chat;
                        PROVISIONING_PROFILE_SPECIFIER = "Chat, QA /* literal */ // profile";
                        HEADER_SEARCH_PATHS = ("$(inherited)", "$(SRCROOT)/Headers with spaces",);
                    };
                };
            };
            rootObject = ABC123; // trailing comment
        }''')
        self.assertEqual(result['archiveVersion'], '1')
        self.assertEqual(result['rootObject'], 'ABC123')
        settings = result['objects']['ABC123']['buildSettings']
        self.assertEqual(settings['CODE_SIGN_STYLE'], 'Manual')
        self.assertEqual(settings['DEVELOPMENT_TEAM'], 'QA12345678')
        self.assertEqual(settings['CODE_SIGN_IDENTITY[sdk=iphoneos*]'],
                         'Apple Development: QA (QA12345678)')
        self.assertEqual(settings['PROVISIONING_PROFILE_SPECIFIER'], 'Chat, QA /* literal */ // profile')
        self.assertEqual(settings['HEADER_SEARCH_PATHS'], ['$(inherited)', '$(SRCROOT)/Headers with spaces'])

    def test_decodes_quoted_escapes_unicode_surrogate_pairs_and_octal(self):
        result = self.load(r'''{
            name = "Caf\U00E9 \uD83D\uDE80";
            script = "printf \"a\\b\"\n\t\101";
            punctuation = "{}()=;, // /* are text */";
        }''')
        self.assertEqual(result['name'], 'Café 🚀')
        self.assertEqual(result['script'], 'printf "a\\b"\n\tA')
        self.assertEqual(result['punctuation'], '{}()=;, // /* are text */')

    def test_accepts_line_continuation_bom_empty_containers_and_final_array_element(self):
        result = self.load('\ufeff{ path = "first\\\nsecond"; empty = {}; list = (); values = (one, two); }')
        self.assertEqual(result, {'path': 'firstsecond', 'empty': {}, 'list': [], 'values': ['one', 'two']})

    def test_xml_and_binary_property_lists_preserve_signing_and_value_types(self):
        project = {'archiveVersion': 1, 'objects': {'ABC': {'buildSettings': {
            'DEVELOPMENT_TEAM': 'TEAM123456', 'CODE_SIGN_STYLE': 'Manual',
            'PROVISIONING_PROFILE_SPECIFIER': 'Café QA',
            'SUPPORTED_PLATFORMS': ['iphoneos', 'iphonesimulator'], 'ENABLED': True,
        }}}, 'rootObject': 'ABC'}
        for format in (plistlib.FMT_XML, plistlib.FMT_BINARY):
            with self.subTest(format=format):
                self.assertEqual(self.load(plistlib.dumps(project, fmt=format)), project)

    def test_malformed_openstep_reports_source_location_instead_of_partial_settings(self):
        for content in ('{ key = value }', '{ key = "unterminated; }', '{ /* never closed',
                        '{ array = (one two); }', '{ key = ; }', '{ key = one; key = two; }',
                        '{ key = value; } unexpected', '{ value = "\\q"; }',
                        '{ value = "\\uD83D"; }', '{ value = "\\uDE80"; }'):
            with self.subTest(content=content), self.assertRaises(ValueError) as error:
                self.load(content)
            self.assertIn(str(self.path), str(error.exception))
            self.assertRegex(str(error.exception), r':\d+:\d+:')

    def test_invalid_utf8_and_invalid_xml_or_binary_are_rejected(self):
        for content in (b'{ name = "\xff"; }', b'<?xml version="1.0"?><plist><dict>', b'bplist00broken'):
            with self.subTest(content=content), self.assertRaises(ValueError) as error:
                self.load(content)
            self.assertIn(str(self.path), str(error.exception))

    def test_non_dictionary_roots_are_rejected_in_every_format(self):
        for content in ('(one, two)', plistlib.dumps(['one']),
                        plistlib.dumps(['one'], fmt=plistlib.FMT_BINARY)):
            with self.subTest(content=content), self.assertRaises(ValueError):
                self.load(content)

    def test_excessively_nested_data_raises_value_error(self):
        with self.assertRaises(ValueError):
            self.load('{ nested = ' + '(' * 140 + 'leaf' + ')' * 140 + '; }')


if __name__ == '__main__':
    unittest.main()
