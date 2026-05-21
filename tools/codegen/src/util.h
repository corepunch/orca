#ifndef ORCA_CG_UTIL_H
#define ORCA_CG_UTIL_H

#include <stdint.h>
#include <stdio.h>

uint32_t cg_fnv1a32(char const *s);
int cg_write_file(char const *path, char const *data, size_t len);
void cg_logf(char const *fmt, ...);

#endif
