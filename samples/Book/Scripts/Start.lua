local ui = require "orca.UIKit"
local scenes = require "Book.Scripts.WondertownScenes"

local Start = {}
local game_initialized = false
local env, game, session_history, runtime
local refresh_ui

local function init_runtime()
    local bootstrap_file = io.open("zilscript/bootstrap.lua", "r")
    if not bootstrap_file then return false end
    local bootstrap_code = bootstrap_file:read("*a")
    bootstrap_file:close()
    return runtime.execute(bootstrap_code, "bootstrap", env)
end

local function init_game()
    if game_initialized then return end

    local ok, mod = pcall(require, "zilscript.runtime")
    if not ok then
        print("ERROR: Failed to load zilscript.runtime: " .. tostring(mod))
        return
    end
    runtime = mod

    package.path = "?.lua;?/init.lua;" .. package.path
    package.zilpath = "?.zil;" .. (package.zilpath or "")

    env = runtime.create_game_env()
    env.rawget = rawget
    env.rawset = rawset
    env.rawequal = rawequal

    local init_ok = init_runtime()
    if not init_ok then
        print("ERROR: Failed to init zilscript bootstrap")
        return
    end

    env.require("zilscript")

    local load_ok = runtime.load_modules(env, { "books.wondertown.wondertown" })
    if not load_ok then
        print("ERROR: Failed to load wondertown module")
        return
    end

    local companion_ok, companion_err = pcall(env.require, "books.wondertown.companion")
    if companion_ok and companion_err ~= nil then
        if type(env.CAPTURE_RESTART_STATE) == "function" then
            env.CAPTURE_RESTART_STATE()
        end
    elseif companion_ok then
        print("Warning: companion module returned nil")
    else
        print("Warning: no companion module: " .. tostring(companion_err))
    end

    env._G = env

    game = runtime.create_game(env)
    session_history = { { cmd = nil, output = game:start() } }
    game_initialized = true
end

local function entries()
    return session_history or {}
end

local function submit(text)
    if not text or text == "" or not game then return nil end
    text = text:gsub("[\r\n]+$", "")
    local entry = { cmd = text, output = game:resume(text) }
    table.insert(entries(), entry)
    return entry
end

local function query_choices()
    if not env or type(env.COMPANION_QUERY) ~= "function" then return nil end
    return env.COMPANION_QUERY()
end

local function select_choice(id)
    if not env or type(env.COMPANION_SELECT) ~= "function" then return nil end
    return env.COMPANION_SELECT(id)
end

local function get_last_output()
    local history = entries()
    if #history == 0 then return "" end
    return history[#history].output or ""
end

local function clear_options(view)
    while view:getFirstChild() do
        view:getFirstChild():removeFromParent()
    end
end

local function show_action(self, choice, entry)
    local action = scenes.actions[choice.id]
    if not action or not entry then
        refresh_ui(self)
        return
    end

    local view = self.view
    local background = view:findChild("Background", true)
    if background then background.Source = action.asset end

    local scene_label = view:findChild("SceneLabel", true)
    if scene_label then scene_label.Text = choice.label end

    local scene_block = view:findChild("SceneDescription", true)
    if scene_block then scene_block.Text = entry.output or "" end

    local options_view = view:findChild("Options", true)
    if options_view then
        clear_options(options_view)
        local continue = ui.TextBlock {
            class = "scene-option continue-option",
            Text = "Continue",
        }
        continue.LeftButtonUp = function() refresh_ui(self, true) end
        options_view:addChild(continue)
    end
end

refresh_ui = function(self, use_scene_copy)
    local view = self.view
    if not view then return end

    init_game()
    if not game_initialized then return end

    local result = query_choices()
    local scene = result and result.scene
    local scene_entry = scene and scenes[scene.key] or scenes.default

    local background = view:findChild("Background", true)
    if background and scene_entry then background.Source = scene_entry.asset end

    local scene_label = view:findChild("SceneLabel", true)
    if scene_label then
        scene_label.Text = (scene_entry and scene_entry.alt)
            or (scene and scene.alt)
            or ""
    end

    local scene_block = view:findChild("SceneDescription", true)
    if scene_block then
        scene_block.Text = (use_scene_copy and scene_entry and scene_entry.alt)
            or get_last_output()
    end

    local options_view = view:findChild("Options", true)
    if options_view then
        clear_options(options_view)

        if result and result.ok and #result.choices > 0 then
            for _, choice in ipairs(result.choices) do
                local tb = ui.TextBlock {
                    class = "scene-option",
                    Text = choice.label,
                }
                tb.LeftButtonUp = function()
                    local selected = select_choice(choice.id)
                    if selected and selected.ok then
                        local entry = submit(selected.command)
                        show_action(self, choice, entry)
                    end
                end
                options_view:addChild(tb)
            end
        end
    end
end

local function bind_view(self, view)
    refresh_ui(self)
end

return setmetatable(Start, {
    __newindex = function(self, key, value)
        rawset(self, key, value)
        if key == "view" and value then bind_view(self, value) end
    end,
})
