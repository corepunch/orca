#include "codegen_export_backend.h"

#include <unistd.h>

int emit_target_pyphp(const char *xml_path) {
    char *const pyphp_argv[] = {"python3", "-m", "pyphp.pyphp", "templates/export.php", (char *)xml_path, NULL};
    execvp("python3", pyphp_argv);
    die("failed to exec pyphp fallback exporter");
    return 1;
}
