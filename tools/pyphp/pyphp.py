#!/usr/bin/env python3
"""pyphp – PHP-like template processor backed by Python model objects.

Translates a restricted subset of PHP template syntax to Python and
executes it in a context that provides PHP built-in functions and the
project's ``model.py`` data model.

Usage::

    python3 -m pyphp.pyphp <template.php> <source.xml> > output.c
"""

import sys
import os
import re

# ---------------------------------------------------------------------------
# Bootstrap: put the tools/ directory on sys.path so model.py is importable
# ---------------------------------------------------------------------------
_tools_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if _tools_dir not in sys.path:
    sys.path.insert(0, _tools_dir)

from model import Model, fnv1a32 as _fnv1a32_int
from model import formatDefault as _formatDefault

# ---------------------------------------------------------------------------
# PHP built-in functions exposed in the template execution context
# ---------------------------------------------------------------------------

def strtoupper(s):            return str(s).upper()
def strtolower(s):            return str(s).lower()
def strlen(s):                return len(str(s))
def trim(s):                  return str(s).strip()
def ltrim(s):                 return str(s).lstrip()
def rtrim(s):                 return str(s).rstrip()
def implode(sep, arr):        return sep.join(str(x) for x in arr)
def explode(sep, s):          return str(s).split(str(sep))
def str_replace(a, b, s):     return str(s).replace(str(a), str(b))
def is_null(v):               return v is None
def empty(v):                 return not v
def intval(v):                return int(v)
def strval(v):                return str(v)
def sprintf(fmt, *args):      return fmt % args
def print_r(v, ret=False):    return repr(v)

def substr(s, start, length=None):
    s = str(s)
    if length is None:  return s[start:]
    if length < 0:      return s[start:length]
    return s[start:start + length]

def count(x):
    if hasattr(x, '__len__'):  return len(x)
    return sum(1 for _ in x)

def hash(algo, s):
    if algo == 'fnv1a32':
        return '%08x' % _fnv1a32_int(str(s))
    raise ValueError(f'Unknown hash algorithm: {algo!r}')

def _php_items(arr):
    """Return iterable of (key, val) pairs; handles dicts and pair iterables."""
    if isinstance(arr, dict):
        return arr.items()
    return arr

_BUILTINS = {
    'strtoupper': strtoupper, 'strtolower': strtolower,
    'strlen': strlen, 'trim': trim, 'ltrim': ltrim, 'rtrim': rtrim,
    'implode': implode, 'explode': explode, 'str_replace': str_replace,
    'substr': substr, 'count': count, 'hash': hash,
    'is_null': is_null, 'empty': empty, 'intval': intval, 'strval': strval,
    'sprintf': sprintf, 'print_r': print_r,
    '_php_items': _php_items,
    'formatDefault': _formatDefault,
    'Model': Model,
    'TRUE': True, 'FALSE': False, 'NULL': None,
    'true': True, 'false': False, 'null': None,
}

# ---------------------------------------------------------------------------
# PHP string interpolation helpers
# ---------------------------------------------------------------------------

def _interp_double_quoted(inner):
    """Expand PHP ``$var`` and ``{$var}`` in a double-quoted string body.

    Returns ``(new_inner, is_fstring)`` where *is_fstring* is True when any
    substitution was made (meaning the result should be an f-string).
    """
    # {$var} → {_var}
    inner, n1 = re.subn(r'\{\$(\w+)\}',
                        lambda m: '{_' + m.group(1) + '}', inner)
    # $var[key] → {_var[key]}
    inner, n2 = re.subn(r'\$(\w+)\[(\w+)\]',
                        lambda m: '{_' + m.group(1) + '[' + m.group(2) + ']}',
                        inner)
    # $var → {_var}
    inner, n3 = re.subn(r'\$(\w+)',
                        lambda m: '{_' + m.group(1) + '}', inner)
    return inner, bool(n1 + n2 + n3)

# ---------------------------------------------------------------------------
# PHP expression → Python expression translator
# ---------------------------------------------------------------------------

def _translate_expr(php_expr):
    """Translate a PHP expression string to a Python expression string.

    Handles ``$var``, ``$obj->prop``, ``$arr[key]``, ``new Cls(args)``,
    PHP string literals (with interpolation), and TRUE/FALSE/NULL.
    """
    result = []
    i = 0
    src = php_expr

    while i < len(src):
        ch = src[i]

        # ── double-quoted string ──────────────────────────────────────────
        if ch == '"':
            j = i + 1
            buf = []
            while j < len(src) and src[j] != '"':
                if src[j] == '\\' and j + 1 < len(src):
                    buf.append(src[j:j + 2])
                    j += 2
                else:
                    buf.append(src[j])
                    j += 1
            inner = ''.join(buf)
            inner, is_f = _interp_double_quoted(inner)
            prefix = 'f' if is_f else ''
            result.append(f'{prefix}"{inner}"')
            i = j + 1
            continue

        # ── single-quoted string ──────────────────────────────────────────
        if ch == "'":
            j = i + 1
            buf = []
            while j < len(src) and src[j] != "'":
                if src[j] == '\\' and j + 1 < len(src):
                    buf.append(src[j:j + 2])
                    j += 2
                else:
                    buf.append(src[j])
                    j += 1
            inner = ''.join(buf).replace("\\'", "'").replace('\\\\', '\\')
            result.append(repr(inner))
            i = j + 1
            continue

        # ── $argv[n] ──────────────────────────────────────────────────────
        m = re.match(r'\$argv\[(\d+)\]', src[i:])
        if m:
            result.append(f'_argv[{m.group(1)}]')
            i += len(m.group(0))
            continue

        # ── $var[key] ─────────────────────────────────────────────────────
        m = re.match(r'\$(\w+)\[(\w+)\]', src[i:])
        if m:
            result.append(f'_{m.group(1)}[{m.group(2)}]')
            i += len(m.group(0))
            continue

        # ── $var ──────────────────────────────────────────────────────────
        m = re.match(r'\$(\w+)', src[i:])
        if m:
            result.append(f'_{m.group(1)}')
            i += len(m.group(0))
            continue

        # ── PHP arrow operator (->) ───────────────────────────────────────
        if src[i:i + 2] == '->':
            result.append('.')
            i += 2
            continue

        result.append(ch)
        i += 1

    py = ''.join(result)
    py = re.sub(r'\bnew\s+(\w+)\(', r'\1(', py)
    py = re.sub(r'\bTRUE\b', 'True', py)
    py = re.sub(r'\bFALSE\b', 'False', py)
    py = re.sub(r'\bNULL\b', 'None', py)
    # PHP ! (not) operator → Python not
    py = re.sub(r'(?<![=!<>])!(?!=)', 'not ', py)
    # PHP && → Python and, PHP || → Python or
    py = py.replace('&&', ' and ').replace('||', ' or ')
    return py

# ---------------------------------------------------------------------------
# PHP code-block → Python code translator  (brace-style → indent-style)
# ---------------------------------------------------------------------------

def _tokenize_php_code(code):
    """Yield ``(kind, value)`` tokens from a PHP code string.

    Token kinds: ``STRING_DQ``, ``STRING_SQ``, ``COMMENT``,
    ``OPEN`` (``{``), ``CLOSE`` (``}``), ``SEMI`` (``;``), ``NL``, ``OTHER``.
    """
    i = 0
    while i < len(code):
        ch = code[i]
        if ch == '"':
            j = i + 1
            while j < len(code) and code[j] != '"':
                if code[j] == '\\':
                    j += 1
                j += 1
            yield ('STRING_DQ', code[i:j + 1])
            i = j + 1
        elif ch == "'":
            j = i + 1
            while j < len(code) and code[j] != "'":
                if code[j] == '\\':
                    j += 1
                j += 1
            yield ('STRING_SQ', code[i:j + 1])
            i = j + 1
        elif code[i:i + 2] == '//':
            j = code.find('\n', i)
            j = j if j != -1 else len(code)
            yield ('COMMENT', code[i:j])
            i = j
        elif ch == '{':
            yield ('OPEN', ch);   i += 1
        elif ch == '}':
            yield ('CLOSE', ch);  i += 1
        elif ch == ';':
            yield ('SEMI', ch);   i += 1
        elif ch == '\n':
            yield ('NL', ch);     i += 1
        else:
            yield ('OTHER', ch);  i += 1


def _translate_statement(raw):
    """Translate the text of a single PHP statement line to Python."""
    raw = raw.rstrip(';').strip()
    if not raw:
        return ''

    # ── foreach ($arr as $k => $v) ────────────────────────────────────────
    m = re.match(r'^foreach\s*\(\s*(.+?)\s+as\s+\$(\w+)\s*=>\s*\$(\w+)\s*\)', raw)
    if m:
        arr = _translate_expr(m.group(1))
        return f'for _{m.group(2)}, _{m.group(3)} in _php_items({arr})'

    m = re.match(r'^foreach\s*\(\s*(.+?)\s+as\s+\$(\w+)\s*\)', raw)
    if m:
        arr = _translate_expr(m.group(1))
        return f'for _{m.group(2)} in {arr}'

    # ── function name($a, $b) ─────────────────────────────────────────────
    m = re.match(r'^function\s+(\w+)\s*\(([^)]*)\)', raw)
    if m:
        name = m.group(1)
        params = [('_' + p.strip().lstrip('$')) for p in m.group(2).split(',') if p.strip()]
        return f'def {name}({", ".join(params)})'

    # ── echo(...) ─────────────────────────────────────────────────────────
    m = re.match(r'^echo\s*\((.+)\)\s*$', raw, re.DOTALL)
    if m:
        return f'_write({_translate_expr(m.group(1).strip())})'
    m = re.match(r'^echo\s+(.+)$', raw, re.DOTALL)
    if m:
        return f'_write({_translate_expr(m.group(1).strip())})'

    # ── $var op= expr ──────────────────────────────────────────────────────
    m = re.match(r'^\$(\w+)\s*([+\-*/])=\s*(.+)$', raw, re.DOTALL)
    if m:
        return f'_{m.group(1)} {m.group(2)}= {_translate_expr(m.group(3).strip())}'

    # ── $var = expr ────────────────────────────────────────────────────────
    m = re.match(r'^\$(\w+)\s*=\s*(.+)$', raw, re.DOTALL)
    if m:
        return f'_{m.group(1)} = {_translate_expr(m.group(2).strip())}'

    # ── return expr ───────────────────────────────────────────────────────
    m = re.match(r'^return\s+(.*)$', raw, re.DOTALL)
    if m:
        return f'return {_translate_expr(m.group(1).strip())}'

    # ── // comment ────────────────────────────────────────────────────────
    if raw.startswith('//'):
        return '# ' + raw[2:].strip()

    # ── general expression ────────────────────────────────────────────────
    return _translate_expr(raw)


def _translate_php_block(php_code):
    """Translate a brace-style PHP code block to Python indented source."""
    tokens = list(_tokenize_php_code(php_code))
    lines = []
    indent = 0
    buf = []          # accumulates tokens for current statement

    def flush(suffix=''):
        nonlocal buf
        parts = []
        for k, v in buf:
            if k == 'SEMI':
                continue
            elif k == 'COMMENT':
                parts.append('#' + v[2:])
            else:
                parts.append(v)
        raw = ''.join(parts).strip()
        if raw:
            py = _translate_statement(raw)
            if py:
                lines.append('\t' * indent + py + suffix)
        buf = []

    for kind, val in tokens:
        if kind == 'NL':
            flush()
        elif kind == 'OPEN':
            flush(':')       # line before { ends with ':'
            indent += 1
        elif kind == 'CLOSE':
            flush()
            indent = max(0, indent - 1)
        elif kind == 'SEMI':
            flush()          # semicolon ends a statement
        else:
            buf.append((kind, val))

    flush()
    return '\n'.join(lines)

# ---------------------------------------------------------------------------
# Template parser  (splits into TEXT / ECHO / CODE segments)
# ---------------------------------------------------------------------------

def _parse_template(content):
    """Split PHP template content into ``(kind, value)`` segment tuples.

    Kinds:

    * ``TEXT`` – literal text output as-is
    * ``ECHO`` – ``<?= expr ?>`` expression evaluated and written
    * ``CODE`` – ``<?php ... ?>`` block executed as code
    """
    segments = []
    pos = 0
    length = len(content)

    while pos < length:
        echo_idx = content.find('<?=',  pos)
        code_idx = content.find('<?php', pos)

        if echo_idx == -1 and code_idx == -1:
            segments.append(('TEXT', content[pos:]))
            break

        if echo_idx == -1:
            next_idx, tag = code_idx, 'CODE'
        elif code_idx == -1:
            next_idx, tag = echo_idx, 'ECHO'
        elif echo_idx < code_idx:
            next_idx, tag = echo_idx, 'ECHO'
        else:
            next_idx, tag = code_idx, 'CODE'

        if next_idx > pos:
            segments.append(('TEXT', content[pos:next_idx]))

        close = content.find('?>', next_idx)
        if close == -1:
            close = length

        if tag == 'ECHO':
            value = content[next_idx + 3:close]   # skip '<?='
        else:
            value = content[next_idx + 5:close]   # skip '<?php'

        segments.append((tag, value))
        pos = close + 2   # skip '?>'

    return segments

# ---------------------------------------------------------------------------
# Foreach pattern matchers
# ---------------------------------------------------------------------------

def _match_foreach_colon(code):
    """Return ``(arr_expr, key_var, val_var)`` if *code* is a colon-style
    foreach header, else ``None``.  *key_var* is ``None`` for single-variable
    iteration.
    """
    code = code.strip()
    m = re.match(
        r'^foreach\s*\(\s*(.+?)\s+as\s+\$(\w+)\s*=>\s*\$(\w+)\s*\)\s*:?\s*$',
        code,
    )
    if m:
        return m.group(1), m.group(2), m.group(3)
    m = re.match(
        r'^foreach\s*\(\s*(.+?)\s+as\s+\$(\w+)\s*\)\s*:?\s*$',
        code,
    )
    if m:
        return m.group(1), None, m.group(2)
    return None


def _match_if_colon(code):
    """Return the condition expression if *code* is a colon-style ``if``."""
    m = re.match(r'^if\s*\(\s*(.+)\s*\)\s*:\s*$', code.strip())
    return m.group(1) if m else None


def _match_elseif_colon(code):
    """Return the condition expression if *code* is a colon-style ``elseif``."""
    m = re.match(r'^elseif\s*\(\s*(.+)\s*\)\s*:\s*$', code.strip())
    return m.group(1) if m else None


def _is_else_colon(code):
    return re.match(r'^else\s*:\s*$', code.strip()) is not None


def _is_endforeach(code):
    return re.match(r'^endforeach\s*;?\s*$', code.strip()) is not None


def _is_endif(code):
    return re.match(r'^endif\s*;?\s*$', code.strip()) is not None


# ---------------------------------------------------------------------------
# Template engine
# ---------------------------------------------------------------------------

class TemplateEngine:
    """Renders PHP template files, accumulating output in ``self.output``."""

    def __init__(self, base_dir, argv):
        self.base_dir = base_dir
        self.output = []
        # Shared mutable context – changes persist across all segments/includes
        self.ctx = dict(_BUILTINS)
        # Make $argv[0] = pyphp (script), $argv[1] = first user arg (xml file), etc.
        # argv[0] = pyphp.py path, argv[1] = template.php, argv[2] = xml, ...
        # The template uses $argv[1] for the xml file, so we skip the template arg.
        self.ctx['_argv'] = [argv[0]] + list(argv[2:]) if len(argv) > 2 else list(argv)
        self.ctx['_write'] = self._write

    # ── public API ──────────────────────────────────────────────────────────

    def _write(self, text):
        self.output.append(str(text) if text is not None else '')

    def render(self, template_file):
        """Render *template_file*, appending all output to ``self.output``."""
        with open(template_file, encoding='utf-8') as fh:
            content = fh.read()
        segments = _parse_template(content)
        self._exec_segments(segments)

    # ── segment executor ────────────────────────────────────────────────────

    def _exec_segments(self, segments):
        i = 0
        while i < len(segments):
            kind, value = segments[i]

            if kind == 'TEXT':
                self._write(value)

            elif kind == 'ECHO':
                py_expr = _translate_expr(value.strip())
                try:
                    result = eval(py_expr, self.ctx)
                except Exception as exc:
                    raise RuntimeError(
                        f'Error evaluating ECHO expression:\n  PHP: {value.strip()}\n'
                        f'  Python: {py_expr}'
                    ) from exc
                self._write('' if result is None else str(result))

            elif kind == 'CODE':
                code = value.strip()

                # ── endforeach / endif (orphan – already consumed above) ──
                if _is_endforeach(code) or _is_endif(code) or _is_else_colon(code) or _match_elseif_colon(code):
                    i += 1
                    continue

                # ── foreach colon-style (template loop) ───────────────────
                fc = _match_foreach_colon(code)
                if fc is not None:
                    arr_expr, kvar, vvar = fc
                    body, i = self._collect_body(segments, i + 1)
                    arr_py = _translate_expr(arr_expr)
                    try:
                        arr = eval(arr_py, self.ctx)
                    except Exception as exc:
                        raise RuntimeError(
                            f'Error evaluating foreach expression:\n  PHP: {arr_expr}\n'
                            f'  Python: {arr_py}'
                        ) from exc
                    if kvar is not None:
                        if isinstance(arr, dict):
                            arr = arr.items()
                        for key_item, val_item in arr:
                            self.ctx[f'_{kvar}'] = key_item
                            self.ctx[f'_{vvar}'] = val_item
                            self._exec_segments(body)
                    else:
                        for val_item in arr:
                            self.ctx[f'_{vvar}'] = val_item
                            self._exec_segments(body)
                    i += 1  # skip past the endforeach segment
                    continue

                # ── if colon-style (template conditional) ─────────────────
                cond_expr = _match_if_colon(code)
                if cond_expr is not None:
                    branches, i = self._collect_if_branches(segments, i + 1)
                    cond_py = _translate_expr(cond_expr)
                    try:
                        cond_val = eval(cond_py, self.ctx)
                    except Exception as exc:
                        raise RuntimeError(
                            f'Error evaluating if condition:\n  PHP: {cond_expr}\n'
                            f'  Python: {cond_py}'
                        ) from exc
                    # branches = [(cond_py|None, body_segments), ...]
                    # First branch has cond_val already evaluated
                    executed = False
                    for b_cond, b_body in branches:
                        if b_cond is None:
                            # else branch
                            if not executed:
                                self._exec_segments(b_body)
                            break
                        if b_cond == '__first__':
                            if cond_val:
                                self._exec_segments(b_body)
                                executed = True
                        else:
                            if not executed:
                                try:
                                    b_val = eval(b_cond, self.ctx)
                                except Exception:
                                    b_val = False
                                if b_val:
                                    self._exec_segments(b_body)
                                    executed = True
                    i += 1  # skip past endif
                    continue

                # ── everything else ───────────────────────────────────────
                self._exec_code(code)

            i += 1

    def _collect_body(self, segments, start):
        """Collect segments from *start* up to (but not including) the
        matching ``endforeach``/``endif``.

        Returns ``(body_segments, end_index)`` where *end_index* points to
        the closing CODE segment.
        """
        body = []
        depth = 1
        j = start
        while j < len(segments):
            kind, value = segments[j]
            if kind == 'CODE':
                code = value.strip()
                if _is_endforeach(code) or _is_endif(code):
                    depth -= 1
                    if depth == 0:
                        return body, j
                elif _match_foreach_colon(code) or _match_if_colon(code):
                    depth += 1
                elif depth == 1 and (_is_else_colon(code) or _match_elseif_colon(code)):
                    # These belong to the outer if, stop collecting for current branch
                    # Return current body; caller handles else/elseif
                    return body, j
            body.append(segments[j])
            j += 1
        return body, j

    def _collect_if_branches(self, segments, start):
        """Collect all branches of a colon-style if/elseif/else/endif block.

        Returns ``(branches, end_index)`` where *branches* is a list of
        ``('__first__'|cond_py|None, body_segments)`` tuples.
        """
        branches = []
        j = start
        current_cond = '__first__'
        current_body = []
        depth = 1

        while j < len(segments):
            kind, value = segments[j]
            if kind == 'CODE':
                code = value.strip()
                if _is_endif(code):
                    depth -= 1
                    if depth == 0:
                        branches.append((current_cond, current_body))
                        return branches, j
                    current_body.append(segments[j])
                elif _match_if_colon(code):
                    depth += 1
                    current_body.append(segments[j])
                elif _match_foreach_colon(code):
                    depth += 1
                    current_body.append(segments[j])
                elif _is_endforeach(code):
                    depth -= 1
                    current_body.append(segments[j])
                elif depth == 1 and _is_else_colon(code):
                    branches.append((current_cond, current_body))
                    current_cond = None   # else branch
                    current_body = []
                elif depth == 1:
                    m = _match_elseif_colon(code)
                    if m:
                        branches.append((current_cond, current_body))
                        current_cond = _translate_expr(m)
                        current_body = []
                    else:
                        current_body.append(segments[j])
                else:
                    current_body.append(segments[j])
            else:
                current_body.append(segments[j])
            j += 1

        branches.append((current_cond, current_body))
        return branches, j

    def _exec_code(self, code):
        """Execute a single PHP code block (already stripped of ``<?php``/``?>``)."""
        code = code.strip()

        # ── require "model.py"  →  no-op (Model is already imported) ───────
        if re.match(r'^require\s+"[^"]+\.py"\s*;?\s*$', code):
            return

        # ── require "templates/foo.php"  →  render the included file ────────
        m = re.match(r'^require\s+"([^"]+)"\s*;?\s*$', code)
        if m:
            include_file = os.path.join(self.base_dir, m.group(1))
            old_dir = self.base_dir
            self.base_dir = os.path.dirname(os.path.abspath(include_file))
            self.render(include_file)
            self.base_dir = old_dir
            return

        # ── $var = new ClassName(args)  →  constructor call ──────────────────
        m = re.match(
            r'^\$(\w+)\s*=\s*new\s+(\w+)\s*\((.*?)?\)\s*;?\s*$', code, re.DOTALL
        )
        if m:
            var, cls, args_php = m.group(1), m.group(2), m.group(3)
            args_py = _translate_expr(args_php)
            self.ctx[f'_{var}'] = eval(f'{cls}({args_py})', self.ctx)
            return

        # ── $var = expr  →  simple assignment ───────────────────────────────
        m = re.match(r'^\$(\w+)\s*=\s*(.+?)\s*;?\s*$', code, re.DOTALL)
        if m:
            var, val_php = m.group(1), m.group(2)
            self.ctx[f'_{var}'] = eval(_translate_expr(val_php), self.ctx)
            return

        # ── general block (brace-style, multi-statement, etc.) ──────────────
        py_code = _translate_php_block(code)
        write_fn = self._write

        def _echo(*args, end=''):
            for a in args:
                write_fn(str(a))

        self.ctx['echo'] = _echo
        try:
            exec(compile(py_code, '<php-block>', 'exec'), self.ctx)
        except Exception as exc:
            raise RuntimeError(
                f'Error executing PHP block:\n{code}\n\n'
                f'Translated Python:\n{py_code}'
            ) from exc

# ---------------------------------------------------------------------------
# CLI entry point
# ---------------------------------------------------------------------------

def main(argv=None):
    if argv is None:
        argv = sys.argv
    if len(argv) < 3:
        print(
            f'Usage: python3 -m pyphp.pyphp <template.php> <source.xml>',
            file=sys.stderr,
        )
        sys.exit(1)

    template_file = argv[1]
    # Use the current working directory as the base for include resolution so
    # that paths like "templates/export_enums.php" resolve correctly when
    # pyphp is invoked from the tools/ directory.
    engine = TemplateEngine(
        base_dir=os.getcwd(),
        argv=argv,
    )
    engine.render(template_file)
    sys.stdout.write(''.join(engine.output))


if __name__ == '__main__':
    main(sys.argv)
