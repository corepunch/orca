#include "codegen_plugin_common.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int codegen_exec_pyphp_template(const char *template_path, const char *xml_path) {
    char *template_arg = strdup(template_path ? template_path : "");
    if (!template_arg) {
        fprintf(stderr, "codegen: out of memory\n");
        return 1;
    }

    char *xml_arg = strdup(xml_path ? xml_path : "");
    if (!xml_arg) {
        fprintf(stderr, "codegen: out of memory\n");
        free(template_arg);
        return 1;
    }

    char *const argv[] = {"python3", "-m", "pyphp.pyphp", template_arg, xml_arg, NULL};
    execvp("python3", argv);

    fprintf(stderr, "codegen: failed to exec pyphp (%s): %s\n", template_path, strerror(errno));
    free(template_arg);
    free(xml_arg);
    return 1;
}
