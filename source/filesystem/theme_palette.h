#ifndef ORCA_THEME_PALETTE_H
#define ORCA_THEME_PALETTE_H

#include <include/orca.h>
#include <source/filesystem/filesystem.h>

#define THEME_COLOR_ACCENT              "$accent"
#define THEME_COLOR_ACCENT_BACKGROUND   "$accent-background"
#define THEME_COLOR_ACCENT_FOREGROUND   "$accent-foreground"
#define THEME_COLOR_CONTROL_BACKGROUND  "$control-background"
#define THEME_COLOR_CONTROL_FOREGROUND  "$control-foreground"
#define THEME_COLOR_CONTROL_BORDER      "$control-border"
#define THEME_COLOR_CONTROL_MUTED       "$control-muted"
#define THEME_COLOR_CARD_BACKGROUND     "$card-background"

static inline lpcString_t
FS_GetThemeValue2(lpcString_t key1, lpcString_t key2)
{
  lpcString_t value = key1 ? FS_GetThemeValue(key1) : NULL;
  return value ? value : (key2 ? FS_GetThemeValue(key2) : NULL);
}

static inline struct color
FS_GetThemeColorOr(lpcString_t key, struct color fallback)
{
  lpcString_t value = key ? FS_GetThemeValue(key) : NULL;
  return value ? COLOR_Parse(value) : fallback;
}

static inline struct color
FS_GetThemeColorOr2(lpcString_t key1, lpcString_t key2, struct color fallback)
{
  lpcString_t value = FS_GetThemeValue2(key1, key2);
  return value ? COLOR_Parse(value) : fallback;
}

static inline struct color
FS_GetThemeColorOr3(lpcString_t key1, lpcString_t key2, lpcString_t key3, struct color fallback)
{
  lpcString_t value = FS_GetThemeValue2(key1, key2);
  if (!value && key3) value = FS_GetThemeValue(key3);
  return value ? COLOR_Parse(value) : fallback;
}

static inline bool_t
FS_IsZeroColor(struct color const *color)
{
  return color->a == 0.0f &&
         color->r == 0.0f &&
         color->g == 0.0f &&
         color->b == 0.0f;
}

#endif
