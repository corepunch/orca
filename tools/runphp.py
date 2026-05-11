#!/usr/bin/env python3
"""
runphp.py — PHP template runner that replaces pyphp.

Preprocesses PHP templates so they behave like pyphp:
  1. Strip leading whitespace from PHP-only lines (<?php ... ?>),
     matching pyphp's behaviour of discarding line-leading whitespace
     on lines that contain only a PHP block.
  2. Add an extra newline after <?= ... ?> at end-of-line so that
     PHP's automatic newline-eating after ?> is compensated.

Usage:
    python3 runphp.py <template.php> <input.xml>

Output goes to stdout (same as pyphp).
"""

import re
import sys
import os
import subprocess
import tempfile
import shutil

# Match <?= expr ?> where the content contains no ?> itself,
# and the closing ?> is immediately followed by a newline.
# We insert an extra newline so PHP eats one and outputs one.
_EXPR_AT_EOL = re.compile(r'(<\?=(?:[^?]|\?(?!>))*\?>)(\n)')

# Match a line that starts with optional whitespace then <?php (not <?=).
# PHP outputs the leading whitespace as literal text; pyphp discards it.
_PHP_ONLY_LINE = re.compile(r'^[ \t]+(<\?php(?!=))', re.MULTILINE)


def preprocess(src: str) -> str:
    src = _PHP_ONLY_LINE.sub(r'\1', src)
    src = _EXPR_AT_EOL.sub(r'\1\n\2', src)
    return src


def run(template: str, xml_file: str) -> None:
    tools_dir = os.path.dirname(os.path.abspath(template))
    # Walk up to find the tools/ root (directory containing model/ and templates/)
    tools_root = os.path.dirname(os.path.abspath(__file__))

    tmpdir = tempfile.mkdtemp(prefix='runphp_')
    try:
        # Copy and preprocess every .php file under tools_root into tmpdir
        for dirpath, _dirs, files in os.walk(tools_root):
            for fn in files:
                if not fn.endswith('.php'):
                    continue
                src_path = os.path.join(dirpath, fn)
                rel_path = os.path.relpath(src_path, tools_root)
                dst_path = os.path.join(tmpdir, rel_path)
                os.makedirs(os.path.dirname(dst_path), exist_ok=True)
                with open(src_path, encoding='utf-8') as f:
                    src = f.read()
                with open(dst_path, 'w', encoding='utf-8') as f:
                    f.write(preprocess(src))

        # Build path to the template inside tmpdir
        rel_tpl = os.path.relpath(os.path.abspath(template), tools_root)
        tpl_in_tmp = os.path.join(tmpdir, rel_tpl)

        result = subprocess.run(
            ['php', tpl_in_tmp, os.path.abspath(xml_file)],
            cwd=tmpdir,
        )
        sys.exit(result.returncode)
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f'Usage: {sys.argv[0]} <template.php> <input.xml>', file=sys.stderr)
        sys.exit(1)
    run(sys.argv[1], sys.argv[2])
