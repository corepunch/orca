-- This device's end-to-end encryption keys. Secret keys stay in secure storage
-- (orca.credentials); only self-signed public keys are published. The crypto and
-- credentials modules are injected so the logic is testable without a Keychain.
local json = require "api.json"

local DeviceKeys = {}
DeviceKeys.__index = DeviceKeys

local LABEL = "orca-chat/device/v1"
local alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
local decodeMap = {}
for i = 1, #alphabet do decodeMap[alphabet:byte(i)] = i - 1 end

-- Unpadded base64url, matching Node's Buffer 'base64url'.
function DeviceKeys.encode(bytes)
  local out = {}
  for i = 1, #bytes, 3 do
    local a, b, c = bytes:byte(i, i + 2)
    local n = a * 65536 + (b or 0) * 256 + (c or 0)
    local chars = 2 + (b and 1 or 0) + (c and 1 or 0)
    for k = 1, chars do
      local index = math.floor(n / 64 ^ (4 - k)) % 64
      out[#out + 1] = alphabet:sub(index + 1, index + 1)
    end
  end
  return table.concat(out)
end

function DeviceKeys.decode(text, size)
  if type(text) ~= "string" or #text % 4 == 1 then return nil end
  local out, bits, value = {}, 0, 0
  for i = 1, #text do
    local digit = decodeMap[text:byte(i)]
    if not digit then return nil end
    value, bits = (value % 2 ^ 18) * 64 + digit, bits + 6
    if bits >= 8 then
      bits = bits - 8
      out[#out + 1] = string.char(math.floor(value / 2 ^ bits) % 256)
    end
  end
  local bytes = table.concat(out)
  if size and #bytes ~= size or DeviceKeys.encode(bytes) ~= text then return nil end
  return bytes
end

-- Signed bytes: label NUL user_id NUL device_id(16) box_public(32).
function DeviceKeys.signatureMessage(userId, deviceId, boxPublic)
  return LABEL .. "\0" .. userId .. "\0" .. deviceId .. boxPublic
end

local function failure(code, message) return nil, { code = code, message = message } end

function DeviceKeys.new(options)
  options = options or {}
  local crypto, credentials = options.crypto, options.credentials
  if type(crypto) ~= "table" or type(credentials) ~= "table" or type(options.scope) ~= "string" or options.scope == "" then
    return failure("configuration", "Device keys need crypto, secure storage and a backend scope.")
  end
  return setmetatable({ crypto = crypto, credentials = credentials, scope = options.scope }, DeviceKeys)
end

function DeviceKeys:_key(userId) return "chat-device:" .. self.scope .. ":" .. userId end

-- Accepts a published row only when its self-signature matches its owner.
function DeviceKeys:verify(row)
  if type(row) ~= "table" or type(row.user_id) ~= "string" then return false end
  local deviceId, boxPublic = DeviceKeys.decode(row.device_id, 16), DeviceKeys.decode(row.box_public, 32)
  local signPublic, signature = DeviceKeys.decode(row.sign_public, 32), DeviceKeys.decode(row.self_signature, 64)
  if not deviceId or not boxPublic or not signPublic or not signature then return false end
  return self.crypto.verify(signPublic, DeviceKeys.signatureMessage(row.user_id, deviceId, boxPublic), signature) == true
end

function DeviceKeys:load(userId)
  local stored, err = self.credentials.get(self:_key(userId))
  if stored == nil then return nil, err end
  local ok, record = pcall(json.decode, stored)
  if not ok or type(record) ~= "table" or record.user_id ~= userId
      or not DeviceKeys.decode(record.box_secret, 32) or not DeviceKeys.decode(record.sign_secret, 64) then
    return failure("corrupt_device", "This device's encryption keys are unreadable.")
  end
  return record
end

function DeviceKeys:_save(record)
  local ok, err = self.credentials.set(self:_key(record.user_id), json.encode(record))
  if not ok then return nil, err end
  return record
end

function DeviceKeys:_create(userId)
  local deviceId, err = self.crypto.random(16)
  if not deviceId then return nil, err end
  local boxSecret, boxPublic = self.crypto.boxKeypair()
  if not boxSecret then return nil, boxPublic end
  local signSecret, signPublic = self.crypto.signKeypair()
  if not signSecret then return nil, signPublic end
  local signature = self.crypto.sign(signSecret, DeviceKeys.signatureMessage(userId, deviceId, boxPublic))
  local encode = DeviceKeys.encode
  -- Saved before publishing, so a published public key always has its secret.
  return self:_save { user_id = userId, device_id = encode(deviceId), box_secret = encode(boxSecret),
    box_public = encode(boxPublic), sign_secret = encode(signSecret), sign_public = encode(signPublic),
    self_signature = encode(signature), published = false }
end

function DeviceKeys.publicView(record)
  return { user_id = record.user_id, device_id = record.device_id, box_public = record.box_public,
    sign_public = record.sign_public, self_signature = record.self_signature }
end

-- Loads or creates this device for the signed-in account and publishes it.
-- A device the server reports as revoked or conflicting is replaced.
function DeviceKeys:ensure(client)
  local session = client and client.session
  if not session then return failure("unauthenticated", "Sign in to continue.") end
  local userId = session.user.id
  local record, err = self:load(userId)
  if not record then
    if err then return nil, err end
    record, err = self:_create(userId)
    if not record then return nil, err end
  end
  if record.published then return DeviceKeys.publicView(record) end
  for _ = 1, 2 do
    local row
    row, err = client:publishDeviceKey(record)
    if row then
      record.published = true
      local saved
      saved, err = self:_save(record)
      if not saved then return nil, err end
      return DeviceKeys.publicView(record)
    end
    if err.code ~= "device_revoked" and err.code ~= "device_conflict" then return nil, err end
    record, err = self:_create(userId)
    if not record then return nil, err end
  end
  return nil, err
end

-- Raw secret keys for this device, or nil when it has none.
function DeviceKeys:secrets(userId)
  local record, err = self:load(userId)
  if not record then return nil, err end
  return { device_id = record.device_id, box_secret = DeviceKeys.decode(record.box_secret, 32),
    sign_secret = DeviceKeys.decode(record.sign_secret, 64) }
end

-- Erases local secrets only, for example after account deletion.
function DeviceKeys:erase(userId)
  return self.credentials.delete(self:_key(userId))
end

-- Sign-out: revoke on the server when reachable, then always erase local secrets.
function DeviceKeys:forget(client)
  local session = client and client.session
  if not session then return true end
  local record = self:load(session.user.id)
  local revoked, revokeError = true, nil
  if record and record.published then revoked, revokeError = client:revokeDevice(record.device_id) end
  local deleted, err = self.credentials.delete(self:_key(session.user.id))
  if not deleted then return nil, err end
  if revoked == nil then return true, revokeError end
  return true
end

return DeviceKeys
