local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
-- Text-sized tags need real font metrics, including in headless layout checks.
require("orca.renderer").init(390, 844, true)
filesystem.init("apps/octopus")
require("orca.core.startup").load_plugins()

local App = require "Octopus/Scripts/App"
local Octopus = require "Octopus/api/octopus"
orca.async = function(fn, ...) return fn(...) end
local NOW = 1761400000

local function programs(app)
    if not app.screen then return end
    core.advanceFrame()
    core.runAllPrograms(app.screen)
    if app.shell and app.shell.screen ~= app.screen then core.runAllPrograms(app.shell.screen) end
end
local function drain(app)
    core.flushQueue()
    while true do
        local message = orca.system.peekMessage()
        if not message then break end
        if not message:is("Window.Paint") then orca.system.dispatchMessage(app.screen, message) end
    end
    core.flushQueue()
    programs(app)
end
local function find(app, name) return assert(app.screen:findChild(name, true), "Missing " .. name) end
local function text(node)
    local value = node.Text
    if value ~= nil and value ~= "" then return value end
    return node:getTextContent()
end
local function press(app, node)
    node:send("Node.LeftButtonUp")
    drain(app)
    app.screen:UpdateLayout(app.screen.Width, app.screen.Height)
    app:afterPaint()
    drain(app)
end
local function click(app, name) press(app, find(app, name)) end
local function go(app, route)
    app:show(route)
    drain(app)
end

local transport, state = require("Octopus/api/mock").new { clock = function() return NOW end }
local client = assert(Octopus.client { mode = "mock", transport = transport, projectId = "octopus-mock",
    clock = function() return NOW end })
local app = App { client = client }
app.activate_controller = function(self, controller) self.screen = controller.view; return controller end
app.repaint = function() end
go(app, "/")

-- Guests can read the rules and the privacy policy before creating an account.
go(app, "/auth/signup")
assert(text(find(app, "RulesAgreement")):find("agree to the community rules", 1, true), "Sign-up states the agreement")
click(app, "AuthRules")
assert(app.currentRoute == "/rules" and find(app, "DocumentText").Source:find("Octopus Community Rules", 1, true))
go(app, "/auth/signup")
click(app, "AuthPrivacy")
assert(app.currentRoute == "/privacy" and text(find(app, "DocumentLink")):find("privacy", 1, true)
    and #find(app, "DocumentText").Source > 1000, "The bundled privacy policy opens with its address")
-- Reporting asks a guest to sign in.
app:openPost(assert(app.client:getPost("p-reef")))
drain(app)
click(app, "ReportPost")
assert(app.currentRoute == "/auth", "Guests sign in before reporting")

assert(app.client:signIn("demo@example.invalid", "password123"))
app:resetSessionViews()
go(app, "/")

-- Your own post offers neither report nor block.
local mine = assert(app.client:createPost { title = "Mine", content = "Hello" })
app:openPost(mine)
drain(app)
assert(not app.screen:findChild("ReportPost", true) and not app.screen:findChild("BlockAuthor", true)
    and app.screen:findChild("DeletePost", true), "Own posts are deleted, not reported")

-- Report someone's post, then block them from the confirmation.
app:openPost(assert(app.client:getPost("p-reef")))
drain(app)
click(app, "ReportPost")
assert(app.currentRoute == "/report" and text(find(app, "ReportSubject")):find("Sam Rivera", 1, true))
find(app, "ReportDetails").Text = "Not kind"
click(app, "ReasonHarassment")
local report = state.reports[#state.reports]
assert(report and report.post == "p-reef" and report.reason == "harassment" and report.details == "Not kind"
    and report.target_user == "u-sam", "The report reaches the service")
assert(text(find(app, "ReportStatus")):find("sent privately", 1, true) and find(app, "ReportDone").Visible
    and find(app, "BlockReported").Visible and not find(app, "ReasonSpam").Visible, "The page confirms the report")
click(app, "ReasonSpam")
assert(#state.reports == 1, "A sent report is not sent twice")
click(app, "BlockReported")
assert(app.currentRoute == "/" and app.client:isBlocked("u-sam"), "Blocking returns to the timeline")
for _, post in ipairs(assert(app.client:listPosts { limit = 30 })) do
    assert(post.authorId ~= "u-sam", "A blocked author's posts are gone")
end

-- Comments by other people carry a report link.
app:openPost(assert(app.client:getPost("p-hello")))
drain(app)
local comment = find(app, "Commentc-welcome")
press(app, assert(comment:findChild("Report", true), "Missing comment report"))
assert(app.currentRoute == "/report" and app.reportTarget.commentId == "c-welcome"
    and app.reportTarget.userId == "u-maya", "A comment report names the comment and its author")
click(app, "ReasonSpam")
assert(state.reports[2].comment == "c-welcome")
click(app, "ReportDone")
assert(app.currentRoute == "/post", "Done returns to the post")

-- A profile's menu reports and blocks; blocked people are managed from me.
app:openProfile("u-maya", "Maya Chen")
drain(app)
assert(text(find(app, "BlockPerson")):find("block Maya Chen", 1, true) and find(app, "ReportPerson"))
click(app, "BlockPerson")
assert(app.client:isBlocked("u-maya") and app.currentRoute ~= "/user", "Blocking leaves the profile")
go(app, "/me")
local listBlocks = client.listBlocks
client.listBlocks = function() return nil, { code = "offline", message = "Could not load blocked people." } end
click(app, "OpenBlocked")
assert(app.currentRoute == "/blocked" and find(app, "RetryBlocks"):getParent().Visible
    and not find(app, "BlocksEmpty").Visible, "The list offers retry after a failure")
client.listBlocks = listBlocks
click(app, "RetryBlocks")
local maya, sam = find(app, "Blockedu-maya"), find(app, "Blockedu-sam")
local unblock = assert(maya:findChild("Unblock", true))
local setUserBlocked = client.setUserBlocked
client.setUserBlocked = function() return nil, { code = "offline", message = "Could not unblock this person." } end
press(app, unblock)
assert(app.client:isBlocked("u-maya") and find(app, "UnblockError").Visible
    and text(find(app, "UnblockError")) == "Could not unblock this person.", "An unblock failure keeps the row")
client.setUserBlocked = setUserBlocked
press(app, unblock)
assert(not app.client:isBlocked("u-maya") and not app.screen:findChild("Blockedu-maya", true)
    and find(app, "Blockedu-sam"):getUniqueID() == sam:getUniqueID() and not find(app, "UnblockError").Visible,
    "Unblocking removes only the chosen row")

-- A write can finish after its page closes. The replacement scope must reload.
local blockedScope = app.transientScope
local run, pending = orca.async, nil
orca.async = function(fn, ...)
    pending = coroutine.create(fn)
    assert(coroutine.resume(pending, ...))
end
client.setUserBlocked = function(...)
    coroutine.yield()
    return setUserBlocked(...)
end
press(app, assert(sam:findChild("Unblock", true)))
orca.async = run
assert(pending and coroutine.status(pending) == "suspended", "The unblock is still in flight")
go(app, "/me")
assert(blockedScope.disposed, "Leaving blocked people disposes its scope")
go(app, "/blocked")
find(app, "Blockedu-sam")
assert(coroutine.resume(pending))
client.setUserBlocked = setUserBlocked
drain(app)
assert(find(app, "BlocksEmpty").Visible and not app.screen:findChild("Blockedu-sam", true),
    "The reopened list shows the empty state after the unblock")

-- Account deletion asks twice, then signs the device out.
go(app, "/me")
local closedScope = setmetatable({}, { __mode = "v" })
do
    local scope = app:octopusScope("Octopus/Data/Blocks.xml")
    scope:dispose()
    closedScope[1] = scope
end
collectgarbage("collect")
assert(closedScope[1] == nil, "The reader's scope registry does not retain closed scopes")
click(app, "OpenRules")
assert(app.currentRoute == "/rules")
go(app, "/me")
click(app, "OpenDeleteAccount")
assert(app.currentRoute == "/delete-account")
click(app, "KeepAccount")
assert(app.currentRoute == "/me" and app.client:getSession(), "Keeping the account goes back")
click(app, "OpenDeleteAccount")
click(app, "ConfirmDeleteAccount")
assert(app.client:getSession() and text(find(app, "ConfirmDeleteAccount")):find("Tap again", 1, true),
    "The first tap only asks")
click(app, "ConfirmDeleteAccount")
assert(not app.client:getSession() and app.currentRoute == "/" and state.deleted["u-ada"],
    "The second tap deletes the account and signs out")
print("PASS: Octopus safety UI: rules and privacy before sign-up, post, comment and profile reports, block and unblock, account deletion")
