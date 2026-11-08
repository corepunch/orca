-- Public posts reuse Chat's active identity without a second sign-in or a
-- private Account cookie. All requests here use an injected offline transport.
package.path = "apps/chat/?.lua;" .. package.path
local Chat = require "api.chat"
local json = require "api.json"
local endpoint, checks = "https://fra.cloud.appwrite.io/v1", 0
local function check(value, message) assert(value, message); checks = checks + 1 end
local requests, responses = {}, {}
local client
client = assert(Chat.appwrite {
  endpoint = endpoint, projectId = "project", functionId = "orca-chat-api",
  clock = function() return 100 end,
  encode = json.encode, decode = json.decode,
  transport = function(request)
    requests[#requests + 1] = request
    check(not request.headers.Cookie and not request.headers["X-Appwrite-Key"], "Public requests must not carry cookies or server keys")
    local response = table.remove(responses, 1)
    assert(response, "Unexpected network request")
    if type(response) == "function" then return response(request) end
    return response
  end,
})
local function identity(id, token)
  assert(client:_setSession { access_token = token or "active-jwt", refresh_token = "refresh", expires_at = 1000,
    user = { id = id or "reader", user_metadata = { display_name = "Reader" } } })
end
responses[1] = { status = 200, body = { documents = {} }, headers = { ["set-cookie"] = "never-export=this" } }
local response = assert(client:publicPostsRequest { method = "GET", path = "/databases/octopus-db/collections/post-feed/documents",
  query = { { method = "limit", values = { 20 } } }, params = { total = false } })
check(response.status == 200 and #response.body.documents == 0, "Guests can read public posts")
check(not requests[1].headers["X-Appwrite-JWT"], "Guest requests carry no identity")
check(not response.headers, "Public responses never export Set-Cookie")
check(requests[1].url:find("queries[0]=", 1, true) and requests[1].url:find("total=false", 1, true), "Octopus query parameters are preserved")

identity()
responses[1] = { status = 201, body = { ["$id"] = "post" } }
response = assert(client:publicPostsRequest { method = "POST", path = "/databases/octopus-db/collections/posts/documents",
  body = { documentId = "unique()", data = { content = "Hello 👋" } },
  headers = { Cookie = "do-not-forward", ["X-Appwrite-Key"] = "do-not-forward", ["X-Appwrite-JWT"] = "wrong-jwt" } })
check(response.status == 201 and response.body["$id"] == "post", "Writes preserve raw HTTP status and body")
check(requests[2].headers["X-Appwrite-JWT"] == "active-jwt", "Posts use Chat's active JWT")
check(json.decode(requests[2].body).data.content == "Hello 👋", "Post payload remains UTF-8 JSON")
local before = #requests
response = assert(client:publicPostsRequest { method = "POST", path = "/account/jwts" })
check(response.body.jwt == "active-jwt" and #requests == before, "Profile RPC uses the existing short-lived JWT")

responses[1] = { status = 200, body = "jpeg\0bytes" }
response = assert(client:publicPostsRequest { method = "GET", path = "/storage/buckets/post-images/files/photo/preview?width=96", binary = true })
check(response.body == "jpeg\0bytes" and requests[#requests].responseType == "binary", "Public photo requests preserve binary response mode")
responses[1] = { status = 201, body = { ["$id"] = "photo" } }
response = assert(client:publicPostsRequest { method = "POST", path = "/storage/buckets/post-images/files", body = "raw\0jpeg",
  headers = { ["Content-Type"] = "multipart/form-data; boundary=photo" } })
check(requests[#requests].body == "raw\0jpeg" and requests[#requests].headers["Content-Type"] == "multipart/form-data; boundary=photo",
  "Multipart uploads retain their content type and bytes")
responses[1] = { status = 403, body = { type = "user_unauthorized", message = "Not yours" } }
response = assert(client:publicPostsRequest { method = "DELETE", path = "/databases/octopus-db/collections/posts/documents/other" })
check(response.status == 403 and response.body.message == "Not yours", "HTTP failures remain visible to the public client")

for _, path in ipairs {
  "https://outside.example/posts", "/account/sessions/current", "/users", "/databases/octopus-db/collections/chat_messages/documents",
  "/databases/octopus-db/collections/posts/documents/../profiles", "/databases/octopus-db/collections/posts/documents/%2e%2e",
  "/storage/buckets/private/files/photo/view", "/functions/other-function/executions", "/databases/octopus-db/collections/posts/documents\r\nInjected: true",
} do
  before = #requests
  local value, err = client:publicPostsRequest { method = "GET", path = path }
  check(not value and err.code == "validation" and #requests == before, "Reject requests outside public-post resources: " .. path)
end
client.session = nil
before = #requests
local value, err = client:publicPostsRequest { method = "POST", path = "/databases/octopus-db/collections/posts/documents", body = {} }
check(not value and err.code == "unauthenticated" and #requests == before, "Signed-out clients cannot write public posts")

identity()
responses[1] = function()
  identity("other-reader", "other-jwt")
  return { status = 200, body = { documents = { { ["$id"] = "old-reader-post" } } } }
end
value, err = client:publicPostsRequest { method = "GET", path = "/databases/octopus-db/collections/posts/documents" }
check(not value and err.code == "session_changed", "Results for an earlier session never reach the new reader")

-- Refresh is owned by Chat. The public feature never creates another login.
identity()
client.session.expires_at = 100
local refreshes = 0
client.refreshSession = function(self)
  refreshes = refreshes + 1
  return self:_setSession { access_token = "renewed-jwt", refresh_token = "refresh", expires_at = 1000,
    user = { id = "reader" } }
end
responses[1] = { status = 200, body = { documents = {} } }
assert(client:publicPostsRequest { method = "GET", path = "/databases/octopus-db/collections/posts/documents" })
check(refreshes == 1 and requests[#requests].headers["X-Appwrite-JWT"] == "renewed-jwt", "Expiring tokens refresh before public requests")
responses[1] = { status = 401, body = { message = "expired" } }
responses[2] = { status = 200, body = { documents = {} } }
response = assert(client:publicPostsRequest { method = "GET", path = "/databases/octopus-db/collections/posts/documents" })
check(response.status == 200 and refreshes == 2, "A rejected JWT refreshes and retries once")
check(#responses == 0, "Every expected offline request was consumed")
print("Chat public posts session: " .. checks .. " checks passed")
