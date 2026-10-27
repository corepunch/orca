-- Exercise the normal chat client against the offline backend, without native networking.
package.path = "apps/chat/?.lua;" .. package.path
local guarded = { "orca.network", "api.orca_transport" }
local savedLoaded, savedPreload, networkLoads = {}, {}, 0
for _, name in ipairs(guarded) do
    savedLoaded[name], savedPreload[name] = package.loaded[name], package.preload[name]
    package.loaded[name] = nil
    package.preload[name] = function()
        networkLoads = networkLoads + 1
        error("Offline chat must not load " .. name)
    end
end

local Chat = require "api.chat"
local checks = 0
local function check(condition, message)
    assert(condition, message)
    checks = checks + 1
end
local function find(items, id)
    for _, item in ipairs(items) do if item.id == id then return item end end
end
local function denied(value, err, code)
    check(value == nil and type(err) == "table" and type(err.message) == "string"
        and (not code or err.code == code), "Expected a structured rejection: " .. tostring(code))
end

local now = 1800000000
local function clock() return now end
local client = assert(Chat.mock { clock = clock })
check(client:getSession() == nil, "A fresh mock must require sign-in")
denied(client:listRooms())
denied(client:listContacts())
denied(client:sendMessage("alex", "Signed out"))
local value, err = client:signIn("demo@example.invalid", "wrong-password")
denied(value, err)
check(client:getSession() == nil, "Failed login must not create a session")
local session = assert(client:signIn("demo@example.invalid", "password123"))
check(session.user.id == "me", "The default account must sign in through the normal client")
local originalToken = session.access_token
session.access_token, session.user.id = "mutated", "not-me"
check(client:getSession().access_token == originalToken and client:getSession().user.id == "me",
    "Returned sessions must not expose stored credentials or user records")

local contacts = assert(client:listContacts())
local alex = assert(find(contacts, "alex"), "The default directory must contain Alex")
check(alex.display_name == "Alex Morgan", "Contacts must expose their display names")
alex.display_name = "Changed outside the backend"
check(find(assert(client:listContacts()), "alex").display_name == "Alex Morgan",
    "Contact result mutation must not change stored profiles")
local rooms = assert(client:listRooms())
check(find(rooms, "alex") and find(rooms, "weekend") and find(rooms, "jules"),
    "The default account must see its fixture conversations")
local originalRoomName = assert(find(rooms, "alex")).name
find(rooms, "alex").name = "Changed outside the backend"
check(find(assert(client:listRooms()), "alex").name == originalRoomName,
    "Room result mutation must not alter backend data")
local history = assert(client:listMessages("alex"))
check(#history > 0, "The fixture conversation must contain history")
local originalCount, firstId, firstBody = #history, history[1].id, history[1].body
history[1].body = "Changed outside the backend"
check(find(assert(client:listMessages("alex")), firstId).body == firstBody,
    "Message results must be defensive copies")
local sent = assert(client:sendMessage("alex", "A real client request to the offline backend"))
check(sent.room_id == "alex" and sent.sender_id == "me", "Sending must persist the signed-in user's message")
local after = assert(client:listMessages("alex"))
check(#after == originalCount + 1 and find(after, sent.id).body == sent.body,
    "Sending must add exactly one stored message without a fabricated reply")
check(assert(client:listMessages("alex", { limit = 1 }))[1].id == sent.id,
    "History limits must select the newest message")
sent.body = "Changed outside the backend"
check(find(assert(client:listMessages("alex")), sent.id).body ~= sent.body,
    "A sent-message result must not expose stored data")
local created = assert(client:createRoom("Offline test conversation", { "alex" }))
check(find(assert(client:listRooms()), created.id), "The creator must become a member of its new room")
check(#assert(client:listMessages(created.id)) == 0, "New rooms must start without invented messages")
check(assert(client:sendMessage(created.id, "Hello Alex")).sender_id == "me",
    "New-room membership must allow its creator to send")
local roomsBeforeRejectedCreate = #assert(client:listRooms())
value, err = client:createRoom("Unknown member", { "no-such-person" })
denied(value, err)
check(#assert(client:listRooms()) == roomsBeforeRejectedCreate, "Rejected members must not leave a partial conversation")

local normalRequests = 0
local normal = assert(Chat.client {
    url = "https://chat.example.test", clock = clock,
    transport = function() normalRequests = normalRequests + 1; error("Invalid input must not reach transport") end,
})
check(getmetatable(client) == getmetatable(normal), "Mock and HTTP must share the same client implementation")
local invalid = {
    function(c) return c:signIn("", "password123") end,
    function(c) return c:signUp("new@example.invalid", "short", "New") end,
    function(c) return c:sendMessage("alex", "  \n ") end,
    function(c) return c:sendMessage("bad&id", "Hello") end,
    function(c) return c:listMessages("alex", { limit = 0 }) end,
    function(c) return c:createRoom("", { "alex" }) end,
    function(c) return c:createRoom("Name", "bad") end,
}
for _, operation in ipairs(invalid) do
    value, err = operation(client); denied(value, err, "validation")
    value, err = operation(normal); denied(value, err, "validation")
end
check(normalRequests == 0, "Shared input validation must run before either transport")

local isolated = assert(Chat.mock { clock = clock })
assert(isolated:signIn("demo@example.invalid", "password123"))
check(not find(assert(isolated:listRooms()), created.id), "Separate mock clients must own separate fixture stores")
check(#assert(isolated:listMessages("alex")) == originalCount, "Messages must not leak between mock clients")
local newcomer = assert(Chat.mock { clock = clock })
local account = assert(newcomer:signUp("new@example.invalid", "password123", "New Person"))
check(newcomer:getSession() and account.user.id ~= "me", "Offline signup must create an independent account")
value, err = newcomer:signUp("new@example.invalid", "password123", "Duplicate")
denied(value, err)
check(newcomer:getSession().user.id == account.user.id, "A rejected signup must retain the existing valid session")
check(#assert(newcomer:listRooms()) == 0 and #assert(newcomer:listMessages("alex")) == 0,
    "New accounts must not read another user's fixture conversations")
value, err = newcomer:sendMessage("alex", "Not a member")
denied(value, err)
check(err.status == 403, "The offline backend must enforce membership for writes")
local ownRoom = assert(newcomer:createRoom("My first conversation", { "alex" }))
check(find(assert(newcomer:listRooms()), ownRoom.id), "A new account must gain membership in rooms it creates")
check(assert(newcomer:sendMessage(ownRoom.id, "My first message")).sender_id == account.user.id,
    "New account sends must use that account's identity")
assert(newcomer:signOut())
assert(newcomer:signIn("new@example.invalid", "password123"))
check(find(assert(newcomer:listRooms()), ownRoom.id), "Signing out must not discard the account's persisted backend data")

local beforeRefresh = client:getSession()
now = beforeRefresh.expires_at - 1
assert(client:listRooms())
local refreshed = client:getSession()
check(refreshed.access_token ~= beforeRefresh.access_token and refreshed.refresh_token ~= beforeRefresh.refresh_token,
    "Near-expiry requests must rotate credentials through the shared refresh path")
check(refreshed.expires_at > now, "Refreshed credentials must receive a future expiry")
local explicit = assert(client:refreshSession())
check(explicit.access_token ~= refreshed.access_token and explicit.refresh_token ~= refreshed.refresh_token,
    "Explicit refresh must rotate both credentials")
now = explicit.expires_at + 1
assert(client:sendMessage(created.id, "Refreshed after expiry"))
check(client:getSession().expires_at > now, "Expired sessions must refresh before protected writes")
assert(client:signOut())
check(client:getSession() == nil, "Sign-out must discard local credentials")
value, err = client:listRooms(); denied(value, err, "unauthenticated")
value, err = client:listContacts(); denied(value, err, "unauthenticated")
value, err = client:refreshSession(); denied(value, err, "unauthenticated")
check(client:signOut(), "Repeated sign-out must be harmless")
local demo = Chat.demo { clock = clock }
check(getmetatable(demo) == getmetatable(client) and demo:getSession().user.id == "me",
    "The demo helper must remain a signed-in convenience for the same normal client")

-- Profile photos come from the shared Octopus profiles, without a session.
local photoFile = assert(io.open("tests/fixtures/chat/photo.jpg", "rb"))
local jpeg = photoFile:read("*a"); photoFile:close()
local Fixtures = require "api.mock_fixtures"
local photos = assert(Chat.mock { clock = clock, fixtures = function(at)
    local data = Fixtures.new(at)
    data.avatars, data.profilePhotos, data.photoFiles = {}, { alex = "photo-alex" }, { ["photo-alex"] = jpeg }
    return data
end })
local lookups, transport = 0, photos.transport
photos.transport = function(request) lookups = lookups + 1; return transport(request) end
local avatars = assert(photos:getAvatars { "alex", "maya", "nobody", "alex" })
check(avatars.alex == "photo-alex" and avatars.maya == false and avatars.nobody == false,
    "Photo lookups return file IDs, or false for people without one")
check(lookups == 1 and photos:getAvatars({ "maya", "alex" }).alex == "photo-alex" and lookups == 1,
    "Photo lookups are batched and cached per client")
check(assert(photos:getAvatar("photo-alex")) == jpeg and lookups == 2, "Profile photo bytes round-trip unchanged")
value, err = photos:getAvatar("missing"); denied(value, err)
value, err = photos:getAvatars { "bad/id" }; denied(value, err, "validation")
value, err = photos:getAvatar("../secret"); denied(value, err, "validation")

-- Editing and deleting follow the Appwrite policy: authors edit their own text;
-- authors, or staff outside direct chats, delete for everyone.
assert(client:signIn("demo@example.invalid", "password123"))
local draft = assert(client:sendMessage("weekend", "Lunch at noon?"))
local edited = assert(client:editMessage(draft.id, "Lunch at one?"))
check(edited.id == draft.id and edited.body == "Lunch at one?" and type(edited.edited_at) == "string",
    "Editing returns the same message with new text and an edit time")
local stored = find(assert(client:listMessages("weekend")), draft.id)
check(stored.body == "Lunch at one?" and stored.edited_at == edited.edited_at, "Edits persist for every reader")
check(find(assert(client:listRooms()), "weekend").subtitle == "Lunch at one?", "Editing the latest message updates the inbox preview")
value, err = client:editMessage(draft.id, " \n "); denied(value, err, "validation")
value, err = client:editMessage(draft.id, string.rep("x", 4001)); denied(value, err, "validation")
value, err = client:editMessage("bad&id", "x"); denied(value, err, "validation")
value, err = client:deleteMessage("bad&id"); denied(value, err, "validation")
value, err = client:editMessage("a1", "Not my words"); denied(value, err)
check(err.status == 403, "Nobody edits another person's message")
value, err = client:deleteMessage("a1"); denied(value, err)
check(err.status == 403 and find(assert(client:listMessages("alex")), "a1"), "Direct chats have no moderators")
check(assert(client:deleteMessage("w1")) == true and not find(assert(client:listMessages("weekend")), "w1"),
    "Group owners delete members' messages")
check(assert(client:deleteMessage(draft.id)) == true and not find(assert(client:listMessages("weekend")), draft.id),
    "Authors delete their own messages for everyone")
check(find(assert(client:listRooms()), "weekend").subtitle == "Start the conversation", "Deleting the last message clears the preview")
value, err = client:deleteMessage(draft.id); denied(value, err, "not_found")
value, err = client:editMessage(draft.id, "Gone"); denied(value, err, "not_found")
value, err = newcomer:deleteMessage("a2"); denied(value, err)
check(err.status == 403, "Non-members cannot delete messages")

check(networkLoads == 0, "Offline operations must never import the network transport")
for _, name in ipairs(guarded) do
    check(package.loaded[name] == nil, "Offline operations must not load " .. name)
    package.loaded[name], package.preload[name] = savedLoaded[name], savedPreload[name]
end
print("PASS: mock chat client (" .. checks .. " checks)")
