-- Private Account credentials only; passwords and short-lived JWTs are never saved.
local json = require "api.json"
local M = {}

function M.new(config, native)
    if not native then
        local ok, module = pcall(require, "orca.credentials")
        if not ok then return nil, { message = "Rebuild Chat with secure session storage support." } end
        native = module
    end
    if not native.supported() then return nil end
    local endpoint = config.endpoint:gsub("/$", "")
    local key = "chat-session:" .. endpoint .. ":" .. config.projectId
    local cookieName = "a_session_" .. config.projectId
    local function valid(credential)
        if type(credential) ~= "table" then return false end
        local name, value = next(credential)
        if not name or next(credential, name) or type(value) ~= "string" or value == ""
            or value:find("[%z\1-\32\127]") then return false end
        if name == "X-Appwrite-Session" then return true end
        if name ~= "Cookie" then return false end
        local field, secret = value:match("^([^=]+)=([^;]+)$")
        return secret ~= nil and (field == cookieName or field == cookieName .. "_legacy")
    end
    return {
        load = function()
            local encoded, err = native.get(key)
            if not encoded then return nil, err end
            local ok, credential = pcall(json.decode, encoded)
            if ok and valid(credential) then return credential end
            -- An invalid local record cannot authenticate; discard it before sign-in.
            local removed, removeError = native.delete(key)
            if not removed then return nil, removeError end
        end,
        save = function(credential)
            if not valid(credential) then
                return nil, { code = "invalid_session", message = "Could not save the account session." }
            end
            return native.set(key, json.encode(credential))
        end,
        clear = function() return native.delete(key) end,
    }
end

return M
