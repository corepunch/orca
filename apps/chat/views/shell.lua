local ui = require "orca.UIKit"
local core = require "orca.core"
local C = require "Chat/views/components"
local colors = C.colors

local Shell = {}
Shell.__index = Shell

local tabs = {
    { key = "Chats", route = "/", label = "Chats", icon = "message-circle" },
    { key = "New", route = "/new", label = "New chat", icon = "plus" },
    { key = "Profile", route = "/profile", label = "Profile", icon = "user-round" },
}

function Shell.new(screen, navigate)
    local self = setmetatable({ screen = screen, pages = {}, items = {}, routes = {} }, Shell)
    screen.Name = "ChatShell"
    local layout = ui.Grid { Rows = "1fr 86px", Background = colors.background }
    self.content = ui.PageHost { Name = "TabContent" }
    self.bar = ui.TabBar {
        Name = "TabBar", Spacing = 14, Padding = "22 12 22 16",
        Background = colors.surface, BorderWidthTop = 1, BorderWidthBottom = 0, BorderColor = colors.line,
        SelectionChanged = function(bar)
            if bar.SelectedValue ~= self.active then navigate(self.routes[bar.SelectedValue]) end
        end,
    }
    layout:addChild(self.content)
    layout:addChild(self.bar)
    screen:addChild(layout)
    local tabWidth = ("({../../../Width} - {../PaddingLeft} - {../PaddingRight} - {../Spacing} * %d) / %d")
        :format(#tabs - 1, #tabs)
    for _, tab in ipairs(tabs) do
        local item = C.navItem(tab.label, tab.icon, tab.key)
        self.bar:addChild(item)
        -- Bind to the finite viewport width, rather than the stack's intrinsic measure.
        item:attachPropertyProgram("Node.Width", tabWidth, "Expression", true)
        self.items[tab.key] = item
        self.routes[tab.key] = tab.route
        self.pages[tab.key] = ui.Page { Name = tab.key, Visible = false }
        self.content:addChild(self.pages[tab.key])
    end
    self.content:attachPropertyProgram("PageHost.ActivePage", "{../TabBar/SelectedValue}", "OneWay", true)
    return self
end

function Shell:has(key)
    return self.pages[key] and self.pages[key]:getFirstChild() ~= nil
end

function Shell:show(key, content)
    assert(self.items[key], "Unknown chat tab: " .. tostring(key))
    if not self:has(key) then
        assert(content, "Missing content for chat tab: " .. key)
        self.pages[key]:addChild(content)
    end
    local previous = self.active
    self.bar.SelectedValue = key
    for value, item in pairs(self.items) do
        local selected = value == key
        if item.IsSelected ~= selected then item.IsSelected = selected end
        if selected ~= (value == previous) then
            local color = selected and colors.accentText or colors.muted
            item:findChild("Tab" .. value .. "Icon", true).Foreground = color
            item:findChild("Tab" .. value .. "Label", true).Foreground = color
        end
    end
    self.active = key
    core.runAllPrograms(self.screen)
    return self.screen
end

return Shell
