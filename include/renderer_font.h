#pragma once

#include <stdint.h>

struct fontface;

struct font_metrics
{
  float raster_scale;
  int ascender;
  int descender;
  int height;
  int underline_position;
  int32_t space_advance;
};

struct font_glyph
{
  unsigned char* bitmap;
  int width;
  int height;
  int xoff;
  int yoff;
};

int FontFace_GetMetrics(struct fontface*, float, float, struct font_metrics*);
uint32_t FontFace_GetGlyphIndex(struct fontface*, uint32_t);
int32_t FontFace_GetGlyphAdvance(struct fontface*, uint32_t, float);
int32_t FontFace_GetGlyphKerning(struct fontface*, uint32_t, uint32_t, float);
int FontFace_RenderGlyph(struct fontface*, uint32_t, float, struct font_glyph*);
void FontFace_FreeGlyph(struct font_glyph*);