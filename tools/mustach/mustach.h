/**
 * mustach.h — minimal Mustache template renderer (C99, single-file)
 *
 * Supported tags:
 *   {{variable}}     — HTML-unescaped variable substitution
 *   {{{variable}}}   — also unescaped (same behaviour; we never HTML-escape)
 *   {{#section}}     — enter / loop
 *   {{/section}}     — close section
 *   {{^section}}     — inverted section
 *   {{! comment }}   — comment (ignored)
 *
 * Standalone-tag rule: a line containing only a section/comment tag (with
 * optional surrounding whitespace) does not produce a blank output line.
 */
#ifndef MUSTACH_H
#define MUSTACH_H

#include <stdio.h>
#include <stddef.h>

/* Return codes */
#define MUSTACH_OK                    0
#define MUSTACH_ERROR_INVALID_TEMPLATE 1
#define MUSTACH_ERROR_SYSTEM          2

/**
 * Callback interface supplied by the caller.
 *
 * enter  — called when {{#name}} or {{^name}} is reached.
 *          Return 1 to process the section body, 0 to skip it.
 *          For {{^name}} the sense is inverted by the renderer.
 *
 * next   — called after the body of a section is rendered to check whether to
 *          loop.  Return 1 to render the body again, 0 to leave the section.
 *
 * leave  — called when a section ends (after the last iteration).
 *
 * put    — called to emit the value of {{name}} or {{{name}}}.
 *          Write to *file*.  Return 0 on success, non-zero on error.
 */
struct mustach_itf {
    int (*enter)(void *closure, const char *name);
    int (*next) (void *closure);
    int (*leave)(void *closure);
    int (*put)  (void *closure, const char *name, FILE *file);
};

/**
 * Render *template* (length *len* bytes, NUL-terminator not required) to
 * *file* using *itf* / *closure*.  Returns MUSTACH_OK on success.
 */
int mustach_file(const char *template, size_t len,
                 const struct mustach_itf *itf, void *closure,
                 FILE *file);

#endif /* MUSTACH_H */
