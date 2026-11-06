#include <include/orca.h>
#include <assert.h>
#include <stdio.h>

/* Private renderer seams: decoding is exercised without creating a GL context. */
extern byte_t *R_DecodeJPEG_RGB(struct AXbuffer *, int *, int *);
extern byte_t *R_DecodeImageRGBA(void const *, uint32_t, int *, int *);
extern void R_FreeImage(byte_t *);
extern struct AXsize R_ImageSize(struct AXbuffer *);

static struct AXbuffer
read_fixture(char const *path)
{
  FILE *file = fopen(path, "rb");
  assert(file);
  assert(!fseek(file, 0, SEEK_END));
  long size = ftell(file);
  assert(size > 0 && size < 1000000);
  rewind(file);
  byte_t *bytes = malloc(size);
  assert(bytes && fread(bytes, 1, size, file) == (size_t)size);
  fclose(file);
  return (struct AXbuffer){ bytes, (int)size, (int)size, 0 };
}

int
main(void)
{
  int width, height;
  struct AXbuffer gray = read_fixture("tests/fixtures/image-decoding/gray.jpg");
  byte_t *pixels = R_DecodeJPEG_RGB(&gray, &width, &height);
  assert(pixels && width == 2 && height == 2);
  for (int i = 0; i < 12; i++) assert(pixels[i] >= 94 && pixels[i] <= 98);
  free(pixels);
  gray.cursize /= 2;
  assert(!R_DecodeJPEG_RGB(&gray, &width, &height));
  free(gray.data);

  struct AXbuffer cmyk = read_fixture("tests/fixtures/image-decoding/cmyk.jpg");
  pixels = R_DecodeJPEG_RGB(&cmyk, &width, &height);
  assert(pixels && width == 2 && height == 2);
  for (int i = 0; i < 4; i++)
    assert(pixels[i * 3] > 245 && pixels[i * 3 + 1] < 10 && pixels[i * 3 + 2] < 10);
  free(pixels);
  free(cmyk.data);

  byte_t corrupt[] = { 0xff, 0xd8, 0xff, 0xe2, 0x00, 0x20, 0x00, 0x00 };
  struct AXbuffer bad = { corrupt, sizeof corrupt, sizeof corrupt, 0 };
  struct AXsize size = R_ImageSize(&bad);
  assert(!size.width && !size.height);
  assert(!R_DecodeJPEG_RGB(&bad, &width, &height));
  assert(!width && !height);

  struct AXbuffer png = read_fixture("tests/fixtures/image-decoding/palette-alpha.png");
  pixels = R_DecodeImageRGBA(png.data, png.cursize, &width, &height);
  assert(pixels && width == 2 && height == 1);
  assert(pixels[0] == 0 && pixels[1] == 0 && pixels[2] == 0 && pixels[3] == 0);
  assert(pixels[4] == 0 && pixels[5] == 128 && pixels[6] == 0 && pixels[7] == 128);
  R_FreeImage(pixels);
  assert(!R_DecodeImageRGBA(png.data, png.cursize - 12, &width, &height));
  free(png.data);
  puts("PASS: grayscale and CMYK JPEG, palette transparency and malformed images");
  return 0;
}
