-- The Appwrite adapter runs without native modules or a live cloud account.
package.path = "apps/chat/?.lua;" .. package.path
local Chat = require "api.chat"
local json = require "api.json"
local Adapter = require "api.appwrite_transport"
local checks = 0
local function check(condition, message)
  assert(condition, message)
  checks = checks + 1
end
local endpoint = "https://fra.cloud.appwrite.io/v1"
local function execution(status, body, encoding)
  return { status = 201, body = { status = "completed", responseStatusCode = 200,
    responseBody = json.encode { status = status, body = body, encoding = encoding } } }
end
local function session(expires)
  return { access_token = "jwt-one", refresh_token = "appwrite-current-session", expires_in = expires or 3600,
    user = { id = "user.with_underscore", email = "you@example.test" } }
end
local function signedIn(tail)
  local responses = {
    { status = 201, body = { secret = "", userId = "user.with_underscore" }, headers = {
      ["set-cookie"] = { "irrelevant=ignored; Path=/", "a_session_69f1c06800389dc6a1a0_legacy=legacy; Path=/",
        "a_session_69f1c06800389dc6a1a0=cookie-one; Path=/; HttpOnly; Secure" } } },
    { status = 200, body = { ["$id"] = "user.with_underscore", email = "you@example.test", name = "You" } },
    { status = 201, body = { jwt = "jwt-one" } },
  }
  for _, response in ipairs(tail or {}) do responses[#responses + 1] = response end
  return responses
end
local function backend(responses, sessionStore)
  local requests, envelopes = {}, {}
  local options = { endpoint = endpoint, projectId = "69f1c06800389dc6a1a0", functionId = "orca-chat-api",
    clock = function() return 1000 end, encode = json.encode, decode = json.decode, sessionStore = sessionStore,
    transport = function(request)
      requests[#requests + 1] = request
      if request.url:find("/functions/", 1, true) then
        local payload = json.decode(request.body)
        envelopes[#envelopes + 1] = json.decode(payload.body)
        check(payload.async == false, "Chat requires synchronous function execution")
      end
      check(request.headers["X-Appwrite-Key"] == nil, "Native clients must never send a server key")
      local response = table.remove(responses, 1)
      assert(response, "Unexpected Appwrite request")
      if response.throws then error("offline") end
      return response
    end }
  return assert(Chat.appwrite(options)), requests, envelopes, options
end

local special = 'Quotes " slash \\ newline\n tab\t NUL\0 café 👋'
check(json.decode(json.encode { text = special }).text == special, "JSON must escape strings and preserve UTF-8/control bytes")
check(json.decode('"\\ud83d\\udc4b"') == "👋", "JSON must decode Unicode surrogate pairs")
for _, bad in ipairs { '{"unterminated', '{"a":1,}', '[1,]', '{}garbage', '{"a":01}', '{"a":1.}',
  '{"a":true false}', '"\\ud800"', '"\\udfff"', '"\\x00"', '"line\n"', '{"a":1e}', '{"a":1e9999}' } do
  check(not pcall(json.decode, bad), "Malformed JSON must be rejected: " .. bad)
end
check(json.decode('{"number":-1.25e+2,"boolean":false}').number == -125, "JSON numbers and booleans must decode")

local client, requests, envelopes, options = backend(signedIn {
  execution(200, { { id = "room.one_1", name = special } }),
  execution(200, { { id = "message.one_1", body = special } }),
})
check(client.mode == "appwrite" and not client:getSession(), "Appwrite starts signed out and never fabricates demo data")
assert(client:signIn("you@example.test", special))
check(requests[1].url == endpoint .. "/account/sessions/email" and #envelopes == 0,
  "Native password sign-in must work independently of the Chat function")
check(requests[1].headers["X-Appwrite-Project"] == options.projectId and not requests[1].headers["X-Appwrite-Key"],
  "Client requests contain only the public project ID")
check(json.decode(requests[1].body).password == special and not requests[1].headers.Cookie, "Native sign-in safely encodes credentials without a stale session")
check(requests[2].url == endpoint .. "/account" and requests[3].url == endpoint .. "/account/jwts"
  and requests[2].headers.Cookie == "a_session_69f1c06800389dc6a1a0=cookie-one",
  "Account lookup and JWT exchange use only this project's private session cookie")
check(client:getSession().access_token == "jwt-one" and client:getSession().expires_at == 1900,
  "Native session mapping must use a short-lived user JWT")
local room = assert(client:createRoom(special, {}))
check(room.id == "room.one_1" and not envelopes[1].session and requests[4].headers["X-Appwrite-JWT"] == "jwt-one",
  "Authenticated executions carry the Appwrite user JWT")
check(not requests[4].headers.Cookie, "Private Account cookies must never be forwarded to the Chat function")
local rawEnvelope = json.decode(requests[4].body).body
check(rawEnvelope:find('"member_ids":[]', 1, true) ~= nil, "Empty member arrays must remain JSON arrays")
assert(client:sendMessage("room.one_1", special))
check(envelopes[2].body.message_body == special, "Escaped message text must survive both JSON layers")
check(json.decode(requests[5].body).body:find('"attachment_ids":[]', 1, true) ~= nil,
  "Empty attachment arrays must remain JSON arrays")

local signupResponses = signedIn()
table.insert(signupResponses, 1, { status = 201, body = { ["$id"] = "user.with_underscore" } })
client, requests, envelopes = backend(signupResponses)
check(client:signUp("you@example.test", "password123", special) ~= nil and #envelopes == 0,
  "Native signup creates an Appwrite account and session without the Chat function")
check(requests[1].url == endpoint .. "/account" and json.decode(requests[1].body).name == special
  and json.decode(requests[1].body).userId == "unique()", "Native signup preserves display names and uses server-generated IDs")

for _, identity in ipairs { { value = "YOU@example.test", normalized = "you@example.test", field = "email" },
  { value = "+49 (151) 1234-5678", normalized = "+4915112345678", field = "phone" } } do
  local otpResponses = signedIn()
  table.insert(otpResponses, 1, { status = 201, body = { userId = "user.with_underscore", secret = "" } })
  client, requests, envelopes = backend(otpResponses)
  check(client:requestCode(identity.value) ~= nil and not client:getSession(), "Requesting OTP must not create a session")
  check(requests[1].url == endpoint .. "/account/tokens/" .. identity.field
    and json.decode(requests[1].body)[identity.field] == identity.normalized, "Native OTP normalizes and sends the selected identifier")
  check(client:verifyCode(identity.value, "123456") ~= nil and #envelopes == 0, "Native OTP verification establishes a session without function auth")
  check(requests[2].url == endpoint .. "/account/sessions/token"
    and json.decode(requests[2].body).userId == "user.with_underscore"
    and json.decode(requests[2].body).secret == "123456", "OTP uses the user ID returned by its matching challenge")
end
client, requests = backend {}
local value, err = client:verifyCode("you@example.test", "123456")
check(not value and err.code == "verification_missing" and #requests == 0, "OTP verification requires this client's pending challenge")

client, requests, envelopes = backend(signedIn {
  { status = 200, body = { ["$id"] = "user.with_underscore", email = "you@example.test", name = "You" } },
  { status = 201, body = { jwt = "jwt-two" } }, execution(200, {}), { status = 204 },
})
assert(client:signIn("you@example.test", "password"))
client.session.expires_at = 1010
assert(client:listRooms())
check(requests[4].headers.Cookie == "a_session_69f1c06800389dc6a1a0=cookie-one"
  and requests[5].url == endpoint .. "/account/jwts" and requests[6].headers["X-Appwrite-JWT"] == "jwt-two",
  "JWT expiry must renew with the private Account cookie before executing a function")
check(client:getSession().access_token == "jwt-two" and client:getSession().refresh_token == "appwrite-current-session",
  "Public sessions expose the JWT and an opaque refresh marker, without the private cookie")
assert(client:signOut())
check(requests[7].method == "DELETE" and requests[7].url == endpoint .. "/account/sessions/current"
  and not client:getSession(), "Sign-out must revoke the native Account session directly")
assert(client:_setSession(session()))
value, err = client:refreshSession()
check(not value and err.status == 401 and #requests == 7, "Signing out must also erase the adapter's private credentials")

local secretResponses = signedIn()
secretResponses[1].body.secret, secretResponses[1].headers = "native-session-secret", nil
client, requests = backend(secretResponses)
assert(client:signIn("you@example.test", "password"))
check(requests[2].headers["X-Appwrite-Session"] == "native-session-secret" and not requests[2].headers.Cookie,
  "A native Session.secret may authenticate Account calls when available")
client, requests = backend { { status = 201, body = { secret = "" } } }
value, err = client:signIn("you@example.test", "password")
check(not value and err.code == "invalid_response" and #requests == 1 and not client:getSession(),
  "A Session response without secret or captured cookie must fail instead of pretending to be signed in")

local otherResponses = signedIn()
otherResponses[1].headers["set-cookie"][3] = "a_session_69f1c06800389dc6a1a0=cookie-two; Path=/"
local firstClient, firstRequests = backend(signedIn())
local otherClient, otherRequests = backend(otherResponses)
assert(firstClient:signIn("first@example.test", "password"))
assert(otherClient:signIn("second@example.test", "password"))
check(firstRequests[2].headers.Cookie:find("cookie-one", 1, true)
  and otherRequests[2].headers.Cookie:find("cookie-two", 1, true)
  and not firstRequests[1].headers.Cookie and not otherRequests[1].headers.Cookie,
  "Independent Appwrite clients must never share their Account cookies")

local bytes = "\255\216\0\1\2\255\217"
local binaryRequest
local adapter = Adapter.new {
  endpoint = endpoint, projectId = options.projectId, functionId = options.functionId,
  encode = json.encode, decode = json.decode,
  transport = function(request)
    binaryRequest = request
    local envelope = json.decode(json.decode(request.body).body)
    check(envelope.encoding == "base64" and envelope.body == "/9gAAQL/2Q==", "Photo bytes must use lossless base64 upload")
    return execution(200, envelope.body, "base64")
  end,
}
local binary = adapter { method = "POST", url = endpoint .. "/storage/v1/object/chat-photos/file",
  headers = { Authorization = "Bearer per-request-secret" }, body = bytes, responseType = "binary" }
check(binary.body == bytes, "Photo download must preserve binary NUL and non-UTF-8 bytes")
check(not binaryRequest.headers.Authorization, "Function execution must not confuse user sessions with Appwrite API headers")
for _, url in ipairs { "https://evil.test/v1/rest/v1/chat_rooms", endpoint .. ".evil/rest/v1/chat_rooms",
  endpoint .. "/../rest/v1/chat_rooms", endpoint .. "/account",
  endpoint .. "/storage/v1/object/chat-photos/../file", endpoint .. "/storage/v1/object/chat-photos/./file",
  endpoint .. "/storage/v1/object/chat-photos/%2e%2e/file", endpoint .. "/storage/v1/object/chat-photos/a%2f..%2ffile",
  endpoint .. "/storage/v1/object/chat-photos/%5cfile", endpoint .. "/rest/v1/chat_rooms\r\n" } do
  local before = binaryRequest
  check(not pcall(adapter, { method = "GET", url = url }), "The adapter must reject foreign or unsupported URLs")
  check(binaryRequest == before, "Rejected URLs must never reach the HTTP transport")
end

local dottedSession = session()
dottedSession.user.id = "person..id"
client, requests, envelopes = backend { execution(200, {}), execution(200, {}) }
assert(client:_setSession(dottedSession))
check(client:listMessages("room..id") ~= nil and envelopes[1].path:find("room_id=eq.room..id", 1, true),
  "Consecutive periods in a valid conversation ID must reach Appwrite")
check(client:listContacts() ~= nil and envelopes[2].path:find("id=neq.person..id", 1, true),
  "Consecutive periods in a valid user ID must remain valid in query filters")

client, requests, envelopes = backend(signedIn {
  execution(401, { code = "user_unauthorized", message = "Expired" }),
  { status = 401, body = { type = "user_unauthorized", code = 401, message = "Sign in again" } },
})
assert(client:signIn("you@example.test", "password"))
value, err = client:listRooms()
check(not value and err.code == "user_unauthorized" and not client:getSession(), "Expired Appwrite session must clear credentials after rejected refresh")
check(requests[5].url == endpoint .. "/account" and #envelopes == 1, "Expired JWT refresh authenticates directly with Account")

client = backend(signedIn { { throws = true } })
assert(client:signIn("you@example.test", "password"))
value, err = client:signOut()
check(not value and err.code == "network" and not client:getSession(), "Offline sign-out must forget local credentials")

-- Recreating the client models a process crash: only the private store survives.
local saved, saves, clears = nil, 0, 0
local store = {
  load = function() return saved and json.decode(saved) end,
  save = function(credential) saved = json.encode(credential); saves = saves + 1; return true end,
  clear = function() saved = nil; clears = clears + 1; return true end,
}
client, requests = backend(signedIn(), store)
assert(client:signIn("you@example.test", "password"))
check(saves == 1 and json.decode(saved).Cookie == "a_session_69f1c06800389dc6a1a0=cookie-one"
  and not saved:find("jwt-one", 1, true) and not saved:find("password", 1, true),
  "Sign-in must save only the private Account credential before it completes")
local account = { status = 200, body = { ["$id"] = "user.with_underscore", name = "You" } }
client, requests, envelopes = backend({ account, { status = 201, body = { jwt = "restored-jwt" } },
  execution(200, {}) }, store)
check(not client:getSession() and #requests == 0, "Constructing a client must not perform network requests")
assert(client:restoreSession())
check(client:getSession().access_token == "restored-jwt" and requests[1].url == endpoint .. "/account"
  and requests[1].headers.Cookie == json.decode(saved).Cookie and requests[2].url == endpoint .. "/account/jwts",
  "Restart must validate the stored Account session and obtain a fresh JWT")
assert(client:listRooms())
check(requests[3].headers["X-Appwrite-JWT"] == "restored-jwt" and not requests[3].headers.Cookie,
  "A restored private cookie must not reach the Chat function")

client, requests = backend({ { throws = true } }, store)
value, err = client:restoreSession()
check(not value and err.code == "network" and saved and clears == 0,
  "An offline launch must preserve the saved login for retry")
client = backend({ { status = 503, body = { message = "Unavailable" } } }, store)
value, err = client:restoreSession()
check(not value and err.status == 503 and saved and clears == 0,
  "A service outage must not erase the saved login")
client = backend({ { status = 401, body = { type = "user_unauthorized" } } }, store)
value, err = client:restoreSession()
check(not value and err.status == 401 and not saved and clears == 1 and not client:getSession(),
  "A revoked Account session must be removed from secure storage")
client, requests = backend({}, store)
value, err = client:restoreSession()
check(not value and err.code == "unauthenticated" and #requests == 0,
  "First launch without a stored credential must not contact Appwrite")

client, requests = backend(signedIn { { throws = true } }, store)
assert(client:signIn("you@example.test", "password"))
client.session.expires_at = 0
value, err = client:signOut()
check(not value and err.code == "network" and not saved and not client:getSession()
  and requests[4].method == "DELETE" and #requests == 4,
  "Offline logout must erase the saved session without refreshing an expired JWT")
client, requests = backend({}, store)
value, err = client:restoreSession()
check(not value and err.code == "unauthenticated" and #requests == 0,
  "Restart after offline logout must stay signed out")

client = backend(signedIn(), store)
assert(client:signIn("you@example.test", "password"))
client, requests = backend({}, store)
assert(client:signOut())
check(not saved and #requests == 0, "Logout before restore must erase the stored credential")

local interrupted = signedIn()
interrupted[2] = { throws = true }
client = backend(interrupted, store)
value, err = client:signIn("you@example.test", "password")
check(not value and err.code == "network" and saved,
  "The Account credential must be durable before account lookup or JWT exchange can fail")
client = backend({ account, { status = 201, body = { jwt = "after-interruption" } } }, store)
check(client:restoreSession().access_token == "after-interruption", "Interrupted sign-in must be recoverable on restart")

local brokenStore = { load = function() error("locked") end,
  save = function() return nil, "locked" end, clear = function() return nil, "locked" end }
client, requests = backend({}, brokenStore)
value, err = client:restoreSession()
check(not value and err.code == "session_storage" and #requests == 0,
  "Keychain read failures must be distinguishable from a missing session")
client, requests = backend(signedIn(), brokenStore)
value, err = client:signIn("you@example.test", "password")
check(not value and err.code == "session_storage" and not client:getSession() and #requests == 1,
  "Failed credential persistence must not claim a durable sign-in succeeded")
client = backend({}, brokenStore)
value, err = client:signOut()
check(not value and err.code == "session_storage", "Failure to erase a stored session must be reported")

saved = json.encode { Cookie = "a_session_other=foreign", Authorization = "Bearer injected" }
client, requests = backend({}, store)
value, err = client:restoreSession()
check(not value and not saved and #requests == 0,
  "A stored cookie belonging to another project must be discarded without a request")
saved = json.encode { ["X-Appwrite-Session"] = "private-secret", Authorization = "Bearer injected" }
client, requests = backend({ account, { status = 201, body = { jwt = "secret-restored" } } }, store)
assert(client:restoreSession())
check(requests[1].headers["X-Appwrite-Session"] == "private-secret" and not requests[1].headers.Authorization,
  "Restore must support native session secrets without restoring arbitrary headers")

for _, response in ipairs {
  { status = 201, body = { status = "failed", responseStatusCode = 500, responseBody = '{}' } },
  { status = 201, body = { status = "waiting", responseStatusCode = 0, responseBody = '' } },
  { status = 201, body = { status = "completed", responseStatusCode = 200, responseBody = '{"status":200}garbage' } },
  { status = 201, body = { status = "completed", responseStatusCode = 200, responseBody = '{"status":"200"}' } },
  { status = 201, body = "bad outer JSON" },
} do
  client = backend { response }
  assert(client:_setSession(session()))
  value, err = client:listRooms()
  check(not value and err.code == "invalid_response" and client.mode == "appwrite", "Malformed executions must fail without switching to mock")
end
client = backend { { status = 429, body = { code = "general_rate_limit_exceeded", message = "Wait" } } }
value, err = client:requestCode("you@example.test")
check(not value and err.status == 429 and err.code == "general_rate_limit_exceeded", "Appwrite HTTP errors must retain status and message")

local savedNetwork, savedNativeJson = package.loaded["orca.network"], package.loaded["orca.parsers.json"]
package.loaded["orca.network"], package.loaded["orca.parsers.json"] = nil, nil
client = backend { execution(200, session()) }
check(client ~= nil and package.loaded["orca.network"] == nil and package.loaded["orca.parsers.json"] == nil,
  "Injected transport and codec must not load native modules")
package.loaded["orca.network"], package.loaded["orca.parsers.json"] = savedNetwork, savedNativeJson

value, err = Chat.appwrite { endpoint = "http://evil.test/v1", projectId = options.projectId, functionId = options.functionId }
check(not value and err.code == "configuration", "Remote Appwrite endpoints require HTTPS")
value, err = Chat.appwrite { endpoint = endpoint, projectId = "bad/project", functionId = options.functionId }
check(not value and err.code == "configuration", "Invalid project IDs must fail before native module loading")
print("PASS: chat Appwrite (" .. checks .. " checks)")
