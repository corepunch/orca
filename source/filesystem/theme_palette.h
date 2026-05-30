#ifndef ORCA_THEME_PALETTE_H
#define ORCA_THEME_PALETTE_H

#include <include/orca.h>
#include <filesystem/filesystem.h>

#define THEME_COLOR_ACCENT              "$accent"
#define THEME_COLOR_ACCENT_BACKGROUND   "$accent-background"
#define THEME_COLOR_ACCENT_FOREGROUND   "$accent-foreground"
#define THEME_COLOR_PANEL_BACKGROUND    "$panel-background"
#define THEME_COLOR_CONTROL_BACKGROUND  "$control-background"
#define THEME_COLOR_CONTROL_FOREGROUND  "$control-foreground"
#define THEME_COLOR_CONTROL_BORDER      "$control-border"
#define THEME_COLOR_CONTROL_MUTED       "$control-muted"
#define THEME_COLOR_CARD_BACKGROUND     "$card-background"

#define COLOR_ACCENT                    ((struct color){0.3f, 0.55f, 0.85f, 1.0f})
#define COLOR_ACCENT_BACKGROUND         ((struct color){0.24f, 0.36f, 0.58f, 1.0f})
#define COLOR_ACCENT_CHECKED            ((struct color){0.898f, 0.561f, 0.133f, 1.0f})
#define COLOR_ACCENT_FOREGROUND         ((struct color){1.0f, 1.0f, 1.0f, 1.0f})
#define COLOR_PANEL_BACKGROUND          ((struct color){0.10f, 0.10f, 0.16f, 0.95f})
#define COLOR_CONTROL_BACKGROUND        ((struct color){0.18f, 0.19f, 0.22f, 0.95f})
#define COLOR_CONTROL_BACKGROUND_SOLID  ((struct color){0.18f, 0.19f, 0.22f, 1.0f})
#define COLOR_CONTROL_FOREGROUND        ((struct color){0.95f, 0.95f, 0.97f, 1.0f})
#define COLOR_CONTROL_BORDER            ((struct color){0.7f, 0.7f, 0.75f, 1.0f})
#define COLOR_CARD_BACKGROUND           ((struct color){0.95f, 0.95f, 0.97f, 1.0f})

static inline lpcString_t
FS_GetThemeValue2(lpcString_t key1, lpcString_t key2)
{
  lpcString_t value = key1 ? FS_GetThemeValue(key1) : NULL;
  return value ? value : (key2 ? FS_GetThemeValue(key2) : NULL);
}

static inline struct color
FS_GetThemeColor(lpcString_t key, struct color fallback)
{
  lpcString_t value = key ? FS_GetThemeValue(key) : NULL;
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
