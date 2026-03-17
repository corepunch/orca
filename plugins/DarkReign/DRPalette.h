#ifndef __DR_PALETTE__
#define __DR_PALETTE__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PALETTE_SIZE 256

typedef struct {
  int shadow_index;
  int standard_palette_multiplier;
  int terrain_palette_multiplier;
  int gamma;
} DrPalInfo;

int dr_palette_from_buffer(const void *buffer, size_t size, const DrPalInfo *info, uint32_t* colors);

#endif
