#include "util.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

uint32_t cg_fnv1a32(char const *s) {
    uint32_t h = 0x811C9DC5u;
    unsigned char const *p = (unsigned char const *)s;
    while (*p) {
        h ^= (uint32_t)*p++;
        h *= 0x01000193u;
    }
    return h;
}

/* Create parent directories for path (like mkdir -p). */
static void make_parent_dirs(char const *path) {
    char *tmp = strdup(path);
    char *p;
    if (!tmp) return;
    for (p = tmp + 1; *p; ++p) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    free(tmp);
}

int cg_write_file(char const *path, char const *data, size_t len) {
    if (!path || !path[0] || !strcmp(path, "-")) {
        if (len && fwrite(data, 1u, len, stdout) != len) return -2;
        return 0;
    }
    make_parent_dirs(path);
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
