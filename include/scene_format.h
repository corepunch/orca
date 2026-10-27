#ifndef ORCA_SCENE_FORMAT_H
#define ORCA_SCENE_FORMAT_H

/* Define ORCA_USE_LISP to opt into Lisp; ordinary builds use XML. */
#ifdef ORCA_USE_LISP
#define ORCA_HAS_LISP 1
#define ORCA_HAS_XML 0
#else
#define ORCA_HAS_LISP 0
#define ORCA_HAS_XML 1
#endif

#if ORCA_HAS_LISP
#define ORCA_SCENE_EXTENSION ".lisp"
#define ORCA_LOAD_SCENE_STRING FS_LoadObjectFromLispString
#define ORCA_SERIALIZE_SCENE FS_SerializeObjectToLispString
#else
#define ORCA_SCENE_EXTENSION ".xml"
#define ORCA_LOAD_SCENE_STRING FS_LoadObjectFromXmlString
#define ORCA_SERIALIZE_SCENE FS_SerializeObjectToXmlString
#endif

#endif
