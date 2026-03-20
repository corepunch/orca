-- Application.lua
-- Portrait/landscape responsive entry point for the Example app.
-- Loads ApplicationLayout.xml (which contains both layout variants)
-- and uses a repeating timer to detect orientation changes, toggling
-- which layout variant is visible.

local ref = require "orca.renderer"
local Layout = require "Example/Screens/ApplicationLayout"

return function()
    local screen = Layout()

    local landscape = screen:findChild("LandscapeRoot")
    local portrait  = screen:findChild("PortraitRoot")

    local current = "landscape"

    screen.onTimer = function(self, _, name)
        if name ~= "orientation" then return end
        local width, height = ref.getSize()
        local orientation = height > width and "portrait" or "landscape"
        if orientation == current then return end
        current = orientation
        landscape.Visible = (orientation == "landscape")
        portrait.Visible  = (orientation == "portrait")
    end

    screen:setTimer("orientation", 200)

    return screen
end
