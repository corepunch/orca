-- Session routing exercises the real app views without a live Appwrite service.
local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
filesystem.init("apps/chat")
require("orca.core.startup").load_plugins()

local App = require "Chat/App"
local Chat = require "api.chat"
local savedAsync = orca.async
orca.async = function(fn, ...) return fn(...) end

local function appFor(client)
    local app = App { client = client }
    -- Keep route/view construction real without painting an uninitialized renderer.
    app.show = function(self, route) self.screen = self:dispatch(route or "/").view end
    return app
end
local function click(app, name)
    assert(app.screen:findChild(name, true), "Missing action: " .. name):send("Node.LeftButtonUp")
    core.flushQueue()
    while true do
        local message = orca.system.peekMessage()
        if not message then break end
        if not message:is("Window.Paint") then orca.system.dispatchMessage(app.screen, message) end
    end
    core.flushQueue()
end
local function mock()
    return Chat.mock { clock = function() return 1700000000 end }
end
local function login(client)
    return assert(client:signIn("demo@example.invalid", "password123"))
end

local client, attempts = mock(), 0
client.restoreSession = function(self)
    attempts = attempts + 1
    if attempts == 1 then return nil, { code = "network", message = "Offline" } end
    return login(self)
end
local app = appFor(client)
app.restorePending = true
app:show("/")
assert(app.restorePending and attempts == 1 and app.screen:findChild("RetrySessionRestore", true),
    "An offline restore must offer retry and keep restoration pending")
assert(not app.screen:findChild("RequestCode", true) and not app.screen:findChild("Room1", true),
    "An offline restore must not expose login or stale conversation views")
click(app, "RetrySessionRestore")
assert(attempts == 2 and not app.restorePending and client:getSession() and app.screen:findChild("Room1", true),
    "Retry must restore the session and open conversations without another login")
app:show("/")
assert(attempts == 2, "A restored session must not be reloaded on every navigation")

for _, err in ipairs {
    { code = "unauthenticated", status = 401 },
    { code = "user_unauthorized", status = 401 },
    { code = "user_blocked", status = 403 },
} do
    client = mock()
    client.restoreSession = function() return nil, err end
    app = appFor(client)
    app.restorePending = true
    app:show("/")
    assert(not app.restorePending and not client:getSession() and app.screen:findChild("RequestCode", true),
        "A missing or revoked session must route to authentication")
end

client = mock()
client.restoreSession = function() return nil, { code = "session_storage", status = 503 } end
app = appFor(client)
app.restorePending = true
app:show("/")
assert(app.restorePending and app.screen:findChild("RetrySessionRestore", true),
    "Locked secure storage must offer retry instead of treating the user as signed out")

client = mock()
login(client)
local signOut = client.signOut
client.signOut = function(self)
    assert(signOut(self))
    return nil, { code = "network", message = "Offline revocation" }
end
app = appFor(client)
app:show("/")
assert(app.screen:findChild("Room1", true))
app:signOut()
assert(not client:getSession() and not app.shell and app.screen:findChild("RequestCode", true)
    and not app.screen:findChild("Room1", true),
    "Offline logout must remove authenticated views after local credentials are cleared")
assert(app.status:getTextContent() == "Offline revocation", "Offline revocation failure must remain visible")

client, attempts = mock(), 0
login(client)
signOut = client.signOut
client.signOut = function(self)
    attempts = attempts + 1
    assert(signOut(self))
    if attempts == 1 then return nil, { code = "session_storage", message = "Keychain locked" } end
    return true
end
app = appFor(client)
app:show("/")
app:signOut()
assert(not client:getSession() and not app.shell and app.logoutPending
    and app.screen:findChild("RetrySignOut", true) and not app.screen:findChild("Room1", true),
    "Failed secure erasure must hide conversations and offer an explicit sign-out retry")
click(app, "RetrySignOut")
assert(attempts == 2 and not app.logoutPending and app.screen:findChild("RequestCode", true),
    "Successful erasure retry must finish logout and show authentication")

orca.async = savedAsync
print("PASS: Chat session restoration and logout routing")
