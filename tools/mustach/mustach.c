/*
 * mustach.c — minimal Mustache renderer implementation
 */
#include "mustach.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

/* Trim leading whitespace in [s, end) and return pointer past it. */
static const char *skip_ws(const char *s, const char *end)
{
    while (s < end && (*s == ' ' || *s == '\t'))
        s++;
    return s;
}

/* Return pointer to first occurrence of needle (length nlen) in
 * haystack [s, end).  Returns NULL if not found. */
static const char *find_mem(const char *s, const char *end,
                             const char *needle, size_t nlen)
{
    if (nlen == 0) return s;
    for (; s + nlen <= end; s++) {
        if (memcmp(s, needle, nlen) == 0)
            return s;
    }
    return NULL;
}

/* -------------------------------------------------------------------------
 * Tag parsing
 * ---------------------------------------------------------------------- */

typedef enum {
    TAG_VARIABLE,   /* {{name}}  or {{{name}}} */
    TAG_SECTION,    /* {{#name}} */
    TAG_INVERTED,   /* {{^name}} */
    TAG_CLOSE,      /* {{/name}} */
    TAG_COMMENT,    /* {{! ... }} */
    TAG_NONE        /* not a tag */
} TagType;

typedef struct {
    TagType   type;
    char      name[256]; /* tag name, NUL-terminated */
    /* byte positions in the source template */
    size_t    tag_start; /* offset of first '{' */
    size_t    tag_end;   /* offset one past final '}' */
    /* standalone detection */
    size_t    line_start;/* offset of first char of the line containing tag */
    int       standalone;/* 1 if the line contains only this tag */
} Tag;

/*
 * Parse the next tag starting from *pos in [tmpl, tmpl+len).
 * On success fills *tag and advances *pos past the tag.
 * Returns 0 on success, -1 if no more tags.
 */
static int next_tag(const char *tmpl, size_t len, size_t *pos, Tag *tag)
{
    size_t i = *pos;

    /* Find "{{" */
    while (i + 1 < len) {
        if (tmpl[i] == '{' && tmpl[i+1] == '{') break;
        i++;
    }
    if (i + 1 >= len) return -1; /* no tag found */

    /* Record line start (scan backward for preceding newline) */
    size_t ls = i;
    while (ls > 0 && tmpl[ls-1] != '\n') ls--;
    tag->line_start = ls;
    tag->tag_start  = i;

    i += 2; /* skip "{{" */
    int triple = (i < len && tmpl[i] == '{');
    if (triple) i++;

    /* Determine tag type from first character */
    if (i >= len) return -1;
    char first = tmpl[i];
    TagType type = TAG_VARIABLE;
    if      (first == '#') { type = TAG_SECTION;  i++; }
    else if (first == '^') { type = TAG_INVERTED; i++; }
    else if (first == '/') { type = TAG_CLOSE;    i++; }
    else if (first == '!') { type = TAG_COMMENT;  i++; }
    tag->type = type;

    /* Skip whitespace before name */
    while (i < len && (tmpl[i] == ' ' || tmpl[i] == '\t')) i++;

    /* Collect name */
    size_t nstart = i;
    const char *close = triple ? "}}}" : "}}";
    size_t clen = triple ? 3 : 2;
    const char *p = find_mem(tmpl + i, tmpl + len, close, clen);
    if (!p) return -1;

    size_t raw_len = (size_t)(p - (tmpl + nstart));

    /* For comments, name is irrelevant; just record empty */
    if (type == TAG_COMMENT) {
        tag->name[0] = '\0';
    } else {
        /* Trim trailing whitespace from name */
        size_t nlen = raw_len;
        while (nlen > 0 && (tmpl[nstart + nlen - 1] == ' ' ||
                             tmpl[nstart + nlen - 1] == '\t'))
            nlen--;
        if (nlen >= sizeof(tag->name)) nlen = sizeof(tag->name) - 1;
        memcpy(tag->name, tmpl + nstart, nlen);
        tag->name[nlen] = '\0';
    }

    tag->tag_end = (size_t)(p - tmpl) + clen;

    /* Standalone detection: does the line contain only this tag?
     * A line is standalone if there is only whitespace before and after
     * the tag on the same line (up to next newline or end of template). */
    if (type == TAG_VARIABLE) {
        tag->standalone = 0;
    } else {
        /* Check whitespace before tag on its line */
        int standalone = 1;
        for (size_t k = tag->line_start; k < tag->tag_start; k++) {
            if (tmpl[k] != ' ' && tmpl[k] != '\t') { standalone = 0; break; }
        }
        if (standalone) {
            /* Check whitespace after tag on its line */
            size_t k = tag->tag_end;
            while (k < len && (tmpl[k] == ' ' || tmpl[k] == '\t')) k++;
            if (k < len && tmpl[k] != '\n') standalone = 0;
        }
        tag->standalone = standalone;
    }

    *pos = tag->tag_end;
    return 0;
}

/* -------------------------------------------------------------------------
 * Renderer
 * ---------------------------------------------------------------------- */

/*
 * Forward declaration: render [start, start+len) to file.
 * section_name: if non-NULL we are inside that section (for close-tag check).
 * Returns offset consumed in the template segment.
 */
static int render(const char *tmpl, size_t len, size_t start,
                  const char *section_name,
                  const struct mustach_itf *itf, void *closure,
                  FILE *file, size_t *consumed);

/*
 * Emit literal text from [from, to) to file.
 * If standalone_newline is true, the trailing newline on this "literal chunk"
 * belongs to a standalone tag and should be suppressed.
 */
static void emit_literal(const char *tmpl, size_t from, size_t to,
                          int suppress_trailing_newline, FILE *file)
{
    if (from >= to) return;
    size_t len = to - from;
    if (suppress_trailing_newline && len > 0 && tmpl[to-1] == '\n')
        len--;
    if (len > 0)
        fwrite(tmpl + from, 1, len, file);
}

static int render(const char *tmpl, size_t len, size_t start,
                  const char *section_name,
                  const struct mustach_itf *itf, void *closure,
                  FILE *file, size_t *consumed)
{
    size_t pos = start;
    size_t last_end = start; /* end of last emitted literal */

    while (pos < len) {
        Tag tag;
        size_t saved_pos = pos;
        if (next_tag(tmpl, len, &pos, &tag) < 0) {
            /* No more tags: emit rest as literal */
            fwrite(tmpl + last_end, 1, len - last_end, file);
            last_end = len;
            break;
        }

        /* Emit literal text before this tag.
         * If the tag is standalone, suppress the newline that precedes the
         * tag's line (i.e., emit up to line_start-1 if last_end < line_start,
         * then suppress the rest of the line including the tag's newline). */
        if (tag.standalone) {
            /* Emit up to (but not including) the tag's line */
            if (tag.line_start > last_end)
                fwrite(tmpl + last_end, 1, tag.line_start - last_end, file);
            /* Skip past the newline after the tag */
            if (tag.tag_end < len && tmpl[tag.tag_end] == '\n')
                pos = tag.tag_end + 1;
            else
                pos = tag.tag_end;
        } else {
            /* Non-standalone: emit literal up to the tag start */
            if (tag.tag_start > last_end)
                fwrite(tmpl + last_end, 1, tag.tag_start - last_end, file);
            pos = tag.tag_end;
        }
        last_end = pos;

        switch (tag.type) {
        case TAG_VARIABLE: {
            /* emit value */
            if (itf->put(closure, tag.name, file) != 0)
                return MUSTACH_ERROR_SYSTEM;
            break;
        }
        case TAG_COMMENT:
            /* nothing to do */
            break;

        case TAG_CLOSE: {
            /* End of a section we were rendering */
            if (section_name && strcmp(tag.name, section_name) == 0) {
                if (consumed) *consumed = pos;
                return MUSTACH_OK;
            }
            /* Unmatched close tag: ignore */
            break;
        }

        case TAG_SECTION:
        case TAG_INVERTED: {
            /* Find the body: everything between this tag and its {{/name}} */
            /* We need to scan ahead to find the matching close tag.
             * We do this by running render() which will stop at the close. */
            size_t body_start = pos;
            size_t body_end   = pos; /* will be set by recursive render */

            int entered = itf->enter(closure, tag.name);
            int inverted = (tag.type == TAG_INVERTED);

            if (inverted ? entered : !entered) {
                /* Skip the body: scan to matching {{/name}} without rendering */
                /* Use a depth counter for nested same-name sections */
                int depth = 1;
                size_t scan = body_start;
                while (scan < len && depth > 0) {
                    Tag t2;
                    size_t sp2 = scan;
                    if (next_tag(tmpl, len, &sp2, &t2) < 0) break;
                    scan = sp2;
                    if (strcmp(t2.name, tag.name) == 0) {
                        if (t2.type == TAG_SECTION || t2.type == TAG_INVERTED)
                            depth++;
                        else if (t2.type == TAG_CLOSE)
                            depth--;
                    }
                    if (t2.standalone) {
                        /* skip the newline we already moved past */
                    }
                }
                pos = scan;
                last_end = pos;
                /* If inverted and we entered, we need to properly leave */
                if (inverted && entered)
                    itf->leave(closure);
                else if (!inverted && !entered) {
                    /* nothing entered, nothing to leave */
                }
            } else {
                /* Render the body one or more times */
                do {
                    size_t cons = 0;
                    int rc = render(tmpl, len, body_start, tag.name,
                                    itf, closure, file, &cons);
                    if (rc != MUSTACH_OK) return rc;
                    body_end = cons;
                } while (itf->next(closure));
                itf->leave(closure);
                pos = body_end;
                last_end = pos;
            }
            break;
        }

        default:
            break;
        }
    }

    /* Emit any remaining literal */
    if (last_end < len)
        fwrite(tmpl + last_end, 1, len - last_end, file);

    if (consumed) *consumed = len;
    return MUSTACH_OK;
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

int mustach_file(const char *template, size_t len,
                 const struct mustach_itf *itf, void *closure,
                 FILE *file)
{
    return render(template, len, 0, NULL, itf, closure, file, NULL);
}
