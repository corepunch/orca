-- HTTP transport for the Octopus client: Appwrite REST over orca.network.
local json = require "Octopus/api/json"
local M = {}

local function urlEncode(value)
    return (tostring(value):gsub("([^%w%-_%.~])", function(c) return string.format("%%%02X", c:byte()) end))
end

-- network.fetch yields until complete, so call it from an ORCA event or
-- orca.async coroutine. HTTP errors are thrown response userdata in this binding.
local function nativeFetch(url, options)
    local network = require "orca.network"
    local _, response = pcall(network.fetch, url, options)
    if type(response) ~= "userdata" and type(response) ~= "table" then error("Octopus network request failed", 0) end
    local statusOk, status = pcall(function() return response:code() end)
    if not statusOk or type(status) ~= "number" or status == 0 then error("Octopus network request failed", 0) end
    local headersOk, headers = pcall(function() return response:headers() end)
    local textOk, body = pcall(function() return response:text() end)
    return { status = status, headers = headersOk and type(headers) == "table" and headers or {},
        body = textOk and body or "" }
end

function M.new(options)
    local endpoint = options.endpoint:gsub("/$", "")
    local fetch = options.fetch or nativeFetch
    return function(request)
        if type(request.path) ~= "string" or request.path:sub(1, 1) ~= "/" or request.path:find("[\r\n\\]")
                or request.path:find("/%.%.?/") or request.path:find("/%.%.?$") then
            error("Unsupported Octopus API path", 0)
        end
        local url, params = endpoint .. request.path, {}
        for i, query in ipairs(request.query or {}) do
            params[#params + 1] = "queries[" .. (i - 1) .. "]=" .. urlEncode(json.encode(query))
        end
        if #params > 0 then url = url .. (url:find("?", 1, true) and "&" or "?") .. table.concat(params, "&") end
        local headers = { ["X-Appwrite-Project"] = options.projectId, ["Content-Type"] = "application/json" }
        for name, value in pairs(request.headers or {}) do headers[name] = value end
        local body = request.body
        if type(body) == "table" then body = json.encode(body) end
        local response = fetch(url, { method = request.method, headers = headers, body = body, nocookies = true })
        local status = response.status
        if status == 204 then return { status = status, headers = response.headers } end
        if request.binary and status >= 200 and status < 300 then
            return { status = status, headers = response.headers, body = response.body }
        end
        local decoded, data = pcall(json.decode, response.body or "")
        if not decoded or type(data) ~= "table" then
            -- Keep the status for error handling even if an intermediary returns HTML.
            if status >= 200 and status < 300 then error("Octopus returned invalid JSON", 0) end
            data = { type = "http_error", message = "Octopus returned an HTTP error (" .. status .. ")." }
        end
        return { status = status, headers = response.headers, body = data }
    end
end

return M
