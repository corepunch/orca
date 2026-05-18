#ifndef ORCA_CODEGEN_PLUGIN_COMMON_H
#define ORCA_CODEGEN_PLUGIN_COMMON_H

int codegen_exec_pyphp_template(const char *template_path, const char *xml_path);
int codegen_exec_pyphp_template_args(const char *template_path, int argc, char **argv);

#endif
