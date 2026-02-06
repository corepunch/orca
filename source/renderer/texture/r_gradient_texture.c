#include "../r_local.h"

#define GRADIENT_SIZE 16

static int lerp_color(struct color a, struct color b, float t) {
  struct color c = COLOR_Lerp(&a, &b, t);
  return *(int*)&(struct color32) {
    .r = c.r*255,
    .g = c.g*255,
    .b = c.b*255,
    .a = c.a*255
  };
}

struct Texture*
R_MakeGradientTexture(float angle, struct color a, struct color b)
{
  struct Texture* pImage = NULL;
  uint32_t data[GRADIENT_SIZE][GRADIENT_SIZE];
  //  FOR_LOOP(x, GRADIENT_SIZE) {
  //    FOR_LOOP(y, GRADIENT_SIZE) {
  //      data[y][x] = lerp_color(a, b, (float)(x)/(float)(GRADIENT_SIZE-1));
  //    }
  //  }
  float dirX = cosf(angle);
  float dirY = sinf(angle);
  
  // Normalize to fit inside 0..1 range over the texture
  float halfSize = (float)(GRADIENT_SIZE - 1) / 2.0f;
  
  FOR_LOOP(x, GRADIENT_SIZE) {
    FOR_LOOP(y, GRADIENT_SIZE) {
      float fx = (float)x - halfSize;
      float fy = (float)y - halfSize;
      
      // Dot product with direction vector
      float projection = (fx * dirX + fy * dirY) / halfSize;
      
      // Remap from [-1, 1] to [0, 1]
      float t = projection * 0.5f + 0.5f;
      t = CLAMP(t, 0.0f, 1.0f);  // Optional if you want hard clamping at borders
      
      data[y][x] = lerp_color(a, b, t);
    }
  }
  
  Texture_Create(&(CREATEIMGSTRUCT){
    .Width = GRADIENT_SIZE,
    .Height = GRADIENT_SIZE,
    .Format = kTextureFormatRgba8,
    .ImageData = data,
    .MagFilter = kTextureFilterLinear,
    .MinFilter = kTextureFilterLinear,
    .WrapMode = kTextureWrapClamp,
  }, &pImage);
  return pImage;
}
