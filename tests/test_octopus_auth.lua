package.path = "apps/shared/?.lua;" .. package.path
local Octopus = dofile("apps/octopus/api/octopus.lua")
local Browser = require "AppwriteAuth/browser"

local saved, locked, revoked, exchanges
local store = {
    load = function() return saved end,
    save = function(credential) saved = credential; return true end,
    clear = function()
        if locked then return nil end
        saved = nil
        return true
    end,
}
local function transport(request)
    local path = request.path
    if path == "/account/tokens/email" then
        assert(request.body.email == "reader@example.invalid")
        assert(next(request.headers) == nil)
        return { status = 201, body = { userId = "reader" } }
    end
    if path == "/account/sessions/token" then
        assert(request.body.userId == "reader")
        exchanges = (exchanges or 0) + 1
        if request.body.secret ~= "123456" and request.body.secret ~= "provider-secret" then
            return { status = 401, body = { type = "user_invalid_token" } }
        end
        return { status = 201, body = { secret = "private-session" } }
    end
    assert(request.headers["X-Appwrite-Session"] == "private-session", path)
    if path == "/account" then
        if revoked then return { status = 401 } end
        return { status = 200, body = { ["$id"] = "reader", email = "reader@example.invalid" } }
    end
    if path == "/account/sessions/current" then
        assert(request.method == "DELETE")
        return { status = 204 }
    end
    assert(path:match("/profiles/documents/reader$"))
    return { status = 200, body = { userId = "reader", username = "Reader" } }
end
local function newClient()
    return assert(Octopus.client { endpoint = "https://example.invalid/v1", projectId = "test",
        sessionStore = store, transport = transport })
end
local client = newClient()
assert(client:requestCode(" Reader@Example.Invalid "))
local session, err = client:verifyCode("other@example.invalid", "123456")
assert(not session and err.code == "verification_missing" and exchanges == nil)
session, err = client:verifyCode("reader@example.invalid", "000000")
assert(not session and err.code == "user_invalid_token" and exchanges == 1)
session = assert(client:verifyCode("reader@example.invalid", "123456"))
assert(session.user.id == "reader" and session.user.name == "Reader" and session.credential == nil)
assert(saved["X-Appwrite-Session"] == "private-session")
assert(not client:verifyCode("reader@example.invalid", "123456") and exchanges == 2)
client = newClient()
assert(client:restoreSession().user.id == "reader")
locked = true
session, err = client:signOut()
assert(not session and err.code == "session_storage" and not client:getSession() and saved)
locked = false
assert(client:signOut() and saved == nil)

local events, opened, signedIn, notices = {}, 0, 0, 0
local native = {
    supported = function() return true end,
    open = function(url, scheme)
        assert(url:match("^https://example%.invalid/v1/account/tokens/oauth2/apple%?"))
        assert(scheme == "appwrite-callback-test")
        opened = opened + 1
        return true
    end,
    poll = function() return table.remove(events, 1) end,
}
local app = { client = client, signedIn = function() signedIn = signedIn + 1 end,
    notice = function() notices = notices + 1 end }
local browser = Browser.new(app, { native = native, manual = true, callbackOnly = true,
    completeLink = client.completeProviderSignIn })
local callback = "appwrite-callback-test://oauth?userId=reader&secret=provider-secret"
assert(browser:start("apple") and not browser:start("apple") and opened == 1)
events = { { type = "url", url = callback }, { type = "callback", url = callback:gsub("oauth", "magic") } }
browser:poll()
assert(signedIn == 0 and notices == 1 and exchanges == 2)
assert(browser:start("apple"))
events = { { type = "callback", url = callback }, { type = "callback", url = callback } }
browser:poll()
assert(signedIn == 1 and exchanges == 3)
assert(client:signOut())
assert(browser:start("apple"))
assert(client:requestCode("reader@example.invalid"))
events = { { type = "callback", url = callback } }
browser:poll()
assert(signedIn == 1 and exchanges == 3 and not client:getSession())
assert(client:verifyCode("reader@example.invalid", "123456"))
revoked = true
client = newClient()
session, err = client:restoreSession()
assert(not session and err.status == 401 and not saved)
print("PASS: Octopus email code, provider callback isolation, secure restore and logout")
