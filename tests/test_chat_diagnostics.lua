local orca = require "orca"
orca.init()
local core = require "orca.core"
local system = require "orca.system"
require("orca.filesystem").init("apps/chat")
require("orca.core.startup").load_plugins()
local App = require "Chat/App"
local Chat = require "api.chat"
local Diagnostics = require "Chat/diagnostics"
local async = require "orca.core.async"
local savedAsync = orca.async
orca.async = function(fn, ...) return fn(...) end

local client = Chat.mock()
assert(client:signIn("demo@example.invalid", "password123"))
local session = client:getSession()
local function sameSession()
    local current = client:getSession()
    return current and current.user.id == session.user.id and current.access_token == session.access_token
end
local app = App { client = client }
app.drafts.alex = "Unsent message"
app:show("/")
local home = app.screen
local function layout()
    core.flushQueue()
    core.runAllPrograms(app.screen)
    app.screen:UpdateLayout(390, 844)
end
layout()
app.diagnostics:checkLayout()
assert(app.screen == home, "A healthy layout must not display an error")

local originalHome = app.home
app.home = function() error("route render failed token=private-value", 0) end
app:show("/")
app.home = originalHome
assert(app.screen.Name == "ChatError", "Route failures must produce an error screen")
assert(app.diagnostics.lastReport:find("route render failed", 1, true))
assert(not app.diagnostics.lastReport:find("private-value", 1, true), "Reports must redact credentials")
layout()
local details = assert(app.screen:findChild("ErrorDetails", true))
local recover = assert(app.screen:findChild("RecoverChat", true))
assert(details.ActualHeight > 100 and recover.ActualHeight >= 44, "Error details and recovery must remain usable")
assert(sameSession() and app.drafts.alex == "Unsent message")
recover:send("Node.LeftButtonUp")
assert(app.screen.Name == "ChatShell", "Recovery must open a rebuilt chat shell")
assert(sameSession() and app.drafts.alex == "Unsent message", "Recovery must retain login and drafts")

async.report_error("tests/handler.lua:12: callback failed\nstack traceback:")
assert(app.screen.Name == "ChatError" and app.diagnostics.lastReport:find("tests/handler.lua:12", 1, true),
    "The installed async handler must show error locations")
local first = app.screen
async.report_error("second failure")
assert(app.screen == first, "Repeated failures must not stack error screens")
app.screen:findChild("RecoverChat", true):send("Node.LeftButtonUp")
layout()
local footer = app.screen:findChild("AppBarFooter", true)
footer.Height = 32000
layout()
app.diagnostics:checkLayout()
assert(app.screen.Name == "ChatError" and app.diagnostics.lastReport:find("footerHeight=", 1, true),
    "Oversized footer failures must show measured geometry even without an exception")
app.screen:findChild("RecoverChat", true):send("Node.LeftButtonUp")

local originalSelect = app.models.Rooms.select
app.models.Rooms.select = function() error("rooms callback exploded", 0) end
app:show("/")
app.models.Rooms.select = originalSelect
assert(not app.inboxRefreshBusy and app.screen.Name == "ChatError"
    and app.diagnostics.lastReport:find("rooms callback exploded", 1, true),
    "Caught refresh exceptions must release locks and retain their error details")
app.screen:findChild("RecoverChat", true):send("Node.LeftButtonUp")

local savedGet, savedDispatch, savedTranslate = system.getMessage, system.dispatchMessage, system.translateMessage
local calls = 0
system.getMessage = function()
    calls = calls + 1
    return { is = function(_, kind) return calls == 2 and kind == "Window.Closed" end }
end
system.dispatchMessage = function() error("event dispatcher failed", 0) end
system.translateMessage = function() end
app:run()
system.getMessage, system.dispatchMessage, system.translateMessage = savedGet, savedDispatch, savedTranslate
assert(app.screen.Name == "ChatError" and app.diagnostics.lastReport:find("event dispatcher failed", 1, true))
assert(sameSession() and app.drafts.alex == "Unsent message")

local redacted = Diagnostics.redact('password="secret-value with spaces" token=token-value\nAUTHORIZATION: Bearer bearer-value\n'
    .. 'Cookie: a_session=private-cookie\nuser=demo@example.invalid phone=+491234567890\n{"secret":"value with spaces"}')
for _, secret in ipairs { "secret-value", "with spaces", "token-value", "bearer-value", "private-cookie", "demo@example.invalid", "+491234567890" } do
    assert(not redacted:find(secret, 1, true), "Private values must not appear in reports")
end
assert(Diagnostics.redact("apps/chat/App.lua:123: test error"):find("App.lua:123", 1, true))
app:show("/")
app.room = app.rooms[1]
app:show("/thread")
app.composer.Text = "Typed since last navigation"
app:reportError("thread failed", "Test")
assert(app.drafts[app.room.id] == "Typed since last navigation", "Entering diagnostics must save the live composer draft")
async.set_error_handler(nil)
orca.async = savedAsync
print("PASS: Chat visible route/async/event/sync/layout errors, recovery and redaction")
