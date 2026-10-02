# ORCA Framework - Quickstart Guide

This guide will help you get started with the ORCA Framework by creating your first project.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Creating a New Project](#creating-a-new-project)
3. [Scene File Formats](#scene-file-formats)
4. [Package.lua Specification](#packagelua-specification)
5. [Building and Running](#building-and-running)
6. [Examples](#examples)

## Prerequisites

Before you begin, make sure you have:

- Installed all required dependencies (see [README.md](README.md) for installation instructions)
- Initialized git submodules: `git submodule update --init --recursive`
- Built the ORCA Framework using `make`
- Access to the required data folder (`../icui`)

## Creating a New Project

To create a new ORCA project, you'll need to set up the following basic structure:

```
MyProject/
├── package.lua          # Project configuration file
├── Screens/             # Directory for screen definitions
│   └── Application.xml  # Main application screen
└── Images/              # Directory for images (optional)
```

### Minimal Project Setup

1. Create a project directory:
```sh
mkdir MyProject
cd MyProject
```

2. Create a `package.lua` file (see [Package.lua Specification](#packagelua-specification))

3. Create a `Screens` directory for your UI definitions:
```sh
mkdir Screens
```

4. Create your first screen file in one of the supported formats (see [Scene File Formats](#scene-file-formats))

## Scene File Formats

ORCA supports three different formats for defining scenes and UI components:

### 1. XML Format (.xml)

XML is the native format for declarative UI definitions in ORCA. It provides a clean, structured way to define screens and components.

**Example: `Screens/Application.xml`**
```xml
<?xml version="1.0"?>
<Screen Name="Application" Height="768" Width="1024">
  <TextBlock Name="TextBlock" FontSize="40" LayoutTransform="400 20 0 1 1" Text="Hello World"/>
  <ImageView Name="Image" Source="MyProject/Images/peacock"/>
</Screen>
```

**Features:**
- Declarative UI definitions
- Native ORCA format
- XML attributes for properties
- Supports all ORCA UI components (Grid, TextBlock, ImageView, etc.)
- Namespace support: `xmlns="http://schemas.corepunch.com/orca/2006/xml/presentation"`

**Common Components:**
- `<Screen>` - Root container for a screen
- `<Grid>` - Grid layout with rows and columns
- `<TextBlock>` - Text display element
- `<ImageView>` - Image display element
- `<StackView>` - Stack layout container

### 2. Lua Format (.lua)

Lua provides a programmatic approach to building UIs with full access to the ORCA API.

**Example: `Scripts/Inspector.lua`**
```lua
local core = require "orca.core"
local xml = require "orca.parsers.xml"
local ui = require "orca.ui"
local geom = require "orca.geometry"

local Inspector = ui.TerminalView:extend {
    onAwake = function (self)
        -- Initialize your component
        self.expanded = {}
    end,
    
    onPaint = function (self)
        self:erase()
        self:println(nil, "Custom UI Component")
    end,
    
    onLeftButtonUp = function (self, _, ...)
        -- Handle user interaction
    end
}

return Inspector
```

**Features:**
- Full programmatic control
- Access to all Lua standard libraries
- Event handling (onAwake, onPaint, mouse events, etc.)
- Component extension system with `:extend`
- Imperative UI construction

**Common Patterns:**
- Extend existing UI components
- Implement custom event handlers
- Access ORCA modules: `orca.core`, `orca.ui`, `orca.geometry`, `orca.parsers.xml`
- Return component class at the end

### 3. MoonScript Format (.moon)

MoonScript is a language that compiles to Lua, offering a more concise and readable syntax inspired by CoffeeScript and Python.

**Example: `App.moon`**
```moonscript
require "html"

routing = require "routing"
ui = require "orca.ui"

import Application from require "routing"

class App extends Application
    @include "applications.users"
    @include "applications.chat"

    @stylesheet require "tailwind"
    @stylesheet "assets/globals.css"

    "/": => Layout page.HomePage
    "/settings": => Layout page.Settings
    "/user/:user": => Layout page.ContactDetails, @params

    Awake: => 
        -- Initialize application
        @navigate '/sign-in' unless pcall Account\auth, Account
```

**Example: UI Component in MoonScript**
```moonscript
ui = require "orca.ui"

class HomePage extends ui.StackView
    title: "Overview"
    apply: => "flex-col w-full gap-2"
    
    body: =>
        HeroSection ".my-2"
        Transactions limit: 5
```

**Features:**
- Clean, indentation-based syntax
- Class definitions with `class ... extends`
- Implicit return values
- String interpolation
- List comprehensions
- Compiles to Lua at runtime
- CSS-like styling with Tailwind support
- Routing support with pattern matching

**Common Patterns:**
- Define classes with inheritance
- Use `@` for `self` in class methods
- Import with `import ... from require`
- Define routes with pattern matching
- Apply CSS classes with `apply` method

## Package.lua Specification

The `package.lua` file is the entry point for your ORCA project. It defines project metadata, libraries, and system messages. It is the only project manifest the runtime reads; `package.xml` is no longer loaded.

### Basic Structure

`package.lua` is a Lua file in which each global assignment sets a property of the project:

```lua
Name = "ProjectName"
StartupScreen = "ProjectName/Screens/ScreenName"
-- Project configuration
```

### Required Properties

- `Name`: The name of your project
- `StartupScreen`: The path to the initial screen to load (format: `ProjectName/Path/To/Screen`). Script-driven applications set `StartupViewController` (and optionally `StartupRoute`) instead, as `samples/Weather` and `samples/Banking` do.

### Optional Properties

#### WindowWidth / WindowHeight

Initial window size in pixels:

```lua
WindowWidth = 1024
WindowHeight = 768
```

#### ProjectReferences

Define external project references and library paths:

```lua
ProjectReferences = {
	{ Name = "views",  Path = "views" },
	{ Name = "assets", Path = "assets" },
	{ Name = "model",  Path = "model" },
}
```

#### ScreenLibrary

Define screen libraries (can be external):

```lua
ScreenLibrary = { IsExternal = true }
```

#### ImageLibrary

Define image libraries:

```lua
ImageLibrary = { IsExternal = true }
```

#### PropertyTypes

Define custom property types:

```lua
PropertyTypes = {
	{ Name = "Title", Category = "NavigationBar", DataType = "String" },
}
```

#### SystemMessages

Define system message handlers:

```lua
SystemMessages = {
	{ Message = "KeyDown", Key = "q", Command = "return" },
	{ Message = "WindowClosed", Command = "return" },
	{ Message = "RequestReload", Command = "window:refresh()" },
}
```

**Note**: Field names are case-sensitive. A `SystemMessage` has the fields `Message`, `Key`, and `Command`.

#### EnginePlugins

Load engine plugins so their classes are available:

```lua
EnginePlugins = {
	{ Name = "orca.UIKit" },
}
```

### Complete Examples

#### Example 1: Minimal Project

```lua
Name = "Example"
StartupScreen = "Example/Screens/Application"
WindowWidth = 1024
WindowHeight = 768
ScreenLibrary = { IsExternal = true }
ImageLibrary = { IsExternal = true }
SystemMessages = {
	{ Message = "KeyDown", Key = "q", Command = "return" },
	{ Message = "WindowClosed", Command = "return" },
}
EnginePlugins = {
	{ Name = "orca.UIKit" },
}
```

#### Example 2: Script Project with Project References

This is `samples/Banking/package.lua`:

```lua
Name = "Banking"
StartupViewController = "Banking/App"
StartupRoute = "/"
WindowWidth = 375
WindowHeight = 812
ProjectReferences = {
	{ Name = "views",    Path = "views"       },
	{ Name = "assets",   Path = "assets"      },
	{ Name = "model",    Path = "model"       },
	{ Name = "appwrite", Path = "lib/appwrite" },
	{ Name = "config",   Path = "config"      },
}
SystemMessages = {
	{ Message = "KeyDown",      Key = "q", Command = "return"             },
	{ Message = "WindowClosed",            Command = "return"             },
	{ Message = "RequestReload",           Command = "window:refresh()"   },
}
```

## Building and Running

Once you've created your project:

1. **Initialize git submodules** (if not already done):
```sh
git submodule update --init --recursive
```

2. **Build the ORCA Framework** (if not already built):
```sh
make
```

3. **Run your project** by passing the project directory (the one containing `package.lua`):
```sh
build/bin/orca /path/to/your/project
```

For example, to run the bundled sample:
```sh
build/bin/orca samples/Example
```

## Examples

### Example 1: Simple Hello World (XML)

**Directory structure:**
```
HelloWorld/
├── package.lua
└── Screens/
    └── Main.xml
```

**package.lua:**
```lua
Name = "HelloWorld"
StartupScreen = "HelloWorld/Screens/Main"
WindowWidth = 1024
WindowHeight = 768
ScreenLibrary = { IsExternal = true }
SystemMessages = {
	{ Message = "KeyDown", Key = "q", Command = "return" },
	{ Message = "WindowClosed", Command = "return" },
}
EnginePlugins = {
	{ Name = "orca.UIKit" },
}
```

**Screens/Main.xml:**
```xml
<?xml version="1.0"?>
<Screen Name="Main" Width="1024" Height="768">
  <TextBlock Text="Hello, ORCA!" FontSize="48" LayoutTransform="300 300 0 1 1"/>
</Screen>
```

### Example 2: Grid Layout (XML)

**Screens/GridExample.xml:**
```xml
<?xml version="1.0"?>
<Screen xmlns="http://schemas.corepunch.com/orca/2006/xml/presentation" id="GridExample">
  <Grid Columns="64px auto" Spacing="10">
    <TextBlock Text="Row 1, Col 1" Background.Color="#FF0000"/>
    <TextBlock Text="Row 1, Col 2" Background.Color="#00FF00"/>
    <TextBlock Text="Row 2, Col 1" Background.Color="#0000FF"/>
    <TextBlock Text="Row 2, Col 2" Background.Color="#FFFF00"/>
  </Grid>
</Screen>
```

### Example 3: Interactive Component (Lua)

**Scripts/Counter.lua:**
```lua
local ui = require "orca.ui"

local Counter = ui.StackView:extend {
    count = 0,
    
    onAwake = function (self)
        self.label = self:findChild("CountLabel")
        self:updateLabel()
    end,
    
    increment = function (self)
        self.count = self.count + 1
        self:updateLabel()
    end,
    
    updateLabel = function (self)
        if self.label then
            self.label:setText(string.format("Count: %d", self.count))
        end
    end
}

return Counter
```

### Example 4: Application with Routing (MoonScript)

**App.moon:**
```moonscript
routing = require "routing"
ui = require "orca.ui"

import Application from require "routing"

class MyApp extends Application
    "/": => HomePage!
    "/about": => AboutPage!
    "/contact": => ContactPage!
    
    Awake: =>
        @navigate '/'
```

## Next Steps

- Explore the `samples/` directory for more complex examples
- Read the documentation in `docs/` for detailed API reference
- Check out the Banking sample (`samples/Banking/`) for a full-featured application
- Review the Editor sample (`samples/Editor/`) to see advanced UI techniques

## Tips

- **XML** is best for declarative, static UIs
- **Lua** is ideal for programmatic UI construction and complex logic
- **MoonScript** offers a cleaner syntax for Lua and is great for application-level code
- You can mix and match formats in the same project
- Use `SystemMessages` in `package.lua` to handle system events like key presses and window closing
- External libraries can be referenced via `ProjectReferences` for code organization

## Common Issues

- **Startup screen not found**: Ensure the `StartupScreen` path in `package.lua` matches your actual file structure
- **Images not loading**: Check that image paths are relative to the project root (e.g., `ProjectName/Images/filename`)
- **Build errors**: Make sure all dependencies are installed (see README.md)

For more help, refer to the samples in the `samples/` directory or check the project documentation.
