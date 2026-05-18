#ifndef ORCA_CODEGEN_PLUGIN_H
#define ORCA_CODEGEN_PLUGIN_H

typedef int (*codegen_plugin_fn)(const char *xml_path);

typedef struct {
    const char *name;
    codegen_plugin_fn run;
} codegen_plugin_t;

int codegen_plugin_header_run(const char *xml_path);
int codegen_plugin_properties_run(const char *xml_path);
int codegen_plugin_export_run(const char *xml_path);

#endif
