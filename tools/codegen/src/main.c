#include "cg_api.h"
#include "model.h"
#include "util.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct args {
    char const *xml;
    char const *plugin;
    char const *out;
} args;

static int parse_args(int argc, char **argv, args *out) {
    int i;
    memset(out, 0, sizeof(*out));
    for (i = 1; i < argc; ++i) {
        char const *a = argv[i];
        char const *v = (i + 1 < argc) ? argv[i + 1] : NULL;
        if (!strcmp(a, "--xml") && v) out->xml = argv[++i];
        else if (!strcmp(a, "--plugin") && v) out->plugin = argv[++i];
        else if (!strcmp(a, "--out") && v) out->out = argv[++i];
        else return -1;
    }
    return (out->xml && out->plugin && out->out) ? 0 : -2;
}

int main(int argc, char **argv) {
    args a;
    cg_model model;
    void *lib;
    cg_plugin_init_fn init;
    cg_plugin_v1 const *plugin;
    cg_host_v1 host = {
        .abi = CG_API_VERSION,
        .logf = cg_logf,
        .write_file = cg_write_file,
        .fnv1a32 = cg_fnv1a32
    };
    int rc;

    if (parse_args(argc, argv, &a) < 0) {
        fprintf(stderr, "usage: %s --xml <module.xml> --plugin <plugin.so> --out <file>\n", argv[0]);
        return 2;
    }
    rc = cg_model_load(a.xml, &model);
    if (rc < 0) {
        fprintf(stderr, "cgen: failed to parse %s (%d)\n", a.xml, rc);
        return 3;
    }

    lib = dlopen(a.plugin, RTLD_NOW | RTLD_LOCAL);
    if (!lib) {
        fprintf(stderr, "cgen: dlopen failed: %s\n", dlerror());
        cg_model_free(&model);
        return 4;
    }

    init = (cg_plugin_init_fn)dlsym(lib, "cg_plugin_init_v1");
    if (!init) {
        fprintf(stderr, "cgen: missing cg_plugin_init_v1 in %s\n", a.plugin);
        dlclose(lib);
        cg_model_free(&model);
        return 5;
    }

    plugin = init();
    if (!plugin || plugin->abi != CG_API_VERSION || !plugin->emit) {
        fprintf(stderr, "cgen: invalid plugin ABI in %s\n", a.plugin);
        dlclose(lib);
        cg_model_free(&model);
        return 6;
    }

    rc = plugin->emit(&host, &model, a.out);
    dlclose(lib);
    cg_model_free(&model);
    return rc ? 7 : 0;
}
