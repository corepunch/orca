# pyphp Developer Feedback

Issues discovered while writing `templates/dtd.php` and `templates/docs.php`.
Each item lists the PHP that fails, the symptom in pyphp, and the workaround used.

---

## 1. `??` null-coalescing fails inside function calls and after `->` access

**Failing PHP:**
```php
$parent = strval($comp->_elem["parent"] ?? "");
$comp   = $globalComponents[$compName] ?? null;
```

**Symptom:** `SyntaxError: invalid syntax` on the line.  
pyphp emits the raw `??` into the Python output when the LHS is a property/subscript chain,
producing invalid Python.

**Workaround:**
```php
$parentAttr = $comp->_elem["parent"];
$parent     = $parentAttr !== null ? strval($parentAttr) : "";

$comp = isset($globalComponents[$compName]) ? $globalComponents[$compName] : null;
```

---

## 2. `(string)` type cast is not supported

**Failing PHP:**
```php
$name = (string)$elem["name"];
```

**Symptom:** `SyntaxError` — pyphp does not recognise `(string)` as a cast.

**Workaround:** Use `strval()`:
```php
$name = strval($elem["name"]);
```

---

## 3. `array_keys()` returns integer indices for `PhpArray` with string keys

**Failing PHP:**
```php
$map = [];
$map["Node2D"] = $comp;
$keys = array_keys($map);   // returns [0, 1, 2, …] not ["Node2D", …]
```

**Symptom:** `_array_keys` checks `isinstance(arr, dict)` to decide whether to return
dict keys or integer indices.  `PhpArray` inherits from `list`, so string keys stored
in `PhpArray._map` are silently ignored and integer indices are returned instead.

**Workaround:** Iterate with `foreach`:
```php
foreach ($map as $k => $v) { /* $k is the string key */ }
```

**Suggested fix:** In `_array_keys`, check for `PhpArray` explicitly and merge
list indices with `._map` keys, in insertion order.

---

## 4. `strpos()` returns `-1` (Python) instead of `false` (PHP) when not found

**Failing PHP:**
```php
if (strpos($name, "2D") !== false) { ... }
```

**Symptom:** Always evaluates as true because Python's `str.find()` returns `-1` for
"not found", and `-1 !== False` is `True` in Python (`-1 != 0`).

**Workaround:** Use `str_contains()`:
```php
if (str_contains($name, "2D")) { ... }
```

**Suggested fix:** `_strpos` should return `False` (not `-1`) when the needle is not
found, matching PHP semantics:
```python
def _strpos(s, needle, offset=0):
    r = str(s).find(str(needle), offset)
    return r if r >= 0 else False
```

---

## 5. `array_map(fn($p) => $a . $b . $p[0], $arr)` — concatenation in arrow body generates wrong Python

**Failing PHP:**
```php
$parts = array_map(fn($p) => $prefix . "." . $p[0], $attribs);
```

**Symptom:** `NameError: name '__p' is not defined`.  
pyphp wraps the `_cat(...)` call around the lambda instead of placing it inside:
```python
# Wrong output:
array_map(_cat(lambda __p: __prefix, ".", __p[0]), __attribs)
# Correct output:
array_map(lambda __p: _cat(__prefix, ".", __p[0]), __attribs)
```

**Workaround:** Use an explicit `foreach` loop:
```php
$parts = [];
foreach ($attribs as $p) {
    $parts[] = $prefix . "." . $p[0];
}
```

**Suggested fix:** The `_cat()` wrapper for string concatenation should be applied
*inside* the lambda body, not around it.

---

## 6. `iterator_to_array()` is not available

**Failing PHP:**
```php
$fields = iterator_to_array($struct->getFields());
```

**Symptom:** `NameError: name 'iterator_to_array' is not defined`.

**Workaround:** Collect into an array with `foreach`:
```php
$fields = [];
foreach ($struct->getFields() as $k => $v) { $fields[$k] = $v; }
```

**Suggested fix:** Add `iterator_to_array` to builtins:
```python
'iterator_to_array': lambda it, preserve_keys=True: dict(it) if preserve_keys else list(v for _, v in it),
```

---

## 7. Template file without a closing `?>` is output as literal text

**Failing PHP template** (`file.php`):
```php
<?php require "model/module.php"; ?>
<?php
// entire script body, no closing ?>
echo "hello\n";
```

**Symptom:** The second `<?php` block and everything after it is emitted verbatim as
output text instead of being executed.

**Root cause:** The pyphp parser requires a matching `?>` to close each PHP block.
Without it, the block is treated as literal content.

**Workaround:** Always close the final PHP block with `?>`:
```php
<?php
echo "hello\n";
?>
```

---

## 8. Octal integer literals (`0755`, `0777`) cause a Python 3 SyntaxError

**Failing PHP:**
```php
mkdir($dir, 0755, true);
```

**Symptom:** `SyntaxError: leading zeros in decimal integer literals are not permitted;
use an 0o prefix for octal integers`.

**Workaround:** Use decimal equivalents (`511` = `0777`, `493` = `0755`) or
convert manually to `0o` prefix before passing to pyphp.

**Suggested fix:** In the preprocessor, replace bare octal PHP literals
`/\b0[0-7]+\b/` with their Python `0o…` equivalents.

---

## 9. `else if` (two words) is not recognised — must use `elseif`

**Failing PHP:**
```php
} else if ($x) {
```

**Symptom:** `SyntaxError: expected ':'`.

**Workaround:** Use `elseif` (one word) which PHP also accepts:
```php
} elseif ($x) {
```

**Note:** Both forms are valid PHP; pyphp only handles `elseif`.

---

## 10. Pass-by-reference parameters (`&$param`) are not supported

**Failing PHP:**
```php
function collect($type_, $path, &$result) {
    $result[] = [$path, $type_];
}
```

**Symptom:** `SyntaxError: invalid syntax` on the function signature line.

**Workaround:** Return the value instead:
```php
function collect($type_, $path) {
    return [[$path, $type_]];
}
$result = array_merge($result, collect($type_, $path));
```

---

## Summary table

| # | PHP construct | Status | Workaround |
|---|--------------|--------|------------|
| 1 | `$x ?? default` after `->` / `[]` | ❌ broken | two-step ternary |
| 2 | `(string)$x` cast | ❌ broken | `strval($x)` |
| 3 | `array_keys()` on string-keyed PhpArray | ❌ broken | `foreach` |
| 4 | `strpos() !== false` | ❌ broken | `str_contains()` |
| 5 | `array_map(fn => concat)` | ❌ broken | `foreach` loop |
| 6 | `iterator_to_array()` | ❌ missing | `foreach` collect |
| 7 | Template without closing `?>` | ❌ silent bug | always add `?>` |
| 8 | Octal literals `0755` | ❌ broken | decimal equivalent |
| 9 | `else if` two words | ❌ not supported | `elseif` |
| 10 | Pass-by-reference `&$param` | ❌ not supported | return value |
