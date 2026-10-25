-- Native Account auth has no dependency on the Chat function or its database.
-- Each adapter owns its session cookie; the HTTP layer disables its shared jar.
local M = {}

local function errorResponse(message)
  return { status = 502, body = { code = "invalid_response", message = message } }
end

local function successful(response)
  return type(response) == "table" and type(response.status) == "number"
    and response.status >= 200 and response.status < 300
end

local function credentials(response, projectId)
  local name = "a_session_" .. projectId
  local cookie, legacy
  local function inspect(value)
    if type(value) == "table" then for _, line in ipairs(value) do inspect(line) end
    elseif type(value) == "string" then
      for line in value:gmatch("[^\r\n]+") do
        local key, content = line:match("^%s*([^=; ]+)=([^;\r\n]+)")
        if content and not content:find("[%z\1-\32\127]") then
          if key == name then cookie = key .. "=" .. content
          elseif key == name .. "_legacy" then legacy = key .. "=" .. content end
        end
      end
    end
  end
  for key, value in pairs(response.headers or {}) do
    if type(key) == "string" and key:lower() == "set-cookie" then inspect(value) end
  end
  if cookie or legacy then return { Cookie = cookie or legacy } end
  local secret = type(response.body) == "table" and response.body.secret
  if type(secret) == "string" and secret ~= "" and not secret:find("[%z\1-\32\127]") then
    return { ["X-Appwrite-Session"] = secret }
  end
end

function M.new(options)
  local current, pending = nil, {}
  local encode, store = options.encode, options.sessionStore
  local endpoint = options.endpoint:gsub("/$", "")
  local function call(method, path, body, credential)
    local headers = { ["X-Appwrite-Project"] = options.projectId, ["Content-Type"] = "application/json" }
    for name, value in pairs(credential or {}) do headers[name] = value end
    local response = options.transport { method = method, url = endpoint .. path, headers = headers,
      body = body and encode(body) or nil }
    if type(response) ~= "table" or type(response.status) ~= "number" then
      return errorResponse("Appwrite returned an invalid Account response.")
    end
    if not successful(response) and type(response.body) == "table" then
      -- Appwrite's numeric code is the HTTP status; type is its stable error ID.
      response.body.code = response.body.type or response.body.code
    end
    return response
  end
  local function stored(operation, value)
    if not store then return true end
    local ok, result, err = pcall(store[operation], value)
    if not ok or err or (operation ~= "load" and not result) then
      return nil, { status = 503, body = { code = "session_storage",
        message = "Could not access your saved login. Please try again." } }
    end
    return result
  end
  local function forget()
    current, pending = nil, {}
    local ok, err = stored("clear")
    return err
  end
  local function rejected(response)
    if response.status == 401 or response.status == 403 then
      return forget() or response
    end
    return response
  end
  local function session(credential, persist)
    if not credential then return errorResponse("Appwrite did not return a private session cookie.") end
    if persist then
      local ok, err = stored("save", credential)
      if not ok then return err end
    end
    current = credential
    local userResponse = call("GET", "/account", nil, credential)
    if not successful(userResponse) then return rejected(userResponse) end
    local user = userResponse.body
    if type(user) ~= "table" or type(user["$id"]) ~= "string" or user["$id"] == "" then
      return errorResponse("Appwrite returned an invalid account.")
    end
    local jwtResponse = call("POST", "/account/jwts", {}, credential)
    if not successful(jwtResponse) then return rejected(jwtResponse) end
    local jwt = type(jwtResponse.body) == "table" and jwtResponse.body.jwt
    if type(jwt) ~= "string" or jwt == "" then return errorResponse("Appwrite did not return a user JWT.") end
    return { status = 200, body = { access_token = jwt, refresh_token = "appwrite-current-session", expires_in = 900,
      user = { id = user["$id"], email = user.email, phone = user.phone,
        user_metadata = { display_name = user.name or "" } } } }
  end
  local function signIn(email, password)
    local response = call("POST", "/account/sessions/email", { email = email, password = password })
    if not successful(response) then return response end
    return session(credentials(response, options.projectId), true)
  end
  return function(request, path)
    local body = request.body or {}
    if request.method == "POST" and path == "/auth/v1/token?grant_type=password" then
      return signIn(body.email, body.password)
    elseif request.method == "POST" and path == "/auth/v1/signup" then
      local response = call("POST", "/account", { userId = "unique()", email = body.email,
        password = body.password, name = body.data and body.data.display_name or "" })
      if not successful(response) then return response end
      return signIn(body.email, body.password)
    elseif request.method == "POST" and path == "/auth/v1/otp" then
      local field = body.phone and "phone" or "email"
      local response = call("POST", "/account/tokens/" .. field,
        { userId = "unique()", [field] = body[field] })
      if not successful(response) then return response end
      local token = response.body
      if type(token) ~= "table" or type(token.userId) ~= "string" or token.userId == "" then
        return errorResponse("Appwrite returned an invalid verification token.")
      end
      pending[field .. ":" .. body[field]] = token.userId
      return { status = 200, body = {} }
    elseif request.method == "POST" and path == "/auth/v1/verify" and (body.type == "email" or body.type == "sms") then
      local field = body.type == "sms" and "phone" or "email"
      local key = field .. ":" .. body[field]
      local userId = pending[key]
      if not userId then return { status = 400, body = { code = "verification_missing", message = "Request a new verification code first." } } end
      local response = call("POST", "/account/sessions/token", { userId = userId, secret = body.token })
      if not successful(response) then return response end
      pending[key] = nil
      return session(credentials(response, options.projectId), true)
    elseif request.method == "POST" and path == "/auth/v1/restore" then
      local saved, err
      if store then saved, err = stored("load") end
      if err then return err end
      if not saved then return { status = 401, body = { code = "unauthenticated", message = "Sign in to continue." } } end
      -- Never restore arbitrary headers, even from private device storage.
      local credential = type(saved) == "table" and credentials({ headers = { ["set-cookie"] = saved.Cookie },
        body = { secret = saved["X-Appwrite-Session"] } }, options.projectId)
      if not credential then
        return forget() or { status = 401, body = { code = "unauthenticated", message = "Sign in again." } }
      end
      return session(credential)
    elseif request.method == "POST" and path == "/auth/v1/token?grant_type=refresh_token" then
      if not current then return { status = 401, body = { code = "user_unauthorized", message = "Sign in again." } } end
      return session(current)
    elseif request.method == "POST" and path == "/auth/v1/logout?scope=local" then
      local credential = current
      local err = forget()
      if err then return err end
      if not credential then return { status = 204 } end
      return call("DELETE", "/account/sessions/current", nil, credential)
    end
  end
end

return M
