#ifndef ORCA_SCENE_FORMAT_H
#define ORCA_SCENE_FORMAT_H

/* Define ORCA_USE_LISP to opt into Lisp; ordinary builds use XML. */
#ifdef ORCA_USE_LISP
#define ORCA_HAS_LISP 1
#define ORCA_HAS_XML 0
#define ORCA_SCENE_EXTENSION ".lisp"
#define ORCA_SCENE_TEMPLATE \
  "(Screen Name=\"Main\" Width=1024 Height=768\n" \
  "  (TextBlock Text=\"Hello, %s!\" FontSize=48 LayoutTransform=\"400 350 0 1 1\"))\n"
#else
#define ORCA_HAS_LISP 0
#define ORCA_HAS_XML 1
#define ORCA_SCENE_EXTENSION ".xml"
#define ORCA_SCENE_TEMPLATE \
  "<Screen Name=\"Main\" Width=\"1024\" Height=\"768\">\n" \
  "  <TextBlock Text=\"Hello, %s!\" FontSize=\"48\" LayoutTransform=\"400 350 0 1 1\"/>\n" \
  "</Screen>\n"
#endif

/* Standalone CLI configuration must not depend on the dynamically loaded engine. */
static inline const char *orca_scene_extension(void) { return ORCA_SCENE_EXTENSION; }
static inline const char *orca_scene_template(void) { return ORCA_SCENE_TEMPLATE; }

#endif
