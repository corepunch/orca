#ifndef __PLUGAPI_H__
#define __PLUGAPI_H__

#include <include/orca.h>
#include <include/version.h>

/*
 * Superclass IDs identify the category of a plugin, similar to 3ds Max's
 * SuperClassID system.  Set ClassDesc.SuperClassID to one of these values to
 * declare what kind of plugin the ClassDesc provides.  The engine (and other
 * subsystems) use OBJ_EnumClassesBySuperClass() to locate all registered
 * plugins of a particular category at runtime.
 */

/** Generic / uncategorised plugin. */
#define SCLASS_GENERIC    0x00000000

/** UI / scene node classes (buttons, panels, 3D nodes, …). */
#define SCLASS_NODE       0x00000001

/** Renderer plugins. */
#define SCLASS_RENDERER   0x00000002

/**
 * Filesystem / package-format loader plugins.
 *
 * A ClassDesc with this SuperClassID must set its ClassData field to a pointer
 * to a PackageLoaderDesc (defined below) that implements the four
 * load/free/read/find operations for the package format it handles.
 */
#define SCLASS_FILESYSTEM 0x00000003

/**
 * PackageLoaderDesc — interface for package-format plugins.
 *
 * A ClassDesc with SuperClassID == SCLASS_FILESYSTEM must set its ClassData
 * field to a pointer to one of these structs.  The filesystem module calls
 * these callbacks when opening, reading, and releasing packages.
 */
typedef struct PackageLoaderDesc {
  /** Try to open a package at the given file path.
   *  Returns an opaque handle on success, NULL if the format is not supported
   *  or the file does not exist. */
  void*        (*LoadPackage)(const char* path);
  /** Release a handle returned by LoadPackage. */
  void         (*FreePackage)(void* pack);
  /** Read a named file from the package.
   *  Returns an allocated struct file* on success, NULL if not found. */
  struct file* (*ReadFile)(const char* filename, void* pack);
  /** Check whether a named file exists in the package. */
  bool_t       (*FindFile)(const char* filename, void* pack);
} PackageLoaderDesc_t, *lpPackageLoaderDesc_t;
typedef PackageLoaderDesc_t const *lpcPackageLoaderDesc_t;

#endif
