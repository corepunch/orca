-- Native, offline screenshots of the production Chat pages. Run from the repo root.
-- The fixture changes only startup/navigation and supplies isolated demo data.
-- Explicit paint dimensions keep X11 exposure events from resetting the viewport.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local renderer = require "orca.renderer"
local system = require "orca.system"
fs.init("apps/chat")
renderer.init(390, 844, false)
require("orca.core.startup").load_plugins()
local client = require("api.chat").mock()
assert(client:signIn("demo@example.invalid", "password123"))
local app = require("Chat/App") { client = client }
app.show = function(self, route) return self:navigate(route) end
local route = os.getenv("ORCA_CAPTURE_ROUTE") or "/"
if route:match("^/posts") then
    local posts = app:posts()
    posts.app.feed = "discover"
    if route == "/posts/user" then posts.app.profileId, posts.app.profileName, posts.app.profileReturn = "alex", "Alex Morgan", "/" end
end
app:activate_route(route)
for message in system.getMessage do
    if message:is("Window.Closed") then break end
    if message:is("Window.Paint") then
        app.screen:send("Window.Paint", { WindowWidth = 390, WindowHeight = 844 })
        app:afterPaint()
    else system.dispatchMessage(app.screen, message) end
    system.translateMessage(message)
end
