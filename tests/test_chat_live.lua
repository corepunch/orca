-- Manual native end-to-end check. Never enroll this in the default test suite.
-- CHAT_LIVE=1 make test-chat-live
if os.getenv("CHAT_LIVE") ~= "1" then
    print("SKIP: native Chat live test (set CHAT_LIVE=1)")
    return
end

package.path = "apps/chat/?.lua;" .. package.path
io.stdout:setvbuf("line")
local Chat = require "api.chat"
local json = require "api.json"
local SessionStore = require "session_store"
local credentials = require "orca.credentials"
require "orca.renderer"
local core = require "orca.core"
local ui = require "orca.UIKit"
local system = require "orca.system"
local orca = require "orca"

local function read(path)
    local file = assert(io.open(path, "rb"), "Missing live accounts or photo fixture: " .. path)
    local bytes = file:read("*a")
    file:close()
    return bytes
end
local saved = json.decode(read(os.getenv("CHAT_LIVE_ACCOUNTS") or "apps/chat/backend/.live-accounts.json"))
assert(type(saved.accounts) == "table", "Run the live account setup first")
assert(credentials.supported(), "Native live session restoration requires secure credential storage on this platform")
local config = {
    endpoint = os.getenv("CHAT_LIVE_ENDPOINT") or os.getenv("APPWRITE_ENDPOINT") or "https://fra.cloud.appwrite.io/v1",
    projectId = os.getenv("CHAT_LIVE_PROJECT_ID") or os.getenv("APPWRITE_PROJECT_ID") or "69f1c06800389dc6a1a0",
    functionId = os.getenv("CHAT_LIVE_FUNCTION_ID") or os.getenv("APPWRITE_CHAT_FUNCTION_ID") or "orca-chat-api",
}
assert(config.endpoint:match("^https://"), "This test requires real Appwrite over HTTPS")
local started = os.time()
local run = tostring(started) .. ":" .. tostring({}):gsub("[^%w]", "")
local checks, clients, stores, messages, photos = 0, {}, {}, {}, {}
local function check(condition, message)
    assert(condition, message)
    checks = checks + 1
end
local function need(label, value, err)
    assert(value ~= nil and value ~= false, label .. " failed (" .. tostring(err and err.code or "unknown")
        .. ", HTTP " .. tostring(err and err.status or "none") .. ")")
    print("LIVE (" .. (os.time() - started) .. "s): " .. label)
    return value
end
local function find(rows, id)
    for _, row in ipairs(rows) do if row.id == id or row.user_id == id then return row end end
end
local function forbidden(label, value, err)
    check(value == nil and type(err) == "table" and err.status == 403, label .. " must return HTTP 403")
end
local function makeClient(name)
    local options = { endpoint = config.endpoint, projectId = config.projectId, functionId = config.functionId,
        sessionStore = stores[name] }
    -- Omit transport overrides: every request goes through api.orca_transport
    -- and the native network.fetch binding, including authentication.
    local client = need("Create native client", Chat.appwrite(options))
    clients[#clients + 1] = client
    check(client.mode == "appwrite", "The live test must stay in Appwrite mode")
    return client
end
local function trackMessage(client, message)
    messages[#messages + 1] = { client = client, id = message.id }
    return message
end

local function exercise()
    local users = {}
    for _, name in ipairs { "alice", "bob", "carol" } do
        local account = saved.accounts[name]
        assert(account and account.email and account.id and account.username, "Missing dedicated test account: " .. name)
        -- Namespace real Keychain calls so the interactive app's saved login is
        -- never overwritten. The storage backend remains orca.credentials.
        local prefix = "chat-live:" .. run .. ":" .. name .. ":"
        stores[name] = assert(SessionStore.new(config, {
            supported = credentials.supported,
            get = function(key) return credentials.get(prefix .. key) end,
            set = function(key, value) return credentials.set(prefix .. key, value) end,
            delete = function(key) return credentials.delete(prefix .. key) end,
        }))
        local client = makeClient(name)
        local session = need("Native password sign-in for " .. name,
            client:signIn(account.email, account.password or saved.password))
        check(session.user.id == account.id, "Signed-in identity must match the provisioned test account")
        check(need("Read private stored session", stores[name].load()) ~= nil, "Session must be stored securely")
        users[name] = client
    end
    local alice, bob, carol = users.alice, users.bob, users.carol
    print("PASS: isolated native password sessions for three test accounts")

    local restored = makeClient("alice")
    check(not restored:getSession(), "A recreated client starts without a JWT")
    local session = need("Restore native Account session", restored:restoreSession())
    check(session.user.id == saved.accounts.alice.id, "Restoration must recover Alice's identity")
    -- Both clients share the same restored Account cookie. Revoke it once;
    -- keep the original cleanup entry only if restoration failed above.
    for index, client in ipairs(clients) do
        if client == alice then table.remove(clients, index); break end
    end
    alice = restored
    -- Force only the local JWT deadline. Account validation and JWT exchange
    -- still go to Appwrite; this avoids a fifteen-minute manual-test delay.
    alice.session.expires_at = 0
    need("Renew JWT before inbox lookup", alice:listRooms())
    check(alice:getSession().expires_at > os.time(), "An expired JWT deadline must renew through native Account HTTP")
    print("PASS: real Keychain restore and native JWT renewal")

    local found = need("Find Bob by username", alice:findPeople("@" .. saved.accounts.bob.username))
    check(find(found, saved.accounts.bob.id), "Username discovery must find Bob")
    local room = need("Create direct conversation", alice:createRoom("Native live Bob", { saved.accounts.bob.id }, { kind = "direct" }))
    check(room.kind == "direct" and room.visibility == "private", "Direct chat must be private")
    local reverse = need("Reopen direct conversation as Bob", bob:createRoom("Native live Alice", { saved.accounts.alice.id }, { kind = "direct" }))
    check(reverse.id == room.id, "Both people must open the same direct conversation")
    check(find(need("List contacts", alice:listContacts()), saved.accounts.bob.id), "Bob must appear as a contact")
    check(find(need("List Bob's inbox", bob:listRooms()), room.id), "The direct conversation must appear in Bob's inbox")
    forbidden("Outsider message history", carol:listMessages(room.id))
    forbidden("Outsider posting", carol:sendMessage(room.id, "unauthorized native live probe"))
    print("PASS: people discovery, direct contacts, inboxes and outsider authorization")

    local ping = trackMessage(alice, need("Send Alice's text", alice:sendMessage(room.id, "native live " .. run .. " \"hello\" \\ café\nsecond line")))
    local pong = trackMessage(bob, need("Send Bob's reply", bob:sendMessage(room.id, "native reply " .. run)))
    for _, client in ipairs { alice, bob } do
        local timeline = need("Read native message roundtrip", client:listMessages(room.id))
        local first, second = find(timeline, ping.id), find(timeline, pong.id)
        check(first and first.body == ping.body and first.sender_id == saved.accounts.alice.id, "Alice's exact text and author must reach both clients")
        check(second and second.body == pong.body and second.sender_id == saved.accounts.bob.id, "Bob's exact reply and author must reach both clients")
    end
    forbidden("Editing another person's text", bob:editMessage(ping.id, "unauthorized edit"))
    local edited = need("Edit own text", alice:editMessage(ping.id, "native edited " .. run))
    local received = find(need("Read edited text as Bob", bob:listMessages(room.id)), ping.id)
    check(received and received.body == edited.body and received.edited_at, "Bob must receive the edited text and timestamp")
    need("Delete own message", alice:deleteMessage(ping.id))
    check(not find(need("Read after deletion", bob:listMessages(room.id)), ping.id), "Deleted text must disappear for the other person")
    print("PASS: bidirectional Unicode text, author checks, edit and delete")

    local prepared = need("Prepare real JPEG with native media", require("orca.media").processPhoto(read("tests/fixtures/chat/photo.jpg")))
    local attachment = need("Upload JPEG and thumbnail through native transport", alice:uploadPhoto(room.id, prepared))
    photos[#photos + 1] = { client = alice, id = attachment.id }
    check(attachment.status == "ready", "Native photo upload must finish ready")
    local sent = trackMessage(alice, need("Send photo message", alice:sendMessage(room.id, "native photo " .. run, { attachment.id })))
    local receivedPhoto = find(need("Read photo as Bob", bob:listMessages(room.id)), sent.id)
    check(receivedPhoto and receivedPhoto.attachments[1].id == attachment.id, "Bob's message must contain the uploaded attachment")
    check(need("Download photo as Bob", bob:getPhoto(attachment.id)) == prepared.jpeg, "Downloaded photo must preserve the exact JPEG bytes")
    check(need("Download thumbnail as Bob", bob:getPhoto(attachment.id, true)) == prepared.thumbnail, "Downloaded thumbnail must preserve exact bytes")
    forbidden("Outsider photo download", carol:getPhoto(attachment.id))
    need("Delete photo message", alice:deleteMessage(sent.id))
    local missing, missingError = bob:getPhoto(attachment.id)
    check(not missing and missingError and missingError.status == 404, "Deleting the message must revoke photo downloads")
    print("PASS: native image preparation, binary photo roundtrip and access revocation")
end

local done, failure
orca.async(function()
    local ok, err = pcall(exercise)
    if not ok then failure = tostring(err); print("FAIL: " .. failure) end
    print("LIVE: cleaning up this run's messages, photo reservations and native sessions")
    -- Delete only this run's messages, then revoke only sessions created here.
    -- The canonical direct conversation and contact relation intentionally stay.
    for _, entry in ipairs(messages) do
        local removed, result, cleanupError = pcall(entry.client.deleteMessage, entry.client, entry.id)
        if not removed or (not result and (not cleanupError or cleanupError.status ~= 404)) then
            failure = failure or "Could not remove a native live-test message"
        end
    end
    for _, entry in ipairs(photos) do pcall(entry.client.discardPhoto, entry.client, entry.id) end
    for _, client in ipairs(clients) do
        if client:getSession() then
            local called, result, cleanupError = pcall(client.signOut, client)
            if not called or (not result and (not cleanupError or cleanupError.status ~= 401)) then
                failure = failure or "Could not revoke a native live-test session"
            end
        end
    end
    for _, store in pairs(stores) do
        local cleared = store.clear()
        if not cleared then failure = failure or "Could not clear a native live-test Keychain entry" end
    end
    done = true
end)

local screen = ui.Screen { Width = 100, Height = 100, ResizeMode = "NoResize" }
local timeout = tonumber(os.getenv("CHAT_LIVE_TIMEOUT_MS")) or 600000
assert(timeout >= 1000 and timeout <= 900000, "Use a live-test timeout between 1000 and 900000 milliseconds")
local deadline = system.getTime() + timeout
local timer = core.Timer { Tick = function() end }
local handle = timer:setTimer(1000)
while not done and system.getTime() < deadline do
    local event = system.getMessage()
    if event and not event:is("Window.Paint") then system.dispatchMessage(screen, event) end
end
timer:cancelTimer(handle)
assert(done, "Native Chat live test timed out; inspect the dedicated accounts for interrupted-run artifacts")
assert(not failure, failure)
print("PASS: native Chat live integration (" .. checks .. " checks; real Appwrite and no injected transport)")
