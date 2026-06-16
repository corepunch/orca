#ifndef OUTBUF_H
#define OUTBUF_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct outbuf { char *s; size_t n; size_t cap; } outbuf;

static inline int ob_grow(outbuf *b, size_t add) {
    if (b->n + add <= b->cap) return 0;
    {
        size_t nc = b->cap ? b->cap * 2u : 2048u;
        while (nc < b->n + add) nc *= 2u;
        b->s = (char *)realloc(b->s, nc);
        if (!b->s) return -1;
        b->cap = nc;
    }
    return 0;
}

static inline int ob_printf(outbuf *b, char const *fmt, ...) {
    va_list ap, cp;
    int need;
    va_start(ap, fmt);
    va_copy(cp, ap);
    need = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (need < 0 || ob_grow(b, (size_t)need + 1u) < 0) { va_end(cp); return -1; }
    vsnprintf(b->s + b->n, b->cap - b->n, fmt, cp);
    va_end(cp);
    b->n += (size_t)need;
    return 0;
}

#endif /* OUTBUF_H */
