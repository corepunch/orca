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

static uint32_t make_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

static uint32_t make_rgb(uint8_t r, uint8_t g, uint8_t b)
{
  return make_argb(0xFF, r, g, b);
}

static int clamp255(int v)
{
  if (v < 0) return 0;
  if (v > 255) return 255;
  return v;
}

static uint32_t read_u32_le(const uint8_t *p)
{
  return ((uint32_t)p[0]) |
  ((uint32_t)p[1] << 8) |
  ((uint32_t)p[2] << 16) |
  ((uint32_t)p[3] << 24);
}

int dr_palette_from_buffer(const void *buffer, size_t size, const DrPalInfo *info, uint32_t* colors)
{
  const uint8_t *p = (const uint8_t*)buffer;
  
  if (size < 8 + PALETTE_SIZE * 6)
    return 1;
  
  if (memcmp(p, "PALS", 4) != 0)
    return 1;
  
  uint32_t version = read_u32_le(p + 4);
  if (version != 0x0102)
    return 1;
  
  p += 8;
  
  const uint8_t *rlist = p;
  const uint8_t *glist = p + 256;
  const uint8_t *blist = p + 512;
  
  for (int i = 0; i < PALETTE_SIZE; i++)
  {
    if (i == 0)
    {
      colors[i] = 0;
      continue;
    }
    
    int mult = (i < 160 || i == 255)
    ? info->standard_palette_multiplier
    : info->terrain_palette_multiplier;
    
    int r = clamp255(rlist[i] * mult + info->gamma);
    int g = clamp255(glist[i] * mult + info->gamma);
    int b = clamp255(blist[i] * mult + info->gamma);
    
    colors[i] = make_rgb((uint8_t)r, (uint8_t)g, (uint8_t)b);
  }
  
  if (info->shadow_index >= 0 && info->shadow_index < PALETTE_SIZE)
    colors[info->shadow_index] = make_argb(112, 0, 0, 0);
  
  return 1;
}
