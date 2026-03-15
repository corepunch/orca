#ifndef __PLUGAPI_H__
#define __PLUGAPI_H__

#include <include/orca.h>
#include <include/version.h>

/*
 * Base ClassIDs for plugin categories.
 *
 * To create a plugin of a given category, list the corresponding ID in the
 * plugin's ClassDesc.ParentClasses array.  The engine locates all plugins of
 * a category at runtime using OBJ_EnumClassesW(ID_FileSystem, ...) etc.
 *
 *   ID_FileSystem — fnv1a32("FileSystem")
 *   ID_Renderer   — fnv1a32("Renderer")
 */
#define ID_FileSystem 0xe12a3bca
#define ID_Renderer   0x594a391e

/*
 * Message IDs for ID_FileSystem plugin procs.
 *
 * Each filesystem plugin sets ClassDesc.ObjProc to a proc that responds to
 * these messages.  ParentClasses must include ID_FileSystem.
 *
 *   kMsgPackageLoad — open a package.
 *     userData: NULL
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
 * Message IDs for ID_Renderer plugin procs.
 *
 * A renderer plugin sets ClassDesc.ObjProc to a proc that responds to these
 * messages.  ParentClasses must include ID_Renderer.
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
