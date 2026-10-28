-- Offline push lifecycle and private Account target requests.
package.path = "apps/chat/?.lua;" .. package.path
package.preload["Chat/polling"] = function() return require "polling" end
local Chat = require "api.chat"
local json = require "api.json"
local Push = require "push"
local checks = 0
local function check(value, message) assert(value, message); checks = checks + 1 end
local record, requests, responses, beforeResponse = nil, {}, {}
local store = { load = function() return record end,
    save = function(value) record = value; return true end,
    clear = function() record = nil; return true end }
local configuration = {
    endpoint = "https://example.test/v1", projectId = "project", functionId = "chat",
    pushProviderId = "apns", pushStore = store,
    transport = function(request)
        requests[#requests + 1] = request
        if beforeResponse then beforeResponse(request) end
        assert(not request.url:find("/functions/", 1, true), "Push must never use the Function")
        local response = table.remove(responses, 1)
        assert(response, "Unexpected request: " .. request.url)
        if response.throws then error("offline") end
        return response
    end,
}
local client = assert(Chat.appwrite(configuration))
local function login(user)
    responses = {
        { status = 201, headers = { ["set-cookie"] = "a_session_project=private; HttpOnly" } },
        { status = 200, body = { ["$id"] = user or "alice" } },
        { status = 201, body = { jwt = "jwt" } },
    }
    assert(client:signIn("alice@example.test", "password"))
end
local token = string.rep("a", 64)
login()
responses = { { status = 404 }, { status = 201, body = { ["$id"] = "target" } } }
assert(client:registerPushTarget(token))
local create = requests[#requests]
check(create.method == "POST" and create.url:match("/account/targets/push$"), "Missing target is created directly through Account")
check(create.headers.Cookie == "a_session_project=private" and not create.headers.Authorization
    and not create.headers["X-Appwrite-Key"], "Target requests use only the private user session")
check(json.decode(create.body).identifier == token and json.decode(create.body).providerId == "apns",
    "APNs identifier and configured provider are sent")
local targetId = record.id
check(targetId == "target", "Create persists canonical target ID when Appwrite reuses a different target")
responses = { { status = 200, body = { ["$id"] = targetId } } }
assert(client:registerPushTarget(string.rep("b", 64)))
check(requests[#requests].method == "PUT" and requests[#requests].url:find(targetId, 1, true), "Token rotation updates the existing device target")

responses = { { throws = true } }
local ok, err = client:signOut()
check(not ok and err.code == "push_cleanup" and client:getSession() and record,
    "Failed target revocation preserves session and target for explicit logout retry")
responses = { { status = 204 }, { status = 204 } }
assert(client:signOut())
check(not record and not client:getSession() and requests[#requests - 1].url:match("/push$")
    and requests[#requests].url:match("/sessions/current$"), "Logout removes target before revoking session")

-- A persisted record for another account is never adopted or deleted as this user.
login("bob")
record = { id = targetId, userId = "alice" }
local count = #requests
ok, err = client:registerPushTarget(token)
check(not ok and err.code == "push_cleanup" and #requests == count, "Account switching cannot take over a previous user's target")
record = nil

-- A revoked login cannot strand the original target behind another account.
responses = { { status = 404 }, { status = 201, body = { ["$id"] = "saved-target" } } }
assert(client:registerPushTarget(token))
responses = { { status = 401, body = { type = "user_unauthorized" } } }
check(not client:refreshSession() and not client:getSession(), "Revoked session is cleared while device target metadata remains")
responses = {
    { status = 201, headers = { ["set-cookie"] = "a_session_project=other; HttpOnly" } },
    { status = 200, body = { ["$id"] = "alice" } },
}
ok, err = client:signIn("alice@example.test", "password")
check(not ok and err.code == "push_cleanup" and not client:getSession(), "Different owner cannot replace a revoked target's login")
login("bob")
check(client:getSession().user.id == "bob" and record.id == "saved-target", "Original target owner can authenticate again and recover cleanup")

-- Yielding registration keeps its lock until the Account request completes.
beforeResponse = function(request)
    if request.url:match("/targets/.*push$") then coroutine.yield("registering") end
end
responses = { { status = 200, body = { ["$id"] = "saved-target" } } }
local registering = coroutine.create(function() return client:registerPushTarget(token) end)
local resumed, state = coroutine.resume(registering)
check(resumed and state == "registering", "Target registration may yield without blocking other app work")
ok, err = client:signOut()
check(not ok and err.code == "push_cleanup" and client:getSession(), "Logout cannot race an in-flight target request")
beforeResponse = nil
assert(coroutine.resume(registering))
responses = { { status = 204 }, { status = 204 } }
assert(client:signOut())
check(not record and not client:getSession(), "Logout retry removes the completed registration")

login("bob")
responses = { { status = 404 }, { status = 201, body = { ["$id"] = "old-provider-target" } } }
assert(client:registerPushTarget(token))
configuration.pushProviderId = "new-apns"
client = assert(Chat.appwrite(configuration))
login("bob")
responses = { { status = 204 }, { status = 404 }, { status = 201, body = { ["$id"] = "new-provider-target" } } }
assert(client:registerPushTarget(token))
check(requests[#requests - 2].method == "DELETE" and json.decode(requests[#requests].body).providerId == "new-apns"
    and record.providerId == "new-apns", "Provider changes revoke the old target before creating one for the new provider")
configuration.pushProviderId = ""
client = assert(Chat.appwrite(configuration))
login("bob")
responses = { { status = 204 }, { status = 204 } }
assert(client:signOut())
check(not record and requests[#requests - 1].url:find("new-provider-target", 1, true),
    "Disabling provider config still revokes a persisted target on logout")
configuration.pushProviderId = "apns"
client = assert(Chat.appwrite(configuration))
login("alice")
responses = { { status = 404 }, { status = 201, body = { ["$id"] = "expired-target" } } }
assert(client:registerPushTarget(token))
responses = { { status = 401, body = { type = "user_unauthorized" } } }
ok, err = client:signOut()
check(not ok and err.code == "push_reauthenticate" and not client:getSession() and record.id == "expired-target",
    "Direct logout with a revoked cookie exposes original-owner login while retaining cleanup metadata")
responses = {
    { status = 201, headers = { ["set-cookie"] = "a_session_project=bob; HttpOnly" } },
    { status = 200, body = { ["$id"] = "bob" } },
}
ok, err = client:signIn("bob@example.test", "password")
check(not ok and err.code == "push_cleanup" and not client:getSession(), "Revoked logout recovery blocks a different account")
login("alice")
responses = { { status = 204 }, { status = 204 } }
assert(client:signOut())
check(not record, "Original-owner reauthentication makes revoked logout cleanup recoverable")

local storedValue, storedKey
local deviceStore = assert(require("push_store").new({ endpoint = "https://example.test/v1", projectId = "project" }, {
    supported = function() return true end,
    get = function(key) storedKey = key; return storedValue end,
    set = function(key, value) storedKey, storedValue = key, value; return true end,
    delete = function() storedValue = nil; return true end,
}))
assert(deviceStore.save { id = "device-target", userId = "alice" })
check(deviceStore.load().id == "device-target" and storedKey:match("^chat%-push:"), "Device metadata persists in a separately namespaced secure store")
assert(deviceStore.clear())
check(not deviceStore.load(), "Successful logout erases persisted device metadata")

local events, permissions, registered, opened = {}, 0, {}, {}
local active = { user = { id = "alice" } }
local app = {
    client = {
        getSession = function() return active end,
        registerPushTarget = function(_, value) registered[#registered + 1] = value; return true end,
    },
    models = { Rooms = { select = function() return { { id = "room" } } end } },
    openRoom = function(_, room) opened[#opened + 1] = room.id end,
}
local native = {
    supported = function() return true end,
    requestAuthorization = function() permissions = permissions + 1; return true end,
    poll = function() return table.remove(events, 1) end,
}
local timers = {}
local function createTimer(fn)
    local timer = { fn = fn, cancel = function(self) self.canceled = true end }
    timers[#timers + 1] = timer
    return timer
end
check(not Push.new(app, {}, { native = native, createTimer = createTimer }) and permissions == 0,
    "Unconfigured push never requests notification permission")
local now = 1000
local push = assert(Push.new(app, { pushProviderId = "apns" }, {
    native = native, createTimer = createTimer, clock = function() return now end }))
local function tick() return push:tick(function() return true end) end
events = { { type = "token", token = token }, { type = "open", conversationId = "room", messageId = "message", userId = "alice" } }
assert(tick())
check(permissions == 1 and #registered == 1 and opened[1] == "room", "Permission, target registration, and authorized conversation opening work")
assert(tick())
check(permissions == 1 and #registered == 1, "Steady state does not repeat permission or registration")
events = { { type = "open", conversationId = "room", messageId = "message", userId = "bob" },
    { type = "open", conversationId = "../bad", messageId = "message", userId = "alice" } }
assert(tick())
check(#opened == 1, "Wrong-account and malformed payloads cannot navigate")
events = { { type = "open", conversationId = "forbidden", messageId = "message", userId = "alice" } }
assert(tick())
check(#opened == 1 and not push.pending, "Unavailable conversation is discarded after authenticated room lookup")

active = nil
assert(tick())
events = { { type = "open", conversationId = "room", messageId = "message", userId = "alice" } }
assert(tick())
check(push.pending and #opened == 1, "Cold-start tap waits for session restoration")
active = { user = { id = "alice" } }
assert(tick())
check(#opened == 2 and #registered == 2, "Restored session registers device and opens the pending room")

app.models.Rooms.select = function() active = { user = { id = "bob" } }; return { { id = "room" } } end
events = { { type = "open", conversationId = "room", messageId = "message", userId = "alice" } }
assert(tick())
check(#opened == 2, "A stale room lookup cannot navigate after account changes")
assert(tick())
check(not push.pending, "Changing account discards prior account's pending tap")

local attempts = 0
app.client.registerPushTarget = function() attempts = attempts + 1; return attempts > 1 end
push.registered = nil
check(not tick() and tick() and attempts == 2, "Transient target failures retry without rejecting the chat session")
push:activate()
local timer = timers[#timers]
push:suspend()
timer.fn()
check(timer.canceled and not push.scheduler.job, "Queued canceled timers do not restart registration after logout")
events = { { type = "error", code = "permission_denied", message = "Denied" } }
assert(tick())
local deniedRequests = permissions
assert(tick())
check(permissions == deniedRequests, "Permission denial does not repeat the system prompt")
push:setForeground(true)
assert(tick())
check(permissions == deniedRequests + 1, "Foreground activation rechecks permission after settings may change")
events = { { type = "error", code = "registration_failed", message = "Offline" } }
assert(tick())
local failedRequests = permissions
assert(tick())
check(permissions == failedRequests, "Transient native failure waits before registration retry")
now = now + 2
assert(tick())
check(permissions == failedRequests + 1, "Transient native registration retries after backoff")
print("Chat push: " .. checks .. " checks passed")
