#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <include/orca.h>
#include <source/core/core_local.h>
#include <UIKit/UIKit.h>

#include "TextBlockText.h"

// MarkdownView: a CommonMark subset rendered as TextBlocks with Run children.
#define MD_MAX_DEPTH 8

enum { MD_STRONG = 1, MD_EM = 2, MD_STRIKE = 4, MD_CODE = 8, MD_LINK = 16 };

struct md_buf {
  char *s;
  size_t n, cap;
};

struct md_text {
  struct Object *block;
  char const *start;
  struct md_buf buf;
  uint32_t style;
  int runs;
};

static void
MD_Append(struct md_buf *b, char const *s, size_t n)
{
  if (b->n + n + 1 > b->cap) {
    size_t cap = MAX(64, (b->n + n + 1) * 2);
    char *s2 = realloc(b->s, cap);
    if (!s2) return;
    b->s = s2, b->cap = cap;
  }
  memcpy(b->s + b->n, s, n);
  b->n += n;
  b->s[b->n] = 0;
}

static void
MD_Set(struct Object *o, uint32_t id, void const *value)
{
  struct Property *p = OBJ_FindLongProperty(o, id);
  if (p) PROP_SetValue(p, value);
}

static void
MD_SetText(struct Object *o, lpcString_t text)
{
  struct Property *p = o ? OBJ_FindLongProperty(o, ID_TextRun_Text) : NULL;
  if (p) PROP_SetStringValue(p, text);
}

// Creates a non-interactive part under parent, which keeps the only reference.
static struct Object *
MD_Part(struct Object *parent, uint32_t class_id, lpcString_t classes)
{
  struct Object *o = OBJ_Create(class_id);
  if (!o) return NULL;
  bool_t ignore = TRUE;
  MD_Set(o, ID_Node2D_IgnoreHitTest, &ignore);
  OBJ_AddChild(parent, o);
  OBJ_ReleaseRef(o);
  // Attached first, so the classes resolve against the page's stylesheet.
  if (classes && *classes) _SendMessage(o, StyleController, AddClasses, .ClassNames = classes);
  return o;
}

static char const *
MD_EOL(char const *p, char const *end)
{
  char const *e = memchr(p, '\n', end - p);
  return e ? e : end;
}

static char const *
MD_Next(char const *e, char const *end)
{
  return e < end ? e + 1 : end;
}

static char const *
MD_SkipSpaces(char const *p, char const *end)
{
  while (p < end && *p == ' ') p++;
  return p;
}

static size_t
MD_RunLength(char const *p, char const *end)
{
  char const *q = p;
  while (q < end && *q == *p) q++;
  return q - p;
}

/* Inline */

static void
MD_Flush(struct md_text *t)
{
  if (!t->buf.n) return;
  char classes[64] = {0};
  snprintf(classes, sizeof(classes), "%s%s%s%s%s",
           t->style & MD_STRONG ? "md-strong " : "", t->style & MD_EM ? "md-em " : "",
           t->style & MD_STRIKE ? "md-strike " : "", t->style & MD_CODE ? "md-code " : "",
           t->style & MD_LINK ? "md-link" : "");
  struct Object *run = MD_Part(t->block, ID_Run, classes);
  if (run) {
    MD_SetText(run, t->buf.s);
    enum FontWeight weight = kFontWeightBold;
    enum FontStyle italic = kFontStyleItalic;
    enum TextDecoration line = t->style & MD_LINK ? kTextDecorationUnderline : kTextDecorationStrikethrough;
    if (t->style & MD_STRONG) MD_Set(run, ID_TextRun_FontWeight, &weight);
    if (t->style & MD_EM) MD_Set(run, ID_TextRun_FontStyle, &italic);
    if (t->style & (MD_LINK | MD_STRIKE)) MD_Set(run, ID_TextRun_TextDecorationType, &line);
  }
  t->buf.n = 0;
  t->runs++;
}

static void
MD_Put(struct md_text *t, char const *s, size_t n, uint32_t style)
{
  // Past the TextBlock's run limit the rest keeps the last style.
  if (style != t->style && t->runs < UI_MAX_TEXT_RUNS - 2) {
    MD_Flush(t);
    t->style = style;
  }
  MD_Append(&t->buf, s, n);
}

static void MD_Inline(struct md_text *t, char const *p, char const *end, uint32_t style, int depth);

// Finds the closing bracket for the opener at p, skipping escapes and nesting.
static char const *
MD_Match(char const *p, char const *end, char open, char close)
{
  int nesting = 0;
  for (; p < end; p++) {
    if (*p == '\\') { p++; continue; }
    if (*p == open) nesting++;
    else if (*p == close && --nesting == 0) return p;
  }
  return NULL;
}

static char const *
MD_CodeSpan(struct md_text *t, char const *p, char const *end, uint32_t style)
{
  size_t n = MD_RunLength(p, end);
  for (char const *q = p + n; q < end; ) {
    size_t m = MD_RunLength(q, end);
    if (*q != '`') { q++; continue; }
    if (m != n) { q += m; continue; }
    char const *s = p + n, *e = q;
    if (e - s >= 2 && *s == ' ' && e[-1] == ' ') s++, e--;
    for (char const *c = s; c < e; c++) MD_Put(t, *c == '\n' ? " " : c, 1, style | MD_CODE);
    return q + n;
  }
  MD_Put(t, p, n, style);
  return p + n;
}

static char const *
MD_Link(struct md_text *t, char const *p, char const *end, uint32_t style, int depth)
{
  bool_t image = *p == '!';
  char const *open = p + image;
  char const *close = MD_Match(open, end, '[', ']');
  char const *paren = close && close + 1 < end && close[1] == '(' ? MD_Match(close + 1, end, '(', ')') : NULL;
  if (!paren) {
    MD_Put(t, p, open + 1 - p, style);
    return open + 1;
  }
  MD_Inline(t, open + 1, close, image ? style : style | MD_LINK, depth + 1);
  return paren + 1;
}

static char const *
MD_Emphasis(struct md_text *t, char const *p, char const *end, uint32_t style, int depth)
{
  char c = *p;
  size_t n = MD_RunLength(p, end);
  bool_t valid = (c == '~' ? n == 2 : n <= 3) && p + n < end && !isspace((unsigned char)p[n]) &&
                 (c != '_' || p == t->start || !isalnum((unsigned char)p[-1]));
  for (char const *q = p + n; valid && q < end; ) {
    if (*q == '\\') { q += 2; continue; }
    if (*q != c) { q++; continue; }
    size_t m = MD_RunLength(q, end);
    if (m == n && !isspace((unsigned char)q[-1]) && (c != '_' || q + n >= end || !isalnum((unsigned char)q[n]))) {
      uint32_t flags = c == '~' ? MD_STRIKE : n == 1 ? MD_EM : n == 2 ? MD_STRONG : MD_STRONG | MD_EM;
      MD_Inline(t, p + n, q, style | flags, depth + 1);
      return q + n;
    }
    q += m;
  }
  MD_Put(t, p, n, style);
  return p + n;
}

static bool_t
MD_IsURL(char const *p, char const *end)
{
  static char const *const prefixes[] = { "https://", "http://", "www." };
  for (size_t i = 0; i < sizeof(prefixes) / sizeof(*prefixes); i++) {
    size_t n = strlen(prefixes[i]);
    if ((size_t)(end - p) > n && !strncmp(p, prefixes[i], n)) return TRUE;
  }
  return FALSE;
}

static char const *
MD_BareURL(struct md_text *t, char const *p, char const *end, uint32_t style)
{
  char const *e = p;
  while (e < end && !isspace((unsigned char)*e) && *e != '<') e++;
  while (e > p && strchr(".,:;!?\"')", e[-1])) e--;
  MD_Put(t, p, e - p, style | MD_LINK);
  return e;
}

static void
MD_Inline(struct md_text *t, char const *p, char const *end, uint32_t style, int depth)
{
  while (p < end) {
    char c = *p;
    char const *q;
    if (c == '\\' && p + 1 < end && (p[1] == '\n' || ispunct((unsigned char)p[1]))) {
      MD_Put(t, p[1] == '\n' ? "\n" : p + 1, 1, style);
      p = p[1] == '\n' ? MD_SkipSpaces(p + 2, end) : p + 2;
    } else if (c == ' ') {
      // Two trailing spaces make a hard break; other line-end spaces go.
      for (q = p; q < end && *q == ' '; q++);
      if (q == end) return;
      if (*q == '\n') {
        MD_Put(t, q - p >= 2 ? "\n" : " ", 1, style);
        p = MD_SkipSpaces(q + 1, end);
      } else {
        MD_Put(t, p, q - p, style);
        p = q;
      }
    } else if (c == '\n') {
      MD_Put(t, " ", 1, style);
      p = MD_SkipSpaces(p + 1, end);
    } else if (c == '`') {
      p = MD_CodeSpan(t, p, end, style);
    } else if (depth < MD_MAX_DEPTH && (c == '[' || (c == '!' && p + 1 < end && p[1] == '['))) {
      p = MD_Link(t, p, end, style, depth);
    } else if (depth < MD_MAX_DEPTH && (c == '*' || c == '_' || c == '~')) {
      p = MD_Emphasis(t, p, end, style, depth);
    } else if (c == '<' && (q = memchr(p, '>', end - p)) && MD_IsURL(p + 1, q) &&
               !memchr(p, ' ', q - p) && !memchr(p, '\n', q - p)) {
      MD_Put(t, p + 1, q - p - 1, style | MD_LINK);
      p = q + 1;
    } else if ((p == t->start || strchr(" \n(", p[-1])) && MD_IsURL(p, end)) {
      p = MD_BareURL(t, p, end, style);
    } else {
      MD_Put(t, p, 1, style);
      p++;
    }
  }
}

static struct Object *
MD_Text(struct Object *parent, lpcString_t classes, char const *s, char const *e, bool_t bold)
{
  struct Object *block = MD_Part(parent, ID_TextBlock, classes);
  if (!block) return NULL;
  enum FontWeight weight = kFontWeightBold;
  if (bold) MD_Set(block, ID_TextRun_FontWeight, &weight);
  struct md_text t = { .block = block, .start = s };
  MD_Inline(&t, s, e, 0, 0);
  MD_Flush(&t);
  free(t.buf.s);
  return block;
}

/* Blocks */

static int
MD_Heading(char const *s, char const *e, char const **ts, char const **te)
{
  size_t n = MD_RunLength(s, e);
  if (*s != '#' || n > 6 || (s + n < e && s[n] != ' ')) return 0;
  char const *b = MD_SkipSpaces(s + n, e), *z = e;
  while (z > b && z[-1] == ' ') z--;
  char const *hashes = z;
  while (hashes > b && hashes[-1] == '#') hashes--;
  if (hashes == b || hashes[-1] == ' ') z = hashes;
  while (z > b && z[-1] == ' ') z--;
  *ts = b, *te = z;
  return (int)n;
}

static bool_t
MD_IsBreak(char const *s, char const *e)
{
  int count = 0;
  if (!strchr("-*_", *s)) return FALSE;
  for (char const *c = s; c < e; c++) {
    if (*c == *s) count++;
    else if (*c != ' ') return FALSE;
  }
  return count >= 3;
}

static size_t
MD_Fence(char const *s, char const *e)
{
  size_t n = MD_RunLength(s, e);
  if ((*s != '`' && *s != '~') || n < 3) return 0;
  return *s == '`' && memchr(s + n, '`', e - s - n) ? 0 : n;
}

// Returns the marker width including its following spaces, or 0.
static size_t
MD_ListMarker(char const *s, char const *e, bool_t *ordered, int *number)
{
  char const *m = s;
  if (strchr("-*+", *s)) {
    *ordered = FALSE;
    m++;
  } else {
    int value = 0;
    while (m < e && m - s < 9 && isdigit((unsigned char)*m)) value = value * 10 + (*m++ - '0');
    if (m == s || m >= e || (*m != '.' && *m != ')')) return 0;
    *ordered = TRUE, *number = value;
    m++;
  }
  if (m < e && *m != ' ') return 0;
  char const *c = MD_SkipSpaces(m, e);
  return (c == e || c - m > 4 ? m + 1 : c) - s;
}

static bool_t
MD_IsSetext(char const *s, char const *e, int *level)
{
  if (*s != '=' && *s != '-') return FALSE;
  char const *z = s + MD_RunLength(s, e);
  if (MD_SkipSpaces(z, e) != e) return FALSE;
  *level = *s == '=' ? 1 : 2;
  return TRUE;
}

// Whether the line starts a block that interrupts a paragraph.
static bool_t
MD_Interrupts(char const *s, char const *e)
{
  char const *ts, *te;
  bool_t ordered;
  int number = 1;
  size_t width = MD_ListMarker(s, e, &ordered, &number);
  return *s == '>' || MD_Heading(s, e, &ts, &te) || MD_Fence(s, e) || MD_IsBreak(s, e) ||
         (width && s + width < e && (!ordered || number == 1));
}

static void MD_Blocks(struct Object *parent, char const *p, char const *end, int depth);

static char const *
MD_CodeBlock(struct Object *parent, char const *p, char const *s, char const *end)
{
  char const *e = MD_EOL(p, end);
  size_t fence = MD_Fence(s, e), indent = s - p;
  struct md_buf code = {0};
  for (p = MD_Next(e, end); p < end; p = MD_Next(e, end)) {
    e = MD_EOL(p, end);
    char const *c = MD_SkipSpaces(p, e);
    if (*c == *s && MD_RunLength(c, e) >= fence && MD_SkipSpaces(c + MD_RunLength(c, e), e) == e) {
      p = MD_Next(e, end);
      break;
    }
    char const *from = MIN(p + indent, c);
    if (code.n) MD_Append(&code, "\n", 1);
    MD_Append(&code, from, e - from);
  }
  struct Object *pre = MD_Part(parent, ID_StackView, "md-pre");
  struct Object *block = pre ? MD_Part(pre, ID_TextBlock, "md-code-block") : NULL;
  if (code.s) MD_SetText(block, code.s);
  free(code.s);
  return p;
}

static char const *
MD_Quote(struct Object *parent, char const *p, char const *end, int depth)
{
  struct md_buf inner = {0};
  bool_t lazy = FALSE;
  while (p < end) {
    char const *e = MD_EOL(p, end), *s = MD_SkipSpaces(p, e);
    if (s < e && *s == '>') {
      s++;
      if (s < e && *s == ' ') s++;
      lazy = MD_SkipSpaces(s, e) < e;
    } else if (!lazy || s == e || MD_Interrupts(s, e)) {
      break;
    }
    MD_Append(&inner, s, e - s);
    MD_Append(&inner, "\n", 1);
    p = MD_Next(e, end);
  }
  struct Object *quote = MD_Part(parent, ID_StackView, "md-quote");
  if (quote && inner.s) MD_Blocks(quote, inner.s, inner.s + inner.n, depth + 1);
  free(inner.s);
  return p;
}

static char const *
MD_List(struct Object *parent, char const *p, char const *end, int depth)
{
  struct Object *list = NULL;
  bool_t ordered = FALSE, first_ordered = FALSE;
  int number = 0, counter = 0;
  while (p < end) {
    char const *e = MD_EOL(p, end), *s = MD_SkipSpaces(p, e);
    size_t marker = s < e ? MD_ListMarker(s, e, &ordered, &number) : 0;
    if (!marker || (list && ordered != first_ordered)) break;
    if (!list) {
      list = MD_Part(parent, ID_StackView, ordered ? "md-list md-ol" : "md-list md-ul");
      first_ordered = ordered, counter = number;
      if (!list) return end;
    }
    // Lines indented to the content column, and lazy lines, belong to the item.
    size_t column = (s - p) + marker;
    struct md_buf body = {0};
    MD_Append(&body, MIN(s + marker, e), e - MIN(s + marker, e));
    MD_Append(&body, "\n", 1);
    bool_t blank = FALSE;
    for (p = MD_Next(e, end); p < end; p = MD_Next(e, end)) {
      e = MD_EOL(p, end);
      char const *c = MD_SkipSpaces(p, e);
      if (c == e) {
        blank = TRUE;
      } else if ((size_t)(c - p) >= column) {
        c = p + column, blank = FALSE;
      } else if (blank || MD_Interrupts(c, e) || MD_ListMarker(c, e, &ordered, &number)) {
        break;
      }
      MD_Append(&body, c, e - c);
      MD_Append(&body, "\n", 1);
    }

    struct Object *item = MD_Part(list, ID_Grid, "md-li");
    if (item) {
      struct Property *columns = OBJ_FindLongProperty(item, ID_Grid_Columns);
      if (columns) PROP_SetStringValue(columns, "auto 1fr");
      char label[16] = "\xE2\x80\xA2";  // U+2022 bullet
      if (first_ordered) snprintf(label, sizeof(label), "%d.", counter++);
      MD_SetText(MD_Part(item, ID_TextBlock, "md-marker"), label);
      struct Object *content = MD_Part(item, ID_StackView, "md-li-body");
      if (content) MD_Blocks(content, body.s, body.s + body.n, depth + 1);
    }
    free(body.s);
  }
  return p;
}

static void
MD_Blocks(struct Object *parent, char const *p, char const *end, int depth)
{
  while (p < end) {
    char const *e = MD_EOL(p, end), *s = MD_SkipSpaces(p, e), *ts, *te;
    bool_t ordered;
    int level, number;
    if (s == e) {
      p = MD_Next(e, end);
    } else if (depth >= MD_MAX_DEPTH) {
      MD_Text(parent, "md-p", s, end, FALSE);
      return;
    } else if (MD_Fence(s, e)) {
      p = MD_CodeBlock(parent, p, s, end);
    } else if ((level = MD_Heading(s, e, &ts, &te))) {
      char classes[8];
      snprintf(classes, sizeof(classes), "md-h%d", level);
      MD_Text(parent, classes, ts, te, TRUE);
      p = MD_Next(e, end);
    } else if (MD_IsBreak(s, e)) {
      struct Object *hr = MD_Part(parent, ID_Node2D, "md-hr");
      float height = 1;
      if (hr) MD_Set(hr, ID_Node_Height, &height);
      p = MD_Next(e, end);
    } else if (*s == '>') {
      p = MD_Quote(parent, p, end, depth);
    } else if (MD_ListMarker(s, e, &ordered, &number)) {
      p = MD_List(parent, p, end, depth);
    } else {
      // A paragraph runs to a blank line, an interrupting block or a setext underline.
      char const *last = e;
      level = 0;
      for (p = MD_Next(e, end); p < end; p = MD_Next(e, end)) {
        e = MD_EOL(p, end);
        char const *c = MD_SkipSpaces(p, e);
        if (c == e) break;
        if (MD_IsSetext(c, e, &level)) { p = MD_Next(e, end); break; }
        if (MD_Interrupts(c, e)) break;
        last = e;
      }
      if (level) {
        char classes[8];
        snprintf(classes, sizeof(classes), "md-h%d", level);
        MD_Text(parent, classes, s, last, TRUE);
      } else {
        MD_Text(parent, "md-p", s, last, FALSE);
      }
    }
  }
}

static void
MD_Rebuild(struct Object *hObject, struct MarkdownView *view)
{
  lpcString_t source = view->Source ? view->Source : "";
  uint32_t hash = fnv1a32(source);
  if (hash == view->_hash && OBJ_GetFirstChild(hObject)) return;
  view->_hash = hash;
  OBJ_Clear(hObject);

  // Normalize line endings and tabs once so the parser sees only '\n' and spaces.
  struct md_buf text = {0};
  for (char const *c = source; *c; c++) {
    if (*c == '\r') MD_Append(&text, "\n", 1), c += c[1] == '\n';
    else if (*c == '\t') MD_Append(&text, "    ", 4);
    else MD_Append(&text, c, 1);
  }
  if (text.s) MD_Blocks(hObject, text.s, text.s + text.n, 0);
  free(text.s);
}

HANDLER(MarkdownView, Object, Start)
{
  MD_Rebuild(hObject, pMarkdownView);
  return FALSE;
}

HANDLER(MarkdownView, Object, PropertyChanged)
{
  if (pPropertyChanged->Property &&
      PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_MarkdownView_Source)
    MD_Rebuild(hObject, pMarkdownView);
  return FALSE;
}
