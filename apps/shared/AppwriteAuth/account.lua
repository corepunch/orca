local M = {}
local Account = {}
Account.__index = Account

function M.credentials(response, projectId)
  local name = "a_session_" .. projectId
  local cookie, legacy
  local function inspect(value)
    if type(value) == "table" then for _, line in ipairs(value) do inspect(line) end
    elseif type(value) == "string" then
      for line in value:gmatch("[^\r\n]+") do
        local key, content = line:match("^%s*([^=; ]+)=([^;\r\n]+)")
        if content and not content:find("[%z\1-\32\127]") then
          key = key:lower()
          if key == name:lower() then cookie = name .. "=" .. content
          elseif key == name:lower() .. "_legacy" then legacy = name .. "_legacy=" .. content end
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

local function escape(value)
    return (value:gsub("[^%w%-%._~]", function(c) return ("%%%02X"):format(c:byte()) end))
  end

function M.new(endpoint, projectId, request)
  return setmetatable({ endpoint = endpoint and endpoint:gsub("/$", ""), projectId = projectId,
    scheme = "appwrite-callback-" .. projectId:lower(), request = request }, Account)
end

function Account:password(email, password)
  return self.request("POST", "/account/sessions/email", { email = email, password = password })
end

function Account:requestCode(email)
  return self.request("POST", "/account/tokens/email", { userId = "unique()", email = email })
end

function Account:exchange(userId, secret)
  return self.request("POST", "/account/sessions/token", { userId = userId, secret = secret })
end

function Account:requestMagicLink(email, redirect)
  return self.request("POST", "/account/tokens/magic-url",
    { userId = "unique()", email = email, url = redirect or (self.scheme .. "://magic") })
end

function Account:providerPage(provider)
  if (provider ~= "apple" and provider ~= "google") or not self.endpoint then
    return { status = 400, body = { code = "validation", message = "Choose Apple or Google." } }
  end
  local back = escape(self.scheme .. "://oauth")
  return { status = 200, body = { scheme = self.scheme, url = self.endpoint .. "/account/tokens/oauth2/" .. provider
    .. "?project=" .. escape(self.projectId) .. "&success=" .. back .. "&failure=" .. back } }
end

function Account:providerCredentials(url)
  local function invalid()
    return nil, nil, { code = "invalid_link", message = "This sign-in link is invalid. Request a new one." }
  end
  if type(url) ~= "string" or #url > 4096 or url:find("#", 1, true) then return invalid() end
  local target, query = url:match("^([^?]+)%?(.+)$")
  if target ~= self.scheme .. "://oauth" then return invalid() end
  local values = {}
  for part in query:gmatch("[^&]+") do
    local key, value = part:match("^([%w_]+)=(.*)$")
    if not key or values[key] or value:gsub("%%[%x][%x]", ""):find("%%") then return invalid() end
    values[key] = value:gsub("%+", " "):gsub("%%(%x%x)", function(hex) return string.char(tonumber(hex, 16)) end)
  end
  if values.error then
    return nil, nil, { code = "provider_failed", message = "The provider did not complete sign-in. Please try again." }
  end
  local id, secret = values.userId, values.secret
  if not id or #id > 36 or not id:match("^[%w][%w%._%-]*$") or not secret or secret == "" or #secret > 1024
      or secret:find("[%z\1-\32\127]") then return invalid() end
  return id, secret
end

return M
