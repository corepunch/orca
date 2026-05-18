#include "codegen_plugin_common.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int codegen_exec_pyphp_template_args(const char *template_path, int argc, const char *const *args) {
    if (argc < 1) {
        fprintf(stderr, "codegen: missing required arguments for %s\n", template_path ? template_path : "(null)");
        return 1;
    }

    size_t pyphp_argc = (size_t)argc + 5; /* python3, -m, pyphp.pyphp, template, args..., NULL */
    char **pyphp_argv = calloc(pyphp_argc, sizeof(char *));
    if (!pyphp_argv) {
        fprintf(stderr, "codegen: out of memory\n");
        return 1;
    }

    pyphp_argv[0] = "python3";
    pyphp_argv[1] = "-m";
    pyphp_argv[2] = "pyphp.pyphp";
    pyphp_argv[3] = strdup(template_path ? template_path : "");
    if (!pyphp_argv[3]) {
        fprintf(stderr, "codegen: out of memory\n");
        free(pyphp_argv);
        return 1;
    }
    for (int i = 0; i < argc; ++i) {
        pyphp_argv[4 + i] = strdup(args[i] ? args[i] : "");
        if (!pyphp_argv[4 + i]) {
            fprintf(stderr, "codegen: out of memory\n");
            for (int j = 3; j < 4 + i; ++j) free(pyphp_argv[j]);
            free(pyphp_argv);
            return 1;
        }
    }
    pyphp_argv[4 + argc] = NULL;

    execvp("python3", pyphp_argv);

    fprintf(stderr, "codegen: failed to exec pyphp (%s): %s\n", template_path, strerror(errno));
    for (int i = 3; i < 4 + argc; ++i) free(pyphp_argv[i]);
    free(pyphp_argv);
    return 1;
}

int codegen_exec_pyphp_template(const char *template_path, const char *xml_path) {
    const char *args[] = {xml_path ? xml_path : ""};
    return codegen_exec_pyphp_template_args(template_path, 1, args);
}
