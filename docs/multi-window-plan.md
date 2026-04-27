# Multi-Window Support Plan

This document describes how to add multi-window support to ORCA, using Cocoa/AppKit as the architectural guideline.

The goal is not only to allow more than one native window, but to introduce the right ownership model so the runtime stays coherent as the feature grows.

## Summary

ORCA is currently a single-window system.

- The platform API exposes one main window.
- The renderer initializes one native window or one offscreen surface.
- The Lua `Application` owns one `screen` and runs one event loop against it.
- Project metadata exposes `WindowWidth` and `WindowHeight` as single scalar values.

The target design should follow the same broad shape used by Cocoa:

- one app-wide coordinator on top
- one controller per window underneath
- one root view tree per window

In ORCA terms, that means:

- `Application` remains process-wide and app-wide
- a new `WindowController`-style abstraction is introduced per native window
- each window owns one `Screen`
- project configuration moves from a single `WindowWidth`/`WindowHeight` pair to a `Windows = { ... }` array

## Why Cocoa Is The Right Guideline

In Cocoa, the process usually has one `NSApplication` singleton that coordinates the whole app. Individual windows are then represented by `NSWindow` objects and are typically managed by `NSWindowController` instances.

That model matters because it separates:

- app-wide state: menus, commands, plugins, shared services, settings, routing policies
- window-specific state: title, geometry, visibility, root content, focused view, detached inspectors, auxiliary tools

ORCA should mirror that split:

- `Application` should be equivalent to `NSApplication`
- `WindowController` should be equivalent to `NSWindowController`
- `Screen` should be the root UI tree bound to a specific window

This avoids the two common bad designs:

1. making `Application` per-window
2. keeping a single global screen while trying to bolt extra windows on the side

## Current State

### Platform

The operating systems themselves support multiple windows:

- Cocoa/AppKit: yes
- Win32: yes
- X11: yes
- Wayland: yes

However, ORCA's platform abstraction currently documents and implements a single-window contract.

Observed constraints:

- `axCreateWindow(...)` is documented as supporting only one window at a time
- Windows backend stores one global `HWND`
- X11 backend stores one global `x_window`
- macOS backend stores one global `wstate.Window`
- Wayland contains some partial list-based groundwork, but the active path still uses one global window object

So the OS supports multi-window, but ORCA's platform layer does not currently expose it as a supported runtime feature.

### Renderer

The renderer is currently initialized once:

- `renderer.init(width, height, offscreen)` creates one native window or one offscreen surface
- renderer state is stored globally in `tr`
- rendering assumes one active framebuffer/context pair
- window-size queries are global (`renderer.getSize()` and platform `axGetSize()`)

This means the renderer currently behaves like a single-window renderer with one global presentation target.

### Lua Runtime

The current Lua startup sequence behaves like a single-application, single-window model:

- `Application.open(path)` initializes filesystem and renderer
- startup metadata is read from the project object
- one startup controller or one startup screen is created
- `Application.app` is stored globally as the singleton app instance
- `Application:run()` loops over `system.getMessage` and binds target screen in `system.dispatchMessage(self.screen, msg)`

This is app-wide startup coupled directly to one screen.

### Project Metadata

Project configuration currently exposes:

```lua
WindowWidth = 900
WindowHeight = 600
```

This expresses one primary window only.

The desired future shape is:

```lua
Windows = {
  {
    Name = "Main Window",
    Width = 900,
    Height = 600,
    StartupViewController = "Weather/App",
    StartupRoute = "/"
  }
}
```

This is a better shape even before true multi-window support exists because it:

- groups related window properties together
- provides a natural place for future window flags
- allows a backward-compatible first-window fallback
- matches the controller-per-window architecture

## Target Architecture

## App-Wide Object: `Application`

`Application` should remain the top-level coordinator for the whole process.

Responsibilities:

- initialize filesystem, plugins, and shared services
- load project metadata
- create windows based on `package.lua`
- manage the collection of window controllers
- route app-wide commands
- own app-wide services such as preferences, project references, hot reload, and editor integration

`Application` should not directly be "the content of one window".

Suggested state:

```lua
Application.windows = {}
Application.main_window = nil
Application.project = nil
```

Possible methods:

- `Application.open(path)`
- `Application:create_window(window_config)`
- `Application:close_window(window_controller)`
- `Application:run()`
- `Application:active_window()`

## Per-Window Object: `WindowController`

Introduce a new Lua class to represent one window.

This is the Cocoa-style `NSWindowController` equivalent.

Responsibilities:

- own the native window handle
- own the `Screen` bound to that window
- own the startup controller/view state for that window
- translate project window config into actual runtime objects
- process events targeted at that window
- trigger paints and layout updates for that window only

Suggested state:

```lua
WindowController = Object:extend {
  name = nil,
  screen = nil,
  controller = nil,
  handle = nil,
  width = nil,
  height = nil,
  route = nil,
}
```

Possible methods:

- `WindowController:open(config)`
- `WindowController:load_controller(path, route)`
- `WindowController:load_screen(path)`
- `WindowController:dispatch_message(msg)`
- `WindowController:post_paint()`
- `WindowController:close()`

## Per-Window View Root: `Screen`

Each native window should have exactly one root `Screen`.

That gives a simple invariant:

- one window controller
- one native window
- one root screen

This aligns well with the current UIKit rendering path, which already treats `Screen` as the window-root object.

## Platform API Changes

The platform layer is the first hard requirement.

Without this, a `Windows = { ... }` config is only descriptive and cannot be executed.

### Current limitation

Current API:

```c
bool_t axCreateWindow(char const *title, uint32_t width, uint32_t height, uint32_t flags);
```

This is effectively global and does not return a handle.

### Required direction

Introduce a real window handle abstraction.

Suggested API direction:

```c
typedef struct AXwindow AXwindow;

AXwindow *axCreateWindow(char const *title, uint32_t width, uint32_t height, uint32_t flags);
void axDestroyWindow(AXwindow *window);
bool_t axSetWindowTitle(AXwindow *window, char const *title);
bool_t axSetWindowSize(AXwindow *window, uint32_t width, uint32_t height, bool_t centered);
uint32_t axGetWindowSize(AXwindow *window, struct AXsize *size);
void axMakeCurrentWindow(AXwindow *window);
```

If an incremental migration is preferred, introduce parallel APIs first:

- keep legacy `axCreateWindow(...)` for compatibility
- add `axWindowCreate(...)` and handle-based functions beside it

That reduces churn and allows the old single-window path to continue working while new code adopts handles.

### Event Routing Changes

The event queue already has a `target` field in platform messages.

That field should become the canonical way to route events to a specific window.

Requirements:

- native events must be posted with the owning window handle as `target`
- resize/paint/input events must preserve the originating window
- window close must target the specific window, not the whole process by default

This is a key architectural advantage: the event model already contains the right idea, but ORCA currently uses it as if there were only one window.

## Renderer Changes

The renderer must evolve from one global presentation target to per-window render contexts.

### Current limitation

Current renderer state is global and implicitly bound to one window.

This causes problems for multi-window support:

- one global size
- one current native surface
- one current framebuffer target for presentation
- no explicit mapping between a `Screen` and a native window

### Required direction

Introduce a render context per native window.

Possible shape:

```c
struct RenderWindow {
  AXwindow *window;
  uint32_t width;
  uint32_t height;
  ... platform GL context/surface state ...
};
```

Then evolve the Lua-facing render path so a specific window is rendered explicitly.

Possible direction:

- `renderer.begin_frame(window_handle, clear_color)`
- `renderer.end_frame(window_handle)`
- `renderer.get_size(window_handle)`

If contexts are shared across windows on some platforms, the renderer still needs a per-window presentation object even if textures/shaders remain globally shareable.

### Key requirement

Rendering one window must not mutate state in a way that corrupts another window's presentation.

This is especially important for:

- framebuffer bindings
- viewport size
- swap/present calls
- current GL context

## Lua Runtime Changes

The Lua app model should become:

1. open application
2. read project config
3. create one or more window controllers
4. enter a process-wide message loop
5. dispatch each message to the correct window controller

### Startup flow

Desired flow:

```lua
local app = Application.open(DATADIR)
return app:run()
```

Inside `Application.open(path)`:

1. initialize filesystem
2. load project metadata
3. initialize plugins and shared services
4. normalize project window configuration
5. create one `WindowController` per window config
6. pick `main_window`
7. return the app object

### Event loop

Current loop:

- assumes one screen
- posts paint requests back to `self.screen`

Target loop:

- read one process-wide queue
- identify which window the message belongs to
- dispatch to the owning window controller
- only repaint the affected window

Possible structure:

```lua
function Application:run()
  while true do
    for msg in system.getMessage do
      local window = self:window_for_message(msg)
      if window then
        window:dispatch_message(msg)
      else
        self:dispatch_app_message(msg)
      end
    end
  end
end
```

## Project Configuration Changes

### Desired new format

Use:

```lua
Windows = {
  {
    Name = "Main Window",
    Width = 900,
    Height = 600,
    StartupViewController = "Weather/App",
    StartupRoute = "/"
  }
}
```

Possible future fields:

- `Title`
- `Resizable`
- `Borderless`
- `Fullscreen`
- `Visible`
- `StartupScreen`
- `Role`
- `X`, `Y`
- `MinWidth`, `MinHeight`
- `OwnerWindow`

### Backward compatibility

Keep supporting existing fields during migration:

```lua
WindowWidth = 900
WindowHeight = 600
StartupViewController = "Weather/App"
StartupRoute = "/"
```

Normalization rule:

- if `Windows` exists and is non-empty, use it
- otherwise synthesize one primary window from legacy fields

This avoids breaking all existing projects immediately.

## Recommended Implementation Phases

## Phase 1: Configuration normalization only

Goal:

- accept `Windows = { ... }`
- still run only one real window

Work:

- add `Windows` parsing to project loading
- normalize legacy config into one synthesized window entry
- use the first entry as the primary window

Benefits:

- new package format becomes available immediately
- no platform breakage yet
- zero-risk transition path for content authors

This phase is purely structural.

## Phase 2: Introduce `WindowController`

Goal:

- separate app-wide state from per-window state in Lua

Work:

- add new Lua class for window ownership
- move `screen` and `controller` off `Application` and into `WindowController`
- let `Application` own `windows[]` and `main_window`
- keep runtime still creating only one native window initially

Benefits:

- the runtime shape becomes correct before platform work begins
- future multi-window platform changes have somewhere to land

## Phase 3: Platform handle-based API

Goal:

- create and manage multiple native windows

Work:

- introduce `AXwindow*` or equivalent handle type
- add create/destroy/resize/title/current-context functions that take a handle
- update backends one by one
- preserve old single-window helpers as compatibility wrappers if needed

Suggested backend order:

1. macOS
2. Windows
3. X11
4. Wayland
5. WebGL remains single-window by design

Rationale:

- Cocoa is the architectural guideline
- macOS has a clean conceptual mapping to app + window controller
- it is easier to design the model there first, then port it

## Phase 4: Renderer per-window presentation

Goal:

- render each `Screen` into its own native window

Work:

- introduce render-window state
- make current/begin/end frame target a specific window
- keep shared GPU resources where possible
- ensure paints, resizes, and swaps are window-specific

## Phase 5: True multi-window package execution

Goal:

- create all configured windows from `package.lua`

Work:

- iterate `project.Windows`
- create one `WindowController` per entry
- instantiate each startup controller or startup screen once
- route close events so closing one window does not necessarily terminate the app

At this point, ORCA becomes a genuine multi-window app runtime.

## Cocoa Mapping

Recommended conceptual mapping:

- `Application` -> `NSApplication`
- `WindowController` -> `NSWindowController`
- native platform handle -> `NSWindow`
- `Screen` -> root content tree hosted in that window
- startup view controller class -> window-specific controller/content owner

This is the cleanest model for tool-style apps, editors, asset browsers, inspector windows, and detached previews.

## Use Cases

Multi-window support is most valuable for tools rather than simple runtime demos.

Examples:

- editor main window + hierarchy window + inspector window
- scene view + material editor + shader preview
- game preview window + live profiler window
- document editor with one window per open document
- detached asset browser on a second display

Xcode is a good mental model:

- one app process
- many windows
- each window has its own controller/state
- shared menus, commands, settings, and services remain app-wide

That is exactly the model ORCA should follow.

## Risks And Design Traps

### Trap 1: Making `Application` per-window

Do not do this.

It duplicates app-wide state and makes plugins, shared services, and global commands confusing.

### Trap 2: Keeping one global `screen`

Do not keep a single global screen reference as the only true screen once multi-window support begins.

That design blocks per-window event routing and makes repaint logic incorrect.

### Trap 3: Adding `Windows = { ... }` without runtime ownership changes

Parsing config without adding `WindowController` only postpones the real design work.

The config format and runtime ownership model should evolve together.

### Trap 4: Platform handles without renderer separation

Even if platform windows can be created, rendering will still break if framebuffer/context ownership stays global.

### Trap 5: Closing one window exits the process unconditionally

A multi-window app must define app-lifetime behavior explicitly.

Recommended default:

- closing a non-main auxiliary window only closes that window
- closing the last open window exits the app
- optionally allow package-configured policies later

## Validation Plan

Each implementation phase should have targeted checks.

### Phase 1 validation

- project with legacy `WindowWidth`/`WindowHeight` still works
- project with `Windows = { { ... } }` uses first window correctly
- startup controller and route still load exactly once

### Phase 2 validation

- `Application` owns `windows[]`
- `main_window.screen` replaces the old global `screen` path
- startup logic no longer conflates app creation with window creation

### Phase 3 validation

- two native windows can be created on Cocoa backend
- resize, focus, and close events target the correct window
- each window can have a different title and size

### Phase 4 validation

- each window paints independently
- resize on one window does not corrupt the other
- shader/texture resources remain valid across both windows

### Phase 5 validation

- package with two windows creates both
- each startup controller is instantiated once
- app remains alive when auxiliary window closes
- closing final window exits cleanly

## Recommended First Deliverable

The best first deliverable is not true multi-window support.

The best first deliverable is:

1. support `Windows = { ... }` in package config
2. synthesize a primary window from legacy fields when `Windows` is absent
3. introduce `WindowController` in Lua
4. keep only one real native window for now

This gives immediate architectural progress without requiring a renderer/platform rewrite in one step.

Once that structure is in place, the platform and renderer work becomes incremental rather than disruptive.

## Final Recommendation

Use Cocoa as the architectural model, not as a literal API template.

The correct ORCA design is:

- one `Application` per process
- one `WindowController` per native window
- one `Screen` per window
- one `Windows = { ... }` array in `package.lua`
- one handle-based platform API for native windows
- one renderer presentation target per window

That is the cleanest path from the current single-window runtime to a true multi-window tool-capable architecture.