-- Shared-client behavior, privacy and authorization against the isolated mock.
package.path = "apps/chat/?.lua;" .. package.path
local Chat, Mock = require "api.chat", require "api.mock_transport"
local checks, now = 0, 1800000000
local function check(value, message) assert(value, message); checks = checks + 1 end
local function denied(value, err, code)
    check(value == nil and type(err) == "table" and type(err.message) == "string"
        and (not code or err.code == code), "Expected rejection " .. tostring(code) .. ": " .. tostring(err and err.code))
end
local function reject(fn, code)
    local value, err = fn(); denied(value, err, code)
end
local function find(rows, id, field)
    for _, row in ipairs(rows) do if row[field or "id"] == id then return row end end
end
local function setup()
    local transport, state = Mock.new { clock = function() return now end }
    local function client(email)
        local c = assert(Chat.supabase { url = "https://mock.invalid", publishableKey = "sb_publishable_test",
            transport = transport, clock = function() return now end })
        if email then assert(c:signIn(email, "password123")) end
        return c
    end
    return client, state, transport
end
local client, state, transport = setup()
local owner, alex, maya, guest = client("demo@example.invalid"), client("alex@example.invalid"), client("maya@example.invalid"), client()
local function register(c, email)
    assert(c:requestCode(email)); return assert(c:verifyCode(email, "123456"))
end
local function raw(c, method, path, body, headers)
    headers = headers or {}; headers.Authorization = "Bearer " .. c:getSession().access_token
    return transport { method = method, url = "https://mock.invalid" .. path, body = body, headers = headers }
end

reject(function() return guest:requestCode("555123") end, "validation")
reject(function() return guest:requestCode("+01234567890") end, "validation")
reject(function() return guest:verifyCode("new@example.invalid", "12345") end, "validation")
check(assert(guest:requestCode("NEW@example.invalid")).identifier == "new@example.invalid", "Normalize email")
reject(function() return guest:requestCode("new@example.invalid") end, "over_request_rate_limit")
for _ = 1, 5 do reject(function() return guest:verifyCode("new@example.invalid", "000000") end, "otp_invalid") end
reject(function() return guest:verifyCode("new@example.invalid", "123456") end, "otp_expired")
check(#state.users == 4, "Unverified OTP must not create an account")
now = now + 60
assert(guest:requestCode("new@example.invalid")); now = now + 300
reject(function() return guest:verifyCode("new@example.invalid", "123456") end, "otp_expired")
assert(guest:requestCode("new@example.invalid")); local newId = assert(guest:verifyCode("new@example.invalid", "123456")).user.id
reject(function() return guest:verifyCode("new@example.invalid", "123456") end, "otp_expired")
check(#assert(guest:listContacts()) == 0 and #assert(guest:listRooms()) == 0, "New accounts have no private directory or rooms")
check(assert(guest:getProfile()).email == "new@example.invalid", "Only verified self identifiers are returned")
assert(guest:requestIdentityLink("+49 (151) 1000-0002"))
reject(function() return owner:verifyIdentityLink("+4915110000002", "123456") end, "otp_expired")
check(assert(guest:verifyIdentityLink("+4915110000002", "123456")).phone == "+4915110000002", "Link verifies canonical phone")
check(guest:getSession().user.id == newId, "Linking preserves account identity")
reject(function() return guest:requestIdentityLink("alex@example.invalid") end, "identity_exists")
local phoneLogin = client(); register(phoneLogin, "+4915110000002")
check(phoneLogin:getSession().user.id == newId, "Verified phone signs in to the same account")

-- Secure email change may require confirmations from both old and new inboxes.
local proofRequests, proofResponses = {}, {
    { status = 200, body = { access_token = "proof-access", refresh_token = "proof-refresh", expires_in = 3600,
        user = { id = "proof-user", email = "old@example.invalid" } } },
    { status = 200, body = { user = { id = "proof-user", email = "old@example.invalid" } } },
    { status = 200, body = { id = "proof-user", email = "old@example.invalid" } },
    { status = 200, body = { user = { id = "proof-user", email = "linked@example.invalid" } } },
    { status = 200, body = { id = "proof-user", email = "linked@example.invalid" } },
    { status = 200, body = { access_token = "wrong-user-access", refresh_token = "wrong-user-refresh", expires_in = 3600,
        user = { id = "someone-else", email = "other@example.invalid" } } },
    { status = 200, body = {} },
}
local proof = assert(Chat.supabase { url = "https://mock.invalid", publishableKey = "sb_publishable_test",
    clock = function() return now end, transport = function(request)
        proofRequests[#proofRequests + 1] = request; return assert(table.remove(proofResponses, 1))
    end })
assert(proof:signIn("old@example.invalid", "password123"))
reject(function() return proof:verifyIdentityLink("linked@example.invalid", "123456") end, "verification_pending")
check(proof:getSession().user.id == "proof-user" and proof:getSession().access_token == "proof-access", "Pending secure change retains the original account session")
check(assert(proof:verifyIdentityLink("linked@example.invalid", "123456")).email == "linked@example.invalid", "Link succeeds only once the verified self profile confirms the requested identifier")
reject(function() return proof:verifyIdentityLink("other@example.invalid", "123456") end, "identity_mismatch")
check(proof:getSession().user.id == "proof-user" and proof:getSession().access_token == "proof-access", "Wrong-user verification cannot replace the existing session")
assert(proof:listContacts())
check(proofRequests[#proofRequests].url:match("id=neq%.proof%-user$") ~= nil, "Remote contacts explicitly exclude the current profile")

check(assert(guest:updateProfile { username = "New_Reader", display_name = " New reader " }).username == "new_reader", "Normalize profile")
reject(function() return guest:updateProfile { username = "alex" } end, "23505")
reject(function() return guest:updateProfile { username = "a!" } end, "validation")
reject(function() return guest:updateProfile { email = "forged@example.invalid" } end, "validation")
check(#assert(owner:findPeople("new@example.invalid")) == 0, "Email discovery defaults private")
assert(guest:updateProfile { discoverable_email = true, discoverable_phone = true })
local result = assert(owner:findPeople("new@example.invalid"))[1]
check(result.id == newId and result.email == nil and result.phone == nil, "Discovery returns public profile fields only")
check(assert(owner:findPeople("@new_reader"))[1].id == newId, "Exact username discovery")
check(#assert(owner:findPeople("reader")) == 0, "No partial username directory")
check(assert(owner:findPeople("+49 151 1000 0002"))[1].id == newId, "Phone lookup uses canonical E164")
assert(guest:updateProfile { discoverable_email = false })
check(#assert(owner:findPeople("new@example.invalid")) == 0, "Discovery opt-out applies immediately")
local rateClient = client(); register(rateClient, "rate@example.invalid")
reject(function() return rateClient:updateProfile { discoverable_phone = true } end, "validation")
for _ = 1, 20 do assert(rateClient:findPeople("alex")) end
reject(function() return rateClient:findPeople("alex") end, "rate_limit")
now = now + 60; check(#assert(rateClient:findPeople("alex")) == 1, "Lookup rate window resets")

check(assert(owner:createRoom("Alex", { "alex" }, { kind = "direct" })).id == "alex", "Canonical direct reuses existing pair")
check(assert(alex:createRoom("You", { "me" }, { kind = "direct" })).id == "alex", "Canonical pair independent of ordering")
reject(function() return owner:createRoom("Invalid direct", {}, { kind = "direct" }) end, "validation")
local group = assert(owner:createRoom("Private group", { "alex", "maya", newId }))
check(group.kind == "group" and group.role == "owner" and group.can_post, "Group creator owns conversation")
assert(owner:setMemberRole(group.id, "alex", "admin"))
reject(function() return alex:setMemberRole(group.id, "maya", "admin") end, "42501")
reject(function() return alex:removeMember(group.id, "me") end, "42501")
assert(alex:removeMember(group.id, "maya"))
check(not find(assert(maya:listRooms()), group.id), "Removal revokes room visibility")
reject(function() return maya:sendMessage(group.id, "No access") end, "42501")
local channel = assert(owner:createRoom("Quiet news", { "alex", "maya", newId }, { kind = "channel", visibility = "public" }))
local guestRoom = assert(find(assert(guest:listRooms()), channel.id))
check(guestRoom.role == "subscriber" and guestRoom.can_post == false, "Channel subscribers cannot post")
reject(function() return guest:sendMessage(channel.id, "Forbidden") end, "42501")
check(#assert(guest:listMembers(channel.id)) == 2, "Subscriber sees self and channel owner, not peer subscribers")
check(not find(assert(rateClient:listContacts()), "maya"), "No unrelated account directory")
assert(owner:setMemberRole(channel.id, "alex", "admin"))
check(#assert(guest:listMembers(channel.id)) == 3, "Subscriber can see channel staff")
assert(alex:sendMessage(channel.id, "Published update"))
reject(function() return owner:setMemberRole(channel.id, "maya", "member") end, "validation")
local publicRow = assert(rateClient:findChannels("quiet"))[1]
check(publicRow.id == channel.id and publicRow.can_post == false, "Public channel search grants no publish rights")
check(assert(rateClient:joinPublicChannel(channel.id)).role == "subscriber", "Public join grants subscriber role")
local scoped = assert(rateClient:listContacts())
check(#scoped == 2 and not find(scoped, "maya") and not find(scoped, newId), "Channel membership does not expose other subscribers as contacts")
for _, person in ipairs(scoped) do check(person.email == nil and person.phone == nil, "Contacts must not expose login identifiers") end
reject(function() return rateClient:joinPublicChannel(group.id) end, "42501")
reject(function() return guest:createInvite(channel.id) end, "42501")
local invite = assert(owner:createInvite(group.id))
check(assert(rateClient:joinInvite(invite.code)).id == group.id, "Private invite grants membership")
assert(rateClient:joinInvite(invite.code)); check(state.invites[invite.code].uses == 1, "Invite rejoin does not consume another use")
assert(owner:revokeInvite(group.id, invite.code))
reject(function() return rateClient:joinInvite(invite.code) end, "invite_unavailable")
check(owner:revokeInvite(group.id, "unknown-invite") == false, "False RPC results must remain false")
local expired = assert(owner:createInvite(group.id)); state.invites[expired.code].expires_epoch = now
reject(function() return rateClient:joinInvite(expired.code) end, "invite_unavailable")
local exhausted = assert(owner:createInvite(group.id)); state.invites[exhausted.code].uses = exhausted.max_uses
reject(function() return rateClient:joinInvite(exhausted.code) end, "invite_unavailable")

local f = assert(io.open("tests/fixtures/chat/photo.jpg", "rb")); local jpeg = f:read("*a"); f:close()
local photo = { jpeg = jpeg, thumbnail = jpeg, width = 32, height = 24 }
local function exifImage(payload)
    local length = #payload + 2
    return jpeg:sub(1, 2) .. "\255\225" .. string.char(math.floor(length / 256), length % 256) .. payload .. jpeg:sub(3)
end
local generated = exifImage("Exif\0\0II\42\0\8\0\0\0\0\0\0\0\0\0")
check(require("api.jpeg").dimensions(generated) == 32, "Generated EXIF without GPS is allowed")
local located = exifImage("Exif\0\0II\42\0\8\0\0\0\1\0\37\136\4\0\1\0\0\0\26\0\0\0\0\0\0\0")
reject(function() return owner:uploadPhoto(group.id, { jpeg = located, thumbnail = jpeg }) end, "validation")
local mediaOk, media = pcall(require, "orca.media")
if mediaOk then
    local prepared = assert(media.processPhoto(jpeg))
    local processed = assert(owner:uploadPhoto(group.id, prepared))
    check(processed.width == 32 and assert(owner:getPhoto(processed.id)) == prepared.jpeg, "Native encoded JPEG passes the real client preflight and mock storage")
    assert(owner:discardPhoto(processed.id))
end
local a = assert(owner:uploadPhoto(group.id, photo))
check(a.status == "ready" and a.byte_count == #jpeg and assert(owner:getPhoto(a.id, true)) == jpeg, "Upload preserves exact JPEG bytes")
reject(function() return guest:getPhoto(a.id) end, "not_found")
reject(function() return guest:uploadPhoto(channel.id, photo) end, "42501")
reject(function() return owner:uploadPhoto(group.id, { jpeg = "bad", thumbnail = jpeg }) end, "validation")
reject(function() return owner:uploadPhoto(group.id, { jpeg = jpeg:sub(1, -3), thumbnail = jpeg }) end, "validation")
reject(function() return owner:uploadPhoto(group.id, { jpeg = jpeg .. "trailing", thumbnail = jpeg }) end, "validation")
reject(function() return owner:uploadPhoto(group.id, { jpeg = jpeg, thumbnail = jpeg, width = 33 }) end, "validation")
reject(function() return owner:uploadPhoto(group.id, { jpeg = jpeg .. string.rep("x", 1000000), thumbnail = jpeg }) end, "validation")
local before = #assert(owner:listMessages(group.id))
reject(function() return owner:sendMessage(group.id, "", { a.id, a.id }) end, "attachment_unavailable")
reject(function() return owner:sendMessage("alex", "", { a.id }) end, "attachment_unavailable")
reject(function() return guest:sendMessage(group.id, "", { a.id }) end, "attachment_unavailable")
check(#assert(owner:listMessages(group.id)) == before and state.attachments[a.id].status == "ready", "Rejected send is atomic")
local message = assert(owner:sendMessage(group.id, "", { a.id }))
check(message.body == "" and #message.attachments == 1 and message.attachments[1].id == a.id, "Photo-only message embeds attachment")
check(assert(guest:getPhoto(a.id)) == jpeg, "Attached photo readable by room member")
reject(function() return owner:sendMessage(group.id, "Again", { a.id }) end, "attachment_unavailable")
reject(function() return owner:discardPhoto(a.id) end, "42501")
assert(owner:removeMember(group.id, newId)); reject(function() return guest:getPhoto(a.id) end, "not_found")
local pending = assert(owner:uploadPhoto(group.id, photo)); assert(owner:discardPhoto(pending.id))
check(state.objects[pending.object_key] == nil and state.objects[pending.thumbnail_key] == nil, "Discard removes both private objects")
reject(function() return owner:getPhoto(pending.id) end, "not_found")
local reserve = raw(owner, "POST", "/rest/v1/rpc/reserve_chat_photo", { target_room = group.id,
    photo_width = 32, photo_height = 24, photo_bytes = #jpeg, thumbnail_width = 32, thumbnail_height = 24, thumbnail_bytes = #jpeg }).body
check(raw(owner, "POST", "/functions/v1/finalize-chat-photo", { attachment_id = reserve.id }).status == 400, "Incomplete photo cannot finalize")
check(raw(owner, "POST", "/storage/v1/object/chat-photos/" .. reserve.object_key, jpeg, { ["Content-Type"] = "image/jpeg" }).status == 200, "Pending uploader can write reserved object")
check(raw(owner, "POST", "/storage/v1/object/chat-photos/" .. reserve.object_key, jpeg, { ["Content-Type"] = "image/jpeg" }).status == 409, "Uploads cannot overwrite existing bytes")
check(raw(alex, "POST", "/storage/v1/object/chat-photos/" .. reserve.thumbnail_key, jpeg, { ["Content-Type"] = "image/jpeg" }).status == 403, "Another room admin cannot upload another user's reservation")
check(raw(owner, "POST", "/rest/v1/chat_messages", { room_id = group.id, sender_id = "me", body = "Bypass RPC" }).status == 403, "REST insertion cannot bypass atomic attachment publishing")
local draft = assert(alex:uploadPhoto(channel.id, photo))
assert(owner:setMemberRole(channel.id, "alex", "subscriber"))
reject(function() return alex:sendMessage(channel.id, "", { draft.id }) end, "42501")
check(state.attachments[draft.id].status == "ready", "Losing posting rights cannot publish a prepared photo")
assert(alex:discardPhoto(draft.id))
local pendingAdmin = raw(owner, "POST", "/rest/v1/rpc/reserve_chat_photo", { target_room = channel.id,
    photo_width = 32, photo_height = 24, photo_bytes = #jpeg, thumbnail_width = 32, thumbnail_height = 24, thumbnail_bytes = #jpeg }).body
check(raw(owner, "POST", "/functions/v1/finalize-chat-photo", { attachment_id = pendingAdmin.id }).status == 400, "Pending finalization requires both object uploads")
local expiring = assert(owner:uploadPhoto(group.id, photo)); now = now + 3600
reject(function() return owner:sendMessage(group.id, "", { expiring.id }) end, "attachment_unavailable")
reject(function() return owner:sendMessage(group.id, "", 5) end, "validation")
local models = require("models").new(owner)
local modeled = assert(models.Rooms:create { name = "Modeled channel", member_ids = {}, kind = "channel", visibility = "public" })
check(modeled.kind == "channel" and modeled.visibility == "public", "Room model forwards conversation options")
local modeledPhoto = assert(owner:uploadPhoto(modeled.id, photo))
check(#assert(models.Messages:create { room_id = modeled.id, body = "", attachment_ids = { modeledPhoto.id } }).attachments == 1, "Message model forwards photo IDs")

-- Exercise the native adapter's JSON shape without opening a network socket.
local oldNetwork, oldJson, oldAdapter = package.loaded["orca.network"], package.loaded["orca.parsers.json"], package.loaded["api.orca_transport"]
local ok, json = pcall(require, "orca.parsers.json")
if not ok then
    local function encode(value)
        if type(value) == "string" then return string.format("%q", value) end
        if type(value) ~= "table" then return tostring(value) end
        local parts = {}
        for key, item in pairs(value) do parts[#parts + 1] = encode(key) .. ":" .. encode(item) end
        return "{" .. table.concat(parts, ",") .. "}"
    end
    json = { encode = encode }
end
local wire
package.loaded["orca.parsers.json"] = json
package.loaded["orca.network"] = { fetch = function(_, request)
    wire = request
    return { code = function() return 200 end, json = function() return {} end, text = function() return jpeg end }
end }
package.loaded["api.orca_transport"] = nil
local adapter = require "api.orca_transport"
adapter { method = "POST", url = "https://mock.invalid", body = { member_ids = {}, room_name = "Solo" }, arrayFields = { member_ids = true } }
check(wire.body:match('"member_ids":%[%]') ~= nil, "Empty member list encodes as JSON array")
check(wire.body:match('"room_name":"Solo"') ~= nil, "Array encoding preserves ordinary string fields")
adapter { method = "POST", url = "https://mock.invalid", body = { attachment_ids = {} }, arrayFields = { attachment_ids = true } }
check(wire.body == '{"attachment_ids":[]}', "Empty attachment list encodes as JSON array")
check(adapter { method = "POST", url = "https://mock.invalid", body = jpeg, responseType = "binary" }.body == jpeg and wire.body == jpeg,
    "Binary request and response must bypass JSON without changing bytes")
package.loaded["orca.network"], package.loaded["orca.parsers.json"], package.loaded["api.orca_transport"] = oldNetwork, oldJson, oldAdapter
print("chat features: " .. checks .. " checks passed")
