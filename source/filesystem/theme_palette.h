#ifndef ORCA_THEME_PALETTE_H
#define ORCA_THEME_PALETTE_H

#include <include/orca.h>
#include <filesystem/filesystem.h>

#define THEME_COLOR_ACCENT              "$accent"
#define THEME_COLOR_ACCENT_BACKGROUND   "$accent-background"
#define THEME_COLOR_ACCENT_FOREGROUND   "$accent-foreground"
#define THEME_COLOR_BACKGROUND          "$background"
#define THEME_COLOR_FOREGROUND          "$foreground"
#define THEME_COLOR_PANEL_BACKGROUND    "$panel-background"
#define THEME_COLOR_SURFACE             "$surface"
#define THEME_COLOR_SURFACE_DEEP        "$surface-deep"
#define THEME_COLOR_CONTROL_BACKGROUND  "$control-background"
#define THEME_COLOR_CONTROL_FOREGROUND  "$control-foreground"
#define THEME_COLOR_CONTROL_BORDER      "$control-border"
#define THEME_COLOR_CONTROL_MUTED       "$control-muted"
#define THEME_COLOR_CARD_BACKGROUND     "$card-background"

static inline struct color
FS_GetThemeColor(lpcString_t key)
{
  lpcString_t value = key ? FS_GetThemeValue(key) : NULL;
  return value ? COLOR_Parse(value) : ((struct color){0});
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
