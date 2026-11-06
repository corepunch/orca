#ifndef ORCA_SCENE_FORMAT_H
#define ORCA_SCENE_FORMAT_H

#define ORCA_SCENE_EXTENSION ".xml"
#define ORCA_SCENE_TEMPLATE \
  "<Screen Name=\"Main\" Width=\"1024\" Height=\"768\">\n" \
  "  <TextBlock Text=\"Hello, %s!\" FontSize=\"48\" LayoutTransform=\"400 350 0 1 1\"/>\n" \
  "</Screen>\n"

/* Standalone CLI configuration must not depend on the dynamically loaded engine. */
static inline const char *orca_scene_extension(void) { return ORCA_SCENE_EXTENSION; }
static inline const char *orca_scene_template(void) { return ORCA_SCENE_TEMPLATE; }

#endif
