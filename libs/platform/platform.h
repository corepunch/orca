/**
 * @file platform.h
 * @brief Cross-platform window, input, and rendering abstraction layer.
 *
 * This header defines the public API for the platform library, providing
 * a unified interface for window management, event handling, OpenGL/EGL
 * rendering context management, file dialogs, and system utilities across
 * macOS, Linux (Wayland or X11), QNX, and WebGL (Emscripten) targets.
 *
 * All public symbols are exported via the #WI_API visibility macro.
 */

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "events.h"

#ifndef LOWORD
/** @brief Extract the low 16 bits of a 32-bit value. */
#define LOWORD(l) ((uint16_t)(l & 0xFFFF))
#endif

#ifndef HIWORD
/** @brief Extract the high 16 bits of a 32-bit value. */
#define HIWORD(l) ((uint16_t)((l >> 16) & 0xFFFF))
#endif

#ifndef MAKEDWORD
/** @brief Combine two 16-bit halves into a 32-bit value. */
#define MAKEDWORD(low, high) ((uint32_t)(((uint16_t)(low)) | ((uint32_t)((uint16_t)(high))) << 16))
#endif

#ifndef MAX
/** @brief Return the larger of two values. */
#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#endif

#ifndef MIN
/** @brief Return the smaller of two values. */
#define MIN(x, y) ((x) <= (y) ? (x) : (y))
#endif

#ifndef CLAMP
/** @brief Clamp @p x to the closed interval [@p min, @p max]. */
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#endif

/**
 * @brief Marks a function as part of the public platform API.
 *
 * On GCC/Clang this expands to `__attribute__((visibility("default")))`,
 * ensuring the symbol is exported from a shared library.  On Windows/MinGW
 * it expands to `__declspec(dllexport)` when building the library
 * (`PLATFORM_BUILD` defined) or `__declspec(dllimport)` when consuming it.
 * Define this macro before including the header to override the default.
 */
#ifndef WI_API
#  if defined(_WIN32) || defined(__MINGW32__)
#    ifdef PLATFORM_BUILD
#      define WI_API __declspec(dllexport)
#    else
#      define WI_API __declspec(dllimport)
#    endif
#  else
#    define WI_API __attribute__((visibility("default")))
#  endif
#endif

/** @defgroup types Basic types
 *  @{
 */
#ifndef TRUE
#define TRUE  1 /**< Boolean true value. */
#endif
#ifndef FALSE
#define FALSE 0 /**< Boolean false value. */
#endif
typedef unsigned int   bool_t;      /**< Boolean: `TRUE` (non-zero) or `FALSE` (0). */
typedef unsigned long  longTime_t;  /**< Millisecond timestamp / duration. */
typedef uint32_t       wParam_t;    /**< Word-sized event parameter. */
typedef void*          lParam_t;    /**< Pointer-sized event parameter. */
typedef unsigned char  byte_t;      /**< Single unsigned byte. */
/** @} */

/**
 * @defgroup keycodes Key codes
 * @brief Virtual key-code constants used in #WI_Message::wParam for key events.
 *
 * ASCII printable characters (32–126) map directly to their ASCII values.
 * Special keys start at 128 to avoid ambiguity.
 * @{
 */
enum
{
  WI_KEY_TAB        = 9,   /**< Tab key. */
  WI_KEY_ENTER      = 13,  /**< Return / Enter key. */
  WI_KEY_ESCAPE     = 27,  /**< Escape key. */
  WI_KEY_SPACE      = 32,  /**< Space bar. */
  WI_KEY_BACKSPACE  = 127, /**< Backspace / Delete key. */
  WI_KEY_UPARROW    = 128, /**< Up arrow. */
  WI_KEY_DOWNARROW  = 129, /**< Down arrow. */
  WI_KEY_LEFTARROW  = 130, /**< Left arrow. */
  WI_KEY_RIGHTARROW = 131, /**< Right arrow. */
  WI_KEY_ALT        = 132, /**< Alt / Option key. */
  WI_KEY_CTRL       = 133, /**< Control key. */
  WI_KEY_SHIFT      = 134, /**< Shift key. */
  WI_KEY_F1         = 135, /**< F1 function key. */
  WI_KEY_F2         = 136, /**< F2 function key. */
  WI_KEY_F3         = 137, /**< F3 function key. */
  WI_KEY_F4         = 138, /**< F4 function key. */
  WI_KEY_F5         = 139, /**< F5 function key. */
  WI_KEY_F6         = 140, /**< F6 function key. */
  WI_KEY_F7         = 141, /**< F7 function key. */
  WI_KEY_F8         = 142, /**< F8 function key. */
  WI_KEY_F9         = 143, /**< F9 function key. */
  WI_KEY_F10        = 144, /**< F10 function key. */
  WI_KEY_F11        = 145, /**< F11 function key. */
  WI_KEY_F12        = 146, /**< F12 function key. */
  WI_KEY_INS        = 147, /**< Insert key. */
  WI_KEY_DEL        = 148, /**< Delete key. */
  WI_KEY_PGDN       = 149, /**< Page Down key. */
  WI_KEY_PGUP       = 150, /**< Page Up key. */
  WI_KEY_HOME       = 151, /**< Home key. */
  WI_KEY_END        = 152, /**< End key. */
  WI_KEY_KP_HOME        = 160, /**< Keypad Home (7). */
  WI_KEY_KP_UPARROW     = 161, /**< Keypad Up (8). */
  WI_KEY_KP_PGUP        = 162, /**< Keypad Page Up (9). */
  WI_KEY_KP_LEFTARROW   = 163, /**< Keypad Left (4). */
  WI_KEY_KP_5           = 164, /**< Keypad 5. */
  WI_KEY_KP_RIGHTARROW  = 165, /**< Keypad Right (6). */
  WI_KEY_KP_END         = 166, /**< Keypad End (1). */
  WI_KEY_KP_DOWNARROW   = 167, /**< Keypad Down (2). */
  WI_KEY_KP_PGDN        = 168, /**< Keypad Page Down (3). */
  WI_KEY_KP_ENTER       = 169, /**< Keypad Enter. */
  WI_KEY_KP_INS         = 170, /**< Keypad Insert (0). */
  WI_KEY_KP_DEL         = 171, /**< Keypad Delete (.). */
  WI_KEY_KP_SLASH       = 172, /**< Keypad /. */
  WI_KEY_KP_MINUS       = 173, /**< Keypad -. */
  WI_KEY_KP_PLUS        = 174, /**< Keypad +. */
  WI_KEY_PAUSE          = 255, /**< Pause / Break key. */
  WI_KEY_MOUSE1         = 200, /**< Primary mouse button (left). */
  WI_KEY_MOUSE2         = 201, /**< Secondary mouse button (right). */
  WI_KEY_MOUSE3         = 202, /**< Middle mouse button. */
  WI_KEY_JOY1           = 203, /**< Joystick button 1. */
  WI_KEY_JOY2           = 204, /**< Joystick button 2. */
  WI_KEY_JOY3           = 205, /**< Joystick button 3. */
  WI_KEY_JOY4           = 206, /**< Joystick button 4. */
  WI_KEY_AUX1           = 207, /**< Auxiliary button 1. */
  WI_KEY_AUX2           = 208, /**< Auxiliary button 2. */
  WI_KEY_AUX3           = 209, /**< Auxiliary button 3. */
  WI_KEY_AUX4           = 210, /**< Auxiliary button 4. */
  WI_KEY_AUX5           = 211, /**< Auxiliary button 5. */
  WI_KEY_AUX6           = 212, /**< Auxiliary button 6. */
  WI_KEY_AUX7           = 213, /**< Auxiliary button 7. */
  WI_KEY_AUX8           = 214, /**< Auxiliary button 8. */
  WI_KEY_AUX9           = 215, /**< Auxiliary button 9. */
  WI_KEY_AUX10          = 216, /**< Auxiliary button 10. */
  WI_KEY_AUX11          = 217, /**< Auxiliary button 11. */
  WI_KEY_AUX12          = 218, /**< Auxiliary button 12. */
  WI_KEY_AUX13          = 219, /**< Auxiliary button 13. */
  WI_KEY_AUX14          = 220, /**< Auxiliary button 14. */
  WI_KEY_AUX15          = 221, /**< Auxiliary button 15. */
  WI_KEY_AUX16          = 222, /**< Auxiliary button 16. */
  WI_KEY_AUX17          = 223, /**< Auxiliary button 17. */
  WI_KEY_AUX18          = 224, /**< Auxiliary button 18. */
  WI_KEY_AUX19          = 225, /**< Auxiliary button 19. */
  WI_KEY_AUX20          = 226, /**< Auxiliary button 20. */
  WI_KEY_AUX21          = 227, /**< Auxiliary button 21. */
  WI_KEY_AUX22          = 228, /**< Auxiliary button 22. */
  WI_KEY_AUX23          = 229, /**< Auxiliary button 23. */
  WI_KEY_AUX24          = 230, /**< Auxiliary button 24. */
  WI_KEY_AUX25          = 231, /**< Auxiliary button 25. */
  WI_KEY_AUX26          = 232, /**< Auxiliary button 26. */
  WI_KEY_AUX27          = 233, /**< Auxiliary button 27. */
  WI_KEY_AUX28          = 234, /**< Auxiliary button 28. */
  WI_KEY_AUX29          = 235, /**< Auxiliary button 29. */
  WI_KEY_AUX30          = 236, /**< Auxiliary button 30. */
  WI_KEY_AUX31          = 237, /**< Auxiliary button 31. */
  WI_KEY_AUX32          = 238, /**< Auxiliary button 32. */
  WI_KEY_MWHEELDOWN     = 239, /**< Mouse wheel scroll down. */
  WI_KEY_MWHEELUP       = 240, /**< Mouse wheel scroll up. */
};
/** @} */

/**
 * @defgroup modifiers Modifier key flags
 * @brief Bit flags ORed into #WI_Message::wParam for keyboard and mouse events.
 * @{
 */
enum
{
  WI_MOD_SHIFT = 1 << 16, /**< Shift key is held. */
  WI_MOD_CTRL  = 1 << 17, /**< Control key is held. */
  WI_MOD_ALT   = 1 << 18, /**< Alt / Option key is held. */
  WI_MOD_CMD   = 1 << 19, /**< Command / Super / Meta key is held. */
};
/** @} */

/**
 * @brief General-purpose byte buffer.
 */
struct WI_Buffer
{
  byte_t* data;      /**< Pointer to the raw byte storage. */
  int     maxsize;   /**< Total capacity in bytes. */
  int     cursize;   /**< Number of bytes currently written. */
  int     readcount; /**< Read cursor position in bytes. */
};

/**
 * @brief Platform-independent event message.
 *
 * Dispatched by #WI_PollEvent.  The @p message field identifies the event
 * type (one of the `kEvent*` constants from events.h) and determines how the
 * parameter unions should be interpreted.
 *
 * Mouse events:   @p wParam = MAKEDWORD(x, y), @p lParam = scroll/drag delta.
 * Keyboard events: @p wParam = key-code | modifier flags, @p lParam = UTF-8 char.
 * Window events:  @p target = window handle, @p wParam = new width/height.
 */
struct WI_Message
{
  void*    target;   /**< Window or object that should receive the event. */
  uint32_t message;  /**< Event type identifier (e.g. #kEventKeyDown). */
  union {
    wParam_t wParam;                         /**< Generic word parameter. */
    struct { uint16_t x, y; };               /**< Pointer position (mouse events). */
    struct { uint16_t keyCode, modflags; };  /**< Key code and modifier flags (key events). */
  };
  union {
    lParam_t lParam;              /**< Generic pointer parameter. */
    struct { int16_t dx, dy; };   /**< Relative delta (drag / scroll events). */
  };
  uint32_t id; /**< Sequence number assigned at post time. */
};

/**
 * @brief 2-D size in pixels.
 */
struct WI_Size
{
  uint32_t width;  /**< Width in pixels. */
  uint32_t height; /**< Height in pixels. */
};

/**
 * @defgroup events Event queue
 * @brief Functions for posting and consuming platform events.
 * @{
 */

/**
 * @brief Post an event to the platform event queue.
 *
 * Thread-safe on all platforms.  Duplicate paint and resize events are
 * coalesced: if an identical message for the same @p hobj is already queued,
 * only its parameters are updated rather than adding a second entry.
 *
 * @param hobj   Target window or object handle (stored in #WI_Message::target).
 * @param event  Event type identifier (e.g. #kEventWindowPaint).
 * @param wparam Word-sized parameter; semantics depend on @p event.
 * @param lparam Pointer-sized parameter; semantics depend on @p event.
 */
WI_API void
WI_PostMessageW(void* hobj, uint32_t event, uint32_t wparam, void* lparam);

/**
 * @brief Retrieve the next event from the queue without blocking.
 *
 * On macOS this also pumps the native Cocoa run loop to collect pending
 * system events before returning.
 *
 * @param[out] msg  Filled with event data when an event is available.
 * @return 1 if an event was written to @p msg, 0 if the queue is empty.
 */
WI_API int
WI_PollEvent(struct WI_Message* msg);

/**
 * @brief Remove all queued events whose target matches @p target.
 *
 * Useful when a window is being destroyed to discard stale events before
 * they are delivered.
 *
 * @param target  Window or object handle to match against.
 */
WI_API void
WI_RemoveFromQueue(void* target);

/**
 * @brief Notify the event system that a file was dropped onto the window.
 *
 * This is called internally by platform-specific drag-and-drop handlers.
 * Applications should not normally call this directly; instead listen for
 * #kEventDragDrop messages via #WI_PollEvent.
 *
 * @param filename  Null-terminated UTF-8 path of the dropped file.
 * @param x         Horizontal drop position in window coordinates.
 * @param y         Vertical drop position in window coordinates.
 */
WI_API void
NotifyFileDropEvent(char const *filename, float x, float y);

/** @} */

/**
 * @defgroup dialogs File dialogs
 * @brief Native file-system dialog functions.
 *
 * All dialog functions are synchronous and modal.  On platforms that do not
 * support native file dialogs (Wayland, QNX, WebGL) the functions return
 * `FALSE` immediately without modifying @p lpstrFile.
 * @{
 */

/** @brief Flag: the chosen file must already exist. */
enum {
  OFN_FILEMUSTEXIST = 1 << 0,
  OFN_PATHMUSTEXIST = 1 << 1, /**< The directory portion of the path must exist. */
};

/**
 * @brief Parameters for file and folder dialog functions.
 */
typedef struct _WI_OpenFileName {
  char       *lpstrFile;   /**< Buffer that receives the chosen path (UTF-8, null-terminated). */
  uint32_t    nMaxFile;    /**< Size of @p lpstrFile in bytes, including the null terminator. */
  char const *lpstrFilter; /**< Optional file-type filter string (platform-specific format). */
  char const *lpstrTitle;  /**< Optional dialog title; NULL uses the platform default. */
  uint32_t    Flags;       /**< Combination of OFN_* flags. */
} WI_OpenFileName;

/**
 * @brief Show a native open-file dialog.
 *
 * Blocks until the user confirms or cancels.  On success the chosen path is
 * written to @p ofn->lpstrFile (null-terminated, truncated to @p ofn->nMaxFile
 * bytes).
 *
 * @param ofn  Dialog parameters.
 * @return `TRUE` if the user selected a file, `FALSE` if cancelled or unsupported.
 */
WI_API bool_t
WI_GetOpenFileName(WI_OpenFileName const *ofn);

/**
 * @brief Show a native save-file dialog.
 *
 * Blocks until the user confirms or cancels.  On success the chosen path is
 * written to @p ofn->lpstrFile (null-terminated, truncated to @p ofn->nMaxFile
 * bytes).
 *
 * @param ofn  Dialog parameters.
 * @return `TRUE` if the user entered a path, `FALSE` if cancelled or unsupported.
 */
WI_API bool_t
WI_GetSaveFileName(WI_OpenFileName const *ofn);

/**
 * @brief Show a native folder-picker dialog.
 *
 * Blocks until the user confirms or cancels.  On success the chosen directory
 * path is written to @p ofn->lpstrFile (null-terminated, truncated to
 * @p ofn->nMaxFile bytes).
 *
 * @param ofn  Dialog parameters.
 * @return `TRUE` if the user selected a folder, `FALSE` if cancelled or unsupported.
 */
WI_API bool_t
WI_GetFolderName(WI_OpenFileName const *ofn);

/** @} */

/**
 * @defgroup lifecycle Platform lifecycle
 * @brief Initialisation and teardown of the platform subsystem.
 * @{
 */

/**
 * @brief Initialise the platform subsystem.
 *
 * Must be called once before any other WI_* function.  On macOS this starts
 * the NSApplication run loop; on Wayland it connects to the Wayland display
 * and initialises EGL; on QNX it creates the Screen context and EGL surface.
 */
WI_API void
WI_Init(void);

/**
 * @brief Shut down the platform subsystem and release all resources.
 *
 * After this call no other WI_* function may be called until #WI_Init is
 * called again.
 */
WI_API void
WI_Shutdown(void);

/** @} */

/**
 * @defgroup system System utilities
 * @brief Timing, theme detection, and platform identification.
 * @{
 */

/**
 * @brief Return the current time as a millisecond-resolution monotonic timestamp.
 *
 * The epoch is unspecified; use the difference between two calls to measure
 * elapsed time.
 *
 * @return Milliseconds since an arbitrary fixed point.
 */
WI_API longTime_t
WI_GetMilliseconds(void);

/**
 * @brief Suspend the calling thread for at least @p msec milliseconds.
 *
 * @param msec  Sleep duration in milliseconds.  A value of 0 yields the CPU.
 */
WI_API void
WI_Sleep(longTime_t msec);

/**
 * @brief Test whether the system is currently using a dark colour scheme.
 *
 * On macOS, queries `NSApp effectiveAppearance`.  On Wayland, checks the
 * `GTK_THEME` / `QT_STYLE_OVERRIDE` environment variables.  Returns `FALSE`
 * on platforms without theme detection (QNX, WebGL).
 *
 * @return `TRUE` if a dark theme is active, `FALSE` otherwise.
 */
WI_API bool_t
WI_IsDarkTheme(void);

/**
 * @brief Return a short string identifying the current platform.
 *
 * @return One of `"macos"`, `"linux (wayland)"`, `"linux (x11)"`, `"qnx"`, or `"webgl"`.
 *         The returned pointer is valid for the lifetime of the process.
 */
WI_API char const *
WI_GetPlatform(void);

/**
 * @brief Return the per-user settings (application support) directory.
 *
 * The directory is created if it does not already exist.  The returned path
 * does not include a trailing slash.
 *
 * @return Null-terminated UTF-8 path string.  Valid for the lifetime of the
 *         process; do not free.
 */
WI_API char const *
WI_SettingsDirectory(void);

/**
 * @brief Return the directory that contains shared read-only application data.
 *
 * On macOS this is `<bundle>/Contents/Resources/`; on Linux it is
 * `<exe>/../share/<appname>/`.
 *
 * @return Null-terminated UTF-8 path string.  Valid for the lifetime of the
 *         process; do not free.
 */
WI_API char const *
WI_ShareDirectory(void);

/**
 * @brief Return the directory that contains platform-specific dynamic libraries.
 *
 * On macOS this is the same as #WI_ShareDirectory; on Linux it is
 * `<exe>/../lib/<appname>/`.
 *
 * @return Null-terminated UTF-8 path string.  Valid for the lifetime of the
 *         process; do not free.
 */
WI_API char const *
WI_LibDirectory(void);

/** @} */

/**
 * @defgroup window Window management
 * @brief Creating and managing the application window and rendering surface.
 * @{
 */

/**
 * @brief Create the main application window.
 *
 * Only one window is supported at a time.  Call #WI_Init before this
 * function.
 *
 * @param title   Null-terminated UTF-8 window title string.
 * @param width   Initial client-area width in logical pixels.
 * @param height  Initial client-area height in logical pixels.
 * @param flags   Reserved; pass 0.
 * @return `TRUE` on success, `FALSE` if window creation failed.
 */
WI_API bool_t
WI_CreateWindow(char const *title, uint32_t width, uint32_t height, uint32_t flags);

/**
 * @brief Create an off-screen rendering surface.
 *
 * On macOS this allocates an IOSurface-backed framebuffer.  On other
 * platforms that do not support off-screen surfaces this returns `FALSE`.
 *
 * @param width   Surface width in pixels.
 * @param height  Surface height in pixels.
 * @return `TRUE` on success, `FALSE` if off-screen surfaces are unsupported.
 */
WI_API bool_t
WI_CreateSurface(uint32_t width, uint32_t height);

/**
 * @brief Return the display scaling factor (HiDPI / Retina multiplier).
 *
 * Multiply logical sizes by this value to obtain physical pixel counts.
 * Returns 1.0 on platforms without scaling support.
 *
 * @return Display scale factor (e.g. 2.0 on a Retina display).
 */
WI_API float
WI_GetScaling(void);

/**
 * @brief Resize the window to the specified logical dimensions.
 *
 * @param width    New client-area width in logical pixels.
 * @param height   New client-area height in logical pixels.
 * @param centered If `TRUE`, re-centre the window on screen after resizing.
 * @return `TRUE` on success, `FALSE` if the operation is unsupported.
 */
WI_API bool_t
WI_SetSize(uint32_t width, uint32_t height, bool_t centered);

/**
 * @brief Query the current window size.
 *
 * @param[out] size  If non-NULL, receives the current width and height.
 * @return MAKEDWORD(width, height) packed into a single 32-bit value.
 */
WI_API uint32_t
WI_GetSize(struct WI_Size* size);

/**
 * @brief Wait for the next event or until @p msec milliseconds have elapsed.
 *
 * On QNX the calling thread blocks on a POSIX semaphore that is signalled by
 * #WI_PostMessageW, so the wait is woken immediately when a new event is
 * posted from any thread.  Pass 0 to block indefinitely.
 *
 * @param msec  Maximum time to wait in milliseconds, or 0 to block indefinitely.
 * @return 1 if at least one event is ready, 0 if the timeout expired.
 */
WI_API int
WI_WaitEvent(longTime_t msec);

/**
 * @brief Make the platform OpenGL/EGL context current on the calling thread.
 *
 * Must be called before issuing any OpenGL commands.  On macOS this binds
 * the NSOpenGLContext; on Wayland/QNX it calls eglMakeCurrent.
 */
WI_API void
WI_MakeCurrentContext(void);

/**
 * @brief Begin a frame: make the context current and flush pending events.
 *
 * Equivalent to calling #WI_MakeCurrentContext followed by a display flush
 * on Wayland, or dispatching the Cocoa run loop on macOS.
 */
WI_API void
WI_BeginPaint(void);

/**
 * @brief End a frame: swap buffers and flush the display connection.
 *
 * Presents the rendered frame to the screen by swapping the EGL/OpenGL
 * back-buffer.  Also ensures the alpha channel is opaque where required
 * by compositors.
 */
WI_API void
WI_EndPaint(void);

/**
 * @brief Bind the platform's default framebuffer.
 *
 * On macOS this binds the IOSurface-backed FBO; on EGL-based platforms the
 * default framebuffer (0) is always active, so this is a no-op.
 */
WI_API void
WI_BindFramebuffer(void);

/** @} */

/**
 * @defgroup input Input utilities
 * @brief Key-code to human-readable string conversion.
 * @{
 */

/**
 * @brief Convert a virtual key-code to a printable name string.
 *
 * For printable ASCII keys (33–126) returns a single-character string.
 * For special keys returns a descriptive name such as `"UPARROW"`.
 * Returns `"<UNKNOWN KEYNUM>"` for unrecognised codes and
 * `"<KEY NOT FOUND>"` when @p keynum is `(uint32_t)-1`.
 *
 * @param keynum  Virtual key code (one of the `WI_KEY_*` constants or a
 *                printable ASCII value).
 * @return Null-terminated ASCII string.  Valid until the next call to this
 *         function (uses an internal static buffer for single-character names).
 */
WI_API char const *
WI_KeynumToString(uint32_t keynum);

/** @} */

#endif
