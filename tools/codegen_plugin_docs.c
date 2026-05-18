#include "codegen_plugin.h"
#include "codegen_plugin_common.h"

int codegen_plugin_docs_run(int argc, char **argv) {
    return codegen_exec_pyphp_template_args("templates/docs.php", argc, argv);
}
