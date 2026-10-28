-- One application API; the transport selects network or in-process storage.
local Chat = {}
local Client = {}
Client.__index = Client

local function copy(value)
  if type(value) ~= "table" then return value end
  local result = {}
  for key, item in pairs(value) do result[key] = copy(item) end
  return result
end

local function failure(code, message, status)
  return nil, { code = code, message = message, status = status }
end

local function text(value)
  return type(value) == "string" and value:match("%S") ~= nil
end

local function escape(value)
  return (tostring(value):gsub("([^%w%-_%.~])", function(c)
    return string.format("%%%02X", string.byte(c))
  end))
end

local function validId(value)
  return type(value) == "string" and #value <= 36 and value:match("^[%w][%w%._%-]*$") ~= nil
end

function Client:getSession()
  return copy(self.session)
end

function Client:_request(method, path, body, authenticated, retried, options)
  options = options or {}
  if authenticated then
    if not self.session then return failure("unauthenticated", "Sign in to continue.") end
    if not retried and self.session.expires_at <= self.clock() + 30 then
      local session, err = self:refreshSession()
      if not session then return nil, err end
    end
  end
  local headers = { ["Content-Type"] = "application/json" }
  for key, value in pairs(options.headers or {}) do headers[key] = value end
  if authenticated then headers.Authorization = "Bearer " .. self.session.access_token end
  local ok, response = pcall(self.transport, {
    method = method, url = self.url .. path, headers = headers, body = body, responseType = options.responseType,
    arrayFields = options.arrayFields,
  })
  if not ok or type(response) ~= "table" or type(response.status) ~= "number" then
    return failure("network", "Could not reach the chat service. Please try again.")
  end
  if response.status == 401 and authenticated and not retried then
    local session, err = self:refreshSession()
    if not session then return nil, err end
    return self:_request(method, path, body, true, true, options)
  end
  if response.status < 200 or response.status >= 300 then
    local data = type(response.body) == "table" and response.body or {}
    return failure(data.code or data.error_code or "http_error",
      data.message or data.msg or data.error_description or "The chat service rejected the request.", response.status)
  end
  if response.body == nil then return true end
  return response.body
end

function Client:_setSession(data)
  if type(data) ~= "table" or not text(data.access_token) or not text(data.refresh_token)
      or type(data.user) ~= "table" or not text(data.user.id) then
    return failure("invalid_response", "The chat service returned an invalid session.")
  end
  local expires = tonumber(data.expires_at)
  if not expires then expires = self.clock() + (tonumber(data.expires_in) or 3600) end
  self.session = { access_token = data.access_token, refresh_token = data.refresh_token,
    expires_at = expires, user = copy(data.user) }
  return self:getSession()
end

function Client:signIn(email, password)
  if not text(email) or not text(password) then return failure("validation", "Enter your email and password.") end
  local data, err = self:_request("POST", "/auth/v1/token?grant_type=password", { email = email, password = password })
  if not data then return nil, err end
  return self:_setSession(data)
end

function Client:signUp(email, password, displayName)
  if not text(email) or not text(password) or #password < 8 then
    return failure("validation", "Enter an email and a password of at least 8 characters.")
  end
  local data, err = self:_request("POST", "/auth/v1/signup", {
    email = email, password = password, data = { display_name = displayName or "" },
  })
  if not data then return nil, err end
  if type(data) ~= "table" then return failure("invalid_response", "The chat service returned an invalid sign-up response.") end
  if data.access_token then return self:_setSession(data) end
  if type(data) ~= "table" or type(data.id) ~= "string" then
    return failure("invalid_response", "The chat service returned an invalid sign-up response.")
  end
  return { confirmationRequired = true }
end

function Client:restoreSession()
  if self.mode ~= "appwrite" then return failure("unauthenticated", "Sign in to continue.") end
  local data, err = self:_request("POST", "/auth/v1/restore")
  if not data then
    if err.status == 401 or err.status == 403 then self.session = nil end
    return nil, err
  end
  return self:_setSession(data)
end

function Client:refreshSession()
  if not self.session then return failure("unauthenticated", "Sign in to continue.") end
  local previous = self.session
  local data, err = self:_request("POST", "/auth/v1/token?grant_type=refresh_token", {
    refresh_token = self.session.refresh_token,
  })
  if self.session ~= previous then
    return failure("session_changed", "The active account changed while refreshing the session.")
  end
  if not data then
    if err.status == 400 or err.status == 401 or err.status == 403 then self.session = nil end
    return nil, err
  end
  return self:_setSession(data)
end

function Client:signOut()
  if not self.session and self.mode ~= "appwrite" then return true end
  -- Appwrite revocation uses the private cookie; an expired JWT must not stop
  -- local erasure or cause a refresh before logout.
  local result, err = self:_request("POST", "/auth/v1/logout?scope=local", nil, self.mode ~= "appwrite")
  -- The local device must forget its credentials even when the server is offline.
  self.session = nil
  return result, err
end

function Client:listRooms()
  local data, err = self:_request("GET", "/rest/v1/chat_rooms?select=*,role,can_post&order=created_at.desc", nil, true)
  if not data then return nil, err end
  if type(data) ~= "table" then return failure("invalid_response", "The chat service returned invalid conversations.") end
  return data
end

function Client:listContacts()
  if not self.session then return failure("unauthenticated", "Sign in to continue.") end
  local path = "/rest/v1/chat_profiles?select=id,display_name&order=display_name.asc&id=neq." .. escape(self.session.user.id)
  local data, err = self:_request("GET", path, nil, true)
  if not data then return nil, err end
  if type(data) ~= "table" then return failure("invalid_response", "The chat service returned invalid contacts.") end
  return data
end

function Client:createRoom(name, memberIds, options)
  options = options or {}
  if type(options) ~= "table" then return failure("validation", "Provide conversation options.") end
  local kind, visibility = options.kind or "group", options.visibility or "private"
  if (kind ~= "direct" and kind ~= "group" and kind ~= "channel") or (visibility ~= "private" and visibility ~= "public") then
    return failure("validation", "Choose a valid conversation type and visibility.")
  end
  if not text(name) or #name > 80 then return failure("validation", "Use a conversation name of 1–80 characters.") end
  if type(memberIds) ~= "table" or #memberIds > 50 then
    return failure("validation", "Choose at most 50 members.")
  end
  for _, id in ipairs(memberIds) do
    if not validId(id) then return failure("validation", "A member's user ID is invalid.") end
  end
  if kind == "direct" and (#memberIds ~= 1 or visibility ~= "private"
      or (self.session and memberIds[1] == self.session.user.id)) then
    return failure("validation", "Choose exactly one other person for a private direct chat.")
  end
  local data, err = self:_request("POST", "/rest/v1/rpc/create_chat_room", {
    room_name = name, member_ids = memberIds, room_kind = kind, room_visibility = visibility,
  }, true, nil, { arrayFields = { member_ids = true } })
  if not data then return nil, err end
  if type(data) ~= "table" or type(data[1]) ~= "table" then
    return failure("invalid_response", "The chat service did not return the new conversation.")
  end
  return data[1]
end

function Client:listMessages(roomId, options)
  if not validId(roomId) then return failure("validation", "Choose a conversation.") end
  options = options or {}
  local limit = tonumber(options.limit) or 100
  if limit % 1 ~= 0 or limit < 1 or limit > 100 then return failure("validation", "Message limit must be between 1 and 100.") end
  local path = "/rest/v1/chat_messages?select=*,attachments&room_id=eq."
    .. escape(roomId) .. "&order=created_at.desc,id.desc&limit=" .. limit
  local data, err = self:_request("GET", path, nil, true)
  if not data then return nil, err end
  if type(data) ~= "table" then return failure("invalid_response", "The chat service returned invalid messages.") end
  local messages = {}
  for i = #data, 1, -1 do messages[#messages + 1] = data[i] end
  return messages
end

-- Cursor pages are chronological; the cursor points to the oldest returned row.
function Client:listMessagePage(roomId, options)
  if not validId(roomId) then return failure("validation", "Choose a conversation.") end
  options = options or {}
  local limit = options.limit == nil and 30 or tonumber(options.limit)
  if not limit or limit % 1 ~= 0 or limit < 1 or limit > 100 then
    return failure("validation", "Message limit must be between 1 and 100.")
  end
  if options.before ~= nil and not validId(options.before) then return failure("validation", "Choose a valid message cursor.") end
  local page, err
  if not self.messagePageRetryAt or self.clock() >= self.messagePageRetryAt then
    page, err = self:_request("POST", "/rest/v1/rpc/list_chat_messages", {
      target_room = roomId, limit = limit, before = options.before,
    }, true)
    if not page and err.status == 404 then self.messagePageRetryAt = self.clock() + 60
    elseif page then self.messagePageRetryAt = nil end
  end
  -- Keep recent conversations usable while the new Function rolls out. The
  -- old endpoint has no cursor support: never pretend it can serve older pages.
  if not page and self.messagePageRetryAt and self.clock() < self.messagePageRetryAt
      and (not err or err.status == 404) then
    if options.before then return failure("history_unavailable", "Older messages are unavailable. Try again later.", 404) end
    local messages
    messages, err = self:listMessages(roomId, { limit = limit })
    if not messages then return nil, err end
    page = { messages = messages, has_more = false }
  end
  if not page then return nil, err end
  if type(page) ~= "table" or type(page.messages) ~= "table" or type(page.has_more) ~= "boolean"
      or #page.messages > limit or (page.has_more and (#page.messages == 0 or not validId(page.next_cursor))) then
    return failure("invalid_response", "The chat service returned an invalid message page.")
  end
  local ids = {}
  for _, message in ipairs(page.messages) do
    if type(message) ~= "table" or not validId(message.id) or ids[message.id] then
      return failure("invalid_response", "The chat service returned invalid messages.")
    end
    ids[message.id] = true
  end
  if page.has_more and (page.next_cursor ~= page.messages[1].id or page.next_cursor == options.before) then
    return failure("invalid_response", "The chat service returned an invalid message cursor.")
  end
  return { messages = page.messages, has_more = page.has_more, next_cursor = page.has_more and page.next_cursor or nil }
end

function Client:sendMessage(roomId, body, attachmentIds)
  if not validId(roomId) then return failure("validation", "Choose a conversation.") end
  if attachmentIds and (type(attachmentIds) ~= "table" or #attachmentIds > 4) then return failure("validation", "Attach at most four photos.") end
  for _, id in ipairs(attachmentIds or {}) do
    if not validId(id) then return failure("validation", "A photo ID is invalid.") end
  end
  body = body or ""
  if type(body) ~= "string" or #body > 4000 or (not text(body) and not (attachmentIds and #attachmentIds > 0)) then
    return failure("validation", "Write a message or attach a photo (maximum 4000 text bytes).")
  end
  if not self.session then return failure("unauthenticated", "Sign in to continue.") end
  local path, payload = "/rest/v1/rpc/post_chat_message", { target_room = roomId, message_body = body, attachment_ids = attachmentIds or {} }
  local data, err = self:_request("POST", path, payload, true, nil, { arrayFields = { attachment_ids = true } })
  if not data then return nil, err end
  if type(data) ~= "table" or type(data[1]) ~= "table" then
    return failure("invalid_response", "The chat service did not return the sent message.")
  end
  return data[1]
end

-- Authors edit their own text; the server records edited_at and keeps photos.
function Client:editMessage(messageId, body)
  if not validId(messageId) then return failure("validation", "Choose a message.") end
  if type(body) ~= "string" or #body > 4000 then
    return failure("validation", "Write a message of at most 4000 text bytes.")
  end
  if not self.session then return failure("unauthenticated", "Sign in to continue.") end
  local data, err = self:_request("POST", "/rest/v1/rpc/edit_chat_message", { target_message = messageId, message_body = body }, true)
  if not data then return nil, err end
  if type(data) ~= "table" or type(data[1]) ~= "table" then
    return failure("invalid_response", "The chat service did not return the edited message.")
  end
  return data[1]
end

-- Deletes for everyone: authors, or group/channel staff for any message.
function Client:deleteMessage(messageId)
  if not validId(messageId) then return failure("validation", "Choose a message.") end
  if not self.session then return failure("unauthenticated", "Sign in to continue.") end
  local data, err = self:_request("POST", "/rest/v1/rpc/delete_chat_message", { target_message = messageId }, true)
  if data == nil then return nil, err end
  return true
end

require("api.features")(Client, { failure = failure, text = text, escape = escape, validId = validId })

-- Builds a client over any transport that serves the chat API paths.
function Chat.client(options)
  options = options or {}
  local url = options.url
  local path = type(url) == "string" and (url:match("^https://[%w%.%-]+(.*)$")
    or url:match("^http://localhost:%d+(.*)$") or url:match("^http://127%.0%.0%.1:%d+(.*)$"))
  if not path or not (path:match("^/?$") or path:match("^/v1/?$")) then
    return failure("configuration", "Use an HTTPS chat service URL (or a localhost URL).")
  end
  if type(options.transport) ~= "function" then return failure("configuration", "Provide an HTTP transport function.") end
  return setmetatable({ mode = options.mode or "custom", url = url:gsub("/$", ""),
    transport = options.transport, clock = options.clock or os.time, avatarIds = {} }, Client)
end

function Chat.appwrite(options)
  options = options or {}
  local endpoint = options.endpoint
  if type(endpoint) ~= "string" or not (endpoint:match("^https://[%w%.%-]+/v1/?$")
      or endpoint:match("^http://localhost:%d+/v1/?$") or endpoint:match("^http://127%.0%.0%.1:%d+/v1/?$")) then
    return failure("configuration", "Set an Appwrite HTTPS endpoint ending in /v1.")
  end
  if not validId(options.projectId) or not validId(options.functionId) then
    return failure("configuration", "Set the public Appwrite project ID and Chat function ID.")
  end
  if options.transport ~= nil and type(options.transport) ~= "function" then
    return failure("configuration", "Provide an HTTP transport function.")
  end
  return Chat.client { mode = "appwrite", url = endpoint, clock = options.clock,
    transport = require("api.appwrite_transport").new(options) }
end

-- Uses exactly the same client, endpoint paths and response validation as HTTP.
function Chat.mock(options)
  options = options or {}
  local transport = require("api.mock_transport").new(options)
  return assert(Chat.client { mode = "mock", url = "https://mock.invalid", transport = transport, clock = options.clock })
end

-- Compatibility helper for existing previews. Interactive mock mode starts at login.
function Chat.demo(options)
  local client = Chat.mock(options)
  assert(client:signIn("demo@example.invalid", "password123"))
  return client
end

return Chat
