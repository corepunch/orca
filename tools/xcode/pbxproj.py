"""Read Xcode OpenStep projects without invoking Apple's developer-tool launchers."""
from pathlib import Path
import plistlib


class _Parser:
    def __init__(self, text, path):
        self.text, self.path, self.position = text, str(path), 0

    def error(self, message, position=None):
        position = self.position if position is None else position
        line = self.text.count('\n', 0, position) + 1
        column = position - self.text.rfind('\n', 0, position)
        return ValueError(f'{self.path}:{line}:{column}: {message}')

    def trivia(self):
        while self.position < len(self.text):
            if self.text[self.position].isspace():
                self.position += 1
            elif self.text.startswith('//', self.position):
                end = self.text.find('\n', self.position + 2)
                self.position = len(self.text) if end < 0 else end + 1
            elif self.text.startswith('/*', self.position):
                start = self.position
                end = self.text.find('*/', start + 2)
                if end < 0:
                    raise self.error('unterminated block comment', start)
                self.position = end + 2
            else:
                break

    def consume(self, character):
        self.trivia()
        if self.position >= len(self.text) or self.text[self.position] != character:
            raise self.error(f'expected {character!r}')
        self.position += 1

    def quoted(self):
        start = self.position
        self.position += 1
        characters = []
        escapes = {'"': '"', '\\': '\\', '/': '/', 'n': '\n', 'r': '\r', 't': '\t',
                   'b': '\b', 'f': '\f', 'a': '\a', 'v': '\v'}
        while self.position < len(self.text):
            character = self.text[self.position]
            self.position += 1
            if character == '"':
                # Both OpenStep and JSON can encode non-BMP characters as UTF-16 pairs.
                value = ''.join(characters)
                result, index = [], 0
                while index < len(value):
                    code = ord(value[index])
                    if 0xD800 <= code <= 0xDBFF:
                        if index + 1 >= len(value) or not 0xDC00 <= ord(value[index + 1]) <= 0xDFFF:
                            raise self.error('unpaired Unicode high surrogate', start)
                        result.append(chr(0x10000 + ((code - 0xD800) << 10) + ord(value[index + 1]) - 0xDC00))
                        index += 2
                    elif 0xDC00 <= code <= 0xDFFF:
                        raise self.error('unpaired Unicode low surrogate', start)
                    else:
                        result.append(value[index])
                        index += 1
                return ''.join(result)
            if character != '\\':
                characters.append(character)
                continue
            escape_position = self.position - 1
            if self.position >= len(self.text):
                raise self.error('unterminated escape sequence', escape_position)
            escape = self.text[self.position]
            self.position += 1
            if escape in escapes:
                characters.append(escapes[escape])
            elif escape in ('u', 'U'):
                digits = self.text[self.position:self.position + 4]
                if len(digits) != 4 or any(digit not in '0123456789abcdefABCDEF' for digit in digits):
                    raise self.error('Unicode escape requires four hexadecimal digits', escape_position)
                characters.append(chr(int(digits, 16)))
                self.position += 4
            elif escape in '01234567':
                digits = escape
                while len(digits) < 3 and self.position < len(self.text) and self.text[self.position] in '01234567':
                    digits += self.text[self.position]
                    self.position += 1
                characters.append(chr(int(digits, 8)))
            elif escape in ('\n', '\r'):
                if escape == '\r' and self.text[self.position:self.position + 1] == '\n':
                    self.position += 1
            else:
                raise self.error('unsupported escape sequence \\' + escape, escape_position)
        raise self.error('unterminated quoted string', start)

    def scalar(self):
        self.trivia()
        if self.position < len(self.text) and self.text[self.position] == '"':
            return self.quoted()
        start = self.position
        while self.position < len(self.text):
            character = self.text[self.position]
            if character.isspace() or character in '{}()=;,"' or self.text.startswith(('//', '/*'), self.position):
                break
            self.position += 1
        if self.position == start:
            raise self.error('expected a string or unquoted scalar')
        return self.text[start:self.position]

    def value(self, depth=0):
        if depth > 128:
            raise self.error('property list nesting is too deep')
        self.trivia()
        if self.position >= len(self.text):
            raise self.error('unexpected end of property list')
        character = self.text[self.position]
        if character == '{':
            self.position += 1
            result = {}
            while True:
                self.trivia()
                if self.position < len(self.text) and self.text[self.position] == '}':
                    self.position += 1
                    return result
                key_position = self.position
                key = self.scalar()
                if key in result:
                    raise self.error('duplicate dictionary key ' + repr(key), key_position)
                self.consume('=')
                result[key] = self.value(depth + 1)
                self.consume(';')
        if character == '(':
            self.position += 1
            result = []
            self.trivia()
            if self.position < len(self.text) and self.text[self.position] == ')':
                self.position += 1
                return result
            while True:
                result.append(self.value(depth + 1))
                self.trivia()
                if self.position < len(self.text) and self.text[self.position] == ')':
                    self.position += 1
                    return result
                self.consume(',')
                self.trivia()
                if self.position < len(self.text) and self.text[self.position] == ')':
                    self.position += 1
                    return result
        return self.scalar()

    def parse(self):
        result = self.value()
        self.trivia()
        if self.position != len(self.text):
            raise self.error('unexpected content after property list')
        if not isinstance(result, dict):
            raise self.error('project property list must contain a root dictionary', 0)
        return result


def load(path):
    """Return a complete OpenStep, XML or binary project dictionary.

    OpenStep bare scalars remain strings. Invalid syntax raises ValueError with
    its source location, so callers can stop instead of overwriting saved settings.
    """
    path = Path(path)
    data = path.read_bytes()
    candidate = data.lstrip()
    if candidate.startswith(b'\xef\xbb\xbf'):
        candidate = candidate[3:].lstrip()
    if (data.startswith(b'bplist') or candidate.startswith((b'<?xml', b'<plist', b'<!DOCTYPE'))
            or data.startswith((b'\xff\xfe', b'\xfe\xff', b'\x00\x00\xfe\xff'))):
        try:
            result = plistlib.loads(data)
        except Exception as error:
            raise ValueError(f'{path}: invalid XML or binary property list: {error}') from error
        if not isinstance(result, dict):
            raise ValueError(f'{path}: project property list must contain a root dictionary')
        return result
    try:
        text = data.decode('utf-8-sig')
    except UnicodeDecodeError as error:
        raise ValueError(f'{path}: invalid UTF-8 at byte {error.start}') from error
    return _Parser(text, path).parse()
