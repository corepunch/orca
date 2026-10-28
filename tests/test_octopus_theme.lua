-- Appearance changes must keep cached Lua colors and newly built chrome in sync.
-- Standalone Lua: no renderer, network, or application reload required.
local dark, override = false, nil
local system = { is_dark_theme = function() return dark end }
local getenv = os.getenv
os.getenv = function(key) if key == "OCTOPUS_APPEARANCE" then return override end; return getenv(key) end
package.loaded["orca.system"] = system
local colors = dofile("apps/octopus/theme.lua")
local retained = colors
assert(colors.appearance == "light" and colors.background == "#ffffff")
assert(not colors.refresh(), "An unchanged appearance must not rebuild pages")
dark = true
assert(colors.refresh() and colors == retained and retained.background == "#000000"
    and retained.ink == "#ffffff", "Existing modules must see the new palette through the same table")
dark = false
assert(colors.refresh() and retained.background == "#ffffff" and retained.ink == "#000000")
override, dark = "light", true
assert(not colors.refresh() and colors.appearance == "light", "A forced light theme ignores OS dark mode")
override = "dark"
assert(colors.refresh() and colors.appearance == "dark")
dark = false
assert(not colors.refresh(), "A forced dark theme ignores OS light mode")
override = "invalid"
assert(colors.refresh() and colors.appearance == "light", "Invalid overrides follow the OS")
override = nil

-- Load the real App methods with inert native/view dependencies.
local function screen(props)
    props.post = function() end
    return props
end
local Application = { extend = function(_, methods) return methods end }
local entered
local transitions = { enter = function(view) entered = view end }
local modules = {
    ["orca"] = {}, ["orca.UIKit"] = { Screen = screen },
    ["orca.core.application"] = Application, ["orca.renderer"] = { getSize = function() return 390, 844 end },
    ["Octopus/views/components"] = { colors = colors }, ["orca.UIKit.PageTransition"] = transitions,
}
for _, name in ipairs { "Octopus/api/octopus", "Octopus/views/shell", "Octopus/views/home", "Octopus/views/thread",
    "Octopus/views/people", "Octopus/views/compose", "Octopus/views/identity", "orca.UIKit.AvatarLoader" } do
    modules[name] = {}
end
for name, value in pairs(modules) do package.loaded[name] = value end
local App = dofile("apps/octopus/App.lua")
local session, client, draft = {}, {}, { content = "unfinished post" }
client.session = session
function client:getSession() return self.session end
local app = setmetatable({ client = client, composeDraft = draft, commentDrafts = { post = "unfinished comment" },
    currentRoute = "/post", navigationEpoch = 1, sessionEpoch = 4, shell = {}, feeds = { discover = {} },
    pendingShow = {}, screen = screen {} }, { __index = App })
local captured, builds = 0, 0
app.leaving = function() captured = captured + 1; draft.content = "captured input" end
function app:dispatch(route)
    assert(route == "/post", "Theme changes retain the current route")
    builds = builds + 1
    return { view = self:frame() }
end
function app:activate_controller(controller) self.screen = controller.view; return controller end
assert(not app:updateAppearance() and builds == 0)
dark = true
for _, route in ipairs { "/auth", "/auth/signup", "/me", "/write" } do
    app.currentRoute = route
    assert(not app:updateAppearance() and colors.appearance == "light" and builds == 0,
        "Changing appearance must not discard the active form: " .. route)
end
app.currentRoute = "/post"
for _, thread in ipairs { { replyTo = { id = "parent" } }, { sending = true }, { input = { Text = "reply draft" } } } do
    app.thread = thread
    assert(not app:updateAppearance() and colors.appearance == "light" and builds == 0,
        "Appearance must wait for pending comments and replies without changing their text or target")
    assert(app.thread == thread)
end
app.thread = nil
assert(app:updateAppearance())
assert(captured == 1 and builds == 1 and app.navigationEpoch == 2 and app.sessionEpoch == 4,
    "Rebuilding captures inputs and invalidates stale view requests without changing the session epoch")
assert(app.client == client and app.client.session == session and app.composeDraft == draft
    and draft.content == "captured input" and app.commentDrafts.post == "unfinished comment",
    "Theme changes must preserve authentication and drafts")
assert(app.shell == nil and next(app.feeds) == nil and app.pendingShow == nil,
    "Retained pages and pending navigation must not resurrect the previous theme")
assert(app.screen.class == "octopus-dark"
    and app.screen.ClearColor == colors.background and entered == app.screen)
assert(not app:updateAppearance() and builds == 1)

-- The custom event loop applies changes before painting the next frame.
local messages = 0
system.getMessage = function()
    messages = messages + 1
    local kind = messages == 1 and "Window.Paint" or "Window.Closed"
    dark = false
    return { is = function(_, name) return name == kind end }
end
system.dispatchMessage = function(view)
    assert(view == app.screen and view.class == "octopus-light" and view.ClearColor == "#ffffff",
        "The event must see the refreshed screen, including its chrome theme")
    return true
end
system.translateMessage = function() end
app.afterPaint = function() end
app:run()
assert(builds == 2 and app.navigationEpoch == 3 and colors.appearance == "light")
os.getenv = getenv
print("Octopus appearance tests passed")
