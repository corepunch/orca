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
 * A ClassDesc with this SuperClassID must set its ObjProc to a
 * PackageLoaderProc that handles the kMsgPackage* and kMsgFile* messages
 * defined below.
 */
#define SCLASS_FILESYSTEM 0x00000003

/*
 * Message IDs for SCLASS_FILESYSTEM plugin procs.
 *
 * Each filesystem plugin implements ClassDesc.ObjProc as a PackageLoaderProc
 * (same signature as objectProc_t) and responds to these messages:
 *
 *   kMsgPackageLoad — open a package.
 *     userData: NULL (no existing pack)
 *     wParam:   (lpcString_t) base directory/path passed to FS_AddSearchPath
 *     lParam:   (void**) output — set *lParam to the opaque pack handle on success
 *     returns:  TRUE on success, FALSE if the format is not recognised
 *
 *   kMsgPackageFree — release a previously loaded pack.
 *     userData: opaque pack handle returned by kMsgPackageLoad
 *     wParam/lParam: unused
 *
 *   kMsgFileRead — read a file from the pack.
 *     userData: opaque pack handle
 *     wParam:   (lpcString_t) relative filename
 *     lParam:   (struct file**) output — set *lParam to the allocated file on success
 *     returns:  TRUE on success
 *
 *   kMsgFileFind — check whether a file exists in the pack.
 *     userData: opaque pack handle
 *     wParam:   (lpcString_t) relative filename
 *     returns:  TRUE if the file exists, FALSE otherwise
 */
#define kMsgPackageLoad 0x49355a7b  /* fnv1a32("PackageLoad") */
#define kMsgPackageFree 0x7f274ae1  /* fnv1a32("PackageFree") */
#define kMsgFileRead    0x163a2bb7  /* fnv1a32("FileRead")    */
#define kMsgFileFind    0x2e2f1280  /* fnv1a32("FileFind")    */

/*
 * Message IDs for SCLASS_RENDERER plugin procs.
 *
 * A renderer plugin sets ClassDesc.ObjProc to a RendererProc and responds to
 * these messages:
 *
 *   kMsgRendererCreate — allocate and initialise a renderer instance.
 *     userData: NULL
 *     wParam/lParam: platform-specific window/context info
 *     returns:  TRUE on success
 *
 *   kMsgRendererDestroy — tear down and free a renderer instance.
 *     userData: renderer instance
 *     wParam/lParam: unused
 *
 *   kMsgRendererFrame — render one frame.
 *     userData: renderer instance
 *     wParam:   (lpObject_t) scene root
 *     lParam:   unused
 */
#define kMsgRendererCreate  0x3a0ee4da  /* fnv1a32("RendererCreate")  */
#define kMsgRendererDestroy 0x5aef7fe0  /* fnv1a32("RendererDestroy") */
#define kMsgRendererFrame   0x1225a18f  /* fnv1a32("RendererFrame")   */

#endif
