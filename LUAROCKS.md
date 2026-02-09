# Using ORCA with LuaRocks

ORCA is now available as a LuaRocks package, making it easy to install and use in your Lua projects.

## Installation

### System Dependencies

Before installing ORCA via LuaRocks, ensure you have the required system libraries installed:

**Ubuntu/Debian:**
```bash
sudo apt update && sudo apt install -y \
    libpng-dev \
    libfreetype6-dev \
    libjpeg-dev \
    liblua5.4-dev \
    libxml2-dev \
    liblz4-dev \
    zlib1g-dev \
    libcurl4-openssl-dev \
    libwayland-dev \
    libegl1-mesa-dev \
    libgles2-mesa-dev
```

**Fedora:**
```bash
sudo dnf install -y \
    libpng-devel \
    freetype-devel \
    libjpeg-turbo-devel \
    lua-devel \
    libxml2-devel \
    lz4-devel \
    zlib-devel \
    libcurl-devel
```

**macOS:**
```bash
brew install libpng freetype jpeg-turbo lua libxml2 lz4 zlib curl pkg-config
```

### Installing from Source

Clone the repository and install using LuaRocks:

```bash
git clone https://github.com/corepunch/orca.git
cd orca
git submodule update --init --recursive
luarocks make
```

### Installing from LuaRocks (when published)

Once published to the LuaRocks repository:

```bash
luarocks install orca
```

## Usage

### Basic Usage

After installation, you can use ORCA in your Lua scripts:

```lua
local orca = require "orca"
local ui = require "orca.ui"
local geometry = require "orca.geometry"

print("ORCA version:", orca.version)
print("Build date:", orca.build)
```

### Using ORCA as a Library

ORCA provides a high-level API through `orca.main` for building applications:

```lua
local main = require "orca.main"

-- Configure your application
local config = {
  dirname = ".",
  name = "MyApp",
  startup_screen = "screens.MainScreen",
  locales = {
    "locales/en"
  },
  messages = {
    -- Quit on 'q' key
    main.messageHandler(
      {Message = "KeyDown", key = "q"},
      "return"
    ),
    -- Reload on Ctrl+R
    main.messageHandler(
      {Message = "KeyDown", key = "r", ctrl = true},
      "window:refresh()"
    )
  }
}

-- Run the application
main.run(config)
```

### Available Modules

ORCA provides the following modules:

| Module | Description |
|--------|-------------|
| `orca` | Core module with async, version info |
| `orca.core` | Object system and behavior framework |
| `orca.ui` | UI components (Node2D, StackView, Button, etc.) |
| `orca.geometry` | Math types (Vector2D/3D, Matrix, Quaternion, Color) |
| `orca.filesystem` | File I/O and workspace management |
| `orca.network` | HTTP networking and fetching |
| `orca.renderer` | Graphics rendering API |
| `orca.localization` | Localization/i18n support |
| `orca.parsers.xml` | XML parsing |
| `orca.parsers.css` | CSS parsing |
| `orca.parsers.json` | JSON encoding/decoding |
| `orca.SceneKit` | 3D scene management |
| `orca.backend` | Backend messaging system |
| `orca.system` | System utilities |
| `orca.editor` | Editor functionality |

### Creating UI Components

```lua
local ui = require "orca.ui"

-- Create a stack view (vertical layout)
local stack = ui.StackView()
stack.Width = 800
stack.Height = 600

-- Add text
local text = ui.TextBlock()
text.Text = "Hello, ORCA!"
text.FontSize = 24

-- Add image
local image = ui.ImageView()
image.Image = "path/to/image.png"
```

### Using with MoonScript

ORCA works great with MoonScript for cleaner syntax:

```moonscript
import StackView, TextBlock, ImageView from require "orca.ui"

class MyScreen extends StackView
  Width: 800
  Height: 600
  
  body: =>
    textblock Text: "Hello, ORCA!", FontSize: 24
    imageview Image: "path/to/image.png"
```

## Using the ORCA Binary

The `orca` binary is also installed and can be used to run ORCA projects:

```bash
orca path/to/project
```

This will load the project's `package.xml` and run the application.

## Development

### Building from Source

```bash
git clone https://github.com/corepunch/orca.git
cd orca
git submodule update --init --recursive
make
```

### Running Examples

```bash
make example  # Runs samples/Example
build/bin/orca samples/Banking
```

## License

MIT License - see [LICENSE](LICENSE) for details.

## Links

- [GitHub Repository](https://github.com/corepunch/orca)
- [Documentation](https://github.com/corepunch/orca/blob/main/README.md)
- [Quick Start Guide](https://github.com/corepunch/orca/blob/main/QUICKSTART.md)
