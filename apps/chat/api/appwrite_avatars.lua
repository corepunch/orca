-- Native profile-photo writes use the same Account, public profiles and
-- post-images bucket as Octopus. No Function or second user record is involved.
local M = {}
local PROFILES = "/databases/octopus-db/collections/profiles/documents"
local FILES = "/storage/buckets/post-images/files"

local function success(response) return response.status >= 200 and response.status < 300 end
local function validId(value)
  return type(value) == "string" and #value <= 36 and value:match("^[%w][%w%._%-]*$") ~= nil
end
local function failure(status, code, message) return { status = status, body = { code = code, message = message } } end

function M.new(options)
  local uploads = 0
  return function(body, credential, userId, current)
    if not credential or not validId(userId) or type(body) ~= "table" or body.userId ~= userId then
      return failure(401, "unauthenticated", "Sign in to choose a profile photo.")
    end
    local bytes = body.bytes
    local kind = type(bytes) == "string" and (bytes:sub(1, 3) == "\255\216\255" and "jpeg"
      or bytes:sub(1, 8) == "\137PNG\r\n\26\n" and "png")
    if not kind or #bytes > 10485760 then return failure(400, "invalid_image", "Choose a JPEG or PNG photo smaller than 10 MB.") end
    local function call(method, path, data, contentType)
      if not current() then return failure(409, "session_changed", "The active account changed. Please retry.") end
      local headers = { ["X-Appwrite-Project"] = options.projectId, ["Content-Type"] = contentType or "application/json" }
      for name, value in pairs(credential) do headers[name] = value end
      local ok, response = pcall(options.transport, { method = method, url = options.endpoint .. path, headers = headers,
        body = contentType and data or data and options.encode(data) })
      if not ok then return failure(503, "network", "Could not reach Appwrite. Please retry.") end
      if type(response) ~= "table" or type(response.status) ~= "number" then
        return failure(502, "invalid_response", "Appwrite returned an invalid profile photo response.")
      end
      if not success(response) and type(response.body) == "table" then response.body.code = response.body.type or response.body.code end
      return response
    end
    local path = PROFILES .. "/" .. userId
    local profile = call("GET", path)
    if profile.status == 404 then
      profile = call("POST", PROFILES, { documentId = userId,
        data = { userId = userId, username = type(body.displayName) == "string" and body.displayName or "", bio = "" } })
      -- Another app may have created the shared profile since the lookup.
      if profile.status == 409 then profile = call("GET", path) end
    end
    if not success(profile) then return profile end
    local doc = profile.body
    if type(doc) ~= "table" or doc["$id"] ~= userId or doc.userId ~= userId then
      return failure(502, "invalid_response", "Appwrite returned an invalid profile.")
    end
    uploads = uploads + 1
    local boundary = "----ChatOrcaAvatar" .. os.time() .. uploads
    local multipart = table.concat { "--", boundary, '\r\nContent-Disposition: form-data; name="fileId"\r\n\r\nunique()\r\n',
      "--", boundary, '\r\nContent-Disposition: form-data; name="file"; filename="avatar.', kind,
      '"\r\nContent-Type: image/', kind, "\r\n\r\n", bytes, "\r\n--", boundary, "--\r\n" }
    local uploaded = call("POST", FILES, multipart, "multipart/form-data; boundary=" .. boundary)
    if not success(uploaded) then return uploaded end
    local fileId = type(uploaded.body) == "table" and uploaded.body["$id"]
    if not validId(fileId) then return failure(502, "invalid_response", "Appwrite did not return the uploaded profile photo.") end
    local saved = call("PATCH", path, { data = { avatarId = fileId } })
    if not success(saved) then
      -- A rejected write leaves an unused upload. A lost response may have
      -- committed, so retain its file rather than break a saved profile photo.
      if saved.status >= 400 and saved.status < 500 then pcall(call, "DELETE", FILES .. "/" .. fileId) end
      return saved
    end
    doc = saved.body
    if type(doc) ~= "table" or doc["$id"] ~= userId or doc.userId ~= userId or doc.avatarId ~= fileId then
      return failure(502, "invalid_response", "Appwrite returned an invalid profile photo.")
    end
    if not current() then return failure(409, "session_changed", "The active account changed. Please retry.") end
    return { status = 200, body = { user_id = userId, avatar_id = fileId } }
  end
end

return M
