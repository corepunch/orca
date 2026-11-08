-- Real settings action and XML confirmation; all service calls are offline.
local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
filesystem.init("apps/chat")
require("orca.core.startup").load_plugins()
local App, Chat = require "Chat/App", require "api.chat"
local savedAsync, queued = orca.async, {}
orca.async = function(fn, ...)
    local args = table.pack(...)
    queued[#queued + 1] = function() return fn(table.unpack(args, 1, args.n)) end
end
local function flushQueue()
    while true do
        local message = orca.system.peekMessage()
        if not message then return end
        if not message:is("Window.Paint") then orca.system.dispatchMessage(nil, message) end
    end
end
local function settle()
    flushQueue()
    while #queued > 0 do table.remove(queued, 1)(); flushQueue() end
end
local function construct()
    local client = Chat.demo()
    local app = App { client = client, language = "en" }
    app.show = function(self, route) self.screen = self:dispatch(route or "/").view; settle() end
    return app, client
end
local function click(app, name, deferred)
    assert(app.screen:findChild(name, true), "Missing " .. name):send("Node.LeftButtonUp")
    flushQueue()
    if not deferred then settle() end
end

local app, client = construct()
local calls, delete = 0, client.deleteAccount
client.deleteAccount = function(self, confirmation) calls = calls + 1; return delete(self, confirmation) end
app:show("/profile")
click(app, "PrivacyPolicy")
local bundled = assert(filesystem.readTextFile("Chat/assets/privacy.md"))
local master = assert(io.open("docs/chat/privacy.md", "rb"))
assert(bundled == master:read("*a"), "The bundled privacy policy must match its public source")
master:close()
assert(app.screen:findChild("PrivacyPolicyText", true).Source == bundled,
    "Settings shows the complete bundled policy without requiring the network")
app:show("/profile")
click(app, "DeleteAccount")
local warning = assert(app.screen:findChild("DeleteAccountWarning", true))
assert(core.getLocalizedString(warning, warning.TextResourceID) == "Delete your account permanently?" and calls == 0,
    "Settings opens an explicit confirmation without deleting: " .. tostring(warning:getTextContent()) .. " / " .. tostring(calls))
assert(app.screen:findChild("ConfirmDeleteAccount", true) and app.screen:findChild("CancelDeleteAccount", true),
    "The authored XML exposes both confirm and cancel")
app.screen:UpdateLayout(390, 844)
click(app, "CancelDeleteAccount")
assert(client:getSession() and calls == 0 and app.screen:findChild("DeleteAccount", true),
    "Cancel preserves the account and returns to settings")
click(app, "DeleteAccount")
click(app, "ConfirmDeleteAccount", true)
click(app, "ConfirmDeleteAccount", true)
-- Dispatch both queued clicks before the scheduled service operation.
table.remove(queued, 1)()
table.remove(queued, 1)()
assert(calls == 0 and app.accountDeletionPending and not app.shell,
    "Confirmation discards cached private pages before scheduling deletion")
settle()
assert(calls == 1 and not client:getSession() and not app.accountDeletionPending
    and app.screen:findChild("RequestCode", true), "Double tapping only deletes once and returns to sign-in")
assert(app.status:getTextContent() == "Your account has been deleted.", "Completed mock deletion is stated accurately")

app, client = construct()
calls, delete = 0, client.deleteAccount
client.deleteAccount = function(self, confirmation)
    calls = calls + 1
    if calls == 1 then return nil, { code = "network", message = "Offline" } end
    return delete(self, confirmation)
end
app:show("/delete-account")
click(app, "ConfirmDeleteAccount")
assert(client:getSession() and app.screen:findChild("DeleteAccountStatus", true):getTextContent() == "Offline"
    and app.screen:findChild("CancelDeleteAccount", true).Visible, "A failed request keeps the account and offers retry or cancel")
click(app, "ConfirmDeleteAccount")
assert(calls == 2 and not client:getSession(), "Retry finishes a previously unaccepted request")

app, client = construct()
calls = 0
client.deleteAccount = function(self)
    calls = calls + 1
    self.session = nil
    if calls == 1 then
        self.accountDeletion = { accepted = true, userId = "me" }
        return nil, { code = "session_storage", message = "Keychain locked" }
    end
    self.accountDeletion = nil
    return { accepted = true, deleted = false }
end
app:show("/delete-account")
click(app, "ConfirmDeleteAccount")
assert(not client:getSession() and app.accountDeletionPending
    and not app.screen:findChild("CancelDeleteAccount", true).Visible, "Accepted deletion cannot be cancelled during local cleanup")
app:show("/profile")
assert(app.screen:findChild("ConfirmDeleteAccount", true) and not app.screen:findChild("ProfileHeader", true),
    "Navigation cannot expose old account data while secure erasure is pending")
click(app, "ConfirmDeleteAccount")
assert(calls == 2 and not app.accountDeletionPending and app.screen:findChild("RequestCode", true),
    "Device cleanup can retry after the server has disabled the account")
assert(app.status:getTextContent():find("has started", 1, true), "Accepted background work is not reported as completed deletion")
orca.async = savedAsync
print("PASS: Chat XML account-deletion confirmation, cancellation, retry and local erasure")
