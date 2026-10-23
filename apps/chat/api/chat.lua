-- Application-level chat API. No engine globals are needed by the demo/tests.
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
  return type(value) == "string" and value:match("^[%w%-]+$") ~= nil
end

function Client:getSession()
  return copy(self.session)
end

function Client:_request(method, path, body, authenticated, retried)
  if authenticated then
    if not self.session then return failure("unauthenticated", "Sign in to continue.") end
    if not retried and self.session.expires_at <= self.clock() + 30 then
      local session, err = self:refreshSession()
      if not session then return nil, err end
    end
  end
  local headers = { apikey = self.key, ["Content-Type"] = "application/json" }
  if authenticated then headers.Authorization = "Bearer " .. self.session.access_token end
  if method == "POST" and path:match("^/rest/") then headers.Prefer = "return=representation" end
  local ok, response = pcall(self.transport, {
    method = method, url = self.url .. path, headers = headers, body = body,
  })
  if not ok or type(response) ~= "table" or type(response.status) ~= "number" then
    return failure("network", "Could not reach the chat service. Please try again.")
  end
  if response.status == 401 and authenticated and not retried then
    local session, err = self:refreshSession()
    if not session then return nil, err end
    return self:_request(method, path, body, true, true)
  end
  if response.status < 200 or response.status >= 300 then
    local data = type(response.body) == "table" and response.body or {}
    return failure(data.code or data.error_code or "http_error",
      data.message or data.msg or data.error_description or "The chat service rejected the request.", response.status)
  end
  return response.body or true
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

function Client:refreshSession()
  if not self.session then return failure("unauthenticated", "Sign in to continue.") end
  local data, err = self:_request("POST", "/auth/v1/token?grant_type=refresh_token", {
    refresh_token = self.session.refresh_token,
  })
  if not data then
    if err.status == 400 or err.status == 401 or err.status == 403 then self.session = nil end
    return nil, err
  end
  return self:_setSession(data)
end

function Client:signOut()
  if not self.session then return true end
  local result, err = self:_request("POST", "/auth/v1/logout?scope=local", nil, true)
  -- The local device must forget its credentials even when the server is offline.
  self.session = nil
  return result, err
end

function Client:listRooms()
  local data, err = self:_request("GET", "/rest/v1/chat_rooms?select=id,name,created_at&order=created_at.desc", nil, true)
  if not data then return nil, err end
  if type(data) ~= "table" then return failure("invalid_response", "The chat service returned invalid conversations.") end
  return data
end

function Client:createRoom(name, memberIds)
  if not text(name) or #name > 80 then return failure("validation", "Use a conversation name of 1–80 characters.") end
  if type(memberIds) ~= "table" or #memberIds == 0 then
    return failure("validation", "Add at least one member's user ID.")
  end
  for _, id in ipairs(memberIds) do
    if not validId(id) then return failure("validation", "A member's user ID is invalid.") end
  end
  local data, err = self:_request("POST", "/rest/v1/rpc/create_chat_room", {
    room_name = name, member_ids = memberIds,
  }, true)
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
  local path = "/rest/v1/chat_messages?select=id,room_id,sender_id,body,created_at&room_id=eq."
    .. escape(roomId) .. "&order=created_at.desc,id.desc&limit=" .. limit
  local data, err = self:_request("GET", path, nil, true)
  if not data then return nil, err end
  if type(data) ~= "table" then return failure("invalid_response", "The chat service returned invalid messages.") end
  local messages = {}
  for i = #data, 1, -1 do messages[#messages + 1] = data[i] end
  return messages
end

function Client:sendMessage(roomId, body)
  if not validId(roomId) then return failure("validation", "Choose a conversation.") end
  if not text(body) or #body > 4000 then return failure("validation", "Write a message of 1–4000 bytes.") end
  if not self.session then return failure("unauthenticated", "Sign in to continue.") end
  local data, err = self:_request("POST", "/rest/v1/chat_messages", {
    room_id = roomId, sender_id = self.session.user.id, body = body,
  }, true)
  if not data then return nil, err end
  if type(data) ~= "table" or type(data[1]) ~= "table" then
    return failure("invalid_response", "The chat service did not return the sent message.")
  end
  return data[1]
end

function Chat.supabase(options)
  options = options or {}
  local url = options.url
  if type(url) ~= "string" or not (url:match("^https://[%w%.%-]+/?$")
      or url:match("^http://localhost:%d+/?$") or url:match("^http://127%.0%.0%.1:%d+/?$")) then
    return failure("configuration", "Set a Supabase HTTPS project URL (or localhost URL).")
  end
  if type(options.publishableKey) ~= "string" or not options.publishableKey:match("^sb_publishable_[%w_%-]+$") then
    return failure("configuration", "Use the project's sb_publishable_ key in the client.")
  end
  local transport = options.transport
  if transport == nil then transport = require "api.orca_transport" end
  if type(transport) ~= "function" then return failure("configuration", "Provide an HTTP transport function.") end
  return setmetatable({ mode = "supabase", url = url:gsub("/$", ""), key = options.publishableKey,
    transport = transport, clock = options.clock or os.time }, Client)
end

local Demo = {}
Demo.__index = Demo
function Demo:getSession() return copy(self.session) end
function Demo:listRooms()
  if not self.session then return failure("unauthenticated", "Open demo mode to continue.") end
  return copy(self.rooms)
end
function Demo:listMessages(roomId, options)
  if not self.session then return failure("unauthenticated", "Open demo mode to continue.") end
  if not self.messages[roomId] then return failure("not_found", "Conversation not found.") end
  local result, messages = {}, self.messages[roomId]
  local limit = options and options.limit or 100
  if type(limit) ~= "number" or limit % 1 ~= 0 or limit < 1 or limit > 100 then
    return failure("validation", "Message limit must be between 1 and 100.")
  end
  for i = math.max(1, #messages - limit + 1), #messages do result[#result + 1] = copy(messages[i]) end
  return result
end
function Demo:sendMessage(roomId, body)
  if not self.session then return failure("unauthenticated", "Open demo mode to continue.") end
  if not self.messages[roomId] then return failure("not_found", "Conversation not found.") end
  if not text(body) or #body > 4000 then return failure("validation", "Write a message of 1–4000 bytes.") end
  self.sequence = self.sequence + 1
  local message = { id = "demo-" .. self.sequence, room_id = roomId,
    sender_id = self.session.user.id, body = body, created_at = os.date("!%Y-%m-%dT%H:%M:%SZ") }
  self.messages[roomId][#self.messages[roomId] + 1] = message
  for _, room in ipairs(self.rooms) do
    if room.id == roomId then room.subtitle = body end
  end
  return copy(message)
end
function Demo:createRoom(name, memberIds)
  if not self.session then return failure("unauthenticated", "Open demo mode to continue.") end
  if not text(name) or #name > 80 then return failure("validation", "Use a conversation name of 1–80 characters.") end
  self.sequence = self.sequence + 1
  local room = { id = "room-" .. self.sequence, name = name, subtitle = "Start the conversation",
    initials = name:sub(1, 2):upper(), color = "#6877d7", unread = 0 }
  self.rooms[#self.rooms + 1] = room
  self.messages[room.id] = {}
  return copy(room)
end
function Demo:signOut() self.session = nil; return true end
function Demo:refreshSession() return self:getSession() end
function Demo:signIn() return failure("demo", "Use Supabase mode to sign in to an account.") end
function Demo:signUp() return failure("demo", "Use Supabase mode to create an account.") end

function Chat.demo()
  local rooms = {
    { id = "alex", name = "Alex Morgan", initials = "AM", color = "#d9a56f", subtitle = "See you at the little coffee place?", unread = 2 },
    { id = "weekend", name = "Weekend people", initials = "WP", color = "#a7a3d3", subtitle = "Maya: That sounds like a plan", unread = 0 },
    { id = "jules", name = "Jules Park", initials = "JP", color = "#82aaa0", subtitle = "The photos turned out so good", unread = 0 },
  }
  local function message(id, room, sender, body, time)
    return { id = id, room_id = room, sender_id = sender, body = body, created_at = "2026-10-23T" .. time .. ":00Z" }
  end
  return setmetatable({ mode = "demo", sequence = 10, rooms = rooms,
    session = { user = { id = "me", email = "demo@example.invalid", user_metadata = { display_name = "You" } } },
    messages = {
      alex = {
        message("a1", "alex", "alex", "Hey! How's your morning going?", "09:32"),
        message("a2", "alex", "me", "Pretty good. Just finished a long walk by the river.", "09:33"),
        message("a3", "alex", "alex", "That sounds lovely. I'm ready for a coffee break.", "09:34"),
        message("a4", "alex", "alex", "See you at the little coffee place?", "09:35"),
      },
      weekend = { message("w1", "weekend", "maya", "That sounds like a plan", "08:42") },
      jules = { message("j1", "jules", "jules", "The photos turned out so good", "08:10") },
    },
  }, Demo)
end

return Chat
