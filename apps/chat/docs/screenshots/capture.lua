-- Native, offline screenshots of the production Chat pages. Run from the repo root.
-- The fixture changes only startup/navigation and supplies isolated demo data.
-- Explicit paint dimensions keep X11 exposure events from resetting the viewport.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local renderer = require "orca.renderer"
local system = require "orca.system"
fs.init("apps/chat")
-- The X11 fixture allocates physical pixels; paint/layout remain logical.
local scale = tonumber(os.getenv("ORCA_CAPTURE_SCALE")) or 1
assert(scale == 1 or scale == 2, "Capture scale must be 1 or 2")
renderer.init(390 * scale, 844 * scale, false)
require("orca.core.startup").load_plugins()
local client = require("api.chat").mock()
assert(client:signIn("demo@example.invalid", "password123"))
local app = require("Chat/App") { client = client }
app.show = function(self, route) return self:navigate(route) end
local route = os.getenv("ORCA_CAPTURE_ROUTE") or "/"
if route:match("^/posts") then
    local posts = app:posts()
    posts.app.feed = "discover"
    if route == "/posts/compose" then
        require("Octopus/views/compose").scope(posts.app)
    end
    if route == "/posts/user" then posts.app.profileId, posts.app.profileName, posts.app.profileReturn = "alex", "Alex Morgan", "/" end
end
app:activate_route(route)
-- Give startup bindings, SVG uploads and the frosted header multiple frames
-- to settle even when an idle X11 window emits no further exposure events.
local warmupFrames = 0
for message in system.getMessage do
    if message:is("Window.Closed") then break end
    if message:is("Window.Paint") then
        app.screen:send("Window.Paint", { WindowWidth = 390, WindowHeight = 844 })
        app:afterPaint()
        warmupFrames = warmupFrames + 1
        if warmupFrames < 20 then app:repaint() end
    else
        if system.dispatchMessage(app.screen, message) then app:repaint() end
    end
    system.translateMessage(message)
end
