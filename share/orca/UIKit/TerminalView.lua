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
    local items = self.__items or {}
    self.__items = items
    local len = #items
    if item ~= nil then
      len = len + 1
      items[len] = item
    else
      len = 0
      self.__items = {}
    end
    return ui.ConsoleView.println(self, len, ...)
  end,

  erase = function(self)
    self.__items = {}
    return ui.ConsoleView.erase(self)
  end,

  unpack = function(self, x, y)
    local index, char = ui.ConsoleView.unpack(self, x, y)
    local items = self.__items
    return items and items[index], index, char
  end,
}

return TerminalView
