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
-- Keep routing and diagnostics real, but do not enqueue Paint against an
-- uninitialized headless renderer (the normal Application activation does).
app.activate_controller = function(self, controller)
    self:installErrorHandler()
    self.controller, self.screen = controller, controller.view
    return controller
end
app.drafts.alex = "Unsent message"
app:show("/")
local home = app.screen
local function flushEvents()
    -- Loaders can request a repaint independently of controller activation.
    -- Dispatch their other queued events, but keep this fixture renderer-free.
    while true do
        local event = system.peekMessage()
        if not event then break end
        if not event:is("Window.Paint") then system.dispatchMessage(app.screen, event) end
    end
end
local function layout()
    flushEvents()
    core.advanceFrame()
    core.runAllPrograms(app.screen)
    app.screen:UpdateLayout(app.screen.Width, app.screen.Height)
end
local function returnToChat()
    app.screen:findChild("RecoverChat", true):send("Node.LeftButtonUp")
    flushEvents()
end
layout()
app.diagnostics:checkLayout()
assert(app.screen == home, "A healthy layout must not display an error")

app:show("/new")
local search = app.screen
search.ResizeMode = "NoResize"
search.Width, search.Height = 430, 932
search.SafeAreaTop, search.SafeAreaBottom = 59, 34
for _, inset in ipairs { 0, 318, 352, 0 } do
    search.KeyboardInsetBottom = inset
    layout()
    app.diagnostics:checkLayout()
    assert(app.screen == search, "Showing, resizing or hiding the keyboard during user search must retain the chat screen")
    local footer = search:findChild("AppBar", true)
    local bottom = math.max(34, inset)
    assert(math.abs(footer.ActualY - (932 - bottom - 58)) < 1,
        "Search commands must stay above the keyboard or home indicator")
    assert(math.abs(footer.ActualY + footer.ActualHeight - 932) < 1,
        "The frosted search bar must extend to the full viewport bottom")
end
search.Width, search.Height = 390, 844
search.SafeAreaTop, search.SafeAreaBottom = 0, 0
app:show("/")
layout()

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
returnToChat()
assert(app.screen.Name == "ChatShell", "Recovery must open a rebuilt chat shell")
assert(sameSession() and app.drafts.alex == "Unsent message", "Recovery must retain login and drafts")

async.report_error("tests/handler.lua:12: callback failed\nstack traceback:")
assert(app.screen.Name == "ChatError" and app.diagnostics.lastReport:find("tests/handler.lua:12", 1, true),
    "The installed async handler must show error locations")
local first = app.screen
async.report_error("second failure")
assert(app.screen == first, "Repeated failures must not stack error screens")
returnToChat()
app.screen.KeyboardInsetBottom = 318
layout()
local bar = app.screen:findChild("AppBar", true)
bar.Height = 32000
layout()
app.diagnostics:checkLayout()
assert(app.screen.Name == "ChatError" and app.diagnostics.lastReport:find("barHeight=", 1, true)
    and app.diagnostics.lastReport:find("keyboardInsetBottom=318.0", 1, true),
    "Oversized bar failures must show measured geometry even without an exception")
returnToChat()

local originalSelect = app.models.Rooms.select
app.models.Rooms.select = function() error("rooms callback exploded", 0) end
app:show("/")
app.models.Rooms.select = originalSelect
assert(not app.inboxRefreshBusy and app.screen.Name == "ChatError"
    and app.diagnostics.lastReport:find("rooms callback exploded", 1, true),
    "Caught refresh exceptions must release locks and retain their error details")
returnToChat()

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
