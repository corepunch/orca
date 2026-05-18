#include "codegen_plugin_common.h"

#include <stdio.h>
#include <unistd.h>

int codegen_exec_pyphp_template(const char *template_name, const char *xml_path) {
    char template_path[256];
    int n = snprintf(template_path, sizeof(template_path), "templates/%s.php", template_name);
    if (n <= 0 || n >= (int)sizeof(template_path)) {
        fprintf(stderr, "codegen: template path too long\n");
        return 1;
    }

    char *const argv[] = {"python3", "-m", "pyphp.pyphp", template_path, (char *)xml_path, NULL};
    execvp("python3", argv);

    fprintf(stderr, "codegen: failed to exec pyphp for plugin '%s'\n", template_name);
    return 1;
}
