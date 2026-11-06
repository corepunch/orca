# Codegen — Data Model

The model represents a parsed `.cgen` module as a **flat array of nodes**. Parent-child relationships use integer `id`/`parent` references, not pointers.

## `cg_node`

```c
typedef struct cg_node {
    uint32_t   id;      // 1-based sequential
    uint32_t   parent;  // parent node id (0 = module root)
    cg_kind    kind;
    uint32_t   flags;   // CG_FLAG_* bitmask
    char const *name;   // primary identifier (varies by kind)
    char const *type;   // C type / routing / prefix (varies by kind)
    char const *extra;  // export name / default / same-as (varies by kind)
    char const *aux;    // alias / routing / output (varies by kind)
    char const *aux2;   // details / xmlns / bufsize (varies by kind)
    char const *doc;    // <summary> text, or NULL
} cg_node;
```

## `cg_model` — Root container

```c
typedef struct cg_model {
    char const  *xml_path;
    char const  *module_name;
    char const  *module_ns;
    char const  *prefix;
    char const  *on_luaopen;
    char const  *after_luaopen;
    cg_node const *nodes;
    size_t         node_count;
} cg_model;
```

## `cg_kind` — Element types

| Kind | `name` | `type` | `extra` | `aux` | `aux2` |
|---|---|---|---|---|---|
| `MODULE` | module name | namespace | — | — | — |
| `REQUIRE` | — | file path | — | — | — |
| `INCLUDE` | — | header path | — | — | — |
| `EXTERNAL` | — | library name | — | — | — |
| `ENUM` | enum name | — | — | — | — |
| `VALUE` | value name | parent enum name | — | — | — |
| `INTERFACE` | interface name | — | — | — | — |
| `STRUCT` | struct name | — | — | — | — |
| `CLASS` | class name | — | — | — | — |
| `METHOD` | method name | return type | export name | lua binding name | output type |
| `MESSAGE` | message name | routing | — | output | bufsize |
| `FIELD` | field name | C type | default value | same-as | fixed-array size |
| `PROPERTY` | property name | C type | default value | inspector group | inspector label |
| `HANDLE` | handle name | — | — | — | — |
| `ARG` | arg name | C type | — | — | — |
| `RETURNS` | — | return type | — | — | — |
| `TOPIC` | topic name | routing | — | — | — |

## `CG_FLAG_*` — Boolean attributes

| Flag | Applies to | XML attribute |
|---|---|---|
| `CG_FLAG_SEALED` | struct | `sealed="true"` |
| `CG_FLAG_STATIC` | method | `static="true"` |
| `CG_FLAG_LUA` | method | `lua="true"` |
| `CG_FLAG_NOEXPORT` | field | `noexport="true"` |
| `CG_FLAG_POINTER` | field/property/arg | `pointer="true"` |
| `CG_FLAG_CONST` | field/method/arg | `const="true"` |
| `CG_FLAG_ARRAY` | property/field | `array="true"` or fixed-array |
| `CG_FLAG_NO_CHECK` | interface | `no-check="true"` |
| `CG_FLAG_PRIVATE` | property/method | `private="true"` |
| `CG_FLAG_INHERITED` | property | `inherited="true"` |

## Iteration helpers (`cg_api.h:82-107`)

- `cg_first(model, parent_id, kind)` — find first child of a node by kind
- `cg_next(node)` — find next sibling of the same kind
- `cg_node_by_id(model, id)` — lookup by 1-based id
- `cg_foreach(node, model, parent, kind)` — convenient for-loop macro
