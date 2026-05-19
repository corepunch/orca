#include "util.h"

#include <stdarg.h>
#include <string.h>

uint32_t cg_fnv1a32(char const *s) {
    uint32_t h = 0x811C9DC5u;
    unsigned char const *p = (unsigned char const *)s;
    while (*p) {
        h ^= (uint32_t)*p++;
        h *= 0x01000193u;
    }
    return h;
}

int cg_write_file(char const *path, char const *data, size_t len) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    if (len && fwrite(data, 1u, len, f) != len) {
        fclose(f);
        return -2;
    }
    fclose(f);
    return 0;
}

void cg_logf(char const *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
}
