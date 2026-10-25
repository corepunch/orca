local network = require "orca.network"
local json = require "api.json"

local function encode(request)
  if type(request.body) == "string" then return request.body end
  if not request.body then return nil end
  if not request.arrayFields then return json.encode(request.body) end
  -- ORCA's JSON encoder treats an empty Lua table as an object. RPC array
  -- arguments need an explicit [] even when there are no members or photos.
  local fields = {}
  for key, value in pairs(request.body) do
    -- This encoder accepts table roots; preserve its escaping via a one-field
    -- object instead of attempting to encode a scalar key or scalar value.
    local field = json.encode({ [key] = value })
    fields[#fields + 1] = request.arrayFields[key] and type(value) == "table" and next(value) == nil
      and (field:sub(2, -4) .. "[]") or field:sub(2, -2)
  end
  return "{" .. table.concat(fields, ",") .. "}"
end

-- network.fetch yields until complete; invoke remote API methods from an ORCA
-- event coroutine. HTTP errors are thrown response userdata in this binding.
return function(request)
  local _, response = pcall(network.fetch, request.url, {
    method = request.method,
    headers = request.headers,
    body = encode(request),
    nocookies = true,
  })
  if type(response) ~= "userdata" and type(response) ~= "table" then
    error("Chat network request failed", 0)
  end
  local statusOk, status = pcall(function() return response:code() end)
  if not statusOk or type(status) ~= "number" or status == 0 then
    error("Chat network request failed", 0)
  end
  local headersOk, headers = pcall(function() return response:headers() end)
  headers = headersOk and type(headers) == "table" and headers or {}
  if status == 204 then return { status = status, headers = headers } end
  if request.responseType == "binary" and status >= 200 and status < 300 then
    return { status = status, body = response:text(), headers = headers }
  end
  local decoded, body = pcall(function() return json.decode(response:text()) end)
  if not decoded or type(body) ~= "table" then
    -- Keep the status for refresh/retry even if an intermediary returns HTML.
    if status >= 200 and status < 300 then error("Chat service returned invalid JSON", 0) end
    body = { code = "http_error", message = "The chat service returned an HTTP error." }
  end
  return { status = status, body = body, headers = headers }
end
