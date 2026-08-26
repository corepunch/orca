local runtime = require "zilscript.runtime"
local interactions = require "Book.Scripts.WorkshopInteractions"
local Session = {}
Session.__index = Session

function Session.new()
    local env = runtime.create_game_env()
    env.rawequal = rawequal
    local file = assert(io.open("zilscript/bootstrap.lua", "r"))
    local bootstrap = file:read("*a")
    file:close()
    assert(runtime.execute(bootstrap, "bootstrap", env), "Cannot initialize ZIL")
    env.require("zilscript")
    assert(runtime.load_modules(env, {"Book.Scripts.WondertownPrototype"}, {silent = true}),
        "Cannot load Wondertown prototype")
    local game = runtime.create_game(env, true)
    game:start()
    return setmetatable({env = env, game = game}, Session)
end

function Session:description()
    -- Read authored room prose without submitting LOOK or advancing the clock.
    return self.env.GETP(self.env.HERE, self.env.PQLDESC) or ""
end

function Session:activate(id)
    if self.pending then return nil end
    for _, target in ipairs(interactions.targets) do
        if target.id == id and interactions.available(self.env, target) then
            local output = self.game:resume(target.command)
            local action = target.action
            if target.takeable and not self.env.INQ(self.env[target.takeable], self.env.WINNER) then
                action = nil
            end
            self.pending = {output = output or "", action = action}
            return self.pending
        end
    end
end

function Session:continue()
    self.pending = nil
end

return Session
