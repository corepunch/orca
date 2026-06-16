#include <string.h>
#include "cg_api.h"
#include "outbuf.h"

/* Emit a single class documentation block to output buffer. */
static int emit_class_docs(cg_host_v1 const *host, cgen_output const *out,
                           cg_model const *model, cg_node const *cls) {
    outbuf b = {0};
    cg_node const *p;
    char const *parent = NULL;
    int has_details = 0, has_properties = 0, has_messages = 0;
    int n_prop = 0, n_msg = 0, n_arg = 0, n_ret = 0;
    int is_view = 0;

    /* Determine parent from type field. */
    if (cls->type && cls->type[0]) {
        parent = cls->type;
        /* Check if this is a View subclass (has children). */
        is_view = (strstr(parent, "View") != NULL);
    }

    /* Count children of this class. */
    for (p = cg_first(model, cls->id, 0); p; p = cg_next(model, p, cls->id, 0)) {
        if (p->kind == CG_KIND_MESSAGE) { n_msg++; has_messages = 1; }
        if (p->kind == CG_KIND_PROPERTY) { n_prop++; has_properties = 1; }
    }
    if (has_messages) {
        for (p = cg_first(model, cls->id, 0); p; p = cg_next(model, p, cls->id, 0))
            if (p->kind == CG_KIND_ARG) n_arg++;
        for (p = cg_first(model, cls->id, 0); p; p = cg_next(model, p, cls->id, 0))
            if (p->kind == CG_KIND_RETURNS) n_ret++;
    }

    /* Build the class header with badges. */
    if (ob_printf(&b,
            "## %s %s\n\n"
            "%s%s%s%s%s\n",
            cls->name,
            parent ? " extends %s" : "",
            parent ? parent : "",
            parent ? " " : "",
            cls->flags & CG_FLAG_SEALED ? "**sealed** " : "",
            cls->flags & CG_FLAG_STATIC ? "**static** " : "",
            cls->doc ? "" : "") < 0) {
        free(b.s);
        return -1;
    }

    /* Emit summary if present. */
    if (cls->doc) {
        if (ob_printf(&b, "\n%s\n", cls->doc) < 0) {
            free(b.s);
            return -1;
        }
    }

    /* Emit <details> sections for properties, messages, etc. */
    if (has_properties) {
        if (ob_printf(&b,
                "\n<details open>\n<summary>Properties</summary>\n\n"
                "| Name | Type | Default | Attributes |\n"
                "|------|------|---------|------------|\n") < 0) {
            free(b.s);
            return -1;
        }
        for (p = cg_first(model, cls->id, 0); p; p = cg_next(model, p, cls->id, 0)) {
            if (p->kind != CG_KIND_PROPERTY) continue;
            char const *attrs = "";
            if (p->flags & CG_FLAG_READONLY)  attrs = "readonly";
            if (p->flags & CG_FLAG_PRIVATE)   attrs = "private";
            if (p->flags & CG_FLAG_INHERITED) attrs = "inherited";
            if (p->flags & CG_FLAG_ARRAY)     attrs = "array";
            if (p->flags & CG_FLAG_POINTER)   attrs = "pointer";

            /* Escape pipe characters in doc. */
            char const *doc_esc = p->doc ? p->doc : "";

            if (ob_printf(&b, "| `%s` | %s | %s | %s |%s\n",
                    p->name, p->type,
                    p->extra ? p->extra : "(none)",
                    attrs ? attrs : "—",
                    doc_esc[0] ? " " : "",
                    doc_esc) < 0) {
                free(b.s);
                return -1;
            }
        }
        if (ob_printf(&b, "\n</details>\n") < 0) {
            free(b.s);
            return -1;
        }
    }

    if (has_messages) {
        if (ob_printf(&b,
                "\n<details open>\n<summary>Messages</summary>\n\n"
                "| Name | Routing | Args | Returns | Same-As |\n"
                "|------|---------|------|-----------|---------|\n") < 0) {
            free(b.s);
            return -1;
        }
        for (p = cg_first(model, cls->id, 0); p; p = cg_next(model, p, cls->id, 0)) {
            if (p->kind != CG_KIND_MESSAGE) continue;
            char const *same_as = p->extra && p->extra[0] ? p->extra : "—";
            char const *routing = p->type && p->type[0] ? p->type : "—";
            char const *doc_esc = p->doc ? p->doc : "";

            if (ob_printf(&b, "| `%s` | %s | %u | %u | %s |%s\n",
                    p->name, routing, n_arg, n_ret, same_as,
                    doc_esc[0] ? " " : "", doc_esc) < 0) {
                free(b.s);
                return -1;
            }
        }
        if (ob_printf(&b, "\n</details>\n") < 0) {
            free(b.s);
            return -1;
        }
    }

    /* Emit child classes (View children) if any. */
    if (is_view) {
        int n_children = 0;
        cg_node const *child;
        for (child = cg_first(model, cls->id, 0); child; child = cg_next(model, child, cls->id, 0))
            if (child->kind == CG_KIND_CLASS) n_children++;

        if (n_children > 0) {
            if (ob_printf(&b,
                    "\n<details open>\n<summary>Child Views (%u)</summary>\n\n",
                    n_children) < 0) {
                free(b.s);
                return -1;
            }
            for (child = cg_first(model, cls->id, 0); child; child = cg_next(model, child, cls->id, 0))
                if (child->kind == CG_KIND_CLASS)
                    if (ob_printf(&b, "- %s\n", child->name) < 0) {
                        free(b.s);
                        return -1;
                    }
            if (ob_printf(&b, "\n</details>\n") < 0) {
                free(b.s);
                return -1;
            }
        }
    }

    /* Emit the class documentation. */
    if (host->write_file(out->path, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", out->path);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
}

static int emit_docs(cg_host_v1 const *host, cg_model const *model, char const *output_dir) {
    size_t i;
    uint32_t n_enum = 0, n_if = 0, n_struct = 0, n_class = 0;
    outbuf b = {0};
    cgen_output *out;
    int errors = 0;

    /* Count nodes by kind. */
    for (i = 0; i < model->node_count; ++i) {
        n_enum += model->nodes[i].kind == CG_KIND_ENUM;
        n_if += model->nodes[i].kind == CG_KIND_INTERFACE;
        n_struct += model->nodes[i].kind == CG_KIND_STRUCT;
        n_class += model->nodes[i].kind == CG_KIND_CLASS;
    }

    /* Build module overview. */
    if (ob_printf(&b,
            "# %s API (cgen preview)\n\n"
            "- source: `%s`\n"
            "- enums: %u\n"
            "- interfaces: %u\n"
            "- structs: %u\n"
            "- classes: %u\n\n"
            "## Classes\n",
            model->module_name, model->xml_path, n_enum, n_if, n_struct, n_class) < 0) {
        free(b.s);
        return -1;
    }

    /* List all classes in the overview. */
    for (i = 0; i < model->node_count; ++i)
        if (model->nodes[i].kind == CG_KIND_CLASS)
            if (ob_printf(&b, "- [%s](classes/%s.md)\n",
                    model->nodes[i].name, model->nodes[i].name) < 0) {
                free(b.s);
                return -1;
            }

    /* Write module overview. */
    if (host->write_file(output_dir, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", output_dir);
        free(b.s);
        return -1;
    }
    free(b.s);

    /* Emit individual class documentation files. */
    for (i = 0; i < model->node_count; ++i) {
        if (model->nodes[i].kind != CG_KIND_CLASS) continue;

        /* Build output path for this class. */
        char path[512];
        size_t base_len = strlen(output_dir);
        if (base_len + 1 + strlen(model->nodes[i].name) + 5 >= sizeof(path)) {
            host->logf("docs: path too long for %s", model->nodes[i].name);
            continue;
        }
        memcpy(path, output_dir, base_len);
        path[base_len] = '/';
        strcpy(path + base_len + 1, "classes/");
        strcpy(path + base_len + 9, model->nodes[i].name);
        strcpy(path + base_len + 9 + strlen(model->nodes[i].name), ".md");

        /* Build output descriptor. */
        out = (cgen_output*)malloc(sizeof(cgen_output));
        if (!out) continue;
        out->path = path;

        if (emit_class_docs(host, out, model, &model->nodes[i]) < 0) {
            host->logf("docs: failed emitting %s", model->nodes[i].name);
            errors++;
        }
        free(out);
    }

    return errors ? -1 : 0;
}

static cg_plugin_v1 const plugin = { .abi = CG_API_VERSION, .name = "docs", .emit = emit_docs };

cg_plugin_v1 const *cg_plugin_init_v1(void) { return &plugin; }
