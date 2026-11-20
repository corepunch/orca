-- Native app pages over deterministic, isolated offline providers.
local orca = require "orca"
orca.init()
local fs, core = require "orca.filesystem", require "orca.core"
require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/chat")
require("orca.core.startup").load_plugins()
local App, Chat = require "Chat/Scripts/App", require "api.chat"
local Social, PublicPost = require "Chat/views/social", require "Chat/Scripts/public_post"
local savedAsync = orca.async
orca.async = function(fn, ...) return fn(...) end
local function settle(app)
    while true do
        local message = orca.system.peekMessage()
        if not message then break end
        if not message:is("Window.Paint") then orca.system.dispatchMessage(nil, message) end
    end
    core.advanceFrame()
    core.runAllPrograms(app.screen)
    app.screen:UpdateLayout(app.screen.Width, app.screen.Height)
end
local function press(app, name)
    assert(app.screen:findChild(name, true), "Missing " .. name):send("Node.LeftButtonUp")
    settle(app)
end
local function newApp(signedIn)
    local client = Chat.mock { clock = function() return 1700000000 end }
    if signedIn then assert(client:signIn("demo@example.invalid", "password123")) end
    local app = App { client = client }
    -- Keep route construction/lifecycle real; animations are separately tested.
    app.show = function(self, route) return self:navigate(route or "/") end
    app.reportError = function(_, err) error(tostring(err), 0) end
    app:show("/")
    settle(app)
    return app
end
local app = newApp(true)
local shell, bar = app.screen, app.shell.bar
for _, name in ipairs { "AppBarChats", "AppBarContacts", "AppBarPosts", "AppBarMe" } do
    assert(shell:findChild(name, true), "Root destination " .. name)
end
assert(not shell:findChild("AppBarNew", true) and not shell:findChild("AppBarProfile", true))
press(app, "NewChat")
assert(app.currentRoute == "/new" and app.screen ~= shell and app.screen:findChild("FindPerson", true))
assert(not app.screen:findChild("AppBar", true), "Creation is a focused task")
app.screen:findChild("FindPerson", true).Text = "Alex"
app:show("/"); app:show("/new"); settle(app)
assert(app.screen:findChild("FindPerson", true).Text == "Alex", "New chat draft survives leaving")
app:show("/"); settle(app)
press(app, "AppBarPosts")
assert(app.shell.active == "Posts" and app.shell.bar == bar)
local feature = app:posts()
local timeline = assert(app.screen:findChild("Timeline", true))
local retained = app.screen:findChild("PostsPage", true)
assert(timeline.Title == "Messenger")
assert(app.screen:findChild("NewPublicPost", true) and app.screen:findChild("SearchPublicPosts", true))
local function top(node)
    local y = 0
    while node do y = y + (node.ActualY or 0); node = node:getParent() end
    return y
end
local newPost = app.screen:findChild("NewPublicPost", true)
assert(top(newPost) + newPost.ActualHeight <= top(bar), "Public tasks must sit above the root footer")
-- Like Messages: the timeline header frost reaches the top edge behind the
-- status bar, and the commands strip and app bar form one frosted panel.
do
    app.screen.SafeAreaTop, app.screen.SafeAreaBottom = 59, 34
    settle(app); settle(app)
    local function header(node)
        local tag = fs.serializeObjectToString(node):match("<[%w_:][^>]*>") or ""
        local classes = tag:match('%sclass="([^"]*)"') or ""
        if (" " .. classes .. " "):find(" pivot-bar ", 1, true) then return node end
        local child = node:getFirstChild()
        while child do local found = header(child); if found then return found end; child = child:getNext() end
    end
    local bar_ = assert(header(timeline), "Timeline header bar")
    assert(top(bar_) == 0 and bar_.ActualHeight > 59, "The posts header extends behind the status bar")
    local commands = app.screen:findChild("PostsCommands", true)
    assert(math.abs(top(commands) + commands.ActualHeight - top(bar)) < 0.5, "The commands sit flush on the app bar")
    assert(commands.BackdropFilterBlurRadius > 0 and commands.BackgroundColor.A < 1, "The commands strip is frosted like the bar")
    assert(bar.BorderWidthTop == 0, "No hairline divides the commands strip from the app bar")
    press(app, "AppBarChats")
    assert(bar.BorderWidthTop > 0, "Other pages keep the app bar's hairline")
    press(app, "AppBarPosts")
    app.screen.SafeAreaTop, app.screen.SafeAreaBottom = 0, 0
    settle(app)
end

assert(feature.client:currentUserId() == app.client:getSession().user.id, "Posts uses the same account")
feature.app:selectFeed("discover"); settle(app)
assert(app.screen:findChild("Postp-small", true), "Real Octopus cards are available in Chat")
app:show("/"); app:show("/posts"); settle(app)
assert(app.screen:findChild("PostsPage", true) == retained and app.shell.bar == bar, "Posts root retained")
press(app, "NewPublicPost")
assert(app.currentRoute == "/posts/write" and app.screen:findChild("Kindtext", true))
assert(not app.screen:findChild("Publish", true), "Type choice precedes editing")
press(app, "Kindtext")
assert(app.currentRoute == "/posts/compose")
assert(app.screen:findChild("Publish", true).TextResourceID == "Publish publicly")
app.screen:findChild("PostTitle", true).Text = "A retained public draft"
app.screen:findChild("PostContent", true).Text = "Publish only from the public editor."
app:show("/"); app:show("/posts/write"); settle(app)
press(app, "Kindtext")
assert(app.screen:findChild("PostTitle", true).Text == "A retained public draft")
press(app, "Publish")
assert(app.currentRoute == "/posts/post" and feature.app.post.title == "A retained public draft")
assert(feature.app.post.authorId == app.client:getSession().user.id)
feature:openProfile("alex", "Alex Morgan"); settle(app)
assert(app.currentRoute == "/posts/user" and app.screen:findChild("MessageProfile", true), "Profile bridges public posts to private chat")
press(app, "MessageProfile")
assert(app.currentRoute == "/thread" and app.room.kind == "direct")
local room = app.room
app.drafts[room.id] = "Keep this draft"
local post = assert(feature.client:getPost("p-small"))
local before = #assert(app.client:listMessages(room.id))
app:sharePost(post); settle(app)
assert(app.currentRoute == "/share-post" and app.screen:findChild("SharedPostTitle", true))
press(app, "ShareRoom" .. room.id)
assert(app.composer.Text:find("Keep this draft", 1, true) and app.composer.Text:find(PublicPost.url(post.id), 1, true))
assert(#assert(app.client:listMessages(room.id)) == before, "Choosing a recipient must not send")
app:sendMessage(); settle(app)
assert(#assert(app.client:listMessages(room.id)) == before + 1)
assert(app.screen:findChild("SharedPostPreview", true), "Canonical shared posts have an actionable preview")
local sentId
for _, message in ipairs(assert(app.client:listMessages(room.id))) do
    if message.body:find(PublicPost.url(post.id), 1, true) then sentId = message.id end
end
assert(app.screen:findChild("MessageEntry" .. assert(sentId), true)):findChild("SharedPostPreview", true):send("Node.LeftButtonUp")
settle(app)
assert(app.currentRoute == "/posts/post" and feature.app.post.id == "p-small")
app:show("/me"); settle(app)
assert(app.shell.active == "Me" and app.screen:findChild("MyPublicProfile", true))
-- Me follows the Settings list: the identity centres on the photo, one row per
-- destination with an icon and a muted line, and sign out last.
do
    local photo, identity = app.screen:findChild("MeAvatarHost", true), app.screen:findChild("MeIdentity", true)
    assert(math.abs(top(identity) + identity.ActualHeight / 2 - (top(photo) + photo.ActualHeight / 2)) < 0.5,
        "Name and handle centre on the photo")
    assert(not app.screen:findChild("MyPosts", true), "Profile and posts are one destination")
    local rows = { "MyPublicProfile", "MeSettings", "MeSignOut" }
    for index, name in ipairs(rows) do
        local row = app.screen:findChild(name, true)
        assert(row.ActualHeight == 64, name .. " is a 64-point row")
        if index > 1 then
            local previous = app.screen:findChild(rows[index - 1], true)
            assert(top(row) >= top(previous) + previous.ActualHeight, name .. " follows " .. rows[index - 1])
        end
    end
end
press(app, "MeSettings")
assert(app.currentRoute == "/profile" and app.screen ~= shell)
assert(app.screen:findChild("ProfileName", true) and not app.screen:findChild("AppBar", true))
assert(PublicPost.parse("https://evil.invalid/octopus/post.html?id=p-small") == nil)
assert(PublicPost.parse(PublicPost.url("p-small") .. "/private") == nil)
assert(PublicPost.parse("A public post\n" .. PublicPost.url("p-small")) == "p-small")
app:show("/posts/me"); settle(app)
app.screen:findChild("EditName", true).Text = "Shared Identity"
press(app, "SaveProfile")
assert(app.client:getProfile().display_name == "Shared Identity")
assert(feature.client:getSession().user.name == "Shared Identity")
app:show("/me"); settle(app)
assert(app.screen:findChild("MeName", true):getTextContent() == "Shared Identity")
-- Session cleanup removes all retained public records, drafts and private rooms.
app:signOut(); settle(app)
assert(not app.postsFeature and not app.client:getSession() and app.shell == nil)
local guest = newApp(false)
press(guest, "BrowsePublicPosts")
assert(guest.currentRoute == "/posts" and guest:posts().app.feed == "discover")
assert(not guest:posts().client:getSession(), "Guests browse without a synthetic login")
for _, width in ipairs { 320, 390, 768 } do
    guest.screen.Width = width
    guest.screen:UpdateLayout(width, 844)
    assert(guest.screen:findChild("NewPublicPost", true).ActualWidth > 44)
end
-- Protected public actions resume their exact route after the one Chat login.
guest:show("/posts/write"); settle(guest)
press(guest, "Kindquote")
press(guest, "WriteSignInButton")
guest.screen:findChild("Email", true).Text = "demo@example.invalid"
press(guest, "RequestCode")
guest.screen:findChild("VerificationCode", true).Text = "123456"
press(guest, "VerifyCode")
assert(guest.client:getSession() and guest.currentRoute == "/posts/compose")
assert(guest:posts().app.composeScope:exposedRecord("Composer").values.Kind == "quote",
    "Sign-in preserves the selected public post type")
assert(guest.screen:findChild("QuoteText", true), "Sign-in returns to the selected public editor")
orca.async = savedAsync
print("Chat social navigation, public profiles, explicit sharing and guest browsing: OK")
