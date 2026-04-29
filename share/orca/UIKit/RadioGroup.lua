-- RadioGroup: a horizontal row of styled pill buttons for mutually-exclusive
-- option selection.
--
-- Usage (Lua):
--   local RadioGroup = require "orca.UIKit.RadioGroup"
--   RadioGroup {
--     value    = "celsius",
--     options  = { { value = "celsius",    label = "°C  Celsius"    },
--                  { value = "fahrenheit", label = "°F  Fahrenheit" } },
--     on_change = function(v) ... end,
--     class    = "flex-row gap-2",  -- optional; overrides the container class
--   }
--
-- Usage (MoonScript):
--   RadioGroup
--     value: current_unit
--     options: { { value: "celsius", label: "°C  Celsius" }, ... }
--     on_change: (v) -> Settings\set_unit v
--
-- RadioGroup returns a StackView node containing one StackView "pill" per
-- option.  The active pill is highlighted with sky-500; inactive pills use
-- slate-700.  Clicking an inactive pill fires on_change(option.value).
local ui = require "orca.UIKit"

local function RadioGroup(opts)
  if type(opts) ~= "table" then
    error("RadioGroup: expected a table of options", 2)
  end

  local value      = opts.value
  local options    = opts.options or {}
  local on_change  = opts.on_change
  local cont_class = opts.class or "flex-row gap-2"

  return ui.StackView({ class = cont_class }, function()
    for _, opt in ipairs(options) do
      local is_active = (opt.value == value)

      local btn_class = is_active
        and "bg-sky-500 rounded-3 px-4 py-2"
        or  "bg-slate-700 rounded-3 px-4 py-2"

      local lbl_class = is_active
        and "text-sm font-bold text-white"
        or  "text-sm text-slate-400"

      local handler = nil
      if on_change and not is_active then
        local v = opt.value
        handler = function() on_change(v) end
      end

      ui.StackView({ class = btn_class, LeftButtonUp = handler }, function()
        ui.TextBlock { class = lbl_class, Text = opt.label or opt.value }
      end)
    end
  end)
end

return RadioGroup
