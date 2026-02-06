# ORCA Framework 

## Installation

### Installing LuaRocks and MoonScript

**LuaRocks** is Lua's package manager, and **MoonScript** is a programming language that compiles to Lua with a more elegant, Python-like syntax.

#### Linux

First, install LuaRocks:

```sh
# Ubuntu/Debian
sudo apt install -y luarocks

# Fedora
sudo dnf install -y luarocks

# Arch Linux
sudo pacman -S luarocks
```

Then install MoonScript via LuaRocks:

```sh
sudo luarocks install moonscript
```

#### macOS

Using Homebrew:

```sh
brew install luarocks
luarocks install moonscript
```

#### Verifying Installation

Check that MoonScript is installed correctly:

```sh
moonc -v
```

### Linux

To install the required dependencies on **Linux**, run the following commands:

```sh
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

For **Fedora**, use:

```sh
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

For **Arch Linux**, use:

```sh
sudo pacman -S libpng freetype2 libjpeg-turbo lua libxml2 lz4 zlib curl
```

### macOS

To install dependencies on **macOS**, use **Homebrew**:

```sh
brew install libpng freetype jpeg lua libxml2 lz4 zlib curl
```

## Building the Project

To build the project, run:

```sh
make
```

To run the project after building, use:

```sh
make run
```

For a clean build and immediate execution, use:

```sh
make andrun
```

This will clean the build, compile the project (which takes 3-5 seconds), and immediately run the application with the `../icui` data folder.

### Required Data Folder

To run the project, ensure that the `../icui` folder is present, as it contains necessary data files. Reach out to **@icherna** to obtain the required data packages.

## Usage

(Explain how to use the project here)

## Why MoonScript?

The Banking sample app demonstrates MoonScript's advantages over XML and pure Lua. See <a>samples/Banking</a> for a complete example.

### Benefits Over XML

**XML approach** (verbose configuration):
```xml
<ProjectReferenceLibrary>
  <ProjectReferenceItem Name="applications">applications</ProjectReferenceItem>
  <ProjectReferenceItem Name="assets">assets</ProjectReferenceItem>
  <ProjectReferenceItem Name="appwrite">lib/appwrite</ProjectReferenceItem>
</ProjectReferenceLibrary>
```

**MoonScript approach** (clean and expressive):
```moonscript
class App extends Application
  @include "applications.users"
  @include "applications.chat"
  
  @stylesheet require "tailwind"
  @stylesheet "assets/globals.css"
```

✅ **60% less code**  
✅ **Native language constructs instead of markup**  
✅ **Better IDE support and refactoring**

### Benefits Over Pure Lua

Let's compare the same UI component in both languages using ORCA's framework:

**Pure Lua with ORCA** (still verbose):
```lua
local StackView = require("orca.ui").StackView

local ContactCard = StackView:extend({
  apply = function(self)
    return "contact-card"
  end,
  
  body = function(self)
    img({ Src = "https://picsum.photos/64" })
    stack(".flex-col", function()
      p(".contact-card-title", self.user.name)
      p(".contact-card-description", "@" .. self.user["$id"])
    end)
  end
})
```

**MoonScript with ORCA** (clean and elegant):
```moonscript
import StackView from require "orca.ui"

class ContactCard extends StackView
  apply: => "contact-card"
  body: =>
    img Src: "https://picsum.photos/64"
    stack ".flex-col", ->
      p ".contact-card-title", @user.name
      p ".contact-card-description", "@"..@user["$id"]
```

### Key Advantages:

1. **Native Class Syntax**
   - No need to call `:extend()` explicitly
   - `extends` keyword is cleaner than method calls
   - `@` shorthand for `self` reduces noise

2. **Implicit Returns**
   - No explicit `return` statements needed
   - Functions automatically return their last expression

3. **Cleaner Method Definitions**
   ```lua
   -- Lua
   apply = function(self)
     return "contact-card"
   end
   ```
   vs
   ```moonscript
   -- MoonScript
   apply: => "contact-card"
   ```

4. **Import/Destructuring**
   ```moonscript
   import Account from require "model"
   import Application from require "routing"
   ```
   vs
   ```lua
   local model = require("model")
   local Account = model.Account
   local routing = require("routing")
   local Application = routing.Application
   ```

5. **Arrow Functions & String Interpolation**
   - Concise route definitions
   - Less punctuation clutter
   - More readable nested callbacks

### Real-World Example: Routing in ORCA

**MoonScript makes routing declarations elegant:**

```moonscript
class App extends Application
  "/": => Layout page.HomePage
  "/send-money": => Layout page.SendMoney
  "/settings": => Layout page.Settings
  "/tweets": => Layout page.Tweets
  "/new-tweet": => Layout page.NewTweet
  "/user/:user": => Layout page.ContactDetails, @params
  "/transaction/:transaction": => Layout page.TransactionDetails, @params
  "/search": => SearchPage!
```

**Same in pure Lua:**

```lua
local App = Application:extend({
  ["/"] = function(self)
    return Layout(page.HomePage)
  end,
  ["/send-money"] = function(self)
    return Layout(page.SendMoney)
  end,
  ["/settings"] = function(self)
    return Layout(page.Settings)
  end,
  ["/tweets"] = function(self)
    return Layout(page.Tweets)
  end,
  ["/new-tweet"] = function(self)
    return Layout(page.NewTweet)
  end,
  ["/user/:user"] = function(self)
    return Layout(page.ContactDetails, self.params)
  end,
  ["/transaction/:transaction"] = function(self)
    return Layout(page.TransactionDetails, self.params)
  end,
  ["/search"] = function(self)
    return SearchPage()
  end
})
```

Notice: **8 lines** in MoonScript vs **24 lines** in Lua for the exact same routing logic!

### The Bottom Line

Even with ORCA's simplified `extend()` API, MoonScript gives you:
- ✅ **40-60% less code** than equivalent Lua
- ✅ **Cleaner class definitions** without manual table construction
- ✅ **Compiles to readable Lua** (easy to debug, uses your `extend()` under the hood)
- ✅ **Full Lua compatibility** (use any Lua library)
- ✅ **Perfect for UI/DSL code** (see the Banking app's clean component structure)

For the complete working example, explore the <a>Banking sample app</a>.

## License

This project is licensed under the MIT License - see the [LICENCE](LICENCE) file for details.

