#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <include/orca.h>
#include <include/renderer.h>
#include <include/renderer_font.h>
#include <UIKit/UIKit.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <plugins/UIKit/TextBlockText.h>

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_renderer(lua_State *L);
extern int luaopen_orca_UIKit(lua_State *L);
/* Exercise renderer shutdown while a FontFamily still owns a face. */
extern struct fontface *Font_Load(char const *, struct FontFamily *);
extern void Font_Shutdown(void);

static void
check_font_lifetime(void)
{
  char const *path = "share/fonts/NotoSans/NotoSans-Regular.ttf";
  struct Object *first = OBJ_Create(ID_FontFamily);
  struct fontface *old = Font_Load(path, GetFontFamily(first));
  assert(old);
  uint32_t const glyph = FontFace_GetGlyphIndex(old, 'A');
  assert(glyph);
  Font_Shutdown();
  struct font_metrics metrics;
  assert(FontFace_GetMetrics(old, 20, 1, &metrics));
  assert(FontFace_GetGlyphAdvance(old, glyph, metrics.raster_scale) > 0);

  struct Object *second = OBJ_Create(ID_FontFamily);
  struct fontface *fresh = Font_Load(path, GetFontFamily(second));
  assert(fresh);
  OBJ_ReleaseRef(first);
  assert(FontFace_GetGlyphIndex(fresh, 'A'));
  Font_Shutdown();
  assert(FontFace_GetMetrics(fresh, 20, 1, &metrics));
  OBJ_ReleaseRef(second);
}

static void
check_sequences(void)
{
  struct { char const *text; size_t length; } const cases[] = {
    { "👍", 4 },
    { "👍🏽 ok", 8 },                       // skin tone
    { "👨‍👩‍👧", 18 },                      // ZWJ family
    { "❤️‍🔥", 13 },                         // ZWJ with a selector inside
    { "🇺🇦🇵🇱", 8 },                        // one flag at a time
    { "1️⃣", 7 },                           // keycap
    { "❤️", 6 },                            // pictograph with U+FE0F
    { "⭐", 3 },                            // emoji presentation without a selector
    { "🏳️‍🌈", 14 },
    { "🏴󠁧󠁢󠁳󠁣󠁴󠁿", 28 },                        // tag sequence (Scotland)
    { "👍\xE2\x80\x8D🐱", 4 },                // unknown ZWJ pair falls apart
    { "❤", 0 }, { "❤︎", 0 },                // text presentation
    { "1", 0 }, { "#", 0 }, { "©", 0 }, { "a", 0 }, { "Й", 0 }, { "", 0 },
  };
  for (size_t i = 0; i < sizeof(cases) / sizeof(*cases); i++) {
    size_t const length = Font_EmojiLength(cases[i].text);
    if (length != cases[i].length)
      fprintf(stderr, "case %zu \"%s\": %zu != %zu\n", i, cases[i].text, length, cases[i].length);
    assert(length == cases[i].length);
  }
}

static struct text_info
measure(char const *value, float scale)
{
  struct TextBlockText text = {
    .numTextRuns = 1, .scale = scale,
    .run = {{ .string = value, .fontSize = 20 }},
  };
  struct text_info info;
  assert(SUCCEEDED(TextBlockText_GetInfo(&text, &info)));
  return info;
}

static void
check_layout(void)
{
  int const advance = Font_GetEmojiAdvance(20) >> 16;
  assert(advance >= 22 && advance <= 26);
  int const plain = measure("ab", 1).txWidth;
  assert(measure("a👍b", 1).txWidth == plain + advance);
  assert(measure("a👨‍👩‍👧b", 1).txWidth == plain + advance);
  assert(measure("a👍🏽b", 1).txWidth == plain + advance);

  // Wrapping counts emoji: four of them do not fit in three emoji widths.
  struct TextBlockText text = {
    .numTextRuns = 1, .scale = 1, .availableWidth = advance * 3,
    .textWrapping = TEXT_WRAP_WRAP, .run = {{ .string = "👍👍👍👍", .fontSize = 20 }},
  };
  struct text_info info;
  assert(SUCCEEDED(TextBlockText_GetInfo(&text, &info)));
  assert(info.txHeight > measure("👍", 1).txHeight);

  // The cursor counts code points: after the 7 of a family + ZWJ it sits past the emoji.
  text = (struct TextBlockText){
    .numTextRuns = 1, .scale = 1, .cursor = 6,
    .run = {{ .string = "a👨‍👩‍👧", .fontSize = 20 }},
  };
  assert(SUCCEEDED(TextBlockText_GetInfo(&text, &info)));
  assert(info.cursor.x == measure("a👨‍👩‍👧", 1).txWidth);
}

static void
check_layers(void)
{
  struct TextBlockText text = {
    .numTextRuns = 1, .scale = 2,
    .run = {{ .string = "Hi 👍", .fontSize = 20 }},
  };
  struct AXsize size;
  unsigned char *bitmap, *emoji;
  assert(SUCCEEDED(TextBlockText_RasterizeLayers(&text, &size, &bitmap, &emoji)));
  assert(emoji);
  // The thumb is yellow: red and green ink with little blue, right of the text.
  int ink = 0, left = size.width;
  for (int i = 0; i < size.width * size.height; i++) {
    unsigned char const *px = emoji + i * 4;
    if (px[3] > 200 && px[0] > 200 && px[1] > 150 && px[2] < 120) {
      ink++;
      left = MIN(left, i % size.width);
    }
    assert(px[0] <= px[3] && px[1] <= px[3] && px[2] <= px[3]);   // premultiplied
  }
  assert(ink > 200);
  assert(left > size.width / 3);
  free(bitmap);
  free(emoji);

  text.run[0].string = "No emoji here";
  assert(SUCCEEDED(TextBlockText_RasterizeLayers(&text, &size, &bitmap, &emoji)));
  assert(!emoji);
  free(bitmap);
}

/* Arrows and backspace step over a whole emoji sequence. */
static void
check_input(void)
{
  char const *family = "👨‍👩‍👧";
  struct Object *input = OBJ_Create(ID_Input);
  _SendMessage(input, Node, TextInput, .text = "a");
  _SendMessage(input, Node, TextInput, .text = family);
  _SendMessage(input, Node, TextInput, .text = "👍🏽");
  int const length = (int)strlen(GetTextRun(input)->Text);
  _SendMessage(input, Node, KeyDown, .keyCode = AX_KEY_LEFTARROW);
  assert(GetInput(input)->Cursor == 1 + (int)strlen(family));
  _SendMessage(input, Node, KeyDown, .keyCode = AX_KEY_LEFTARROW);
  assert(GetInput(input)->Cursor == 1);
  _SendMessage(input, Node, KeyDown, .keyCode = AX_KEY_RIGHTARROW);
  _SendMessage(input, Node, KeyDown, .keyCode = AX_KEY_RIGHTARROW);
  assert(GetInput(input)->Cursor == length);
  _SendMessage(input, Node, KeyDown, .keyCode = AX_KEY_BACKSPACE);
  _SendMessage(input, Node, KeyDown, .keyCode = AX_KEY_BACKSPACE);
  assert(!strcmp(GetTextRun(input)->Text, "a"));
  OBJ_ReleaseRef(input);
}

/* Composite black text and the emoji layer on white, for a visual check. */
static void
save_preview(char const *path)
{
  enum { width = 900, height = 320 };
  unsigned char *pixels = malloc(width * height * 3);
  memset(pixels, 255, width * height * 3);
  char const *lines[] = { "Hello 👋 world 🌍!", "👨‍👩‍👧 🏳️‍🌈 🇺🇦 1️⃣ ❤️‍🔥 👍🏽 ⭐", "Small 😀 text 🎉 inline 🚀" };
  float const sizes[] = { 24, 24, 15 };
  for (int row = 0; row < 3; row++) {
    struct TextBlockText text = {
      .numTextRuns = 1, .scale = 3,
      .run = {{ .string = lines[row], .fontSize = sizes[row] }},
    };
    struct AXsize size;
    unsigned char *bitmap, *emoji;
    assert(SUCCEEDED(TextBlockText_RasterizeLayers(&text, &size, &bitmap, &emoji)));
    int const top = 10 + row * 105;
    for (int y = 0; y < size.height && top + y < height; y++)
      for (int x = 0; x < size.width && x + 10 < width; x++) {
        int const i = x + (size.height - y - 1) * size.width;
        unsigned char *p = pixels + ((top + y) * width + x + 10) * 3;
        for (int k = 0; k < 3; k++) {
          int v = 255 - bitmap[i];
          if (emoji) v = emoji[i * 4 + k] + v * (255 - emoji[i * 4 + 3]) / 255;
          p[k] = (unsigned char)v;
        }
      }
    free(bitmap);
    free(emoji);
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
  check_sequences();
  check_layout();
  check_layers();
  check_input();
  char const *preview = getenv("ORCA_TEXT_EMOJI_PREVIEW");
  if (preview) save_preview(preview);
  check_font_lifetime();
  lua_close(L);
  puts("PASS: emoji sequences, advances, wrapping, cursor, Input editing and the RGBA emoji layer");
  return 0;
}
