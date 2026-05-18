#include "codegen_plugin.h"
#include "codegen_plugin_common.h"

int codegen_plugin_header_run(const char *xml_path) {
    return codegen_exec_pyphp_template("templates/header.php", xml_path);
}
