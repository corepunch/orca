# CSS Binding System Knowledge Base

## Overview

ORCA uses a CSS-influenced styling system that converts CSS values with units (like `1.5rem`) into property bindings (like `{../Font.Size} * 1.5`).

---

## Binding Syntax

### Basic Syntax
- `{../PropertyName}` - binding with parent path traversal (uses FullIdentifier hash)
- `{../TextRun.FontSize} * 1.5` - binding with arithmetic expression
- `.PropertyName` - relative to current object
- `..` - parent traversal
- `{#ChildName}` - named child lookup
- `{DataContext/...}` - DataContext traversal

### Parsing
**File:** `source/core/property/property_program.c` (lines 145-217)

The `{}` syntax is tokenized by the property program tokenizer. The `tok_argument` operator (line 624 in `property_runtime.c`) resolves paths at runtime.

---

## CSS Value Processing Pipeline

### Entry Point
**File:** `plugins/UIKit/Css.c`

1. `UIKit_LoadObjectFromCssString()` (line 1336) - parses CSS string
2. `css_strip_comments()` - removes block comments
3. `css_parse_text()` - parses into `css_doc_t` structure
4. `css_resolve_apply()` - resolves `@apply` directives
5. `css_apply_decl_to_rule()` (line 1017) - applies declarations:
   - Maps CSS property names via `k_css_prop_map[]` (line 206)
   - Normalizes values via `css_normalize_decl_value()` (line 376)
   - For units: calls `css_apply_orca_value_to_rule()` (line 951)

---

## CSS Unit Handling

### Key Functions (in `plugins/UIKit/Css.c`)

| Function | Line | Purpose |
|----------|------|---------|
| `css_value_has_unit()` | 94 | Detects if value has rem/em (creates bindings) |
| `css_is_viewport_unit()` | 147 | Detects vw/vh/vmin/vmax units |
| `css_strip_unit()` | 116 | Extracts numeric portion (e.g., "1.5rem" -> "1.5") |
| `css_get_binding_path()` | 135 | Maps ORCA property to binding path |
| `css_make_binding_expr()` | 165 | Creates binding expression string |

### Current Unit Behavior

| Unit | Behavior |
|------|----------|
| `rem`, `em` | Creates binding to parent's FontSize via `{../TextRun.FontSize}` |
| `vh`, `vw`, `vmin`, `vmax` | Returns FALSE (not yet supported - needs Screen.Width/Height access) |
| `px`, `pt` | Treated as literal values |

### Unit to Binding Conversion

**Function:** `css_make_binding_expr()` (Css.c:165-193)

```c
// Example: "1.5rem" + "TextRun.FontSize" -> "{../TextRun.FontSize} * 1.5"
// Example: "1rem" + "TextRun.FontSize" -> "{../TextRun.FontSize}" (multiplier is 1.0)
```

**Applied in:** `css_apply_orca_value_to_rule()` (Css.c:951-1013)

```c
if (css_value_has_unit(css_value)) {
    char binding_expr[CSS_MAX_VALLEN] = {0};
    if (css_make_binding_expr(css_value, orca_name, binding_expr, sizeof(binding_expr))) {
        PROP_SetBinding(prop, binding_expr, kBindingModeOneWay, TRUE);
        return;
    }
    // Fall through for unsupported units (px, pt, vw, vh)
}
```

---

## Adding vh/vw Support

**Current limitation (Css.c:170-173):**
```c
// Don't create bindings for viewport units (vw, vh, vmin, vmax) yet
// They require Screen.Width/Height which isn't accessible via parent traversal
if (css_is_viewport_unit(css_value)) {
    return FALSE;
}
```

**Viewport units (vh, vw, vmin, vmax):**
```c
// Css.c css_make_binding_expr() - viewport unit handling
if (css_is_viewport_unit(css_value)) {
  // vw → Screen.Width, vh → Screen.Height
  // vmin → Screen.Height (approx), vmax → Screen.Width (approx)
  // vw/vh are percentages: 50vw → {Screen.Width} * 0.5
}
```

**Key insight:** Viewport bindings use bare paths (`{Screen.Width}`) which resolve via `tok_argument`'s `_BindingTemplateOrRoot` fallback to the root object. No `##Root/` prefix needed.

---

## Key Files

| File | Purpose |
|------|---------|
| `plugins/UIKit/Css.c` | CSS parsing, unit detection, binding expression creation |
| `source/core/components/StyleController.c` | Style resolution, CSS unit binding transfer |
| `source/core/property/property_program.c` | Tokenizer/lexer for `{}` binding expressions |
| `source/core/property/property_runtime.c` | Binding expression evaluation (tok_argument) |
| `source/core/property/property_events.c` | `PROP_SetBinding()` API |
| `source/core/object/object_hierarchy.c` | `OBJ_FindPropertyByPath()` for `..` and `.` traversal |

---

## Test Coverage

**Lua tests:** `tests/test_styles_lua.lua`
- `test_css_unit_rem_binding` - Tests `1.5rem` → `{../TextRun.FontSize} * 1.5` binding resolution
- `test_css_unit_em_binding` - Tests `2em` → `{../TextRun.FontSize} * 2` binding resolution
- `test_css_unit_px_literal` - Tests `14px` stored as literal 14 (no binding)
- `test_css_unit_no_binding_on_non_inherited` - Tests `100px` for width
- `test_css_unit_binding_updates_on_parent_change` - Tests binding re-evaluation after parent FontSize change
- `test_css_unit_vh_binding` - Tests `50vh` → `{Screen.Height} * 0.5` viewport binding
- `test_css_unit_vw_binding` - Tests `25vw` → `{Screen.Width} * 0.25` viewport binding
- `test_css_unit_vh_full` - Tests `100vh` → `{Screen.Height}` viewport binding

**C tests:** `tests/test_styles.c`
- `test_css_unit_binding_inherits_from_parent` - Tests binding `{../Width} * 1.5` resolution via Token_Create/OBJ_RunProgram
- `test_css_unit_binding_expression_generation` - Placeholder (tested via Lua tests)

---

## CSS Property Mapping

**Location:** `k_css_prop_map[]` in Css.c:206

Maps CSS property names (e.g., `font-size`) to ORCA properties (e.g., `TextRun.FontSize`).

---

## Important Constants

- `CSS_MAX_VALLEN` - maximum CSS value length
- `MAX_DEPTH` (16 in codegen) - maximum property path depth
