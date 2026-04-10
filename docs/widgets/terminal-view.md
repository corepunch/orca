# TerminalView

`TerminalView` is a character-cell text widget built on top of the `ConsoleView` C component. It renders monospaced text in a fixed-size grid (like an old-school terminal), and extends that rendering engine with **item tracking** so that each line of text can carry an arbitrary Lua value — making it straightforward to build tree views, menus, log consoles, property inspectors, inline text editors, and any other widget where you need to map a screen position back to a data item.

---

## Architecture

The widget is split into two layers:

| Layer | File | Purpose |
|---|---|---|
| **ConsoleView** | `plugins/UIKit/ConsoleView.c` | C component: allocates the character buffer, renders glyphs, handles scroll and mouse routing |
| **TerminalView** | `share/orca/UIKit/TerminalView.lua` | Lua extension: wraps `ConsoleView` with per-line item storage (`__items`), a `selectedItem` accessor, `numItems` count, and overrides of `println`/`erase`/`unpack` |

In your Lua code, you always work with `ui.TerminalView`. `ui.ConsoleView` is available if you need the bare buffer without item tracking.

---

## The Character Buffer

`ConsoleView` maintains a flat `uint32_t` array of `BufferWidth × BufferHeight` cells. Every cell stores:

| Bits | Field | Notes |
|---|---|---|
| 7–0 | glyph | ASCII character code |
| 11–8 | foreground colour | 4-bit palette index (0–15) |
| 15–12 | background colour | 4-bit palette index (0–15) |
| 31–16 | item index | 16-bit integer passed to `println` |

Each glyph is rendered in a fixed 8×16 pixel cell (`CONSOLE_CHAR_WIDTH = 8`, `CONSOLE_CHAR_HEIGHT = 16`). The physical size of the widget in pixels is therefore `BufferWidth × 8` by `BufferHeight × 16` at most, though only the rows written so far (`ContentHeight`) are drawn.

---

## Key Properties

### ConsoleView properties (set in XML or Lua)

| Property | Type | Default | Description |
|---|---|---|---|
| `BufferWidth` | `int` | `256` | Number of character columns |
| `BufferHeight` | `int` | `256` | Number of character rows |
| `Cursor` | `int` | `0` | Current write position (cell index) |
| `SelectedIndex` | `int` | `0` | 1-based index of the highlighted row; 0 means none |
| `DropShadow` | `bool` | `false` | Render a drop shadow behind the widget |

### TerminalView adds (Lua-side only)

| Field / Method | Description |
|---|---|
| `self.__items` | `table` — list of items, one per row printed (managed by `println`) |
| `self:numItems()` | Returns `#self.__items` (or 0 before the first `println`) |
| `self:selectedItem()` | Returns `self.__items[self.SelectedIndex]` |

---

## Methods

### `println(item, ...)` — write a row

```lua
cv:println(item, ...)
```

**TerminalView** appends `item` to `__items`, then dispatches a `ConsoleView.Println` message with the item's 1-based index. The extra `...` arguments are converted to strings (via `tostring`) and concatenated into the current row.

Passing `nil` as `item` writes a header/separator row that has no associated item and **resets `__items`** to an empty table. This is convenient for separating sections of output:

```lua
tv:println(nil, "\x21[0;15m== Section ==")   -- header row, no item
tv:println(some_object, "  ", some_object.name)
```

**ConsoleView** dispatches the `Println` message directly using table syntax:

```lua
cv:Println { Index = 42, Text = "text for row 42" }
```

Return value: TerminalView's `println` returns the item index stored for this row (`len`), or 0 if `item` was nil.

### `erase()` — clear the buffer

```lua
tv:erase()    -- TerminalView
cv:Erase()    -- ConsoleView (message dispatch)
```

Zeroes the entire character buffer, resets `Cursor` to 0, `ContentHeight` to 0.  
**TerminalView** also clears `__items`.  
Call this at the start of every `onPaint` to redraw from scratch.

### `unpack(x, y)` → `item, index, char`

```lua
local item, index, char = tv:unpack(x, y)
```

Given screen-space pixel coordinates `(x, y)`, transforms them into buffer space, looks up the cell, and returns:

- **TerminalView**: `item` (the Lua value from `__items`), `index` (1-based row index), `char` (the ASCII character as a 1-byte string)
- **ConsoleView**: `index`, `char`

Returns `nil, 0, "\0"` when the point is outside the buffer.

Used in mouse event handlers to determine which item the user clicked.

### `getIndexPosition(index [, offx [, offy [, world]]])` → `x, y`

```lua
local x, y = cv:getIndexPosition(index, 0, 0, false)
```

Scans the buffer for the first cell whose stored index equals `index` and returns its pixel position, optionally offsetting by `offx` / `offy` character cells. Pass `true` as the fourth argument to get world-space coordinates (multiplied by the node's transformation matrix); omit it or pass `false` for buffer-local coordinates (with scroll applied).

Returns nothing when `index == 0` or the index is not found.

### `invalidate()` — request repaint

```lua
tv:invalidate()    -- TerminalView
cv:Invalidate()    -- ConsoleView (message dispatch)
```

Posts a paint message to the widget. Call this after changing `SelectedIndex` or any other property that should update the display.

---

## Inline Escape Codes

`println` supports a small set of VT100-inspired inline escape sequences triggered by the byte `\x21` (`!`) followed by `[`. These control the foreground/background colour and cursor position without affecting items.

| Escape | Effect |
|---|---|
| `\x21[<fg>;<bg>m` | Set foreground to `fg`, background to `bg` (4-bit palette indices 0–15) |
| `\x21[<n>m` | Set foreground to `n`, background to 0 |
| `\x21[<col>;<row>f` | Move cursor to column `col`, row `row` |
| `\x21[<n>c` | Override item index for subsequent characters to `n` |
| `\x21[G` | Move cursor to column 0 of the current row |

Colour indices follow the classic 16-colour terminal palette:

| Index | Name | Index | Name |
|---|---|---|---|
| 0 | Black | 8 | Dark grey |
| 1 | Dark red | 9 | Bright red |
| 2 | Dark green | 10 | Bright green |
| 3 | Dark yellow | 11 | Bright yellow |
| 4 | Dark blue | 12 | Bright blue |
| 5 | Dark magenta | 13 | Bright magenta |
| 6 | Dark cyan | 14 | Bright cyan |
| 7 | Light grey | 15 | White |

---

## XML Usage

```xml
<!-- Plain ConsoleView — only needs the C component -->
<ConsoleView Name="Log"
             BufferWidth="80" BufferHeight="40"
             Width="640" Height="400" />

<!-- TerminalView — loaded as a Lua-class placeholder -->
<LayerPrefabPlaceholder
    Name="Tree"
    Width="240" Height="600"
    BufferWidth="80" BufferHeight="128"
    PlaceholderTemplate="orca.UIKit.TerminalView" />
```

The `LayerPrefabPlaceholder` approach is needed for `TerminalView` because it is a Lua class, not a C component. The `PlaceholderTemplate` attribute names the Lua module to instantiate.

---

## Lua Usage

### Basic log console

```lua
local ui = require "orca.UIKit"

local log = screen + ui.TerminalView {
  Name         = "Log",
  Width        = 640,
  Height       = 400,
  BufferWidth  = 80,
  BufferHeight = 40,
}

-- Write lines (item = the log entry table)
for _, entry in ipairs(log_entries) do
  log:println(entry, "\x21[2;0m", os.date("%H:%M:%S"), "  ", entry.message)
end
log:invalidate()
```

### Extending TerminalView for a custom widget

All widgets in the editor sample extend `ui.TerminalView` via `:extend { … }`:

```lua
local MyList = ui.TerminalView:extend {
  -- Rebuild output on every repaint
  onPaint = function(self)
    self:erase()
    for _, item in ipairs(self.items) do
      self:println(item, "\x21[7;0m ", item.label, " ")
    end
  end,

  -- Click: select item and notify
  onLeftMouseDown = function(self, _, x, y)
    local item, index = self:unpack(x, y)
    if item then
      self.SelectedIndex = index
      self:invalidate()
      if self.onSelect then self.onSelect(item) end
    end
  end,
}

return MyList
```

### Keyboard-driven inline editor (TextInput pattern)

```lua
local TextInput = ui.TerminalView:extend {
  Height = 16,                  -- single character row
  onPaint = function(self)
    self:erase()
    self:println(nil, "\x21[15;0m ", self.text or "", " ")
    self.Cursor = (self.caret or 0) + 1   -- show blinking cursor
  end,
  onChar = function(self, _, event)
    local t, c = self.text or "", self.caret or 0
    self.text  = t:sub(1, c) .. event.text .. t:sub(c + 1)
    self.caret = c + 1
    self:invalidate()
  end,
  onKeyDown = function(self, _, event)
    if event.text == "enter" then
      if self.onAccept then self.onAccept(self.text) end
      self:removeFromParent()
    elseif event.text == "escape" then
      self:removeFromParent()
    end
  end,
}
```

---

## Use Cases

| Use case | How TerminalView helps |
|---|---|
| **Log / output console** | Each log entry is a `println` item; `selectedItem()` retrieves the selected entry for detail display |
| **Tree view** | Recursive `println` calls build indented rows; items are tree nodes; clicking calls `unpack` to navigate |
| **Dropdown menu** | Each row is a menu item; `SelectedIndex` tracks hover; `unpack` on mouse-up triggers the callback |
| **Property inspector** | Rows are property objects; inline colour codes highlight names and values |
| **Single-line text input** | `println(nil, ...)` renders the text and caret on a 1-row buffer; `onChar`/`onKeyDown` update the text |
| **Retro / pixel graphics** | Characters act as coloured "pixels" in an 8×16 grid; palette codes drive colour |

---

## Two-Layer Design

```
┌───────────────────────────────────────────────────┐
│                  TerminalView (Lua)                │
│  • __items[]   – one value per row                 │
│  • println(item, ...) – appends to __items         │
│  • selectedItem() / numItems()                     │
│  • erase()  – clears buffer + __items              │
│  • unpack(x,y) → item, index, char                 │
└───────────────────────┬───────────────────────────┘
                        │  extends (ConsoleView:extend)
┌───────────────────────▼───────────────────────────┐
│                  ConsoleView (C)                   │
│  • uint32_t _buffer[BufferWidth × BufferHeight]    │
│  • println(index, ...) – writes glyphs + metadata  │
│  • unpack(x,y) → index, char                       │
│  • erase() / invalidate() / getIndexPosition()     │
│  • DrawBrush handler → R_DrawConsole(...)          │
│  • ScrollWheel handler → _scroll.y adjustment      │
└───────────────────────────────────────────────────┘
```

The split keeps rendering concerns in C (fast, low-level) and business logic in Lua (flexible, garbage-collected). You can use `ConsoleView` directly when you only need raw character output with no associated data items, and reach for `TerminalView` — or your own `:extend` subclass — when each row needs to carry a Lua value.
