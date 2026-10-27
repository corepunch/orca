local ui = require "orca.UIKit"
local core = require "orca.core"
local Footer = require "orca.UIKit.Footer"
local C = require "Octopus/views/components"
local colors = C.colors

local Shell = {}
Shell.__index = Shell

-- Root pages share one ApplicationBar. The button for the page in view is
-- disabled, which also marks where you are.
Shell.pages = {
    { key = "Home", route = "/", label = "home", icon = "house" },
    { key = "Search", route = "/search", label = "search", icon = "search" },
    { key = "Write", route = "/write", label = "write", icon = "pen-line" },
    { key = "Me", route = "/me", label = "me", icon = "user-round" },
}

function Shell.new(screen, navigate, menu)
    local self = setmetatable({ screen = screen, pages = {}, items = {}, routes = {} }, Shell)
    screen.Name = "OctopusShell"
    self.content = ui.PageHost { Name = "TabContent" }
    local buttons = {}
    for _, page in ipairs(Shell.pages) do
        buttons[#buttons + 1] = { "AppBar" .. page.key, page.icon, page.label, function()
            if self.active ~= page.key then navigate(page.route) end
        end }
        self.routes[page.key] = page.route
        self.pages[page.key] = ui.Page { Name = page.key, Visible = false }
        self.content:addChild(self.pages[page.key])
    end
    self.bar = C.appBar(buttons, menu)
    for _, page in ipairs(Shell.pages) do self.items[page.key] = self.bar:findChild("AppBar" .. page.key) end
    local footer = Footer { Name = "AppBarFooter", Rows = "58px", Background = colors.chrome }
    footer:addChild(self.bar)
    Footer.layout(screen, self.content, footer, { Foreground = colors.ink })
    return self
end

function Shell:has(key)
    return self.pages[key] and self.pages[key]:getFirstChild() ~= nil
end

-- Replace a page's content, for pages whose structure depends on state.
function Shell:replace(key, content)
    C.clear(self.pages[key])
    return self:show(key, content)
end

function Shell:show(key, content)
    assert(self.items[key], "Unknown Octopus page: " .. tostring(key))
    if not self:has(key) then
        assert(content, "Missing content for Octopus page: " .. key)
        self.pages[key]:addChild(content)
    end
    self.content.ActivePage = key
    for value, item in pairs(self.items) do
        if item.IsEnabled ~= (value ~= key) then item.IsEnabled = value ~= key end
    end
    self.active = key
    core.runAllPrograms(self.screen)
    return self.screen
end

return Shell
