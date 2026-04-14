# Welcome to ORCA

ORCA is a lightweight, cross-platform UI framework powered by Lua. Write your application logic and UI **once** and run it on **mobile**, **desktop**, and **embedded** devices — from Android and iOS phones to Linux desktops, macOS laptops, and embedded Linux boards.

[▶ Live Demo](demo/orca.html){ .md-button .md-button--primary }

## Use Cases

| Platform | Examples |
|---|---|
| **Mobile** | iOS and Android apps built from a single codebase |
| **Desktop** | Linux, macOS, and Windows applications |
| **Embedded** | In-vehicle dashboards, instrument clusters, kiosk displays |

Because ORCA separates UI markup (XML) from application logic (Lua / MoonScript), the same codebase can be compiled and deployed across every target with minimal platform-specific glue code.

---

## Installation

The quickest way to get started is via **LuaRocks**:

```sh
luarocks install orca
```

Then in your Lua code:

```lua
local orca = require "orca"
local ui   = require "orca.ui"
```

See [LUAROCKS.md](../LUAROCKS.md) for detailed installation instructions, and [README.md](../README.md) for building from source.

---

## Your First Screen

### Project layout

```
MyProject/
├── package.xml          # Project configuration
└── Screens/
    └── Application.xml  # Main screen
```

### XML example

XML is the native declarative format for ORCA screens and components. It is the recommended format for graphics-heavy apps, instrument clusters, and any workflow that involves a visual editor.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Screen SYSTEM "https://corepunch.github.io/orca/schemas/orca.dtd">
<Screen Name="Application" Height="768" Width="1024">
  <Grid Name="Layout">
    <Grid.Columns Value="auto auto"/>
    <ImageView Name="Image" Source="MyProject/Images/logo" Stretch="None"/>
    <TextBlock Name="Greeting" FontSize="40" LayoutTransform="10 10 0 1 1"
               Text="Hello, ORCA!"/>
  </Grid>
</Screen>
```

> **Tip — use the DTD for autocomplete.**  
> The `<!DOCTYPE … SYSTEM "…/orca.dtd">` declaration tells your editor which elements and attributes are valid, enabling full autocomplete and inline validation. Always include it at the top of every screen file.

You can also reference the DTD locally after cloning the repository:

```xml
<!DOCTYPE Screen SYSTEM "../../tools/schemas/orca.dtd">
```

---

## Editor Setup

Good XML tooling makes a real difference. Below are recommended plugins for popular editors.

### VSCode — Red Hat XML (recommended)

1. Open the **Extensions** panel (`Ctrl+Shift+X` / `⌘⇧X`).
2. Search for **XML** and install the [Red Hat XML](https://marketplace.visualstudio.com/items?itemName=redhat.vscode-xml) extension.
3. Include the hosted `DOCTYPE` declaration at the top of every screen file (as shown in the XML example above). The Red Hat XML extension reads the `SYSTEM` URL automatically and provides full autocomplete and inline validation — no additional configuration needed.

**Speed up file creation with a snippet.** Add an XML snippet so you can type `orca` and press Tab to get the full boilerplate with the element type as the first editable field:

1. Open **File → Preferences → Configure User Snippets** and choose **xml.json** (or your workspace snippets file).
2. Paste:

```json
{
  "ORCA Element": {
    "prefix": "orca",
    "body": [
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>",
      "<!DOCTYPE ${1:Screen} SYSTEM \"https://corepunch.github.io/orca/schemas/orca.dtd\">",
      "<${1:Screen} Name=\"${2:MyElement}\" Height=\"${3:768}\" Width=\"${4:1024}\">",
      "\t$0",
      "</${1:Screen}>"
    ],
    "description": "New ORCA element file"
  }
}
```

Tab through the placeholders: **①** sets the element type (e.g. `Screen`, `Prefab`) and is mirrored automatically in the `DOCTYPE` and closing tag; **②–④** fill in `Name`, `Height`, and `Width`.

### Other editors

| Editor | Plugin / Built-in |
|---|---|
| **IntelliJ IDEA / CLion** | Built-in XML support — associate the DTD in *File → Settings → Languages & Frameworks → Schemas and DTDs* |
| **Vim / Neovim** | [vim-lsp](https://github.com/prabirshrestha/vim-lsp) with `lemminx` (Red Hat Language Server) |
| **Emacs** | `nxml-mode` (built-in) with `rng-c-load-schema` |
| **Sublime Text** | [LSP-lemminx](https://packagecontrol.io/packages/LSP-lemminx) via Package Control |

---

## Lua Example

Use Lua for full programmatic control over your UI and application logic.

```lua
local ui   = require "orca.ui"
local core = require "orca.core"

-- A simple card component
local ContactCard = ui.StackView:extend {
    apply = function(self)
        return "contact-card"
    end,

    body = function(self)
        img { src = self.avatarUrl }
        stack(".flex-col", function()
            p(".card-title",       self.user.name)
            p(".card-description", "@" .. self.user.id)
        end)
    end
}

return ContactCard
```

Run your project with:

```sh
build/bin/orca MyProject
```

---

## MoonScript Example

[MoonScript](https://moonscript.org) is a language that compiles to Lua and offers a cleaner, Python-like syntax — ideal for building larger ORCA applications.

### Install MoonScript via LuaRocks

```sh
luarocks install moonscript
```

Verify the installation:

```sh
moonc -v
```

### MoonScript screen component

```moonscript
import StackView from require "orca.ui"

class ContactCard extends StackView
    apply: => "contact-card"

    body: =>
        img src: @avatarUrl
        stack ".flex-col", ->
            p ".card-title",       @user.name
            p ".card-description", "@"..@user.id
```

### MoonScript routing

```moonscript
import Application from require "orca.routing"
import Layout      from require "root.Layout"
import page        from require "root.pages"

class App extends Application
    "/":        => Layout page.Home
    "/settings": => Layout page.Settings
    "/user/:id": => Layout page.UserDetail, @params
```

MoonScript compiles to readable Lua, so you get the best of both worlds: concise syntax during development and full Lua compatibility at runtime.

---

## Next Steps

- Explore the [sample applications](../samples/) for real-world examples.
- Read the [Module XML Guide](MODULE_XML_GUIDE.md) to learn how to define API bindings between C modules and Lua.
- Check [LUAROCKS.md](../LUAROCKS.md) for advanced LuaRocks configuration.
