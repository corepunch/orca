-- Upload preflight only: no renderer, network or backend emulator.
package.path = "apps/chat/?.lua;" .. package.path
local Client = {}
local writes = 0
require("api.features")(Client, {
    validId = function(value) return type(value) == "string" and value ~= "" end,
    escape = function(value) return value end,
    failure = function(code, message) return nil, { code = code, message = message } end,
})
function Client:_rpc()
    writes = writes + 1
    return { id = "photo", object_key = "photo.jpg", thumbnail_key = "thumbnail.jpg" }
end
function Client:_request() writes = writes + 1; return { id = "photo", status = "ready" } end
local client = setmetatable({}, { __index = Client })
-- Structural JPEGs exercise preflight dimensions and byte limits, not decoding.
local function jpeg(width, height, size)
    local bytes = "\255\216\255\192" .. string.pack(">I2BI2I2B", 11, 8, height, width, 1)
        .. "\1\17\0\255\218\0\8\1\1\0\0\63\0\1\255\217"
    return bytes:sub(1, -3) .. string.rep("x", (size or 40) - #bytes) .. bytes:sub(-2)
end
local thumb = jpeg(256, 192)
for _, dimensions in ipairs { { 1400, 1050 }, { 1600, 900 }, { 1200, 1250 }, { 32, 24 } } do
    assert(client:uploadPhoto("room", { jpeg = jpeg(dimensions[1], dimensions[2], 250000), thumbnail = thumb }))
end
for _, photo in ipairs {
    { jpeg = jpeg(1400, 1050, 250001), thumbnail = thumb },
    { jpeg = jpeg(1250, 1250), thumbnail = thumb },
    { jpeg = jpeg(1601, 800), thumbnail = thumb },
    { jpeg = jpeg(1400, 1050), thumbnail = jpeg(257, 192) },
    { jpeg = jpeg(1400, 1050), thumbnail = jpeg(256, 192, 250001) },
} do
    local before = writes
    local result, err = client:uploadPhoto("room", photo)
    assert(not result and err.code == "validation")
    assert(writes == before, "Oversized photos must fail before reserving or uploading")
end
print("PASS: Chat JPEG upload limits: 250000 bytes, 1.5MP, 1600px, no enlargement requirement")
