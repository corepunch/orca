/*
 * main.c — ORCA C codegen tool entry point.
 *
 * Usage:
 *   codegen <input.xml> <output.file> <template.ctmpl>
 *
 * Reads <input.xml>, builds the data model, then renders <template.ctmpl>
 * and writes the result to <output.file> (or stdout if "-").
 *
 * Template files live in tools/templates/ and use a simple {{...}} syntax
 * described in tmpl.h.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>

#include "model.h"
#include "tmpl.h"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr,
            "Usage: codegen <input.xml> <output.file> <template.ctmpl>\n"
            "\n"
            "  <input.xml>        Module definition file (e.g. source/core/core.xml)\n"
            "  <output.file>      Destination file, or '-' for stdout\n"
            "  <template.ctmpl>   Template file (e.g. tools/templates/header.ctmpl)\n");
        return 1;
    }

    const char *xml_file      = argv[1];
    const char *out_file      = argv[2];
    const char *template_file = argv[3];

    /* Initialise libxml2 */
    xmlInitParser();
    LIBXML_TEST_VERSION

    /* Load module (recursively loads requires) */
    Module *m = load_module(xml_file, NULL);
    if (!m) {
        fprintf(stderr, "codegen: failed to load module from '%s'\n", xml_file);
        return 1;
    }

    /* Open output */
    FILE *out;
    if (strcmp(out_file, "-") == 0) {
        out = stdout;
    } else {
        out = fopen(out_file, "w");
        if (!out) {
            fprintf(stderr, "codegen: cannot open output file '%s'\n", out_file);
            return 1;
        }
    }

    /* Derive template directory from template_file path */
    char template_dir[1024] = ".";
    const char *slash = strrchr(template_file, '/');
    if (slash) {
        size_t dlen = (size_t)(slash - template_file);
        if (dlen < sizeof(template_dir) - 1) {
            strncpy(template_dir, template_file, dlen);
            template_dir[dlen] = '\0';
        }
    }

    /* Render template */
    tmpl_render(template_file, template_dir, m, out);

    if (out != stdout) fclose(out);

    xmlCleanupParser();
    return 0;
}
