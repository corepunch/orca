#ifndef ORCA_CODEGEN_EXPORT_BACKEND_H
#define ORCA_CODEGEN_EXPORT_BACKEND_H

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdint.h>
#include <stdio.h>

#define OUT(...) fprintf(out, __VA_ARGS__)
#define LINE(...) do { fprintf(out, __VA_ARGS__); fputc('\n', out); } while (0)
#define EACH_ELEMENT(var, parent, tag) \
    for (xmlNodePtr var = (parent) ? (parent)->children : NULL; var; var = var->next) \
        if (var->type == XML_ELEMENT_NODE && ((tag) == NULL || xmlStrEqual(var->name, BAD_CAST(tag))))

typedef struct {
    const char *key;
    const char *value;
} kv_t;

typedef struct module_t {
    char *path;
    xmlDocPtr doc;
    xmlNodePtr root;
    struct module_t **requires;
    size_t require_count;
} module_t;

typedef enum {
    KIND_FLOAT, KIND_INT, KIND_UINT, KIND_LONG, KIND_BOOL, KIND_STRING,
    KIND_ENUM, KIND_STRUCT, KIND_INTERFACE, KIND_COMPONENT, KIND_EXTERNAL_STRUCT,
    KIND_NRESULTS, KIND_VOID, KIND_UNKNOWN
} kind_t;

typedef struct {
    const char *kind_name;
    const char *decl;
    const char *check;
    const char *push;
} type_rule_t;

void die(const char *msg);
char *dupstr(const char *s);
char *xml_prop_dup(xmlNodePtr n, const char *name);
int xml_prop_true(xmlNodePtr n, const char *name);
void emit_template(FILE *out, const char *tmpl, const kv_t *kvs, size_t kv_count);
xmlNodePtr find_child(xmlNodePtr parent, const char *name);
xmlNodePtr find_named(xmlNodePtr parent, const char *container, const char *tag, const char *name);
int has_child(xmlNodePtr parent, const char *name);
module_t *load_module(const char *path, char ***seen, size_t *seen_count);
kind_t resolve_kind(module_t *m, const char *type);
const type_rule_t *rule_for(kind_t kind);
void subst2(char *dst, size_t cap, const char *tmpl, const char *type, const char *arg);
char *lcfirst_dup(const char *s);
uint32_t fnv1a32(const char *s);
char *prop_token(const char *path);
const char *dtype_for(kind_t kind, const char *type);
char *export_type_name(module_t *m, const char *type, kind_t kind);
void free_seen(char **seen, size_t seen_count);

int emit_target_c(FILE *out, module_t *m, const char *xml_path);

#endif
