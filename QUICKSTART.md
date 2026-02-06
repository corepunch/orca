# ORCA Framework - Quickstart Guide

This guide will help you get started with the ORCA Framework by creating your first project.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Creating a New Project](#creating-a-new-project)
3. [Scene File Formats](#scene-file-formats)
4. [Package.xml Specification](#packagexml-specification)
5. [Building and Running](#building-and-running)
6. [Examples](#examples)

## Prerequisites

Before you begin, make sure you have:

- Installed all required dependencies (see [README.md](README.md) for installation instructions)
- Built the ORCA Framework using `make`
- Access to the required data folder (`../icui`)

## Creating a New Project

To create a new ORCA project, you'll need to set up the following basic structure:

```
MyProject/
├── package.xml          # Project configuration file
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

2. Create a `package.xml` file (see [Package.xml Specification](#packagexml-specification))

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
  <ImageView Name="Image" Image="MyProject/Images/peacock"/>
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
    
    onLeftMouseUp = function (self, _, ...)
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

    onAwake: => 
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

## Package.xml Specification

The `package.xml` file is the entry point for your ORCA project. It defines project metadata, libraries, and system messages.

### Basic Structure

```xml
<?xml version="1.0"?>
<Project Name="ProjectName" StartupScreen="ProjectName/Screens/ScreenName">
  <!-- Project configuration -->
</Project>
```

### Required Attributes

- `Name`: The name of your project
- `StartupScreen`: The path to the initial screen to load (format: `ProjectName/Path/To/Screen`)

### Optional Elements

#### ProjectReferenceLibrary

Define external project references and library paths:

```xml
<ProjectReferenceLibrary>
  <ProjectReferenceItem Name="applications">applications</ProjectReferenceItem>
  <ProjectReferenceItem Name="assets">assets</ProjectReferenceItem>
  <ProjectReferenceItem Name="routing">lib/routing</ProjectReferenceItem>
</ProjectReferenceLibrary>
```

#### ScreenLibrary

Define screen libraries (can be external):

```xml
<ScreenLibrary Name="Screens" IsExternal="true"/>
```

#### ImageLibrary

Define image libraries:

```xml
<ImageLibrary Name="Images" IsExternal="true"/>
```

#### PropertyTypeLibrary

Define custom property types:

```xml
<PropertyTypeLibrary Name="Property Types">
  <PropertyType Name="Speed" Type="float" Namespace="Custom"/>
</PropertyTypeLibrary>
```

#### MessageLibrary

Define system message handlers:

```xml
<MessageLibrary Name="Messages">
  <SystemMessage Message="KeyDown" Key="q">return</SystemMessage>
  <SystemMessage Message="WindowClosed">return</SystemMessage>
  <SystemMessage Message="RequestReload">window:refresh()</SystemMessage>
</MessageLibrary>
```

**Note**: The attribute names are case-insensitive in ORCA. You may see both `Message`/`Key` (capital) and `message`/`key` (lowercase) in different samples. Both work correctly.

### Complete Examples

#### Example 1: Minimal Project

```xml
<?xml version="1.0"?>
<Project Name="Example" StartupScreen="Example/Screens/Application">
  <ScreenLibrary Name="Screens" IsExternal="true"/>
  <ImageLibrary Name="Images" IsExternal="true"/>
  <MessageLibrary Name="Messages">
    <SystemMessage Message="KeyDown" Key="q">return</SystemMessage>
    <SystemMessage Message="WindowClosed">return</SystemMessage>
  </MessageLibrary>
</Project>
```

#### Example 2: Complex Project with Libraries

```xml
<?xml version="1.0"?>
<Project Name="Banking" StartupScreen="Banking/App">
  <ProjectReferenceLibrary>
    <ProjectReferenceItem Name="applications">applications</ProjectReferenceItem>
    <ProjectReferenceItem Name="assets">assets</ProjectReferenceItem>
    <ProjectReferenceItem Name="appwrite">lib/appwrite</ProjectReferenceItem>
    <ProjectReferenceItem Name="html">lib/html</ProjectReferenceItem>
    <ProjectReferenceItem Name="routing">lib/routing</ProjectReferenceItem>
    <ProjectReferenceItem Name="model">model</ProjectReferenceItem>
    <ProjectReferenceItem Name="root">root</ProjectReferenceItem>
    <ProjectReferenceItem Name="config">config</ProjectReferenceItem>
    <ProjectReferenceItem Name="tailwind">tailwind</ProjectReferenceItem>
  </ProjectReferenceLibrary>
  <MessageLibrary Name="Messages">
    <SystemMessage Message="KeyDown" Key="q">return</SystemMessage>
    <SystemMessage Message="WindowClosed">return</SystemMessage>
    <SystemMessage Message="RequestReload">window:refresh()</SystemMessage>
  </MessageLibrary>
</Project>
```

## Building and Running

Once you've created your project:

1. **Build the ORCA Framework** (if not already built):
```sh
make
```

2. **Run your project**:
```sh
make run PROJECT=/path/to/your/project/package.xml
```

Or for a quick clean build and run:
```sh
make andrun PROJECT=/path/to/your/project/package.xml
```

## Examples

### Example 1: Simple Hello World (XML)

**Directory structure:**
```
HelloWorld/
├── package.xml
└── Screens/
    └── Main.xml
```

**package.xml:**
```xml
<?xml version="1.0"?>
<Project Name="HelloWorld" StartupScreen="HelloWorld/Screens/Main">
  <ScreenLibrary Name="Screens" IsExternal="true"/>
  <MessageLibrary Name="Messages">
    <SystemMessage Message="KeyDown" Key="q">return</SystemMessage>
    <SystemMessage Message="WindowClosed">return</SystemMessage>
  </MessageLibrary>
</Project>
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
    
    onAwake: =>
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
- Use the MessageLibrary to handle system events like key presses and window closing
- External libraries can be referenced via ProjectReferenceLibrary for code organization

## Common Issues

- **Startup screen not found**: Ensure the StartupScreen path in package.xml matches your actual file structure
- **Images not loading**: Check that image paths are relative to the project root (e.g., `ProjectName/Images/filename`)
- **Build errors**: Make sure all dependencies are installed (see README.md)

For more help, refer to the samples in the `samples/` directory or check the project documentation.
