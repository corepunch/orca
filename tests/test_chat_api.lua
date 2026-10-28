-- Standalone Lua / ORCA headless test. No network or Appwrite account required.
package.path = "apps/chat/?.lua;" .. package.path
local Chat = require "api.chat"
local checks = 0
local function check(condition, message)
  assert(condition, message)
  checks = checks + 1
end

local function session(access, refresh, expires)
  return { access_token = access or "access-1", refresh_token = refresh or "refresh-1",
    expires_in = expires or 3600, user = { id = "user-1", email = "you@example.test" } }
end

local function backend(responses)
  local requests = {}
  local client = assert(Chat.client {
    url = "https://chat.example.test/",
    clock = function() return 1000 end,
    transport = function(request)
      requests[#requests + 1] = request
      local response = table.remove(responses, 1)
      assert(response, "Unexpected HTTP request")
      if response.throws then error("offline") end
      return response
    end,
  })
  return client, requests
end

local demo = Chat.demo()
local rooms = assert(demo:listRooms())
local roomId = rooms[1].id
rooms[1].name = "Mutated"
check(demo:listRooms()[1].name ~= "Mutated", "Demo results must be defensive copies")
local original = #assert(demo:listMessages(roomId))
local sent = assert(demo:sendMessage(roomId, "hello"))
check(sent.sender_id == demo:getSession().user.id and sent.body == "hello", "Demo send should use the current user")
check(#demo:listMessages(roomId) == original + 1, "Demo send must append exactly one message without a fabricated reply")
local value, err = demo:sendMessage(roomId, "  \n ")
check(value == nil and err.code == "validation", "Blank messages must fail")
value, err = demo:sendMessage("missing", "hello")
check(value == nil and err.status == 403, "Unknown conversations must reject writes like the remote API")
check(#demo:listMessages(roomId, { limit = 1 }) == 1, "Limit must select the newest messages")
-- Multiple same-timestamp pages must remain stable and bounded.
for i = 1, 65 do assert(demo:sendMessage(roomId, "History " .. i)) end
local page = assert(demo:listMessagePage(roomId))
check(#page.messages == 30 and page.has_more and page.next_cursor == page.messages[1].id, "Initial message page defaults to 30 with an oldest-row cursor")
local seen, total = {}, 0
while true do
  for _, message in ipairs(page.messages) do
    check(not seen[message.id], "Cursor pages must not repeat message IDs")
    seen[message.id], total = true, total + 1
  end
  if not page.has_more then break end
  page = assert(demo:listMessagePage(roomId, { before = page.next_cursor }))
  check(#page.messages <= 30, "Every history page stays within the request limit")
end
check(total == original + 66, "Traversing history returns every message exactly once")
page, err = demo:listMessagePage(roomId, { before = "deleted-message" })
check(not page and err.status == 409 and err.code == "stale_cursor", "Deleted cursors require explicit recovery")
for _, options in ipairs { { limit = 0 }, { limit = 101 }, { limit = 1.5 }, { limit = "invalid" }, { before = "bad&id" } } do
  page, err = demo:listMessagePage(roomId, options)
  check(not page and err.code == "validation", "Invalid page sizes and cursors fail before transport")
end
demo:signOut()
value, err = demo:listRooms()
check(value == nil and err.code == "unauthenticated", "Signed-out demo must reject room access")

local client, requests = backend {
  { status = 200, body = session() },
  { status = 200, body = { { id = "room-1", name = "Friends" } } },
  { status = 200, body = { { id = "m2", body = "new" }, { id = "m1", body = "old" } } },
  { status = 201, body = { { id = "m3", body = "hello" } } },
  { status = 200, body = { { id = "room-2", name = "New" } } },
}
value, err = client:listRooms()
check(value == nil and err.code == "unauthenticated" and #requests == 0, "Protected operations must not fetch without a session")
local signedIn = assert(client:signIn("you@example.test", "password"))
check(requests[1].url == "https://chat.example.test/auth/v1/token?grant_type=password", "Use the password grant endpoint")
check(requests[1].headers.Authorization == nil, "Sign-in requests must not carry a bearer token")
signedIn.access_token = "modified"
check(client:getSession().access_token == "access-1", "Session result mutation must not change stored credentials")
check(client:listRooms()[1].id == "room-1", "Room results must be returned")
check(requests[2].headers.Authorization == "Bearer access-1", "Protected requests must use the user's JWT")
local messages = assert(client:listMessages("room-1", { limit = 20 }))
check(messages[1].id == "m1" and messages[2].id == "m2", "Latest messages must be returned in display order")
check(requests[3].url:match("limit=20$") ~= nil, "Message limit must reach the backend")
check(client:sendMessage("room-1", "hello").id == "m3", "Send returns the stored message")
check(requests[4].body.target_room == "room-1" and requests[4].body.message_body == "hello"
  and requests[4].body.sender_id == nil and requests[4].url:match("/rpc/post_chat_message$"),
  "Send must use the atomic RPC, whose sender comes from the authenticated session")
check(client:createRoom("New", { "user-2" }).id == "room-2", "Room creation returns the atomic RPC result")
check(requests[5].url:match("/rpc/create_chat_room$") ~= nil, "Room membership writes must go through the RPC")
value, err = client:sendMessage("bad&id", "hello")
check(value == nil and err.code == "validation" and #requests == 5, "IDs must not inject PostgREST filters")
value, err = client:sendMessage("room-1", string.rep("a", 4001))
check(value == nil and err.code == "validation" and #requests == 5, "Oversized messages must fail before HTTP")

client, requests = backend {
  { status = 200, body = session() },
  { status = 200, body = { messages = { { id = "m1" }, { id = "m2" } }, has_more = true, next_cursor = "m1" } },
  { status = 200, body = { messages = { { id = "m0" } }, has_more = false } },
  { status = 200, body = { messages = { { id = "x" }, { id = "x" } }, has_more = false } },
  { status = 200, body = { messages = { { id = "x" } }, has_more = true, next_cursor = "wrong" } },
}
assert(client:signIn("a@b.test", "password"))
page = assert(client:listMessagePage("room-1"))
check(requests[2].method == "POST" and requests[2].url:match("/rpc/list_chat_messages$")
    and requests[2].body.target_room == "room-1" and requests[2].body.limit == 30 and requests[2].body.before == nil,
    "Latest loading uses one bounded authenticated RPC")
assert(client:listMessagePage("room-1", { before = page.next_cursor, limit = 100 }))
check(requests[3].body.before == "m1" and requests[3].body.limit == 100, "Older RPC carries an opaque cursor and allows the explicit maximum")
page, err = client:listMessagePage("room-1")
check(not page and err.code == "invalid_response", "Repeated response IDs must not corrupt the timeline")
page, err = client:listMessagePage("room-1")
check(not page and err.code == "invalid_response", "Inconsistent response cursors must not loop history loading")

client, requests = backend {
  { status = 200, body = session() },
  { status = 404, body = { code = "not_found" } },
  { status = 200, body = { { id = "m2" }, { id = "m1" } } },
  { status = 200, body = { { id = "m2" }, { id = "m1" } } },
  { status = 200, body = { messages = { { id = "m1" }, { id = "m2" } }, has_more = true, next_cursor = "m1" } },
}
assert(client:signIn("a@b.test", "password"))
page = assert(client:listMessagePage("room-1"))
check(#page.messages == 2 and page.messages[1].id == "m1" and not page.has_more,
  "Older deployed Functions keep a bounded recent conversation usable")
check(requests[3].method == "GET" and requests[3].url:match("limit=30$"),
  "Legacy fallback must retain the requested page limit")
assert(client:listMessagePage("room-1"))
check(#requests == 4 and requests[4].method == "GET", "Missing RPC is cached briefly instead of requested on each poll")
page, err = client:listMessagePage("room-1", { before = "m1" })
check(not page and err.code == "history_unavailable" and #requests == 4,
  "Legacy endpoints must never fabricate older cursor pages")
client.clock = function() return 1061 end
page = assert(client:listMessagePage("room-1"))
check(page.has_more and requests[5].method == "POST" and client.messagePageRetryAt == nil,
  "Cursor pagination must activate automatically after the new Function rolls out")

client, requests = backend {
  { status = 200, body = session() },
  { status = 403, body = { code = "forbidden" } },
}
assert(client:signIn("a@b.test", "password"))
page, err = client:listMessagePage("room-1")
check(not page and err.status == 403 and #requests == 2, "Access failures must never trigger a legacy fallback")

client, requests = backend {
  { status = 200, body = session() },
  { status = 200, body = { { id = "m3", body = "edited", edited_at = "2026-10-27T10:00:00Z" } } },
  { status = 200, body = true },
  { status = 403, body = { code = "forbidden", message = "You cannot delete this message." } },
}
assert(client:signIn("a@b.test", "password"))
local edited = assert(client:editMessage("m3", "edited"))
check(edited.body == "edited" and edited.edited_at and requests[2].url:match("/rpc/edit_chat_message$")
  and requests[2].body.target_message == "m3" and requests[2].body.message_body == "edited",
  "Editing uses the edit RPC with the message ID and new text")
check(client:deleteMessage("m3") == true and requests[3].url:match("/rpc/delete_chat_message$")
  and requests[3].body.target_message == "m3", "Deleting uses the delete RPC")
value, err = client:deleteMessage("m4")
check(value == nil and err.status == 403 and err.code == "forbidden", "Delete permission failures reach the caller")
for _, operation in ipairs {
  function() return client:editMessage("bad&id", "x") end,
  function() return client:editMessage("m3", string.rep("a", 4001)) end,
  function() return client:editMessage("m3", nil) end,
  function() return client:deleteMessage("bad&id") end,
} do
  value, err = operation()
  check(value == nil and err.code == "validation", "Invalid edits and deletes fail before HTTP")
end
check(#requests == 4, "Rejected input never reaches the transport")

client, requests = backend {
  { status = 200, body = session("old", "old-refresh", 10) },
  { status = 200, body = session("new", "new-refresh") },
  { status = 200, body = {} },
}
assert(client:signIn("a@b.test", "password"))
assert(client:listRooms())
check(requests[2].body.refresh_token == "old-refresh", "Near-expiry sessions must refresh before fetching data")
check(requests[3].headers.Authorization == "Bearer new" and client:getSession().refresh_token == "new-refresh", "Rotated credentials must replace both tokens")

client, requests = backend {
  { status = 200, body = session() },
  { status = 401, body = { message = "expired" } },
  { status = 200, body = session("retried", "rotated") },
  { status = 401, body = { message = "still unauthorized", code = "denied" } },
}
assert(client:signIn("a@b.test", "password"))
value, err = client:listRooms()
check(value == nil and err.code == "denied" and #requests == 4, "HTTP 401 must refresh and retry only once")

client, requests = backend {
  { status = 200, body = session() },
  { throws = true },
}
assert(client:signIn("a@b.test", "password"))
value, err = client:signOut()
check(value == nil and err.code == "network" and client:getSession() == nil, "Sign-out must forget local credentials when offline")

client, requests = backend {
  { status = 200, body = session("old", "old-refresh", 1) },
  { status = 400, body = { error_code = "refresh_token_not_found", msg = "Sign in again" } },
}
assert(client:signIn("a@b.test", "password"))
value, err = client:listRooms()
check(value == nil and err.code == "refresh_token_not_found" and client:getSession() == nil, "Rejected refresh token must clear the unusable session")

client = backend { { status = 200, body = { id = "user-2" } } }
value = assert(client:signUp("a@b.test", "password", "A"))
check(value.confirmationRequired and client:getSession() == nil, "Email confirmation must not fabricate a session")
value, err = Chat.client { url = "https://chat.example.test" }
check(value == nil and err.code == "configuration", "A client requires a transport")
value, err = Chat.client { url = "http://chat.example.test", transport = function() end }
check(value == nil and err.code == "configuration", "Remote services require HTTPS")

-- The native network binding throws its response object on non-2xx. Verify
-- that the adapter preserves its status instead of turning HTTP 401 into an
-- undifferentiated network failure.
local oldNetwork = package.loaded["orca.network"]
local oldJson = require "api.json"
local oldTransport = package.loaded["api.orca_transport"]
local requestOptions
local throwsResponse = true
local response = { code = function() return 401 end,
  text = function() return '{"message":"expired"}' end,
  headers = function() return { ["set-cookie"] = { "first=value", "second=value" } } end }
package.loaded["orca.network"] = { fetch = function(url, options)
  requestOptions = options
  if throwsResponse then error(response) end
  return response
end }
package.loaded["api.json"] = { encode = function() return "encoded-json" end, decode = oldJson.decode }
package.loaded["api.orca_transport"] = nil
local transport = require "api.orca_transport"
local result = transport { url = "https://chat.example.test", method = "POST", headers = {}, body = {} }
check(result.status == 401 and result.body.message == "expired", "Thrown HTTP responses must retain status and body")
check(#result.headers["set-cookie"] == 2, "Response headers must preserve repeated Set-Cookie values for native authentication")
check(requestOptions.body == "encoded-json" and requestOptions.nocookies == true, "Transport must encode JSON and disable shared cookie loading")
response.code = function() return 204 end
response.text = function() error("204 has no body") end
result = transport { url = "https://chat.example.test", method = "POST", headers = {} }
check(result.status == 204 and result.body == nil, "204 responses must not attempt JSON parsing")
throwsResponse = false
response.code = function() return 200 end
response.text = function() return '{"id":"ok"}' end
result = transport { url = "https://chat.example.test", method = "GET", headers = {} }
check(result.status == 200 and result.body.id == "ok", "Successful responses must decode JSON")
response.text = function() return "invalid JSON" end
local parsed = pcall(transport, { url = "https://chat.example.test", method = "GET", headers = {} })
check(not parsed, "Malformed success JSON must not masquerade as successful data")
response.code = function() return 503 end
result = transport { url = "https://chat.example.test", method = "GET", headers = {} }
check(result.status == 503 and result.body.code == "http_error", "Malformed error JSON must preserve the HTTP status")
package.loaded["orca.network"] = oldNetwork
package.loaded["api.json"] = oldJson
package.loaded["api.orca_transport"] = oldTransport

-- Staged photos reuse the authenticated file path and fetch metadata only once.
local photoRows = { { object_key = "full.jpg", thumbnail_key = "small.jpg" } }
for _, previewStatus in ipairs { 200, 404 } do
  client, requests = backend {
    { status = 200, body = session() },
    { status = 200, body = photoRows },
    { status = previewStatus, body = previewStatus == 200 and "small-bytes" or { message = "Missing thumbnail" } },
    { status = 200, body = "full-bytes" },
  }
  assert(client:signIn("a@b.test", "password"))
  local previews = 0
  local bytes = assert(client:getPhoto("photo-1", false, function(preview)
    previews = previews + 1
    check(preview == "small-bytes" and #requests == 3, "Preview arrives before the full file request")
  end))
  check(bytes == "full-bytes" and previews == (previewStatus == 200 and 1 or 0),
    "A missing optional thumbnail must not stop the full download")
  check(#requests == 4 and requests[3].url:match("/small.jpg$") and requests[4].url:match("/full.jpg$")
    and requests[3].responseType == "binary" and requests[4].responseType == "binary"
    and requests[3].headers.Authorization == "Bearer access-1"
    and requests[4].headers.Authorization == "Bearer access-1", "Both stages use authenticated binary requests")
end
for _, cancel in ipairs { false, true } do
  client, requests = backend {
    { status = 200, body = session() }, { status = 200, body = photoRows },
    { status = 200, body = "small-bytes" }, { status = 200, body = "full-bytes" },
  }
  assert(client:signIn("a@b.test", "password"))
  value, err = client:getPhoto("photo-1", false, function()
    if cancel then return false end
    error("Broken optional preview consumer")
  end)
  check(cancel and value == nil and err.code == "cancelled" and #requests == 3
    or not cancel and value == "full-bytes" and #requests == 4,
    "Preview cancellation skips full transfer; callback errors leave it usable")
end
client, requests = backend {
  { status = 200, body = session() }, { status = 200, body = photoRows },
  { status = 200, body = "full-bytes" },
}
assert(client:signIn("a@b.test", "password"))
check(client:getPhoto("photo-1", false) == "full-bytes" and #requests == 3
  and requests[3].url:match("/full.jpg$"), "Callers without a preview callback keep one file request")
client, requests = backend {
  { status = 200, body = session() }, { status = 200, body = photoRows },
  { status = 200, body = "small-bytes" },
}
assert(client:signIn("a@b.test", "password"))
local called = false
check(client:getPhoto("photo-1", true, function() called = true end) == "small-bytes"
  and not called and #requests == 3 and requests[3].url:match("/small.jpg$"),
  "Thumbnail-only requests keep their original one-file behavior")

print("PASS: chat API (" .. checks .. " checks)")
