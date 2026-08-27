local runtime = require "zilscript.runtime"
local scenes = require "Book.Scripts.WondertownScenes"
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
    local rooms_by_id = {}
    for name in pairs(scenes.rooms) do
        local id = env[(name:gsub("-", "_"))]
        if id then rooms_by_id[id] = name end
    end
    return setmetatable({
        env = env, game = game, objects = objects, scenes = scenes,
        rooms_by_id = rooms_by_id, focus = nil, focus_text = nil, pending = nil,
    }, Session)
end

function Session:description()
    -- Read authored room prose without submitting LOOK or advancing the clock.
    return self.env.GETP(self.env.HERE, self.env.PQLDESC) or ""
end

-- Canonical name of the ZIL room Pip physically occupies (HERE), not the focus.
function Session:room()
    return self.rooms_by_id[self.env.HERE]
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

-- A currently reachable subject resolves to how the reader interacts with it:
-- a portable object defaults to TAKE, an authored subject opens a FOCUS, and any
-- other reachable object simply EXAMINEs. Returns nil when out of reach.
function Session:subject_kind(name)
    local target = self:target(name)
    if not target then return nil end
    if self.env.FSETQ(target.object, self.env.TAKEBIT) then return "take", target end
    if self.scenes.focuses[canonical(name)] then return "focus", target end
    return "examine", target
end

-- The screenshot camera framing whatever the reader currently sees.
function Session:current_camera()
    if self.focus then return self.scenes.focuses[self.focus].camera end
    local spec = self.scenes.rooms[self:room()]
    return spec and spec.camera or "WorkshopEstablishing"
end

local function room_subject(spec, canon)
    if not spec then return nil end
    for _, entry in ipairs(spec.subjects) do
        if canonical(entry.name) == canon then return entry end
    end
end

-- A single description of what to render now: a held action beat, a focused
-- subject with its local choices, or the room's establishing page.
function Session:view()
    if self.pending then
        return {kind = "beat", camera = self.pending.camera,
                image = self.scenes.image(self.pending.camera),
                text = self.pending.text, continue = true}
    end
    if self.focus then
        local focus = self.scenes.focuses[self.focus]
        local choices = {}
        for _, choice in ipairs(focus.choices(self.env, self) or {}) do
            choices[#choices + 1] = {label = choice.label, command = choice.command}
        end
        return {kind = "focus", subject = self.focus, camera = focus.camera,
                image = self.scenes.image(focus.camera), text = self.focus_text or "",
                choices = choices, exit = focus.exit or "Step back"}
    end
    local spec = self.scenes.rooms[self:room()]
    local camera = spec and spec.camera or "WorkshopEstablishing"
    local subjects, exits = {}, {}
    if spec then
        for _, entry in ipairs(spec.subjects) do
            local kind, target = self:subject_kind(entry.name)
            if kind then
                local object = self.objects[canonical(entry.name)]
                subjects[#subjects + 1] = {name = canonical(entry.name), node = entry.node or canonical(entry.name),
                    kind = kind, command = target.command, art = entry.art,
                    label = object and object.noun or canonical(entry.name)}
            end
        end
        for _, exit in ipairs(spec.exits or {}) do
            exits[#exits + 1] = {label = exit.label, command = exit.command}
        end
    end
    return {kind = "room", camera = camera, image = self.scenes.image(camera),
            text = self:description(), subjects = subjects, exits = exits}
end

-- Enter a focused subject. Its EXAMINE prose becomes the page's opening text, so
-- directing attention reads as an in-world action rather than opening a panel.
function Session:enter_focus(name, target)
    target = target or self:target(name)
    if not target then return nil end
    self.focus_text = self.game:resume(target.command) or ""
    self.focus = canonical(name)
    return self:view()
end

-- Submit a command and hold it as a story beat. Dedicated `art` owns the frame;
-- otherwise the current establishing/focus frame is held with the result prose.
function Session:run_beat(command, art)
    local before = self.env.HERE
    local output = self.game:resume(command)
    self.pending = {text = output or "", camera = art or self:current_camera(),
                    room_changed = self.env.HERE ~= before}
end

-- Tap a subject on the establishing page.
function Session:tap(name)
    if self.pending then return nil end
    local kind, target = self:subject_kind(name)
    if not kind then return nil end
    if kind == "focus" then return self:enter_focus(canonical(name), target) end
    local entry = room_subject(self.scenes.rooms[self:room()], canonical(name))
    self:run_beat(target.command, entry and entry.art)
    return self:view()
end

-- Choose one of the focused subject's local actions.
function Session:choose(index)
    if self.pending or not self.focus then return nil end
    local focus = self.scenes.focuses[self.focus]
    local choice = (focus.choices(self.env, self) or {})[index]
    if not choice then return nil end
    local before = self.env.HERE
    local output = self.game:resume(choice.command)
    if self.env.HERE ~= before then
        self.pending = {text = output or "", camera = choice.art or self:current_camera(),
                        room_changed = true}
    elseif choice.art then
        self.pending = {text = output or "", camera = choice.art, room_changed = false}
    else
        self.focus_text = output or ""
    end
    return self:view()
end

-- Take a room-level exit. A real move lands on the next establishing page; a
-- refused move is held as a beat so the reason reads before returning.
function Session:exit(command)
    if self.pending or self.focus then return nil end
    local before = self.env.HERE
    local output = self.game:resume(command)
    if self.env.HERE == before then
        self.pending = {text = output or "", camera = self:current_camera(), room_changed = false}
    end
    return self:view()
end

-- Leave the focused subject without spending a turn.
function Session:leave_focus()
    if self.pending then return nil end
    self.focus, self.focus_text = nil, nil
    return self:view()
end

function Session:continue()
    if not self.pending then return nil end
    if self.pending.room_changed then self.focus, self.focus_text = nil, nil end
    self.pending = nil
    return self:view()
end

return Session
