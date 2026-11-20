/*
 * Color emoji from the bundled Twemoji set (share/emoji/twemoji.emj, made by
 * tools/pack-emoji.py). Text layout asks Font_EmojiLength() whether an emoji
 * starts at a position, and Font_RenderEmoji() for its premultiplied RGBA
 * image at the run's em size. Without the asset file no text is an emoji.
 */
#include "r_local.h"
#include <include/renderer_font.h>

extern byte_t* R_DecodeImageRGBA(void const*, uint32_t, int*, int*);
extern void R_FreeImage(byte_t*);

#define EMOJI_MAGIC MAKE_FOURCC('E', 'M', 'J', '1')
#define EMOJI_BOX 1.15f       /* image side, in em */
#define EMOJI_SPACING 0.04f   /* side bearing on each side, in em */
#define EMOJI_DESCENT 0.2f    /* part of the image below the baseline */

struct emoji_entry
{
  uint32_t key_offset;
  uint32_t key_length;
  uint32_t png_offset;
  uint32_t png_length;
};

struct emoji_image
{
  byte_t* rgba;
  int width;
  int height;
};

static struct _EMOJIGLOBALS
{
  struct file* pack;
  struct emoji_entry const* entries;
  struct emoji_image* images;
  uint32_t count;
  bool_t tried;
} eg = { 0 };

static bool_t
E_LoadPack(void)
{
  if (eg.tried) return eg.pack != NULL;
  eg.tried = TRUE;
  lpcString_t const paths[] = { CORE_FindFontFamily("emoji"),
                                "build/share/emoji/twemoji.emj", "share/emoji/twemoji.emj" };
  FOR_LOOP(i, 3) if (paths[i] && (eg.pack = FS_LoadFile(paths[i]))) break;
  if (!eg.pack) return FALSE;

  uint32_t const* header = (uint32_t const*)eg.pack->data;
  if (eg.pack->size < 12 || header[0] != EMOJI_MAGIC
      || 12 + (uint64_t)header[1] * sizeof(struct emoji_entry) > eg.pack->size)
  {
    Con_Error("EMOJI: invalid twemoji.emj");
    SafeDelete(eg.pack, FS_FreeFile);
    return FALSE;
  }
  eg.count = header[1];
  eg.entries = (struct emoji_entry const*)(header + 3);
  eg.images = ZeroAlloc(eg.count * sizeof(*eg.images));
  return TRUE;
}

/* ── Code points ──────────────────────────────────────────────── */

static uint32_t
E_Read(lpcString_t* text)
{
  byte_t const* s = (byte_t const*)*text;
  uint32_t ch = s[0];
  int n = ch < 0x80 ? 0 : ch < 0xE0 ? 1 : ch < 0xF0 ? 2 : 3;
  if (n) ch &= 0x3F >> n;
  for (int i = 1; i <= n; i++) {
    if ((s[i] & 0xC0) != 0x80) { *text += i; return 0xFFFD; }
    ch = ch << 6 | (s[i] & 0x3F);
  }
  *text += n + 1;
  return ch;
}

/* Emoji_Presentation=Yes outside the regional-indicator pairs: these show as
 * emoji without U+FE0F. Other pictographs need the selector, as keyboards send. */
static bool_t
E_IsPresentation(uint32_t ch)
{
  static uint32_t const bmp[][2] = {
    { 0x231A, 0x231B }, { 0x23E9, 0x23EC }, { 0x23F0, 0x23F0 }, { 0x23F3, 0x23F3 },
    { 0x25FD, 0x25FE }, { 0x2614, 0x2615 }, { 0x2648, 0x2653 }, { 0x267F, 0x267F },
    { 0x2693, 0x2693 }, { 0x26A1, 0x26A1 }, { 0x26AA, 0x26AB }, { 0x26BD, 0x26BE },
    { 0x26C4, 0x26C5 }, { 0x26CE, 0x26CE }, { 0x26D4, 0x26D4 }, { 0x26EA, 0x26EA },
    { 0x26F2, 0x26F3 }, { 0x26F5, 0x26F5 }, { 0x26FA, 0x26FA }, { 0x26FD, 0x26FD },
    { 0x2705, 0x2705 }, { 0x270A, 0x270B }, { 0x2728, 0x2728 }, { 0x274C, 0x274C },
    { 0x274E, 0x274E }, { 0x2753, 0x2755 }, { 0x2757, 0x2757 }, { 0x2795, 0x2797 },
    { 0x27B0, 0x27B0 }, { 0x27BF, 0x27BF }, { 0x2B1B, 0x2B1C }, { 0x2B50, 0x2B50 },
    { 0x2B55, 0x2B55 },
  };
  if (ch >= 0x1F000 && ch <= 0x1FAFF) return TRUE;
  FOR_LOOP(i, sizeof(bmp) / sizeof(*bmp)) {
    if (ch >= bmp[i][0] && ch <= bmp[i][1]) return TRUE;
  }
  return FALSE;
}

static bool_t
E_IsKeycapBase(uint32_t ch)
{
  return (ch >= '0' && ch <= '9') || ch == '#' || ch == '*';
}

static bool_t
E_IsRegional(uint32_t ch)
{
  return ch >= 0x1F1E6 && ch <= 0x1F1FF;
}

/* Bytes of the longest emoji sequence the grammar allows at text, or 0. */
static size_t
E_SequenceLength(lpcString_t text)
{
  lpcString_t p = text;
  uint32_t const base = E_Read(&p);
  lpcString_t after = p;
  uint32_t const next = *p ? E_Read(&after) : 0;

  if (E_IsRegional(base)) {
    return E_IsRegional(next) ? (size_t)(after - text) : (size_t)(p - text);
  }
  if (E_IsKeycapBase(base)) {
    if (next == 0xFE0F && *after) {
      lpcString_t cap = after;
      if (E_Read(&cap) == 0x20E3) return (size_t)(cap - text);
    }
    return next == 0x20E3 ? (size_t)(after - text) : 0;
  }
  if (next == 0xFE0E) return 0;
  if (!E_IsPresentation(base) && next != 0xFE0F) return 0;

  /* Variation selectors, skin tones, tags, and ZWJ-joined pictographs. */
  for (;;) {
    lpcString_t q = p;
    uint32_t const ch = *q ? E_Read(&q) : 0;
    if (ch == 0xFE0F || ch == 0x20E3 || (ch >= 0x1F3FB && ch <= 0x1F3FF)
        || (ch >= 0xE0020 && ch <= 0xE007F))
    {
      p = q;
    } else if (ch == 0x200D && *q) {
      lpcString_t r = q;
      if (E_Read(&r) < 0x2190) break;
      p = r;
    } else {
      break;
    }
  }
  return (size_t)(p - text);
}

/* ── Lookup ───────────────────────────────────────────────────── */

static int
E_Compare(struct emoji_entry const* e, char const* key, size_t length)
{
  size_t const n = MIN(e->key_length, length);
  int const c = memcmp(eg.pack->data + e->key_offset, key, n);
  return c ? c : (int)e->key_length - (int)length;
}

/* Keys drop U+FE0F, like tools/pack-emoji.py. */
static int
E_Find(lpcString_t text, size_t length)
{
  char key[128];
  size_t n = 0;
  for (size_t i = 0; i < length; i++) {
    if (i + 3 <= length && !memcmp(text + i, "\xEF\xB8\x8F", 3)) { i += 2; continue; }
    if (n == sizeof(key)) return -1;
    key[n++] = text[i];
  }
  int lo = 0, hi = (int)eg.count - 1;
  while (lo <= hi) {
    int const mid = (lo + hi) / 2;
    int const c = E_Compare(&eg.entries[mid], key, n);
    if (!c) return mid;
    if (c < 0) lo = mid + 1; else hi = mid - 1;
  }
  return -1;
}

/* Shorten an unknown sequence one code point at a time, so a ZWJ sequence
 * newer than the set falls apart into the emoji it joins. */
static int
E_Match(lpcString_t text, size_t* length)
{
  size_t n = E_SequenceLength(text);
  while (n > 0) {
    int const index = E_Find(text, n);
    if (index >= 0) return *length = n, index;
    do n--; while (n > 0 && (text[n] & 0xC0) == 0x80);
    while (n >= 3 && !memcmp(text + n - 3, "\xE2\x80\x8D", 3)) n -= 3;
  }
  return -1;
}

size_t
Font_EmojiLength(lpcString_t text)
{
  size_t length = 0;
  byte_t const lead = text ? (byte_t)*text : 0;
  /* Every emoji starts with a keycap base or a code point from U+2190 up. */
  if (lead < 0xE2 && !E_IsKeycapBase(lead)) return 0;
  if (!E_LoadPack()) return 0;
  return E_Match(text, &length) >= 0 ? length : 0;
}

/* ── Rendering ────────────────────────────────────────────────── */

static int
E_Box(float em)
{
  return MAX(1, (int)lroundf(em * EMOJI_BOX));
}

int32_t
Font_GetEmojiAdvance(float em)
{
  int const bearing = (int)lroundf(em * EMOJI_SPACING);
  return (int32_t)(E_Box(em) + 2 * bearing) << 16;
}

static struct emoji_image const*
E_Image(int index)
{
  struct emoji_image* image = &eg.images[index];
  if (image->rgba) return image;
  struct emoji_entry const* e = &eg.entries[index];
  if ((uint64_t)e->png_offset + e->png_length > eg.pack->size) return NULL;
  image->rgba = R_DecodeImageRGBA(eg.pack->data + e->png_offset, e->png_length,
                                  &image->width, &image->height);
  return image->rgba ? image : NULL;
}

/* Premultiplied bilinear sample, clamped to the image. */
static void
E_Sample(struct emoji_image const* img, float x, float y, float out[4])
{
  x = MAX(0, MIN(x, img->width - 1.0f));
  y = MAX(0, MIN(y, img->height - 1.0f));
  int const x0 = (int)x, y0 = (int)y;
  int const x1 = MIN(x0 + 1, img->width - 1), y1 = MIN(y0 + 1, img->height - 1);
  float const fx = x - x0, fy = y - y0;
  byte_t const* a = img->rgba + (y0 * img->width + x0) * 4;
  byte_t const* b = img->rgba + (y0 * img->width + x1) * 4;
  byte_t const* c = img->rgba + (y1 * img->width + x0) * 4;
  byte_t const* d = img->rgba + (y1 * img->width + x1) * 4;
  FOR_LOOP(k, 4) {
    out[k] += (a[k] * (1 - fx) + b[k] * fx) * (1 - fy) + (c[k] * (1 - fx) + d[k] * fx) * fy;
  }
}

int
Font_RenderEmoji(lpcString_t text, size_t length, float em, struct font_emoji* out)
{
  size_t matched = 0;
  if (!out || !E_LoadPack()) return FALSE;
  memset(out, 0, sizeof(*out));
  int const index = E_Match(text, &matched);
  struct emoji_image const* img = index >= 0 && matched == length ? E_Image(index) : NULL;
  if (!img) return FALSE;

  int const box = E_Box(em);
  out->rgba = ZeroAlloc((size_t)box * box * 4);
  if (!out->rgba) return FALSE;
  out->width = out->height = box;
  out->xoff = (int)lroundf(em * EMOJI_SPACING);
  out->yoff = -(int)lroundf(box * (1 - EMOJI_DESCENT));

  /* 4x4 bilinear taps per pixel: a box filter when shrinking, smooth when growing. */
  float const sx = (float)img->width / box, sy = (float)img->height / box;
  for (int y = 0; y < box; y++) for (int x = 0; x < box; x++) {
    float sum[4] = { 0 };
    for (int j = 0; j < 4; j++) for (int i = 0; i < 4; i++) {
      E_Sample(img, (x + (i + 0.5f) / 4) * sx - 0.5f, (y + (j + 0.5f) / 4) * sy - 0.5f, sum);
    }
    byte_t* px = out->rgba + (y * box + x) * 4;
    FOR_LOOP(k, 4) px[k] = (byte_t)MIN(255, lroundf(sum[k] / 16));
  }
  return TRUE;
}

void
Font_FreeEmoji(struct font_emoji* emoji)
{
  if (!emoji) return;
  free(emoji->rgba);
  emoji->rgba = NULL;
}

void
Emoji_Shutdown(void)
{
  for (uint32_t i = 0; eg.images && i < eg.count; i++) R_FreeImage(eg.images[i].rgba);
  free(eg.images);
  SafeDelete(eg.pack, FS_FreeFile);
  memset(&eg, 0, sizeof(eg));
}
