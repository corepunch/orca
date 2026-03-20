<img width="1489" height="588" alt="7aaa527d-7867-418a-be61-5612fda46339" src="https://github.com/user-attachments/assets/98fa803c-bd18-4dc9-9a68-2b6d6fa77c8a" />

# Axiom Platform Abstraction Toolkit

[![Build](https://github.com/corepunch/platform/actions/workflows/build.yml/badge.svg)](https://github.com/corepunch/platform/actions/workflows/build.yml)

A lightweight, cross-platform C library providing a unified API for window management, input/event handling, and system utilities. Designed for applications that need direct control over windowing and input without heavy framework dependencies.

## Features

- **Cross-platform**: Native support for Windows (Win32/WGL via MinGW), Linux (Wayland or X11), macOS (AppKit/Cocoa), QNX, and WebGL (browser)
- **Window Management**: Create and manage windows with OpenGL/EGL context support
- **Event System**: Unified event handling for mouse, keyboard, and window events
- **Input Processing**: Comprehensive keyboard and mouse input with modifier support
- **File Dialogs**: Native file open/save dialogs
- **System Integration**: Theme detection, directory paths, timing utilities
- **Minimal Dependencies**: Simple C API with no bloat

## Quickstart

Here's a minimal example showing how to create a window and process input events:

```c
#include "platform.h"
#include <stdio.h>

int main(void) {
    // Initialize the platform library
    WI_Init();
    
    // Create a window (800x600 pixels)
    if (!WI_CreateWindow("My Application", 800, 600, 0)) {
        fprintf(stderr, "Failed to create window\n");
        return 1;
    }
    
    // Main event loop
    struct WI_Message msg;
    int running = 1;
    
    while (running) {
        // Wait for events (with 16ms timeout for ~60 FPS)
        WI_WaitEvent(16);
        
        // Process all pending events
        while (WI_PollEvent(&msg)) {
            switch (msg.message) {
                case kEventWindowClosed:
                    running = 0;
                    break;
                    
                case kEventKeyDown:
                    printf("Key pressed: %d (modifiers: 0x%x)\n", 
                           msg.keyCode, msg.modflags);
                    
                    // Exit on ESC key
                    if (msg.keyCode == WI_KEY_ESCAPE) {
                        running = 0;
                    }
                    break;
                    
                case kEventLeftMouseDown:
                    printf("Mouse clicked at: %d, %d\n", msg.x, msg.y);
                    break;
                    
                case kEventMouseMoved:
                    // Mouse position in msg.x, msg.y
                    break;
                    
                case kEventWindowResized:
                    printf("Window resized\n");
                    break;
            }
        }
        
        // Render your frame here
        WI_BeginPaint();
        
        // ... OpenGL rendering calls ...
        
        WI_EndPaint();
    }
    
    // Cleanup
    WI_Shutdown();
    return 0;
}
```

### Compiling the Example

On **Windows (MinGW/MSYS2)**:
```bash
gcc -o myapp main.c -L. -lplatform -lopengl32 -lgdi32 -luser32
```

On **Linux (X11)**:
```bash
gcc -o myapp main.c -L. -lplatform -lX11 -lEGL -lGL
```

On **Linux (Wayland)**:
```bash
gcc -o myapp main.c -L. -lplatform -lwayland-client -lwayland-egl -lEGL -lGL
```

On **macOS**:
```bash
clang -o myapp main.c -L. -lplatform -framework AppKit -framework OpenGL
```

On **WebGL (browser via Emscripten)**:
```bash
emcc -o myapp.js main.c -L. -lplatform -sUSE_WEBGL2=1 -sEXPORTED_RUNTIME_METHODS=ccall,cwrap
```

Make sure `libplatform.so` (Linux) or `libplatform.dylib` (macOS) is in your library path or current directory.

## API Overview

### Initialization and Cleanup

```c
void WI_Init(void);                    // Initialize the platform library
void WI_Shutdown(void);                // Clean up and shutdown
```

### Window Management

```c
bool_t WI_CreateWindow(const char* title, uint32_t width, uint32_t height, uint32_t flags);
bool_t WI_CreateSurface(uint32_t width, uint32_t height);
bool_t WI_SetSize(uint32_t width, uint32_t height, bool_t centered);
uint32_t WI_GetSize(struct WI_Size* size);
float WI_GetScaling(void);             // Get display scaling factor
```

### Event Processing

```c
int WI_PollEvent(struct WI_Message* msg);          // Poll for next event (non-blocking)
int WI_WaitEvent(longTime_t timeout_ms);           // Wait for events with timeout
void WI_PostMessageW(void* hobj, uint32_t event, uint32_t wparam, void* lparam);
```

### Event Types

The library supports comprehensive event handling through the `WI_Message` structure:

- **Mouse Events**: `kEventLeftMouseDown`, `kEventLeftMouseUp`, `kEventMouseMoved`, `kEventScrollWheel`
- **Keyboard Events**: `kEventKeyDown`, `kEventKeyUp`, `kEventChar`
- **Window Events**: `kEventWindowClosed`, `kEventWindowResized`, `kEventWindowPaint`
- **Drag & Drop**: `kEventDragDrop`, `kEventDragEnter`

### Rendering

```c
void WI_MakeCurrentContext(void);      // Make OpenGL context current
void WI_BeginPaint(void);              // Begin rendering frame
void WI_EndPaint(void);                // End rendering and swap buffers
void WI_BindFramebuffer(void);         // Bind default framebuffer
```

### File Dialogs

```c
bool_t WI_GetOpenFileName(WI_OpenFileName const* params);
bool_t WI_GetSaveFileName(WI_OpenFileName const* params);
bool_t WI_GetFolderName(WI_OpenFileName const* params);
```

### System Utilities

```c
longTime_t WI_GetMilliseconds(void);   // Get current time in milliseconds
void WI_Sleep(longTime_t msec);        // Sleep for specified milliseconds
bool_t WI_IsDarkTheme(void);           // Check if dark theme is active
const char* WI_GetPlatform(void);      // Get platform name
const char* WI_SettingsDirectory(void); // Get settings directory path
const char* WI_ShareDirectory(void);   // Get share directory path
const char* WI_LibDirectory(void);     // Get library directory path
```

### Key Codes

The library defines comprehensive key codes including:
- Standard ASCII characters
- Function keys (`WI_KEY_F1` - `WI_KEY_F12`)
- Arrow keys (`WI_KEY_UPARROW`, `WI_KEY_DOWNARROW`, etc.)
- Special keys (`WI_KEY_ENTER`, `WI_KEY_ESCAPE`, `WI_KEY_SPACE`, etc.)
- Mouse buttons (`WI_KEY_MOUSE1`, `WI_KEY_MOUSE2`, `WI_KEY_MOUSE3`)
- Modifier flags (`WI_MOD_SHIFT`, `WI_MOD_CTRL`, `WI_MOD_ALT`, `WI_MOD_CMD`)

See `platform.h` for the complete list of key codes and event definitions.

## Building

Simple Makefile that auto-detects your platform and builds the appropriate dynamic library using wildcards for source files.

### Prerequisites

**Windows (MinGW/MSYS2):**
- MinGW-w64 toolchain (GCC for Windows):
  ```bash
  # Install via MSYS2 (https://www.msys2.org/)
  pacman -S mingw-w64-x86_64-gcc make
  ```
- No extra libraries required — uses Win32, WGL, and Winsock2 (all included with Windows)

**WebGL (Emscripten):**
- Emscripten SDK (`emcc` compiler):
  ```bash
  # Install via emsdk (https://emscripten.org/docs/getting_started/downloads.html)
  git clone https://github.com/emscripten-core/emsdk.git
  cd emsdk && ./emsdk install latest && ./emsdk activate latest
  source ./emsdk_env.sh
  ```

**Linux:**
- GCC compiler
- Wayland development libraries (preferred, for Wayland support):
  ```bash
  sudo apt-get install libwayland-dev libwayland-egl1-mesa libxkbcommon-dev libegl-dev libgl-dev
  ```
- Or X11 development libraries (fallback, if Wayland is unavailable):
  ```bash
  sudo apt-get install libx11-dev libegl-dev libgl-dev
  ```
- Build order: **Wayland** (preferred) → **X11** (fallback) → **unix-only** (networking only)

**macOS:**
- Clang compiler (included with Xcode Command Line Tools)
- AppKit framework (included with macOS)

### Build Commands

```bash
make              # Build the dynamic library
emmake make       # Build for WebGL (requires Emscripten environment sourced)
make clean        # Clean build artifacts
make install      # Install to /usr/local/lib (requires sudo; Linux/macOS only)
```

### Output

- **Windows**: `libplatform.dll` (+ `libplatform.dll.a` import library)
- **Linux**: `libplatform.so`
- **macOS**: `libplatform.dylib`
- **WebGL**: `libplatform.wasm` (Emscripten side module)

The library exports platform-specific functions for window management, event handling, networking, and system utilities.

## Platform Support

- **Windows**: Full support via Win32 API and WGL — window creation, OpenGL rendering, native file dialogs, dark-theme detection, Winsock2 networking; built with MinGW
- **Linux (Wayland)**: Full support with Wayland, EGL, and OpenGL (preferred when available)
- **Linux (X11)**: Full support with Xlib, EGL, and OpenGL (fallback when Wayland is unavailable)
- **macOS**: Full support with AppKit and Cocoa frameworks
- **QNX**: Source files available (qnx/)
- **WebGL (browser)**: Full support via Emscripten — mouse, keyboard, scroll, and resize events; WebGL 1/2 rendering context

The Linux backend is selected automatically at build time:
1. If Wayland libraries are detected via `pkg-config`, the Wayland backend is used
2. If Wayland is unavailable but X11 libraries are detected, the X11 backend is used
3. If neither is available, a unix-only build is produced (networking only, no windowing)

## Header

Include `platform.h` in your project to access platform types and APIs.
