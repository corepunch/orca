local ui = require "orca.UIKit"
local C = require "Chat/views/components"
local Diagnostics = {}
Diagnostics.__index = Diagnostics

local function message(err)
    if type(err) == "table" then err = err.message or err.code or "Unknown error" end
    local ok, value = pcall(tostring, err)
    return ok and value or "Unprintable error"
end

function Diagnostics.traceback(err)
    return debug.traceback(message(err), 2)
end

-- Reports contain error text and source locations, never request/session dumps.
-- Also hide common credentials and identifiers accidentally embedded in errors.
function Diagnostics.redact(err)
    local value = message(err)
    for _, header in ipairs { "[Aa]uthorization", "[Cc]ookie", "[Ss]et%-[Cc]ookie" } do
        value = value:gsub(header .. "%s*[:=][^\r\n]*", "[credentials redacted]")
    end
    value = value:gsub("([%w_%-]+)([\"']?%s*[:=]%s*[\"']?)([^%s,;\"'}]+)", function(key, separator, text)
        local name = key:lower()
        if name:find("password") or name:find("secret") or name:find("token") or name:find("jwt")
            or name:find("session") or name == "code" or name == "otp" then text = "[redacted]" end
        return key .. separator .. text
    end)
    value = value:gsub("[%w._%%+%-]+@[%w.%-]+%.[%a]+", "[email redacted]")
    value = value:gsub("%+%d[%d%s%-]+%d", "[phone redacted]")
    value = value:gsub("[%w_%-]+%.[%w_%-]+%.[%w_%-]+", function(token)
        return #token > 80 and "[token redacted]" or token
    end)
    return value:sub(1, 6000)
end

function Diagnostics.new(app)
    return setmetatable({ app = app }, Diagnostics)
end

function Diagnostics:controller(err, stage)
    local app = self.app
    if self.screen and app.screen == self.screen then return { view = self.screen } end
    if app.polling then app.polling:stop() end
    local details = Diagnostics.redact((stage or "Application") .. "\nRoute: " .. (app.currentRoute or "/")
        .. "\nPlatform: " .. tostring(PLATFORM or "unknown") .. "\n\n" .. message(err))
    self.lastReport = details
    io.stderr:write("Chat error: " .. details .. "\n")
    -- A separate Screen stays readable even when the underlying shell is broken.
    -- Fixed chrome rows keep the recovery UI independent of auto footer sizing.
    local layout = ui.Grid { Rows = "44px 52px 1fr 48px", Padding = "16", Spacing = 12,
        Background = C.colors.background, Foreground = C.colors.ink }
    layout:addChild(C.text("Chat error", { FontSize = 28, Foreground = C.colors.error }))
    layout:addChild(C.text("Something went wrong. Please share the details below.", { WordWrap = true }))
    local list = ui.ListBox { Name = "ErrorDetails", Spacing = 8 }
    list:addChild(C.text(details, { Name = "ErrorMessage", WordWrap = true, FontSize = 13 }))
    layout:addChild(list)
    layout:addChild(C.button("Return to chat", function()
        -- Rebuild the shell without signing out or discarding message drafts.
        app.shell, app.roomLists, app.homeStatus, app.newStatus = nil, nil, nil, nil
        app:show("/")
    end, { Name = "RecoverChat", Background = C.colors.accent }))
    local screen = app:frame(layout)
    screen.Name = "ChatError"
    self.screen = screen
    return { view = screen }
end

function Diagnostics:show(err, stage)
    if self.reporting then return end
    self.reporting = true
    local ok, failure = pcall(function()
        local controller = self:controller(err, stage)
        self.app.navigationEpoch = (self.app.navigationEpoch or 0) + 1
        self.app:activate_controller(controller)
    end)
    self.reporting = false
    if not ok then io.stderr:write("Could not display Chat error: " .. Diagnostics.redact(failure) .. "\n") end
end

-- Layout failures need not throw a Lua error. Check the visible shell after a
-- completed paint, when native dimensions and safe-area bindings are resolved.
function Diagnostics:checkLayout()
    local app, screen = self.app, self.app.screen
    if not screen or not app.shell or app.shell.screen ~= screen then return end
    local footer = screen:findChild("AppBarFooter", true)
    local root, bar = screen:getFirstChild(), app.shell.bar
    local height = screen.Height
    if not footer or not root or not bar or height <= 0 then return end
    local bottom = root.ActualY + footer.ActualY + footer.ActualHeight
    local expected = 58 + screen.SafeAreaBottom
    if math.abs(bottom - height) <= 2 and math.abs(footer.ActualHeight - expected) <= 2 then return end
    self:show(string.format("Chat footer is outside its expected bounds.\n" ..
        "screen=%.1fx%.1f rootHeight=%.1f footerY=%.1f footerHeight=%.1f barHeight=%.1f safeBottom=%.1f",
        screen.Width, height, root.ActualHeight, footer.ActualY, footer.ActualHeight,
        bar.ActualHeight, screen.SafeAreaBottom), "Layout")
end

return Diagnostics
