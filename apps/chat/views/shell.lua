local ui = require "orca.UIKit"
local core = require "orca.core"
local C = require "Chat/views/components"
local colors = C.colors

local Shell = {}
Shell.__index = Shell

-- Root pages share one ApplicationBar. The button for the page in view is
-- disabled, which also marks where you are.
local pages = {
    { key = "Chats", route = "/", label = "chats", icon = "message-circle" },
    { key = "New", route = "/new", label = "new chat", icon = "plus" },
    { key = "Profile", route = "/profile", label = "profile", icon = "user-round" },
}

function Shell.new(screen, navigate, menu)
    local self = setmetatable({ screen = screen, pages = {}, items = {}, routes = {} }, Shell)
    screen.Name = "ChatShell"
    local layout = ui.Grid { Rows = "1fr auto", Background = colors.background, Foreground = colors.ink }
    self.content = ui.PageHost { Name = "TabContent" }
    local buttons = {}
    for _, page in ipairs(pages) do
        buttons[#buttons + 1] = { "AppBar" .. page.key, page.icon, page.label, function()
            if self.active ~= page.key then navigate(page.route) end
        end }
        self.routes[page.key] = page.route
        self.pages[page.key] = ui.Page { Name = page.key, Visible = false }
        self.content:addChild(self.pages[page.key])
    end
    self.bar = C.appBar(buttons, menu)
    for _, page in ipairs(pages) do self.items[page.key] = self.bar:findChild("AppBar" .. page.key) end
    -- Paint the home-indicator inset without moving the controls into it.
    local footer = ui.Grid { Name = "AppBarFooter", Rows = "58px", Background = colors.chrome }
    footer:addChild(self.bar)
    layout:addChild(self.content)
    layout:addChild(footer)
    screen:addChild(layout)
    screen.RespectSafeArea = false
    for _, edge in ipairs { "Left", "Top", "Right" } do
        layout:attachPropertyProgram("Node.Padding" .. edge, "{Screen.SafeArea" .. edge .. "}", "OneWay", true)
    end
    footer:attachPropertyProgram("Node.PaddingBottom", "{Screen.SafeAreaBottom}", "OneWay", true)
    return self
end

function Shell:has(key)
    return self.pages[key] and self.pages[key]:getFirstChild() ~= nil
end

function Shell:show(key, content)
    assert(self.items[key], "Unknown chat page: " .. tostring(key))
    if not self:has(key) then
        assert(content, "Missing content for chat page: " .. key)
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
