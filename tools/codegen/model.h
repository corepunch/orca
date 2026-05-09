#ifndef CODEGEN_MODEL_H
#define CODEGEN_MODEL_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ TypeKind */

typedef enum {
    TK_FLOAT, TK_INT, TK_UINT, TK_LONG, TK_BOOL, TK_STRING, TK_VOID,
    TK_LUA_STATE, TK_STRARRAY, TK_NRESULTS,
    TK_ENUM, TK_STRUCT, TK_INTERFACE, TK_COMPONENT, TK_EXTERNAL_STRUCT,
    TK_UNKNOWN
} TypeKind;

/* ------------------------------------------------------------------ Forward declarations */

struct Module_;
struct Struct_;
struct Enum_;
struct Interface_;
struct Component_;

/* ------------------------------------------------------------------ Type */

typedef struct {
    char       *type_name;   /* value of type="..." attr */
    TypeKind    kind;
    void       *data;        /* ptr to Enum_/Struct_/Interface_/Component_ or NULL */
    int         fixed_array; /* 0 = none */
    int         is_array;    /* array="true" */
    int         is_const;
    int         is_pointer;
    char       *export_name; /* export attr or type_name for structs */
    char       *default_val;
    char       *output_attr;
    char       *bufsize_attr;
} Type;

/* ------------------------------------------------------------------ FieldName / PropertyName */

typedef struct {
    char *name;  /* raw name */
    char *id;    /* "0x" + fnv1a32(name) */
    char *addr;  /* == name */
} FieldName;

typedef struct {
    char *classname;
    char **path;     /* array of path segments */
    int   path_len;
    char *addr;      /* join(path, '.') */
    char *display;   /* __toString() result: ucfirst+axis_rename+strip */
    char *id;        /* "0x" + fnv1a32(display) */
} PropertyName;

/* ------------------------------------------------------------------ Field / Property */

typedef struct {
    FieldName name;
    Type      type;
    char     *doc;
    int       noexport;
} Field;

typedef struct {
    PropertyName name;
    Type         type;
    char        *doc;
} Property;

/* ------------------------------------------------------------------ Arg */

typedef struct {
    char *param_name;
    Type  param_type;
} Arg;

/* ------------------------------------------------------------------ Method */

typedef struct {
    char *name;
    char *full_name;  /* prefix+name */
    char *export_name;/* export attr or lcfirst(name) */
    char *id;         /* "0x" + fnv1a32(lcfirst(name)) */
    char *doc;
    char *alias;      /* alias attr if present */
    Arg  *args;
    int   num_args;
    Type *returns;    /* NULL = void */
    int   is_static;
    int   is_lua;     /* lua="true" */
    int   is_const;
} Method;

/* ------------------------------------------------------------------ Enum */

typedef struct {
    char *name;
    char *doc;
} EnumValue;

typedef struct Enum_ {
    char      *name;
    char      *doc;
    EnumValue *values;
    int        num_values;
} Enum;

/* ------------------------------------------------------------------ Event */

typedef struct {
    char  *name;
    char  *ns;          /* msgns — interface/component name */
    char  *parent_name; /* same-as attr value, or NULL */
    Field *fields;
    int    num_fields;
    char  *doc;
} Event;

/* ------------------------------------------------------------------ ExternalStruct */

typedef struct {
    char *name;
} ExternalStruct;

/* ------------------------------------------------------------------ Interface */

typedef struct {
    char   *name;
    char   *prefix;
    char   *export_name;
    char   *no_check;
    char   *doc;
    Method *methods;
    int     num_methods;
    /* messages stored in Module.events keyed as "NS.Name" */
} Interface;

/* ------------------------------------------------------------------ Struct */

typedef struct Struct_ {
    char   *name;
    char   *prefix;
    char   *export_name;
    char   *doc;
    int     sealed;
    Field  *fields;
    int     num_fields;
    Method *methods;
    int     num_methods;
    struct Module_ *module; /* back-pointer for type resolution */
} Struct;

/* ------------------------------------------------------------------ Component */

typedef struct Component_ {
    char     *name;
    char     *prefix;
    char     *export_name;
    char     *doc;
    int       sealed;
    int       attach_only;
    /* parents: array of names */
    char    **parents;
    int       num_parents;
    /* properties (raw, non-recursive) */
    Property *properties;
    int       num_properties;
    /* fields */
    Field    *fields;
    int       num_fields;
    /* methods */
    Method   *methods;
    int       num_methods;
    /* handles: "NS.Event" strings */
    char    **handles;
    int       num_handles;
    /* messages: Event* array */
    Event   **messages;
    int       num_messages;
    struct Module_ *module;
} Component;

/* ------------------------------------------------------------------ Include */

typedef struct {
    char *file;
} Include;

/* ------------------------------------------------------------------ Module */

typedef struct Module_ {
    char          *name;
    char          *prefix;      /* module-level prefix (for globals) */
    char          *ns;          /* namespace attr */
    char          *on_luaopen;
    char          *after_luaopen;
    char          *source;      /* xml file path (for require resolution) */

    Include       *includes;
    int            num_includes;

    ExternalStruct *externals;
    int             num_externals;

    Enum           *enums;
    int             num_enums;

    Struct         *structs;
    int             num_structs;

    Interface      *interfaces;
    int             num_interfaces;

    Component      *components;
    int             num_components;

    /* all events from interfaces + components, ordered */
    Event         **events;
    int             num_events;

    /* functions (global) */
    Method         *functions;
    int             num_functions;

    /* required modules */
    struct Module_ **requires;
    int              num_requires;
} Module;

/* ------------------------------------------------------------------ API */

uint32_t    fnv1a32(const char *s);
char       *fnv1a32hex(const char *s);     /* "0xABCDEF12" (malloc'd) */
char       *axis_rename(const char *in);   /* apply axis patterns + strip (malloc'd) */
char       *str_ucfirst(const char *s);    /* malloc'd copy with first char uppercased */
char       *str_lcfirst(const char *s);    /* malloc'd copy with first char lowercased */
char       *str_toupper(const char *s);    /* malloc'd UPPERCASE copy */
char       *str_dup(const char *s);        /* strdup wrapper (NULL-safe) */

/* Property name construction (malloc'd strings inside PropertyName) */
PropertyName make_property_name(const char *classname, const char **path, int path_len);
void         free_property_name(PropertyName *pn);

/* Type string representation: C declaration string (malloc'd) */
char       *type_decl(const Type *t);
/* Emit push/check/default/format from TypeInfos — result malloc'd */
char       *type_get(const Type *t, const char *key, const char *arg, const char *addr);

/* Load module from XML file (and recursively loads requires) */
Module     *load_module(const char *xml_file, const char *source_override);

/* Type resolution */
TypeKind    resolve_kind(Module *m, const char *type_name, void **data_out);

/* Walk properties: recursive expansion (result array, caller must free) */
Property   *walk_properties(const char *classname, const char *prop_attr, Type *base_type,
                             const char *doc, Module *m, int *out_count);

/* Emitters */
void emit_header(Module *m, FILE *out);
void emit_props(Module *m, FILE *out);
void emit_export(Module *m, FILE *out);

/* Event helpers (implemented in model.c) */
int    event_has_fields(Event *ev);
int    event_has_any_fields(Event *ev, Module *m);
char  *event_get_effective_struct_name(Event *ev, Module *m);
Event *module_find_event(Module *m, const char *name);

#endif /* CODEGEN_MODEL_H */
