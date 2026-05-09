#ifndef CODEGEN_TMPL_H
#define CODEGEN_TMPL_H

#include "model.h"

/*
 * Minimal template engine for ORCA codegen.
 *
 * Template syntax (line-oriented):
 *   {{!-- comment --}}             ignored
 *   {{module.name}}                scalar substitution
 *   {{module.name|upper}}          with filter (upper/lower/ucfirst/lcfirst)
 *   {{for enums as e}}...{{end}}   loop over a named collection
 *   {{for e.values as v}}          nested loop
 *   {{if e.doc}}...{{end}}         conditional (truthy = non-null/non-empty)
 *   {{else}}                       inside {{if}}
 *   {{include path/to/sub.ctmpl}}  include sub-template (relative to template dir)
 *   {{fnv1a "string"}}             emit fnv1a32 hex of a literal string
 *   {{fnv1a e.name}}               emit fnv1a32 hex of a runtime value
 *
 * Collections available in module scope:
 *   enums, structs, interfaces, components, functions, events, includes, externals, requires
 *
 * Variable paths use dot notation. Arrays use the loop variable.
 *
 * Implementation strategy: recursive descent interpreter — no compiled bytecode.
 */

/* Render template file against module, writing to out.
 * template_dir is used to resolve {{include}} paths. */
void tmpl_render(const char *template_file, const char *template_dir,
                 Module *m, FILE *out);

#endif /* CODEGEN_TMPL_H */
