#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <include/orca.h>
#include "test_local.h"

/* Renderer-internal helper; the test links it from liborca like any other
 * internal symbol so it needs no GL context. */
extern void *texture_downscale(void const *src, int w, int h, int bpp, int dw, int dh);

static void test_box_average_halves_square(void)
{
  uint8_t const src[16] = { 0, 2, 10, 12,
                            4, 6, 14, 16,
                            100, 100, 200, 200,
                            100, 100, 200, 200 };
  uint8_t *dst = texture_downscale(src, 4, 4, 1, 2, 2);
  RUN_TEST("box average halves a square alpha8 image", {
    EXPECT(dst);
    EXPECT(dst[0] == 3 && dst[1] == 13);
    EXPECT(dst[2] == 100 && dst[3] == 200);
  });
  free(dst);
}

static void test_only_tall_axis_shrinks(void)
{
  /* A 2x6 column pair shrinks to 2x3: columns stay distinct, rows pair up. */
  uint8_t src[12];
  for (int y = 0; y < 6; y++) { src[y * 2] = 10 * y; src[y * 2 + 1] = 255; }
  uint8_t *dst = texture_downscale(src, 2, 6, 1, 2, 3);
  RUN_TEST("per-axis clamp keeps the narrow axis intact", {
    EXPECT(dst);
    EXPECT(dst[0] == 5 && dst[2] == 25 && dst[4] == 45);
    EXPECT(dst[1] == 255 && dst[3] == 255 && dst[5] == 255);
  });
  free(dst);
}

static void test_uneven_ratio_covers_every_row(void)
{
  /* 1x5 → 1x2: windows [0,2) and [2,5) so the last rows are not dropped. */
  uint8_t const src[5] = { 10, 30, 60, 60, 90 };
  uint8_t *dst = texture_downscale(src, 1, 5, 1, 1, 2);
  RUN_TEST("uneven ratio averages all source rows", {
    EXPECT(dst);
    EXPECT(dst[0] == 20 && dst[1] == 70);
  });
  free(dst);
}

static void test_rgba_channels_average_separately(void)
{
  uint8_t const src[16] = { 255, 0, 0, 255,   0, 255, 0, 255,
                            0, 0, 255, 255,    255, 255, 255, 255 };
  uint8_t *dst = texture_downscale(src, 2, 2, 4, 1, 1);
  RUN_TEST("rgba pixels average per channel", {
    EXPECT(dst);
    EXPECT(dst[0] == 127 && dst[1] == 127 && dst[2] == 127 && dst[3] == 255);
  });
  free(dst);
}

int main(void)
{
  setbuf(stdout, NULL);
  test_box_average_halves_square();
  test_only_tall_axis_shrinks();
  test_uneven_ratio_covers_every_row();
  test_rgba_channels_average_separately();
  printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
  return s_tests_failed ? 1 : 0;
}
