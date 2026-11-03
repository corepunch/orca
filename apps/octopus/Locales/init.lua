-- App resources use ORCA's shared locale selection, plural rules and native dictionaries.
local Localization = require "orca.core.localization"
local Locales = {}
function Locales.new(language)
    return Localization.new { locale = language, fallback = "en", catalogs = {
        en = require "Octopus/Locales/en", ru = require "Octopus/Locales/ru",
    } }
end

-- Translate known service errors at the app boundary, retaining codes and server details.
-- Successful DTOs (including user text) are passed through unchanged.
function Locales.client(client, i18n)
    if not client then return nil end
    return setmetatable({}, {
        __index = function(_, key)
            local value = client[key]
            if type(value) ~= "function" or key == "clock" then return value end
            return function(_, ...)
                local result, err, extra = value(client, ...)
                if result == nil and type(err) == "table" and type(err.message) == "string" then
                    local translated = {}
                    for name, item in pairs(err) do translated[name] = item end
                    translated.message = i18n:t(err.message)
                    return result, translated, extra
                end
                return result, err, extra
            end
        end,
        __newindex = function(_, key, value) client[key] = value end,
    })
end
return Locales
