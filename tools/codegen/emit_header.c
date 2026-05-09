/*
 * emit_header.c — generate *.h output mirroring tools/templates/header.php
 */
#include "model.h"
#include <ctype.h>

/* --- helper: get return type string (malloc'd) --- */
static char *method_return_type(Method *meth) {
    if (!meth->returns) return strdup("void");
    return type_decl(meth->returns);
}

/* --- helper: get arg types as comma-separated string (malloc'd) --- */
static char *method_args_types(Method *meth) {
    if (meth->num_args == 0) return strdup("void");
    char buf[4096] = "";
    for (int i = 0; i < meth->num_args; i++) {
        if (i > 0) strncat(buf, ", ", sizeof(buf)-strlen(buf)-1);
        char *td = type_decl(&meth->args[i].param_type);
        strncat(buf, td, sizeof(buf)-strlen(buf)-1);
        free(td);
    }
    return strdup(buf);
}

static void emit_struct_contents_fields(Field *fields, int n, FILE *out) {
    for (int i = 0; i < n; i++) {
        Field *f = &fields[i];
        char *td = type_decl(&f->type);
        fprintf(out, "\t%s %s", td, f->name.name);
        free(td);
        if (f->type.fixed_array) fprintf(out, "[%d]", f->type.fixed_array);
        fprintf(out, ";");
        if (f->doc) fprintf(out, " ///< %s", f->doc);
        fprintf(out, "\n");
    }
}

static void emit_struct_contents_props(Property *props, int n, FILE *out) {
    for (int i = 0; i < n; i++) {
        Property *p = &props[i];
        char *td = type_decl(&p->type);
        fprintf(out, "\t%s %s", td, p->name.display);
        free(td);
        if (p->type.fixed_array) fprintf(out, "[%d]", p->type.fixed_array);
        fprintf(out, ";");
        if (p->doc) fprintf(out, " ///< %s", p->doc);
        fprintf(out, "\n");
    }
}

void emit_header(Module *m, FILE *out) {
    char *upper = str_toupper(m->name);

    fprintf(out, "// Auto-generated from %s.xml by tools/templates/header.php\n", m->name);
    fprintf(out, "// DO NOT EDIT \xe2\x80\x94 run 'cd tools && make' to regenerate.\n");
    fprintf(out, "#ifndef __%s_H__\n", upper);
    fprintf(out, "#define __%s_H__\n", upper);
    fprintf(out, "\n");
    fprintf(out, "#include <stdbool.h>\n");
    fprintf(out, "#include <stdint.h>\n");
    fprintf(out, "\n");
    fprintf(out, "typedef struct lua_State lua_State;\n");
    fprintf(out, "\n");
    free(upper);

    /* external struct forwards */
    for (int i = 0; i < m->num_externals; i++)
        fprintf(out, "struct %s;\n", m->externals[i].name);

    fprintf(out, "\n");

    /* includes */
    for (int i = 0; i < m->num_includes; i++)
        fprintf(out, "#include <%s>\n", m->includes[i].file);

    fprintf(out, "\n");
    fprintf(out, "#include \"%s_properties.h\"\n", m->name);

    /* required module headers */
    for (int i = 0; i < m->num_requires; i++) {
        Module *req = m->requires[i];
        const char *src = req->source;
        size_t slen = strlen(src);
        if (slen > 4 && strcmp(src+slen-4,".xml")==0) {
            fprintf(out, "#include \"%.*s.h\"\n", (int)(slen-4), src);
        } else {
            fprintf(out, "#include \"%s.h\"\n", src);
        }
    }

    fprintf(out, "\n");

    /* events: typedef lines */
    for (int i = 0; i < m->num_events; i++) {
        Event *ev = m->events[i];
        if (event_has_any_fields(ev, m)) {
            char *sname = event_get_effective_struct_name(ev, m);
            fprintf(out, "typedef struct %s %s_%sMsg_t,* %s_%sMsgPtr;\n",
                    sname, ev->ns, ev->name, ev->ns, ev->name);
            free(sname);
        } else {
            fprintf(out, "typedef struct %s_%sEventArgs %s_%sMsg_t,* %s_%sMsgPtr;\n",
                    ev->ns, ev->name, ev->ns, ev->name, ev->ns, ev->name);
        }
    }

    /* enums */
    fprintf(out, "\n");
    for (int i = 0; i < m->num_enums; i++) {
        Enum *e = &m->enums[i];
        if (e->doc) fprintf(out, "\n/// @brief %s\n", e->doc);
        fprintf(out, "/** %s enum */\n", e->name);
        fprintf(out, "typedef enum %s {\n", e->name);
        for (int j = 0; j < e->num_values; j++) {
            fprintf(out, "\tk%s%s,", e->name, e->values[j].name);
            if (e->values[j].doc) fprintf(out, " ///< %s", e->values[j].doc);
            fprintf(out, "\n");
        }
        fprintf(out, "} e%s_t;\n", e->name);
        fprintf(out, "#define %s_Count %d\n", e->name, e->num_values);
        fprintf(out, "ORCA_API const char *%sToString(enum %s value);\n", e->name, e->name);
        fprintf(out, "ORCA_API enum %s luaX_check%s(lua_State *L, int idx);\n", e->name, e->name);
        fprintf(out, "ORCA_API void luaX_push%s(lua_State *L, enum %s value);\n", e->name, e->name);
    }

    /* global functions */
    fprintf(out, "\n");
    for (int i = 0; i < m->num_functions; i++) {
        Method *f = &m->functions[i];
        if (f->doc) fprintf(out, "\n/// @brief %s\n", f->doc);
        char *rt = method_return_type(f);
        char *at = method_args_types(f);
        fprintf(out, "ORCA_API %s\n%s(%s);\n", rt, f->full_name, at);
        free(rt); free(at);
    }

    /* interfaces */
    fprintf(out, "\n");
    for (int i = 0; i < m->num_interfaces; i++) {
        Interface *iface = &m->interfaces[i];
        for (int j = 0; j < iface->num_methods; j++) {
            Method *meth = &iface->methods[j];
            /* Check if this method has a topic (stored in topic_title field) */
            if (meth->topic_title) {
                fprintf(out, "\n/// @name %s\n", meth->topic_title);
                if (meth->topic_desc) fprintf(out, "/// %s\n", meth->topic_desc);
            }
            if (meth->doc) fprintf(out, "\n/// @brief %s\n", meth->doc);
            char *rt = method_return_type(meth);
            char *at = method_args_types(meth);
            fprintf(out, "ORCA_API %s\n%s(%s);\n", rt, meth->full_name, at);
            free(rt); free(at);
        }
    }

    /* structs: definition + push/check */
    fprintf(out, "\n");
    for (int i = 0; i < m->num_structs; i++) {
        Struct *s = &m->structs[i];
        if (s->doc) fprintf(out, "/// @brief %s\n", s->doc);
        fprintf(out, "/** %s struct */\n", s->name);
        fprintf(out, "struct %s {\n", s->name);
        emit_struct_contents_fields(s->fields, s->num_fields, out);
        fprintf(out, "};\n");
        fprintf(out, "ORCA_API void luaX_push%s(lua_State *L, struct %s const* %s);\n", s->name, s->name, s->name);
        fprintf(out, "ORCA_API struct %s* luaX_check%s(lua_State *L, int idx);\n", s->name, s->name);
    }

    /* struct methods */
    fprintf(out, "\n");
    for (int i = 0; i < m->num_structs; i++) {
        Struct *s = &m->structs[i];
        for (int j = 0; j < s->num_methods; j++) {
            Method *meth = &s->methods[j];
            if (meth->doc) fprintf(out, "/// @brief %s\n", meth->doc);
            char *rt = method_return_type(meth);
            char *at = method_args_types(meth);
            fprintf(out, "ORCA_API %s\n%s(%s);\n", rt, meth->full_name, at);
            free(rt); free(at);
        }
    }

    /* event EventArgs structs (root events only) */
    fprintf(out, "\n");
    for (int i = 0; i < m->num_events; i++) {
        Event *ev = m->events[i];
        if (ev->parent_name) continue;
        fprintf(out, "/** %s_%sEventArgs struct */\n", ev->ns, ev->name);
        fprintf(out, "struct %s_%sEventArgs {\n", ev->ns, ev->name);
        emit_struct_contents_fields(ev->fields, ev->num_fields, out);
        fprintf(out, "};\n");
        fprintf(out, "ORCA_API void luaX_push%s_%sEventArgs(lua_State *L, struct %s_%sEventArgs const* data);\n",
                ev->ns, ev->name, ev->ns, ev->name);
        fprintf(out, "ORCA_API struct %s_%sEventArgs* luaX_check%s_%sEventArgs(lua_State *L, int idx);\n",
                ev->ns, ev->name, ev->ns, ev->name);
    }

    /* components */
    fprintf(out, "\n");
    for (int i = 0; i < m->num_components; i++) {
        Component *comp = &m->components[i];
        if (comp->doc) fprintf(out, "\n/// @brief %s\n", comp->doc);
        fprintf(out, "/** %s component */\n", comp->name);
        fprintf(out, "struct %s {\n", comp->name);
        emit_struct_contents_props(comp->properties, comp->num_properties, out);
        emit_struct_contents_fields(comp->fields, comp->num_fields, out);
        for (int j = 0; j < comp->num_messages; j++)
            fprintf(out, "\tevent_t %s;\n", comp->messages[j]->name);
        fprintf(out, "};\n");
        fprintf(out, "ORCA_API void luaX_push%s(lua_State *L, struct %s const* %s);\n", comp->name, comp->name, comp->name);
        fprintf(out, "ORCA_API struct %s* luaX_check%s(lua_State *L, int idx);\n", comp->name, comp->name);
        for (int j = 0; j < comp->num_methods; j++) {
            Method *meth = &comp->methods[j];
            if (meth->doc) fprintf(out, "\n/// @brief %s\n", meth->doc);
            char *rt = method_return_type(meth);
            char *at = method_args_types(meth);
            fprintf(out, "ORCA_API %s\n%s(%s);\n", rt, meth->full_name, at);
            free(rt); free(at);
        }
    }

    fprintf(out, "\n#endif\n");
}
