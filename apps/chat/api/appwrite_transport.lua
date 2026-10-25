-- Keep the shared Chat API contract while executing its requests in Appwrite.
local M = {}
local alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

local function base64(bytes)
  local result = {}
  for i = 1, #bytes, 3 do
    local a, b, c = bytes:byte(i, i + 2)
    local n = a * 65536 + (b or 0) * 256 + (c or 0)
    result[#result + 1] = alphabet:sub(math.floor(n / 262144) + 1, math.floor(n / 262144) + 1)
      .. alphabet:sub(math.floor(n / 4096) % 64 + 1, math.floor(n / 4096) % 64 + 1)
      .. (b and alphabet:sub(math.floor(n / 64) % 64 + 1, math.floor(n / 64) % 64 + 1) or "=")
      .. (c and alphabet:sub(n % 64 + 1, n % 64 + 1) or "=")
  end
  return table.concat(result)
end

local function unbase64(value)
  if type(value) ~= "string" or #value % 4 ~= 0 or value:find("[^%w+/=]") then return nil end
  local result = {}
  for i = 1, #value, 4 do
    local chunk = value:sub(i, i + 3)
    local a, b, c, d = chunk:sub(1, 1), chunk:sub(2, 2), chunk:sub(3, 3), chunk:sub(4, 4)
    if a == "=" or b == "=" or (c == "=" and d ~= "=")
        or ((c == "=" or d == "=") and i + 3 ~= #value) then return nil end
    local function digit(ch) return ch == "=" and 0 or alphabet:find(ch, 1, true) - 1 end
    local n = digit(a) * 262144 + digit(b) * 4096 + digit(c) * 64 + digit(d)
    result[#result + 1] = string.char(math.floor(n / 65536))
      .. (c ~= "=" and string.char(math.floor(n / 256) % 256) or "")
      .. (d ~= "=" and string.char(n % 256) or "")
  end
  local bytes = table.concat(result)
  if base64(bytes) ~= value then return nil end
  return bytes
end

local function invalid(message)
  return { status = 502, body = { code = "invalid_response", message = message } }
end

function M.new(options)
  local encode, decode = options.encode, options.decode
  if not encode or not decode then
    local json = require "api.json"
    encode, decode = encode or json.encode, decode or json.decode
  end
  local transport = options.transport or require "api.orca_transport"
  local endpoint = options.endpoint:gsub("/$", "")
  local authenticate = require("api.appwrite_auth").new {
    endpoint = endpoint, projectId = options.projectId, transport = transport, encode = encode,
  }
  return function(request)
    -- The adapter may only forward this client's relative API paths.
    if type(request.url) ~= "string" or request.url:sub(1, #endpoint + 1) ~= endpoint .. "/" then
      error("Chat request does not belong to this Appwrite project", 0)
    end
    local path = request.url:sub(#endpoint + 1)
    local pathname = path:match("^[^?]*"):gsub("%%(%x%x)", function(hex)
      return string.char(tonumber(hex, 16))
    end)
    local traversal = false
    for segment in pathname:gmatch("[^/]+") do
      if segment == "." or segment == ".." then traversal = true; break end
    end
    if path:find("[\r\n\\]") or pathname:find("[\r\n\\]") or traversal
        or not (path:match("^/auth/v1/") or path:match("^/rest/v1/")
          or path:match("^/storage/v1/") or path:match("^/functions/v1/")) then
      error("Unsupported Chat API path", 0)
    end
    local accountResponse = authenticate(request, path)
    if accountResponse then return accountResponse end
    local headers = request.headers or {}
    local jwt = headers.Authorization and headers.Authorization:match("^Bearer (.+)$")
    local envelope = { method = request.method, path = path }
    local encoded
    if type(request.body) == "string" then
      envelope.body, envelope.encoding = base64(request.body), "base64"
      encoded = encode(envelope)
    elseif request.body ~= nil then
      -- The native encoder uses {} for empty tables. Preserve explicit RPC
      -- array arguments without changing string escaping or Unicode handling.
      local fields = {}
      for key, value in pairs(request.body) do
        local field = encode({ [key] = value })
        fields[#fields + 1] = request.arrayFields and request.arrayFields[key]
          and type(value) == "table" and next(value) == nil
          and (field:sub(2, -4) .. "[]") or field:sub(2, -2)
      end
      encoded = encode(envelope)
      encoded = encoded:sub(1, -2) .. ',"body":{' .. table.concat(fields, ",") .. "}}"
    else encoded = encode(envelope) end
    local response = transport {
      method = "POST", url = endpoint .. "/functions/" .. options.functionId .. "/executions",
      headers = { ["X-Appwrite-Project"] = options.projectId, ["Content-Type"] = "application/json",
        ["X-Appwrite-JWT"] = jwt },
      body = encode({ body = encoded, async = false }),
    }
    if type(response) ~= "table" or type(response.status) ~= "number" then
      return invalid("Appwrite returned an invalid execution response.")
    end
    if response.status < 200 or response.status >= 300 then
      if type(response.body) == "table" then response.body.code = response.body.type or response.body.code end
      return response
    end
    local execution = response.body
    if type(execution) ~= "table" or execution.status ~= "completed"
        or type(execution.responseStatusCode) ~= "number" or type(execution.responseBody) ~= "string" then
      return invalid("The Appwrite chat function did not complete successfully.")
    end
    local ok, result = pcall(decode, execution.responseBody)
    if not ok or type(result) ~= "table" or type(result.status) ~= "number"
        or result.status % 1 ~= 0 or result.status < 100 or result.status > 599 then
      return invalid("The Appwrite chat function returned invalid JSON.")
    end
    if execution.responseStatusCode < 200 or execution.responseStatusCode >= 300 then
      if result.status < 400 then return invalid("The Appwrite chat function failed.") end
    end
    if result.encoding ~= nil then
      if result.encoding ~= "base64" or request.responseType ~= "binary" then
        return invalid("The Appwrite chat function returned an unsupported encoding.")
      end
      result.body = unbase64(result.body)
      if not result.body then return invalid("The Appwrite chat function returned invalid photo data.") end
    elseif request.responseType == "binary" and result.status >= 200 and result.status < 300 then
      return invalid("The Appwrite chat function returned invalid photo data.")
    end
    return result
  end
end

return M
