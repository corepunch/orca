#ifndef ORCA_CODEGEN_PLUGIN_H
#define ORCA_CODEGEN_PLUGIN_H

typedef int (*codegen_plugin_fn)(int argc, char **argv);

typedef struct {
    const char *name;
    codegen_plugin_fn run;
} codegen_plugin_t;

int codegen_plugin_header_run(int argc, char **argv);
int codegen_plugin_properties_run(int argc, char **argv);
int codegen_plugin_export_run(int argc, char **argv);
int codegen_plugin_docs_run(int argc, char **argv);
int codegen_plugin_docs_index_run(int argc, char **argv);
int codegen_plugin_dtd_run(int argc, char **argv);

#endif
