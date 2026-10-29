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
    for _, header in ipairs { "authorization", "cookie", "set%-cookie" } do
        local pattern = header:gsub("%a", function(letter) return "[" .. letter .. letter:upper() .. "]" end)
        value = value:gsub(pattern .. "%s*[:=][^\r\n]*", "[credentials redacted]")
    end
    local parts, cursor = {}, 1
    while true do
        local first, last, key = value:find("([%w_%-]+)[\"']?%s*[:=]%s*", cursor)
        if not first then parts[#parts + 1] = value:sub(cursor); break end
        parts[#parts + 1] = value:sub(cursor, last)
        local name = key:lower()
        if name:find("password") or name:find("secret") or name:find("token") or name:find("jwt")
            or name:find("session") or name == "code" or name == "otp" then
            local quote = value:sub(last + 1, last + 1)
            local finish
            if quote == '"' or quote == "'" then finish = value:find(quote, last + 2, true) end
            finish = finish or ((value:find("[\r\n,;}]", last + 1) or (#value + 1)) - 1)
            parts[#parts + 1] = "[redacted]"
            last = finish
        end
        cursor = last + 1
    end
    value = table.concat(parts)
    value = value:gsub("[%w._%%+%-]+@[%w.%-]+%.[%a]+", "[email redacted]")
    value = value:gsub("%+%d[%d \t%-]+%d", "[phone redacted]")
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
    if app.room and app.composer then app.drafts[app.room.id] = app.composer.Text or "" end
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
        app.shell, app.homeStatus, app.newStatus = nil, nil, nil
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
    -- The docked bar extends through the home indicator or keyboard to the bottom.
    local root, bar = screen:getFirstChild(), app.shell.bar
    local height = screen.Height
    if not root or not bar or height <= 0 then return end
    local bottom = root.ActualY + bar.ActualY + bar.ActualHeight
    local expected = 58 + math.max(screen.SafeAreaBottom, screen.KeyboardInsetBottom)
    if math.abs(bottom - height) <= 2 and math.abs(bar.ActualHeight - expected) <= 2 then return end
    self:show(string.format("Chat application bar is outside its expected bounds.\n" ..
        "screen=%.1fx%.1f rootHeight=%.1f barY=%.1f barHeight=%.1f safeBottom=%.1f keyboardInsetBottom=%.1f",
        screen.Width, height, root.ActualHeight, bar.ActualY, bar.ActualHeight,
        screen.SafeAreaBottom, screen.KeyboardInsetBottom), "Layout")
end

return Diagnostics
