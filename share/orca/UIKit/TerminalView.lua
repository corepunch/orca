local ui = require "orca.UIKit"

local UINT32 = 0x100000000

local function to_signed32(value)
  if value >= 0x80000000 then
    return value - UINT32
  end
  return value
end

local function unpack_index_glyph(packed)
  if packed == nil then
    return nil, nil
  end
  return (packed >> 32) & 0xffffffff, string.char(packed & 0xff)
end

local function unpack_position(packed)
  if packed == nil then
    return nil, nil
  end
  return to_signed32(((packed >> 32) & 0xffffffff) ~ 0x80000000),
    to_signed32((packed & 0xffffffff) ~ 0x80000000)
end

local TerminalView = ui.ConsoleView:extend {
  onAwake = function(self)
    self.__items = {}
  end,

  selectedItem = function(self)
    local items = self.__items
    return items and items[self.SelectedIndex]
  end,

  numItems = function(self)
    local items = self.__items
    return items and #items or 0
  end,

  println = function(self, item, ...)
    local n = select('#', ...)
    local text
    if n == 1 then
      text = tostring(select(1, ...))
    elseif n == 0 then
      text = ""
    else
      local parts = {}
      for i = 1, n do parts[i] = tostring(select(i, ...)) end
      text = table.concat(parts)
    end
    if item ~= nil then
      local items = self.__items or {}
      self.__items = items
      local len = #items + 1
      items[len] = item
      self:Println { Index = len, Text = text }
      return len
    else
      self.__items = {}
      self:Println { Index = 0, Text = text }
      return 0
    end
  end,

  erase = function(self)
    self.__items = {}
    self:Erase()
  end,

  unpack = function(self, x, y)
    local index, char = unpack_index_glyph(self:send("ConsoleView.Unpack", x or 0, y or 0))
    local items = self.__items
    local item = (items and index ~= nil) and items[index] or nil
    return item, index, char
  end,

  getIndexPosition = function(self, index, offx, offy, global)
    return unpack_position(self:send("ConsoleView.GetIndexPosition",
      index or 0,
      offx or 0,
      offy or 0,
      not not global))
  end,

  invalidate = function(self)
    self:Invalidate()
  end,
}

return TerminalView
