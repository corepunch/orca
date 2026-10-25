local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
filesystem.init("apps/octopus")
require("orca.core.startup").load_plugins()

local App = require "Octopus/App"
local Octopus = require "Octopus/api/octopus"
-- Run background work inline so each step can assert its result.
orca.async = function(fn, ...) return fn(...) end
local NOW = 1761400000

local function drain(app)
    core.flushQueue()
    while true do
        local message = orca.system.peekMessage()
        if not message then break end
        if not message:is("Window.Paint") then orca.system.dispatchMessage(app.screen, message) end
    end
    core.flushQueue()
end

local function find(app, name)
    return assert(app.screen:findChild(name, true), "Missing " .. name)
end

local function click(app, name)
    find(app, name):send("Node.LeftButtonUp")
    drain(app)
end

local function isShown(node)
    while node do
        if node.Visible == false then return false end
        node = node:getParent()
    end
    return true
end

local function newApp()
    local app = App { client = Octopus.mock { clock = function() return NOW end } }
    -- Keep navigation real while avoiding Paint on an uninitialized renderer.
    app.activate_controller = function(self, controller) self.screen = controller.view; return controller end
    app.repaint = function() end
    app:show("/")
    drain(app)
    return app
end

-- Guests land on the public timeline.
local app = newApp()
local home = app.screen
assert(home.Width == 390 and home.Height == 844 and home.ResizeMode == "NoResize")
assert(not app.client:getSession(), "Octopus starts as a guest")
assert(home:findChild("Timeline", true) and find(app, "Feeddiscover") and find(app, "Feedfollowing"))
assert(find(app, "Postp-reef") and find(app, "Postp-quote"), "The discover timeline lists posts for guests")
assert(find(app, "Imagep-reef").Src == app.media:path("f-reef"), "Photo posts load their image")
home:UpdateLayout(390, 844)
local card = find(app, "Postp-small")
assert(card.ActualWidth > 300 and card.ActualHeight > 100, "Cards need a usable size: "
    .. card.ActualWidth .. "x" .. card.ActualHeight)
assert(find(app, "Tagsoftware").ActualWidth > 40, "Tags need hit areas")
assert(find(app, "FollowingSignIn"), "Following asks guests to sign in")
assert(find(app, "AppBarHome").IsEnabled == false and find(app, "AppBarSignIn"), "Guests get sign-in in the menu")

-- Guests can read a post and its comments, but writing asks them to sign in.
click(app, "Openp-hello")
assert(app.currentRoute == "/post" and find(app, "PostPage"))
assert(find(app, "Commentc-welcome") and find(app, "SignInToComment"))
assert(find(app, "PostStats"):getTextContent() == "1 like · 0 reposts · 3 comments",
    find(app, "PostStats"):getTextContent())
click(app, "LikePost")
assert(app.currentRoute == "/auth" and find(app, "SignInPage"), "Liking as a guest opens sign in")
assert(find(app, "AuthStatus"):getTextContent() == "Sign in to like posts.")
click(app, "ContinueAsGuest")
assert(app.currentRoute == "/post", "Continuing as a guest returns to the post")
click(app, "BackFromPost")
assert(app.currentRoute == "/" and app.screen == home, "Back returns to the retained timeline")

-- Profiles and tags are public too.
click(app, "Authorp-reef")
assert(app.currentRoute == "/user" and find(app, "ProfileName"):getTextContent() == "Sam Rivera")
assert(find(app, "Countposts"):getTextContent() == "2" and find(app, "Countfollowers"):getTextContent() == "–")
assert(find(app, "Postp-tide") and find(app, "SignInToFollow"))
click(app, "BackFromProfile")
click(app, "Tagreef")
assert(app.currentRoute == "/search" and find(app, "SearchInput").Text == "#reef")
local results = find(app, "SearchResults")
assert(results:findChild("Postp-reef", true) and results:findChild("Postp-arms", true)
    and not results:findChild("Postp-small", true), "Tag search lists only tagged posts")
find(app, "SearchInput").Text = "maya"
click(app, "RunSearch")
assert(find(app, "Personu-maya"), "Search finds people")
click(app, "AppBarWrite")
assert(find(app, "WriteSignIn"), "Writing needs an account")

-- Sign in from the write page and come back to it.
click(app, "WriteSignInButton")
find(app, "Email").Text = "demo@example.invalid"
find(app, "Password").Text = "nope"
click(app, "SignIn")
assert(not app.client:getSession() and find(app, "AuthStatus").Foreground ~= nil)
find(app, "Password").Text = "password123"
click(app, "SignIn")
assert(app.client:getSession() and app.currentRoute == "/write", "Sign-in returns to where the guest was")
assert(find(app, "AppBarSignOut") and not app.screen:findChild("AppBarSignIn", true), "The menu follows the session")

-- Publish a quote, which opens the new post.
click(app, "Kindquote")
assert(find(app, "QuoteText") and not app.screen:findChild("PostTitle", true))
click(app, "Publish")
assert(find(app, "WriteStatus"):getTextContent() == "Quote text is required.")
find(app, "QuoteText").Text = "Less, but better."
find(app, "QuoteSource").Text = "Dieter Rams"
find(app, "PostTags").Text = "design"
click(app, "Kindtext")
click(app, "Kindquote")
assert(find(app, "QuoteText").Text == "Less, but better.", "Drafts survive switching post types")
click(app, "Publish")
assert(app.currentRoute == "/post" and app.post.postType == "quote" and app.post.authorId == "u-ada")
assert(find(app, "DeletePost"), "Authors can delete their posts")
local published = app.post.id

-- Comment and reply.
find(app, "CommentInput").Text = "Adding a source link later."
click(app, "BackFromPost")
assert(find(app, "Post" .. published), "New posts top the timeline")
click(app, "Open" .. published)
assert(find(app, "CommentInput").Text == "Adding a source link later.", "Comment drafts survive navigation")
local thread = app.thread
require("Octopus/views/thread").send(app, thread)
drain(app)
assert(app.screen:findChild("PostStats", true):getTextContent():match("1 comment$"))
local comments = app.client:listComments(published)
assert(#comments == 1 and comments[1].body == "Adding a source link later.")
click(app, "Reply" .. comments[1].id)
assert(isShown(find(app, "ReplyBar")))
find(app, "CommentInput").Text = "Done."
require("Octopus/views/thread").send(app, app.thread)
drain(app)
comments = app.client:listComments(published)
assert(#comments == 2 and comments[2].parentId == comments[1].id, "Replies thread under their comment")

-- Like and repost from the post page and timeline.
click(app, "LikePost")
assert(find(app, "PostStats"):getTextContent():match("^1 like"))
assert(find(app, "LikePost").Text == "unlike")
click(app, "LikePost")
assert(find(app, "PostStats"):getTextContent():match("^0 likes"))
click(app, "BackFromPost")
click(app, "Likep-small")
local stats = app.client:stats { "p-small" }
assert(stats["p-small"].likes == 2 and stats["p-small"].liked, "Timeline likes reach the service")
click(app, "Repostp-small")
assert(app.client:stats({ "p-small" })["p-small"].reposted)

-- Following updates the following timeline.
click(app, "Authorp-small")
assert(find(app, "Countfollowers"):getTextContent() == "0")
click(app, "FollowButton")
assert(find(app, "Countfollowers"):getTextContent() == "1" and app.client:followState("u-jules"))
click(app, "BackFromProfile")
app.pivot.SelectedIndex = 1
app:selectFeed("following")
drain(app)
assert(find(app, "Postp-small") and app.feeds.following, "Following shows the people you follow")

-- Your own profile: edit and sign out.
click(app, "AppBarMe")
assert(find(app, "ProfileEditor") and find(app, "EditName").Text == "Ada Reef")
find(app, "EditBio").Text = "Testing the phone app"
click(app, "SaveProfile")
assert(find(app, "ProfileBio"):getTextContent() == "Testing the phone app")
assert(find(app, "Countposts"):getTextContent() == "2")

-- Deleting needs a second tap.
click(app, "AppBarHome")
click(app, "Open" .. published)
click(app, "DeletePost")
assert(app.client:getPost(published), "The first tap only asks for confirmation")
click(app, "DeletePost")
local _, err = app.client:getPost(published)
assert(err.status == 404 and app.currentRoute ~= "/post", "The second tap deletes the post")

click(app, "AppBarMe")
click(app, "SignOut")
assert(not app.client:getSession() and app.currentRoute == "/" and find(app, "FollowingSignIn"))

-- Sign up creates the account and its public profile.
local fresh = newApp()
click(fresh, "AppBarMe")
click(fresh, "MeSignUp")
find(fresh, "DisplayName").Text = "Pat Example"
find(fresh, "Email").Text = "pat@example.invalid"
find(fresh, "Password").Text = "password123"
find(fresh, "ConfirmPassword").Text = "password124"
click(fresh, "SignUp")
assert(not fresh.client:getSession() and find(fresh, "AuthStatus"):getTextContent() == "Passwords do not match.")
find(fresh, "ConfirmPassword").Text = "password123"
click(fresh, "SignUp")
assert(fresh.client:getSession() and fresh.currentRoute == "/me", "Sign-up returns to the profile")
assert(find(fresh, "ProfileName"):getTextContent() == "Pat Example" and find(fresh, "Countposts"):getTextContent() == "0")

-- Service errors stay visible rather than falling back to demo data.
local offline = App { client = Octopus.client { projectId = "p", transport = function() error("offline") end } }
offline.activate_controller = function(self, controller) self.screen = controller.view; return controller end
offline.repaint = function() end
offline:show("/")
drain(offline)
assert(offline.screen:findChild("HomeStatus", true):getTextContent():match("Could not reach Octopus"))

print("Octopus UI: guest timeline/post/profile/search, sign-in return, compose drafts and publish, comments and replies, likes, reposts, follows, profile editing, delete confirmation, sign-up and errors passed")
