local runtime = require "zilscript.runtime"
local Session = {}
Session.__index = Session

local function canonical(name)
    return type(name) == "string" and name:upper():gsub("_", "-") or ""
end

function Session.new()
    local env = runtime.create_game_env()
    env.rawequal = rawequal
    local file = assert(io.open("zilscript/bootstrap.lua", "r"))
    local bootstrap = file:read("*a")
    file:close()
    assert(runtime.execute(bootstrap, "bootstrap", env), "Cannot initialize ZIL")
    env.require("zilscript")
    local objects, define_object = {}, env.OBJECT
    -- Record actual declarations, not numeric globals that happen to equal object IDs.
    env.OBJECT = function(definition)
        local result = define_object(definition)
        local symbol = definition.ZIL_NAME or definition.NAME
        if definition.SYNONYM and definition.SYNONYM[1] then
            local nouns, adjective = {}
            for _, noun in ipairs(definition.SYNONYM) do nouns[noun] = true end
            -- A synonym used as an adjective can be parsed as a second noun.
            for _, word in ipairs(definition.ADJECTIVE or {}) do
                if not nouns[word] then adjective = word; break end
            end
            objects[canonical(symbol)] = {
                id = env[symbol],
                noun = ((adjective and adjective .. " " or "") .. definition.SYNONYM[1]):lower(),
            }
        end
        return result
    end
    assert(runtime.load_modules(env, {"Book.Scripts.WondertownPrototype"}, {silent = true}),
        "Cannot load Wondertown prototype")
    env.OBJECT = define_object
    local game = runtime.create_game(env, true)
    game:start()
    return setmetatable({env = env, game = game, objects = objects}, Session)
end

function Session:description()
    -- Read authored room prose without submitting LOOK or advancing the clock.
    return self.env.GETP(self.env.HERE, self.env.PQLDESC) or ""
end

function Session:target(name)
    local env, object = self.env, self.objects[canonical(name)]
    if not object then return nil end
    local id = object.id
    local current, seen = id, {}
    while current ~= env.HERE do
        if not current or current == 0 or seen[current] or current == env.WINNER
            or env.FSETQ(current, env.INVISIBLE) then return nil end
        seen[current] = true
        local parent = env.LOC(current)
        if parent ~= env.HERE and (not parent or parent == 0
            or (not env.FSETQ(parent, env.SURFACEBIT) and not env.FSETQ(parent, env.OPENBIT))) then
            return nil
        end
        current = parent
    end
    local verb = env.FSETQ(id, env.TAKEBIT) and "take" or "examine"
    return {name = canonical(name), object = id, verb = verb, command = verb .. " " .. object.noun}
end

function Session:activate(name)
    if self.pending then return nil end
    local target = self:target(name)
    if not target then return nil end
    local output = self.game:resume(target.command)
    local action = target.name .. "." .. target.verb
    if target.verb == "take" and not self.env.INQ(target.object, self.env.WINNER) then action = nil end
    self.pending = {output = output or "", action = action}
    return self.pending
end

function Session:continue()
    self.pending = nil
end

return Session
