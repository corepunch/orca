local network = require "orca.network"
local json = require "orca.parsers.json"

-- network.fetch yields until complete; invoke remote API methods from an ORCA
-- event coroutine. HTTP errors are thrown response userdata in this binding.
return function(request)
  local _, response = pcall(network.fetch, request.url, {
    method = request.method,
    headers = request.headers,
    body = request.body and json.encode(request.body) or nil,
    nocookies = true,
  })
  if type(response) ~= "userdata" and type(response) ~= "table" then
    error("Chat network request failed", 0)
  end
  local statusOk, status = pcall(function() return response:code() end)
  if not statusOk or type(status) ~= "number" or status == 0 then
    error("Chat network request failed", 0)
  end
  if status == 204 then return { status = status } end
  local decoded, body = pcall(function() return response:json() end)
  if not decoded or type(body) ~= "table" then
    -- Keep the status for refresh/retry even if an intermediary returns HTML.
    if status >= 200 and status < 300 then error("Chat service returned invalid JSON", 0) end
    body = { code = "http_error", message = "The chat service returned an HTTP error." }
  end
  return { status = status, body = body }
end
