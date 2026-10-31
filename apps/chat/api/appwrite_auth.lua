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
  local currentUser, pushBusy, liveRecord
  local encode, store = options.encode, options.sessionStore
  local endpoint = options.endpoint:gsub("/$", "")
  local saveAvatar = require("api.appwrite_avatars").new {
    endpoint = endpoint, projectId = options.projectId, transport = options.transport, encode = options.encode,
  }
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
    current, pending, currentUser = nil, {}, nil
    local ok, err = stored("clear")
    return err
  end
  local function pushError(message)
    return { status = 503, body = { code = "push_cleanup", message = message } }
  end
  local function pushRecord(operation, value)
    if not options.pushStore then return nil, pushError("Notification target storage is unavailable.") end
    if operation == "load" and liveRecord then return liveRecord end
    local ok, result, err = pcall(options.pushStore[operation], value)
    if not ok or err or (operation ~= "load" and not result) then
      return nil, pushError("Could not access the saved notification target. Please retry.")
    end
    if operation == "clear" then liveRecord = nil end
    return result
  end
  local function removePush(credential, userId)
    if not options.pushStore then return true end
    if pushBusy then return nil, pushError("Notification registration is still running. Please retry signing out.") end
    local record, err = pushRecord("load")
    if err then return nil, err end
    if not record then return true end
    credential, userId = credential or current, userId or currentUser
    local function reauthenticate()
      return nil, forget() or { status = 401, body = { code = "push_reauthenticate",
        message = "Sign in with the previous account to finish disabling notifications, then retry signing out." } }
    end
    if not credential then return reauthenticate() end
    if record.userId ~= userId then
      return nil, pushError("Sign in with the previous account to remove this device's notification target.")
    end
    pushBusy = true
    local ok, response = pcall(call, "DELETE", "/account/targets/" .. record.id .. "/push", nil, credential)
    pushBusy = false
    if ok and (response.status == 401 or response.status == 403) then return reauthenticate() end
    if not ok or (not successful(response) and response.status ~= 404) then
      return nil, pushError("Could not disable notifications. Reconnect and retry signing out.")
    end
    return pushRecord("clear")
  end
  local function rejected(response, credential)
    if current == credential and (response.status == 401 or response.status == 403) then
      return forget() or response
    end
    return response
  end
  local function session(credential, persist)
    if not credential then return errorResponse("Appwrite did not return a private session cookie.") end
    local userResponse
    if options.pushStore and credential ~= current then
      if pushBusy then return pushError("Notification registration is still running. Please retry.") end
      local record, recordError = pushRecord("load")
      if recordError then return recordError end
      if record then
        -- Verify the new owner before replacing a private credential. A login
        -- to the saved target's owner also recovers after session revocation.
        userResponse = call("GET", "/account", nil, credential)
        if not successful(userResponse) then return userResponse end
        local userId = type(userResponse.body) == "table" and userResponse.body["$id"]
        if userId ~= record.userId then
          if not current or currentUser ~= record.userId then
            return pushError("Sign in with the previous account to remove this device's notification target.")
          end
          local removed, err = removePush()
          if not removed then return err end
        end
      end
    end
    if persist then
      local ok, err = stored("save", credential)
      if not ok then return err end
    end
    current = credential
    userResponse = userResponse or call("GET", "/account", nil, credential)
    if not successful(userResponse) then return rejected(userResponse, credential) end
    local user = userResponse.body
    if type(user) ~= "table" or type(user["$id"]) ~= "string" or user["$id"] == "" then
      return errorResponse("Appwrite returned an invalid account.")
    end
    if current ~= credential then return pushError("The active account changed. Please retry.") end
    currentUser = user["$id"]
    local jwtResponse = call("POST", "/account/jwts", {}, credential)
    if not successful(jwtResponse) then return rejected(jwtResponse, credential) end
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
    if request.method == "POST" and path == "/auth/v1/avatar" then
      local credential = current
      return saveAvatar(body, credential, currentUser, function() return current == credential end)
    elseif request.method == "POST" and path == "/auth/v1/push" then
      if not current or not currentUser or body.userId ~= currentUser then
        return { status = 401, body = { code = "unauthenticated", message = "Sign in to enable notifications." } }
      end
      if type(options.pushProviderId) ~= "string" or options.pushProviderId == "" then
        return { status = 400, body = { code = "push_disabled", message = "Push notifications are not configured." } }
      end
      if type(body.token) ~= "string" or #body.token < 32 or #body.token > 512 or body.token:find("[^%x]") then
        return { status = 400, body = { code = "validation", message = "Invalid notification token." } }
      end
      if pushBusy then return pushError("Notification registration is still running. Please retry.") end
      local record, err = pushRecord("load")
      if err then return err end
      if record and record.userId ~= currentUser then
        return pushError("Sign in with the previous account to remove this device's notification target.")
      end
      if record and record.providerId and record.providerId ~= options.pushProviderId then
        local removed, removalError = removePush()
        if not removed then return removalError end
        record = nil
      end
      record = record or { id = "apns-" .. body.token:sub(1, 30), userId = currentUser }
      record.providerId = options.pushProviderId
      local saved, saveError = pushRecord("save", record)
      if not saved then return saveError end
      local credential = current
      pushBusy = true
      local ok, response = pcall(function()
        local payload = { identifier = body.token }
        local result = call("PUT", "/account/targets/" .. record.id .. "/push", payload, credential)
        if result.status == 404 then
          payload.targetId = record.id
          payload.providerId = options.pushProviderId
          result = call("POST", "/account/targets/push", payload, credential)
        end
        return result
      end)
      pushBusy = false
      if not ok then return pushError("Could not register notifications. Please retry.") end
      if current ~= credential then return pushError("The active account changed. Please retry.") end
      if successful(response) then
        local id = type(response.body) == "table" and response.body["$id"]
        if type(id) ~= "string" or #id > 36 or not id:match("^[%w][%w%._%-]*$") then
          return errorResponse("Appwrite returned an invalid notification target.")
        end
        record.id, liveRecord = id, record
        local persisted, persistenceError = pushRecord("save", record)
        if not persisted then return persistenceError end
      end
      return response
    elseif request.method == "POST" and path == "/auth/v1/token?grant_type=password" then
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
      local removed, pushFailure = removePush()
      if not removed then return pushFailure end
      local credential = current
      local err = forget()
      if err then return err end
      if not credential then return { status = 204 } end
      return call("DELETE", "/account/sessions/current", nil, credential)
    end
  end
end

return M
