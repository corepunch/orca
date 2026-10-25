-- Standalone Lua / ORCA headless test. No network or Supabase account required.
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
  local client = assert(Chat.supabase {
    url = "https://example.supabase.co/", publishableKey = "sb_publishable_test",
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
check(requests[1].url == "https://example.supabase.co/auth/v1/token?grant_type=password", "Use the password grant endpoint")
check(requests[1].headers.Authorization == nil and requests[1].headers.apikey == "sb_publishable_test", "Publishable key must not be used as a bearer token")
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
value, err = Chat.supabase { url = "https://example.supabase.co", publishableKey = "sb_secret_forbidden", transport = function() end }
check(value == nil and err.code == "configuration", "Secret keys must be rejected by client configuration")
value, err = Chat.supabase { url = "http://example.supabase.co", publishableKey = "sb_publishable_test", transport = function() end }
check(value == nil and err.code == "configuration", "Remote credentials require HTTPS")

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
local result = transport { url = "https://example.supabase.co", method = "POST", headers = {}, body = {} }
check(result.status == 401 and result.body.message == "expired", "Thrown HTTP responses must retain status and body")
check(#result.headers["set-cookie"] == 2, "Response headers must preserve repeated Set-Cookie values for native authentication")
check(requestOptions.body == "encoded-json" and requestOptions.nocookies == true, "Transport must encode JSON and disable shared cookie loading")
response.code = function() return 204 end
response.text = function() error("204 has no body") end
result = transport { url = "https://example.supabase.co", method = "POST", headers = {} }
check(result.status == 204 and result.body == nil, "204 responses must not attempt JSON parsing")
throwsResponse = false
response.code = function() return 200 end
response.text = function() return '{"id":"ok"}' end
result = transport { url = "https://example.supabase.co", method = "GET", headers = {} }
check(result.status == 200 and result.body.id == "ok", "Successful responses must decode JSON")
response.text = function() return "invalid JSON" end
local parsed = pcall(transport, { url = "https://example.supabase.co", method = "GET", headers = {} })
check(not parsed, "Malformed success JSON must not masquerade as successful data")
response.code = function() return 503 end
result = transport { url = "https://example.supabase.co", method = "GET", headers = {} }
check(result.status == 503 and result.body.code == "http_error", "Malformed error JSON must preserve the HTTP status")
package.loaded["orca.network"] = oldNetwork
package.loaded["api.json"] = oldJson
package.loaded["api.orca_transport"] = oldTransport

print("PASS: chat API (" .. checks .. " checks)")
