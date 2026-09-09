local runtime = require "zilscript.runtime"
local scenes = require "Book.Scripts.WondertownScenes"
local Session = {}
Session.__index = Session

local function canonical(name)
    return type(name) == "string" and name:upper():gsub("_", "-") or ""
end

-- ----------------------------------------------------------------- CoC images

-- All renders for this scene live in one directory (per .blks file, not per room).
local source_path = require("Book.Scripts.WorkshopCamera").source_path
local directory, scene_name = source_path:match("^(.*)/([^/]+)%.blks$")
local render_dir = assert(directory, "invalid workshop source path") .. "/render/" .. scene_name .. "/"

local function image_exists(path)
    local f = io.open(path, "r")
    if f then f:close() return true end
    return false
end

-- camera: the CoC-derived intent name (always returned, even if the image does
--         not exist yet — used for scene-node lookup and test assertions).
-- image:  best available file path, falling back toward the room establishing shot.
--
-- Convention (ILLUSTRATING_ADVENTURES.md):
--   {subject}-{verb}.jpg  →  {subject}.jpg  →  {room}.jpg
local function coc(room, subject, verb)
    local cam = subject and verb and (subject:lower() .. "-" .. verb)
             or (subject and subject:lower())
             or room:lower()
    local function try(name)
        local p = render_dir .. name .. ".jpg"
        return image_exists(p) and p or nil
    end
    local img = (subject and verb and try(subject:lower() .. "-" .. verb))
             or (subject and try(subject:lower()))
             or try(room:lower())
             or (render_dir .. room:lower() .. ".jpg")
    return cam, img
end

-- ------------------------------------------------------------------ bootstrap

function Session.new()
    local env = runtime.create_game_env()
    env.rawequal = rawequal
    local file = assert(io.open("zilscript/bootstrap.lua", "r"))
    local bootstrap = file:read("*a")
    file:close()
    assert(runtime.execute(bootstrap, "bootstrap", env), "Cannot initialize ZIL")
    env.require("zilscript")
    local objects, define_object = {}, env.OBJECT
    env.OBJECT = function(definition)
        local result = define_object(definition)
        local symbol = definition.ZIL_NAME or definition.NAME
        if definition.SYNONYM and definition.SYNONYM[1] then
            local nouns, adjective = {}
            for _, noun in ipairs(definition.SYNONYM) do nouns[noun] = true end
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

-- ------------------------------------------------------------------ queries

function Session:description()
    return self.env.GETP(self.env.HERE, self.env.PQLDESC) or ""
end

function Session:room()
    return self.rooms_by_id[self.env.HERE]
end

function Session:current_camera()
    return self.focus and self.focus:lower() or self:room():lower()
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

function Session:subject_kind(name)
    local target = self:target(name)
    if not target then return nil end
    if self.env.FSETQ(target.object, self.env.TAKEBIT) then return "take", target end
    if self.scenes.focuses[canonical(name)] then return "focus", target end
    return "examine", target
end

-- ------------------------------------------------------------------ view

function Session:view()
    local room = self:room()
    if self.pending then
        return {kind = "beat", camera = self.pending.camera,
                image = self.pending.image,
                text = self.pending.text, continue = true}
    end
    if self.focus then
        local focus = self.scenes.focuses[self.focus]
        local cam, img = coc(room, self.focus)
        local choices = {}
        for _, choice in ipairs(focus.choices(self.env, self) or {}) do
            choices[#choices + 1] = {label = choice.label, command = choice.command}
        end
        return {kind = "focus", subject = self.focus, camera = cam, image = img,
                text = self.focus_text or "",
                choices = choices, exit = focus.exit or "Step back"}
    end
    local spec = self.scenes.rooms[room]
    local cam, img = coc(room)
    local subjects, exits = {}, {}
    if spec then
        for _, name in ipairs(spec.subjects) do
            local kind, target = self:subject_kind(name)
            if kind then
                local object = self.objects[canonical(name)]
                subjects[#subjects + 1] = {
                    name = canonical(name),
                    node = canonical(name),   -- ZIL name = .blks group name (CoC)
                    kind = kind,
                    command = target.command,
                    label = object and object.noun or canonical(name):lower(),
                }
            end
        end
        for _, exit in ipairs(spec.exits or {}) do
            exits[#exits + 1] = {label = exit.label, command = exit.command}
        end
    end
    return {kind = "room", camera = cam, image = img,
            text = self:description(), subjects = subjects, exits = exits}
end

-- ------------------------------------------------------------------ actions

function Session:enter_focus(name, target)
    target = target or self:target(name)
    if not target then return nil end
    self.focus_text = self.game:resume(target.command) or ""
    self.focus = canonical(name)
    return self:view()
end

-- Tap a subject on the room establishing page.
function Session:tap(name)
    if self.pending then return nil end
    local kind, target = self:subject_kind(name)
    if not kind then return nil end
    if kind == "focus" then return self:enter_focus(canonical(name), target) end
    local room = self:room()
    local before = self.env.HERE
    local output = self.game:resume(target.command)
    -- CoC: use subject image for the beat (falls back to room image if not rendered yet)
    local cam, img = coc(room, canonical(name))
    self.pending = {text = output or "", camera = cam, image = img,
                    room_changed = self.env.HERE ~= before}
    return self:view()
end

-- Choose a focus-page action by 1-based index.
function Session:choose(index)
    if self.pending or not self.focus then return nil end
    local focus = self.scenes.focuses[self.focus]
    local choice = (focus.choices(self.env, self) or {})[index]
    if not choice then return nil end
    local room = self:room()
    local before = self.env.HERE
    local output = self.game:resume(choice.command)
    if choice.beat then
        -- Explicit beat: action camera (focus + first verb), room_changed tracks transition
        local verb = choice.command:match("^(%S+)") or ""
        local cam, img = coc(room, self.focus, verb)
        self.pending = {text = output or "", camera = cam, image = img,
                        room_changed = self.env.HERE ~= before}
    elseif self.env.HERE ~= before then
        -- Implicit room transition (no beat flag): show new room image
        local cam, img = coc(self:room())
        self.pending = {text = output or "", camera = cam, image = img, room_changed = true}
    else
        self.focus_text = output or ""
    end
    return self:view()
end

-- Take a room-level exit.
function Session:exit(command)
    if self.pending or self.focus then return nil end
    local room = self:room()
    local before = self.env.HERE
    local output = self.game:resume(command)
    if self.env.HERE == before then
        -- Refused: hold prose as beat on current room image
        local cam, img = coc(room)
        self.pending = {text = output or "", camera = cam, image = img, room_changed = false}
    end
    return self:view()
end

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
