// Instagram-style colour grades, ported from the image editor's GLSL presets
// (mapview/ui/apps/imageeditor/share/filters) at full strength. Each filter
// maps one sRGB pixel in [0, 1] and its texture coordinate; vignettes use the
// distance from the centre, so the bitmap's row order does not matter.

#include <math.h>
#include <string.h>
#include "media_native.h"

typedef float vec3[3];
typedef void (*filter_fn)(vec3 c, float u, float v);

static float clampf(float x) { return x < 0 ? 0 : x > 1 ? 1 : x; }
static float mixf(float a, float b, float t) { return a + (b - a) * t; }
static float lum(vec3 c) { return c[0] * .299f + c[1] * .587f + c[2] * .114f; }
static float vignette(float u, float v, float sx, float k) {
  float x = (u - .5f) * sx, y = v - .5f;
  return 1 - (x * x + y * y) * k;
}
static void clamp3(vec3 c) { for (int i = 0; i < 3; i++) c[i] = clampf(c[i]); }
// c = mix(c, c * m + a, t)
static void grade(vec3 c, float t, float mr, float mg, float mb, float ar, float ag, float ab) {
  float m[3] = { mr, mg, mb }, a[3] = { ar, ag, ab };
  for (int i = 0; i < 3; i++) c[i] = mixf(c[i], c[i] * m[i] + a[i], t);
}
// c = mix(c, vec3(lum(c)) * s + o, t)
static void desaturate(vec3 c, float t, float s, float o) {
  float l = lum(c) * s + o;
  for (int i = 0; i < 3; i++) c[i] = mixf(c[i], l, t);
}
static void smooth(vec3 c, float t) {
  for (int i = 0; i < 3; i++) c[i] = mixf(c[i], c[i] * c[i] * (3 - 2 * c[i]), t);
}
static void gamma3(vec3 c, float g) { for (int i = 0; i < 3; i++) c[i] = powf(c[i], g); }
static void scale3(vec3 c, float s) { for (int i = 0; i < 3; i++) c[i] *= s; }

static void f_normal(vec3 c, float u, float v) { (void)c; (void)u; (void)v; }
static void f_aden(vec3 c, float u, float v) {
  (void)u; (void)v;
  grade(c, .6f, .9f, 1, 1.1f, 0, .05f, .08f);
  desaturate(c, .15f, 1, 0);
  grade(c, 1, .95f, .95f, .95f, .04f, .04f, .04f);
}
static void f_brannan(vec3 c, float u, float v) {
  (void)u; (void)v;
  desaturate(c, .2f, 1, 0);
  grade(c, 1, .95f, .9f, 1, .04f, 0, .06f);
}
static void f_clarendon(vec3 c, float u, float v) {
  (void)u; (void)v;
  grade(c, .6f, .8f, .95f, 1.05f, .05f, .02f, -.02f);
  clamp3(c);
  float l = lum(c);
  for (int i = 0; i < 3; i++) c[i] = mixf(c[i], mixf(1, c[i], .8f) * l, .25f);
  clamp3(c);
  grade(c, 1, 1, 1, 1, -.02f, 0, .06f);
}
static void f_crema(vec3 c, float u, float v) {
  (void)u; (void)v;
  grade(c, .6f, 1, .97f, .88f, .05f, .03f, .02f);
  desaturate(c, .08f, 1, 0);
  grade(c, 1, .92f, .92f, .92f, .06f, .06f, .06f);
}
static void f_earlybird(vec3 c, float u, float v) {
  scale3(c, vignette(u, v, 1, 1));
  grade(c, 1, 1.05f, .98f, .88f, .04f, .02f, 0);
}
static void f_gingham(vec3 c, float u, float v) {
  (void)u; (void)v;
  grade(c, 1, 1.05f, 1.05f, 1.05f, -.02f, -.02f, -.02f);
  grade(c, .4f, 1, .97f, .9f, 0, 0, 0);
  desaturate(c, .1f, 1, 0);
}
static void f_hefe(vec3 c, float u, float v) {
  scale3(c, vignette(u, v, 1, 1.4f));
  grade(c, 1, 1.06f, 1, .88f, 0, 0, 0);
  smooth(c, .2f);
}
static void f_hudson(vec3 c, float u, float v) {
  scale3(c, vignette(u, v, 1, 1.2f));
  grade(c, 1, .9f, .95f, 1.1f, 0, .02f, .08f);
}
static void f_inkwell(vec3 c, float u, float v) {
  (void)u; (void)v;
  float l = powf(lum(c), 1.1f) * 1.1f - .04f;
  c[0] = c[1] = c[2] = l;
}
static void f_juno(vec3 c, float u, float v) {
  (void)u; (void)v;
  grade(c, 1, 1.04f, 1, .9f, 0, 0, 0);
  float l = lum(c);
  for (int i = 0; i < 3; i++) c[i] += (1 - c[i]) * (1 - l) * .12f;
  clamp3(c);
  gamma3(c, .95f);
}
static void f_lark(vec3 c, float u, float v) {
  (void)u; (void)v;
  grade(c, 1, 1.02f, 1.03f, .95f, 0, 0, 0);
  smooth(c, .2f);
  grade(c, 1, 1, 1, 1, .04f, .03f, -.04f);
}
static void f_ludwig(vec3 c, float u, float v) {
  (void)u; (void)v;
  gamma3(c, .92f);
  grade(c, 1, 1.02f, 1, .96f, 0, 0, 0);
  grade(c, 1, 1.06f, 1.06f, 1.06f, -.03f, -.03f, -.03f);
}
static void f_mayfair(vec3 c, float u, float v) {
  scale3(c, vignette(u, v, 1, .7f));
  grade(c, .5f, 1.05f, 1.02f, .96f, 0, 0, 0);
}
static void f_moon(vec3 c, float u, float v) {
  (void)u; (void)v;
  float l = powf(lum(c), .9f) * 1.1f - .05f;
  c[0] = c[1] = c[2] = l;
}
static void f_nashville(vec3 c, float u, float v) {
  (void)u; (void)v;
  grade(c, 1, 1, .9f, .85f, .05f, .02f, 0);
  smooth(c, .15f);
}
static void f_perpetua(vec3 c, float u, float v) {
  (void)u; (void)v;
  c[0] = mixf(c[0], powf(c[0], .85f), .5f);
  c[2] = mixf(c[2], c[2] * 1.1f + .05f, .5f);
  c[1] = mixf(c[1], c[1] * 1.04f, .5f);
}
static void f_reyes(vec3 c, float u, float v) {
  (void)u; (void)v;
  desaturate(c, .1f, 1, 0);
  grade(c, 1, .9f, .9f, .9f, .1f, .1f, .1f);
  grade(c, .5f, 1.05f, 1.02f, .95f, 0, 0, 0);
}
static void f_rise(vec3 c, float u, float v) {
  float k = vignette(u, v, 1.2f, .5f);
  grade(c, 1, k, k, k, .06f * (1 - k), .03f * (1 - k), 0);
  grade(c, .5f, 1.04f, 1, .88f, .04f, .04f, .02f);
}
static void f_slumber(vec3 c, float u, float v) {
  (void)u; (void)v;
  grade(c, .7f, .9f, .85f, 1, .04f, 0, .06f);
  desaturate(c, .2f, .9f, .05f);
}
static void f_stinson(vec3 c, float u, float v) {
  (void)u; (void)v;
  grade(c, 1, .9f, .9f, .9f, .08f, .08f, .08f);
  grade(c, 1, 1, 1.02f, 1.04f, 0, 0, 0);
}
static void f_valencia(vec3 c, float u, float v) {
  (void)u; (void)v;
  grade(c, 1, 1.08f, .98f, .88f, .02f, 0, -.04f);
  desaturate(c, .06f, 1, 0);
}
static void f_vesper(vec3 c, float u, float v) {
  (void)u; (void)v;
  desaturate(c, .2f, 1, 0);
  grade(c, 1, 1, .95f, .88f, .04f, .02f, 0);
}
static void f_walden(vec3 c, float u, float v) {
  (void)u; (void)v;
  grade(c, 1, .92f, .95f, 1.05f, .04f, .04f, .06f);
  desaturate(c, .1f, 1, 0);
}

static struct { char const *id; filter_fn apply; } const filters[] = {
  { "normal", f_normal }, { "clarendon", f_clarendon }, { "gingham", f_gingham },
  { "moon", f_moon }, { "lark", f_lark }, { "reyes", f_reyes }, { "juno", f_juno },
  { "slumber", f_slumber }, { "crema", f_crema }, { "ludwig", f_ludwig },
  { "aden", f_aden }, { "perpetua", f_perpetua }, { "valencia", f_valencia },
  { "mayfair", f_mayfair }, { "rise", f_rise }, { "hudson", f_hudson },
  { "earlybird", f_earlybird }, { "brannan", f_brannan }, { "hefe", f_hefe },
  { "nashville", f_nashville }, { "stinson", f_stinson }, { "vesper", f_vesper },
  { "walden", f_walden }, { "inkwell", f_inkwell },
};

int Media_FilterCount(void) { return (int)(sizeof(filters) / sizeof(filters[0])); }

char const *Media_FilterId(int index) {
  return index >= 0 && index < Media_FilterCount() ? filters[index].id : NULL;
}

int Media_FindFilter(char const *id) {
  for (int i = 0; id && i < Media_FilterCount(); i++)
    if (!strcmp(filters[i].id, id)) return i;
  return -1;
}

void Media_FilterPixels(int index, unsigned char *rgba, size_t w, size_t h, size_t stride) {
  if (index <= 0 || index >= Media_FilterCount() || !rgba || !w || !h) return;
  filter_fn apply = filters[index].apply;
  for (size_t y = 0; y < h; y++) {
    unsigned char *p = rgba + y * stride;
    float v = (y + .5f) / h;
    for (size_t x = 0; x < w; x++, p += 4) {
      vec3 c = { p[0] / 255.f, p[1] / 255.f, p[2] / 255.f };
      apply(c, (x + .5f) / w, v);
      for (int i = 0; i < 3; i++) p[i] = (unsigned char)lrintf(clampf(c[i]) * 255);
    }
  }
}
