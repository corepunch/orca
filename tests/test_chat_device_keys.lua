package.path = "apps/chat/?.lua;" .. package.path
local crypto = require "orca.crypto"
local Chat = require "api.chat"
local DeviceKeys = require "api.device_keys"

local function fakeCredentials()
  local items = {}
  return { items = items,
    get = function(key) return items[key] end,
    set = function(key, value) items[key] = value; return true end,
    delete = function(key) items[key] = nil; return true end }
end
local function count(t) local n = 0; for _ in pairs(t) do n = n + 1 end; return n end

-- base64url matches Node's Buffer 'base64url' (unpadded, canonical)
assert(DeviceKeys.encode("") == "" and DeviceKeys.encode("\0\0\0") == "AAAA")
assert(DeviceKeys.encode("\255") == "_w" and DeviceKeys.encode("\251\255") == "-_8")
assert(DeviceKeys.decode("-_8") == "\251\255" and DeviceKeys.decode("_w", 1) == "\255")
assert(DeviceKeys.decode("_x") == nil, "Non-canonical trailing bits are rejected")
assert(DeviceKeys.decode("AAAA", 2) == nil and DeviceKeys.decode("A") == nil and DeviceKeys.decode("A+") == nil)
for _ = 1, 50 do
  local bytes = crypto.random(math.random(1, 70))
  assert(DeviceKeys.decode(DeviceKeys.encode(bytes), #bytes) == bytes)
end

-- Fixed vector shared with the Node backend test: RFC 8032 test 1 key signs
-- a device for "alice" with device_id 0x01*16 and box_public 0x02*32.
local seed = ("9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60"):gsub("..", function(x) return string.char(tonumber(x, 16)) end)
local public = ("d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a"):gsub("..", function(x) return string.char(tonumber(x, 16)) end)
local vector = crypto.sign(seed .. public, DeviceKeys.signatureMessage("alice", string.rep("\1", 16), string.rep("\2", 32)))
assert(DeviceKeys.encode(vector) == "Q2RrcM-yDLiaEJQtV8gHJzVcA-aw_WPDcCNLUHqQLIhkEIRGXnX39pqdKx4LBquNYdlqsAzGzYQRdksUBt6cAQ",
  "Device signature vector changed: " .. DeviceKeys.encode(vector))

assert(not DeviceKeys.new {}, "Configuration is required")
local credentials = fakeCredentials()
local store = assert(DeviceKeys.new { crypto = crypto, credentials = credentials, scope = "https://mock.invalid:project" })

local client = Chat.demo()
client:setDeviceKeys(store)
local result, err = DeviceKeys.new { crypto = crypto, credentials = fakeCredentials(), scope = "s" }:ensure(Chat.mock())
assert(not result and err.code == "unauthenticated")

-- Create, save before publish, publish once
local device = assert(store:ensure(client))
assert(device.user_id == "me" and #device.device_id == 22 and #device.box_public == 43)
assert(count(credentials.items) == 1)
local stored = next(credentials.items)
assert(stored == "chat-device:https://mock.invalid:project:me")
assert(not credentials.items[stored]:find(device.box_public .. device.box_public, 1, true))
assert(credentials.items[stored]:find('"published":true', 1, true))
assert(store:ensure(client).device_id == device.device_id, "An existing published device is reused")
local secrets = assert(store:secrets("me"))
assert(#secrets.box_secret == 32 and #secrets.sign_secret == 64 and secrets.device_id == device.device_id)
assert(crypto.boxPublicKey(secrets.box_secret) == DeviceKeys.decode(device.box_public, 32))

-- Published rows verify; tampered rows do not
local rows = assert(client:listDeviceKeys { "me" })
assert(#rows == 1 and rows[1].device_id == device.device_id and store:verify(rows[1]))
local forged = {}
for k, v in pairs(rows[1]) do forged[k] = v end
forged.user_id = "alex"
assert(not store:verify(forged), "A signature is bound to its owner")
forged.user_id, forged.box_public = "me", DeviceKeys.encode(crypto.random(32))
assert(not store:verify(forged), "A signature is bound to the encryption key")
assert(not store:verify { user_id = "me" })

-- Client-side validation happens before transport
result, err = client:publishDeviceKey { device_id = "short" }
assert(not result and err.code == "validation")
result, err = client:listDeviceKeys {}
assert(not result and err.code == "validation")
result, err = client:listDeviceKeys { "alex", "alex" }
assert(not result and err.code == "validation")
result, err = client:revokeDevice("bad")
assert(not result and err.code == "validation")

-- Sign-out revokes and erases; a new sign-in creates a new device
assert(client:signOut())
assert(count(credentials.items) == 0, "Sign-out erases this device's secrets")
local mine = assert(client:signIn("demo@example.invalid", "password123") and client:listDeviceKeys { "me" })
assert(#mine == 1 and mine[1].revoked_at, "Sign-out revokes the device but keeps its public record")
local second = assert(store:ensure(client))
assert(second.device_id ~= device.device_id, "Signing in again creates a new device")
assert(#assert(client:listDeviceKeys { "me" }) == 2)

-- A stored device the server rejects as revoked is replaced
local record = assert(store:load("me"))
record.published = false
record.device_id = device.device_id
credentials.items[stored] = require("api.json").encode(record)
local replaced = assert(store:ensure(client))
assert(replaced.device_id ~= device.device_id and replaced.device_id ~= second.device_id)

-- Corrupt storage is reported, never silently replaced
credentials.items[stored] = "{not json"
result, err = store:ensure(client)
assert(not result and err.code == "corrupt_device")
credentials.items[stored] = nil

-- Offline sign-out still erases local secrets
assert(store:ensure(client))
client.transport = function() return nil, { code = "network", message = "offline" } end
client:signOut()
assert(count(credentials.items) == 0 and not client.session, "Local erasure does not depend on the network")
assert(#assert(Chat.demo():listDeviceKeys { "alex" }) == 0, "People without devices have no keys")

print("PASS: Chat device keys: storage, publishing, verification, revocation and replacement")
