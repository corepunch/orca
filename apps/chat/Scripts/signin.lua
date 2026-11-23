local Browser = require "AppwriteAuth/browser"
local I18n = require "Chat/Scripts/i18n"
return {
    new = function(app, options)
        return Browser.new(app, setmetatable({
            name = "ChatSignIn", translate = function(key) return I18n:t(key) end,
        }, { __index = options }))
    end,
}
