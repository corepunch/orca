#include "codegen_export_backend.h"

#include <stdlib.h>
#include <string.h>

typedef int (*target_emit_fn)(FILE *out, module_t *m, const char *xml_path);

typedef struct {
    const char *name;
    target_emit_fn emit;
} export_target_t;

static int emit_target_pyphp_adapter(FILE *out, module_t *m, const char *xml_path) {
    (void)out;
    (void)m;
    return emit_target_pyphp(xml_path);
}

static const export_target_t k_targets[] = {
    {"pyphp", emit_target_pyphp_adapter},
    {"c", emit_target_c},
};

static const export_target_t *select_target(void) {
    const char *target_name = getenv("ORCA_CODEGEN_EXPORT_TARGET");
    if (!target_name || !target_name[0]) {
        const char *experimental = getenv("ORCA_CODEGEN_EXPORT_EXPERIMENTAL");
        target_name = (experimental && strcmp(experimental, "1") == 0) ? "c" : "pyphp";
    }

    for (size_t i = 0; i < sizeof(k_targets) / sizeof(k_targets[0]); ++i)
        if (strcmp(k_targets[i].name, target_name) == 0)
            return &k_targets[i];

    die("unknown ORCA_CODEGEN_EXPORT_TARGET (expected: pyphp or c)");
}

int main(int argc, char **argv) {
    if (argc != 2) die("usage: codegen_export <module.xml>");
    xmlInitParser();

    char **seen = NULL;
    size_t seen_count = 0;
    module_t *m = load_module(argv[1], &seen, &seen_count);
    if (!m) die("failed to load module");

    const export_target_t *target = select_target();
    int rc = target->emit(stdout, m, argv[1]);

    free_seen(seen, seen_count);
    xmlCleanupParser();
    return rc;
}
