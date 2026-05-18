#include "codegen_plugin.h"
#include "codegen_plugin_common.h"
#include <stdio.h>

int codegen_plugin_properties_run(int argc, char **argv) {
    if (argc != 1) {
        fprintf(stderr, "codegen: properties plugin expects exactly 1 argument (<module.xml>)\n");
        return 1;
    }
    return codegen_exec_pyphp_template("templates/properties.php", argv[0]);
}
