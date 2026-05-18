#include "codegen_export_backend.h"
#include "codegen_plugin.h"
#include "codegen_plugin_common.h"

#include <stdlib.h>
#include <string.h>

static int select_c_target(void) {
    const char *target_name = getenv("ORCA_CODEGEN_EXPORT_TARGET");
    if (!target_name || !target_name[0]) {
        /* TODO: remove ORCA_CODEGEN_EXPORT_EXPERIMENTAL fallback after migration */
        const char *experimental = getenv("ORCA_CODEGEN_EXPORT_EXPERIMENTAL");
        return experimental && strcmp(experimental, "1") == 0;
    }

    if (strcmp(target_name, "c") == 0) return 1;
    if (strcmp(target_name, "pyphp") == 0) return 0;
    die("unknown ORCA_CODEGEN_EXPORT_TARGET (expected: pyphp or c)");
}

/* Export plugin entry point for `codegen -plugin=export <module.xml>`. */
int codegen_plugin_export_run(const char *xml_path) {
    if (!select_c_target()) {
        return codegen_exec_pyphp_template("templates/export.php", xml_path);
    }

    xmlInitParser();

    char **seen = NULL;
    size_t seen_count = 0;
    module_t *m = load_module(xml_path, &seen, &seen_count);
    if (!m) die("failed to load module");

    int rc = emit_target_c(stdout, m, xml_path);

    free_seen(seen, seen_count);
    xmlCleanupParser();
    return rc;
}
