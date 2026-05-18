#include "codegen_plugin.h"

#include <stdio.h>
#include <string.h>

static const codegen_plugin_t k_plugins[] = {
    {"header", codegen_plugin_header_run},
    {"properties", codegen_plugin_properties_run},
    {"export", codegen_plugin_export_run},
    {"docs", codegen_plugin_docs_run},
    {"docs-index", codegen_plugin_docs_index_run},
    {"dtd", codegen_plugin_dtd_run},
};

static const codegen_plugin_t *find_plugin(const char *name) {
    for (size_t i = 0; i < sizeof(k_plugins) / sizeof(k_plugins[0]); ++i)
        if (strcmp(k_plugins[i].name, name) == 0)
            return &k_plugins[i];
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 3 || strncmp(argv[1], "-plugin=", 8) != 0) {
        fprintf(stderr, "usage: codegen -plugin=<header|properties|export|docs|docs-index|dtd> <args...>\n");
        return 1;
    }

    const char *plugin_name = argv[1] + 8;
    const codegen_plugin_t *plugin = find_plugin(plugin_name);
    if (!plugin) {
        fprintf(stderr, "codegen: unknown plugin '%s'\n", plugin_name);
        return 1;
    }

    return plugin->run(argc - 2, &argv[2]);
}
