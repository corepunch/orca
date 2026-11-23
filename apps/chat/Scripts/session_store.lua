local Store = require "AppwriteAuth/session_store"
return { new = function(config, native) return Store.new(config, "chat-session", native) end }
