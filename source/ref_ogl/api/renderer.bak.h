#ifndef __API_RENDERER_H__
#define __API_RENDERER_H__

#include "../r_local.h"

typedef const char *string;
typedef int nresults;
typedef bool_t BOOL;
typedef HWND WindowPtr;
typedef void *PVOID;

int api_Window_new(lua_State* L);

#define API_PushWindow(...)
#define API_NewWindow api_Window_new
#define INTERFACE WindowPtr

/// @brief Window interface abstraction.
/// 
/// The `Window` interface defines the contract for window creation and
/// management inside the framework. Each window instance operates in isolation,
/// backed by its own `lua_State`, similar in spirit to Electron processes.
/// This ensures complete separation of state and scripts across windows,
/// avoiding unintended side effects between them.
///
/// \snippet docs/creating_window.lua
///
/// The main application script `main.lua` is responsible for creating and
/// managing windows. Once created, each window runs independently,
/// maintaining its own Lua environment and bindings.
///
/// The event loop for windows is cooperative and integrated with the core
/// runtime. Events are dispatched and processed in a manner similar to how
/// native windowing systems propagate events, while remaining script-driven
/// and cross-platform within the engine.
LUA_INTERFACE(IWindow)
{
  /// @brief Garbage collection handler for the window object.
  /// Called automatically when the window is destroyed.
  void LUA_METHOD(__gc);
  
  /// @brief Sets the position of the window on screen.
  /// @param x Horizontal coordinate in pixels.
  /// @param y Vertical coordinate in pixels.
  void LUA_METHOD(set_position, float x, float y);
  
  /// @brief Loads a UI page into the window.
  /// @param path Filesystem path to the page file.
  /// @param keep_state Whether to preserve current state when switching.
  /// @return Number of results pushed onto the Lua stack.
  nresults LUA_METHOD(load_page, string path, BOOL keep_state);
  
  /// @brief Registers an additional filesystem path as a source of asset for this window.
  /// @param path Filesystem path to the UI fragment.
  void LUA_METHOD(mount, string path);
  
  /// @brief Adds a localization file for UI text within this window only.
  /// @param path Filesystem path to the localization data (XML format expected).
  void LUA_METHOD(add_localization, string path);
};

/// @brief Creates a GPU render target.
/// @param width Render target width in pixels.
/// @param height Render target height in pixels.
/// @return Pointer to the created render target object.
PVOID LUA_FUNCTION(createRenderTarget, int width, int height);

/// @brief Creates a shader program from source code.
/// @param vertex GLSL/HLSL vertex shader source.
/// @param fragment GLSL/HLSL fragment shader source.
/// @return Pointer to the created shader object.
PVOID LUA_FUNCTION(createShader, string vertex, string fragment);

/// @brief Loads an image into GPU memory.
/// @param path Filesystem path to the image file.
/// @return Pointer to the loaded image object.
PVOID LUA_FUNCTION(loadImage, string path);

/// @brief Retrieves the size of the current window.
/// @return Number of results pushed onto the Lua stack (typically width, height).
nresults LUA_FUNCTION(getSize);

/// @brief Draws the current image to the active render target.
/// Parameters are expected to be read from the Lua stack
/// (e.g. image handle, coordinates, scaling).
void LUA_FUNCTION(drawImage);

#endif
