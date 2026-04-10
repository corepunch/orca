local ui = require "orca.UIKit"

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
    local parts = {}
    local n = select('#', ...)
    for i = 1, n do parts[i] = tostring(select(i, ...)) end
    local text = table.concat(parts)
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
    local index, char = ui.consoleViewUnpack(self, x, y)
    local items = self.__items
    return items and items[index], index, char
  end,

  getIndexPosition = function(self, ...)
    return ui.consoleViewGetIndexPosition(self, ...)
  end,

  invalidate = function(self)
    self:Invalidate()
  end,
}

return TerminalView
