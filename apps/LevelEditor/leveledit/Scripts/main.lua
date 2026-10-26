local app = require "AppKit"
local keymap = require "AppKit.keymap"
local reg = { "app", "doc" }

app.ready = function()
  for _, name in ipairs(reg) do
    require("commands." .. name)
  end
end

keymap.add {
  ["q"] = "doc:raise1-qa",
  ["a"] = "doc:lower1-qa",
  ["w"] = "doc:raise1-ws",
  ["s"] = "doc:lower1-ws",
  ["e"] = "doc:raise1-ed",
  ["d"] = "doc:lower1-ed",
  ["r"] = "doc:raise1-rf",
  ["f"] = "doc:lower1-rf",
  ["shift+q"] = "doc:raise4-qa",
  ["shift+a"] = "doc:lower4-qa",
  ["shift+w"] = "doc:raise4-ws",
  ["shift+s"] = "doc:lower4-ws",
  ["shift+e"] = "doc:raise4-ed",
  ["shift+d"] = "doc:lower4-ed",
  ["shift+r"] = "doc:raise4-rf",
  ["shift+f"] = "doc:lower4-rf",
  ["alt+q"] = "doc:raise1-qa-smooth",
  ["alt+a"] = "doc:lower1-qa-smooth",
  ["alt+w"] = "doc:raise1-ws-smooth",
  ["alt+s"] = "doc:lower1-ws-smooth",
  ["alt+e"] = "doc:raise1-ed-smooth",
  ["alt+d"] = "doc:lower1-ed-smooth",
  ["alt+r"] = "doc:raise1-rf-smooth",
  ["alt+f"] = "doc:lower1-rf-smooth",
  ["alt+shift+q"] = "doc:raise4-qa-smooth",
  ["alt+shift+a"] = "doc:lower4-qa-smooth",
  ["alt+shift+w"] = "doc:raise4-ws-smooth",
  ["alt+shift+s"] = "doc:lower4-ws-smooth",
  ["alt+shift+e"] = "doc:raise4-ed-smooth",
  ["alt+shift+d"] = "doc:lower4-ed-smooth",
  ["alt+shift+r"] = "doc:raise4-rf-smooth",
  ["alt+shift+f"] = "doc:lower4-rf-smooth",
  ["ctrl+alt+w"] = "doc:texture-walls",
  ["ctrl+alt+i"] = "doc:texture-floor",
  ["ctrl+alt+v"] = "doc:texture-ceiling",
  ["1"] = "doc:set-mode-overview",
  ["2"] = "doc:set-mode-geometry",
  ["3"] = "doc:set-mode-faceedit",
  ["4"] = "doc:set-mode-lighting",
  ["p"] = "doc:toggle-portals",
}
