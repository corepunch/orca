-- Device target metadata is private device state, separate from the login.
local json = require "api.json"
local M = {}

function M.new(config, native)
    native = native or require "orca.credentials"
    if not native.supported() then return nil end
    local key = "chat-push:" .. config.endpoint:gsub("/$", "") .. ":" .. config.projectId
    return {
        load = function()
            local value, err = native.get(key)
            if not value then return nil, err end
            local ok, record = pcall(json.decode, value)
            if ok and type(record) == "table" and type(record.id) == "string"
                and #record.id <= 36 and record.id:match("^[%w][%w%._%-]*$")
                and type(record.userId) == "string" and record.userId ~= "" then return record end
            return nil, { code = "push_storage", message = "Invalid saved notification target." }
        end,
        save = function(record) return native.set(key, json.encode(record)) end,
        clear = function() return native.delete(key) end,
    }
end

return M
