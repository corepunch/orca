#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <include/orca.h>
#include <include/renderer.h>
#include <UIKit/UIKit.h>
#include <assert.h>
#include <stdio.h>
// The shared raster/upload seam exposes prepared text for device-scale pixel checks.
#include <plugins/UIKit/TextBlockText.h>

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_renderer(lua_State *L);
extern int luaopen_orca_UIKit(lua_State *L);

static int
first_ink(unsigned char const *bitmap, struct AXsize size)
{
  for (int y = 0; y < size.height; y++)
    for (int x = 0; x < size.width; x++)
      if (bitmap[x + (size.height - y - 1) * size.width]) return y;
  return -1;
}

static void
check_raster(char const *value, int scale, int width, bool_t mixed)
{
  struct TextBlockText text = {
    .numTextRuns = mixed ? 2 : 1, .scale = scale,
    .availableWidth = width, .textWrapping = TEXT_WRAP_WRAP,
    .run = {{ .string = value, .fontSize = 20 }},
  };
  if (mixed) text.run[1] = (struct TextBlockTextRun){ .string = "ÉЙ larger\nsecond line", .fontSize = 28 };
  struct AXsize full_size, trimmed_size;
  unsigned char *full, *trimmed;
  struct text_info full_info, trimmed_info;
  assert(SUCCEEDED(TextBlockText_GetInfo(&text, &full_info)));
  assert(SUCCEEDED(TextBlockText_Rasterize(&text, &full_size, &full)));
  int const leading = first_ink(full, full_size);
  assert(leading > 0);
  text.trimLeading = TRUE;
  assert(SUCCEEDED(TextBlockText_GetInfo(&text, &trimmed_info)));
  assert(trimmed_info.txInsets.top == 0 && trimmed_info.txInsets.bottom == 0);
  assert(SUCCEEDED(TextBlockText_Rasterize(&text, &trimmed_size, &trimmed)));
  assert(trimmed_size.width == full_size.width);
  assert(first_ink(trimmed, trimmed_size) == 0);
  assert(trimmed_info.txHeight < full_info.txHeight);
  // Exact crop equality proves glyph shape, accents and all interline spacing survive.
  for (int y = 0; y < MAX(full_size.height - leading, trimmed_size.height); y++)
    for (int x = 0; x < full_size.width; x++) {
      unsigned char a = y < trimmed_size.height
        ? trimmed[x + (trimmed_size.height - y - 1) * trimmed_size.width] : 0;
      unsigned char b = y + leading < full_size.height
        ? full[x + (full_size.height - y - leading - 1) * full_size.width] : 0;
      assert(a == b);
    }
  free(full);
  free(trimmed);
}

static void
check_blank_lines(void)
{
  struct TextBlockText text = {
    .numTextRuns = 1, .scale = 1, .trimLeading = TRUE,
    .run = {{ .string = "\nHello", .fontSize = 20 }},
  };
  struct AXsize size;
  unsigned char *bitmap;
  assert(SUCCEEDED(TextBlockText_Rasterize(&text, &size, &bitmap)));
  assert(first_ink(bitmap, size) > 20);
  free(bitmap);
  text.run[0].string = "   ";
  struct text_info info;
  assert(SUCCEEDED(TextBlockText_GetInfo(&text, &info)));
  assert(info.txWidth == 0 && info.txHeight > 0);
  text.run[0].string = "";
  assert(SUCCEEDED(TextBlockText_GetInfo(&text, &info)));
  assert(info.txWidth == 0);
}

static void
save_preview(char const *path)
{
  enum { width = 1040, height = 400 };
  unsigned char *pixels = malloc(width * height * 3);
  memset(pixels, 255, width * height * 3);
  char const *labels[] = { "Raven Software's Jedi Academy", "Хаос «Русского мира» в Донецке", "ÉÀÅЙ accents and lowercase" };
  for (int row = 0; row < 3; row++) {
    int top = 20 + row * 128;
    for (int y = top; y < top + 88; y++)
      for (int x = 20; x < 220; x++) {
        if (x >= 108 && x < 132) continue;
        unsigned char *p = pixels + (y * width + x) * 3;
        p[0] = 15; p[1] = 143; p[2] = 137;
      }
    struct TextBlockText text = {
      .numTextRuns = 1, .scale = 2, .trimLeading = TRUE,
      .run = {{ .string = labels[row], .fontSize = 20 }},
    };
    struct AXsize size;
    unsigned char *bitmap;
    assert(SUCCEEDED(TextBlockText_Rasterize(&text, &size, &bitmap)));
    for (int y = 0; y < size.height && top + y < height; y++)
      for (int x = 0; x < size.width && x + 244 < width; x++) {
        unsigned char alpha = bitmap[x + (size.height - y - 1) * size.width];
        memset(pixels + ((top + y) * width + x + 244) * 3, 255 - alpha, 3);
      }
    free(bitmap);
  }
  FILE *file = fopen(path, "wb");
  assert(file);
  fprintf(file, "P6\n%d %d\n255\n", width, height);
  assert(fwrite(pixels, 3, width * height, file) == width * height);
  fclose(file);
  free(pixels);
}

int
main(void)
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  assert(luaL_dostring(L, "package.path = 'share/?.lua;share/?/init.lua;' .. package.path") == LUA_OK);
  luaL_requiref(L, "orca", luaopen_orca, 1);
  luaL_requiref(L, "orca.core", luaopen_orca_core, 1);
  luaL_requiref(L, "orca.renderer", luaopen_orca_renderer, 1);
  luaL_requiref(L, "orca.UIKit", luaopen_orca_UIKit, 1);
  struct Object *object = OBJ_Create(ID_TextBlock);
  assert(!GetTextBlockConcept(object)->TrimLeading);
  bool_t trim = TRUE;
  PROP_SetValue(OBJ_FindLongProperty(object, ID_TextBlockConcept_TrimLeading), &trim);
  _SendMessage(object, TextBlockConcept, MakeText, .availableSpace = 300);
  assert(GetTextBlockConcept(object)->_text->trimLeading);
  OBJ_ReleaseRef(object);
  for (int scale = 1; scale <= 3; scale++) {
    check_raster("Hello World\nSecond line", scale, 400, FALSE);
    check_raster("Хаос Русского мира\nВторая строка", scale, 400, FALSE);
    check_raster("ÉÀÅЙ éàå\nAccents stay intact", scale, 400, FALSE);
    check_raster("   lower case words wrap across lines", scale, 100, FALSE);
    check_raster("small", scale, 400, TRUE);
  }
  check_blank_lines();
  char const *preview = getenv("ORCA_TEXT_LEADING_PREVIEW");
  if (preview) save_preview(preview);
  lua_close(L);
  puts("PASS: exact leading crop, unchanged glyphs/line spacing, mixed runs, whitespace and wrapping at 1x/2x/3x");
  return 0;
}
