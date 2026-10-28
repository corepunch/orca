local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
-- Text-sized tags need real font metrics, including in headless layout checks.
require("orca.renderer").init(390, 844, true)
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
    app.screen:UpdateLayout(app.screen.Width, app.screen.Height)
    app:afterPaint()
    drain(app)
end

-- Pages you go into go back from their round title button, like a user would.
local function back(app)
    local screen = app.screen
    screen:UpdateLayout(screen.Width, screen.Height)
    for _, message in ipairs { "LeftButtonDown", "LeftButtonUp" } do
        orca.system.dispatchMessage { target = screen, message = message, x = 20, y = 20 }
    end
    drain(app)
    app.screen:UpdateLayout(app.screen.Width, app.screen.Height)
    app:afterPaint()
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

local function assertDetailFooter(app)
    local screen = app.screen
    local bar = find(app, "AppBar")
    local function position(node)
        local x, y = 0, 0
        while node and node ~= screen do
            x, y = x + node.ActualX, y + node.ActualY
            node = node:getParent()
        end
        return x, y
    end
    for _, size in ipairs { { 430, 932, 59, 34, 0, 0 }, { 932, 430, 0, 21, 59, 59 } } do
        screen.Width, screen.Height = size[1], size[2]
        screen.SafeAreaTop, screen.SafeAreaBottom = size[3], size[4]
        screen.SafeAreaLeft, screen.SafeAreaRight = size[5], size[6]
        for _, keyboard in ipairs { 0, 300, 0 } do
            screen.KeyboardInsetBottom = keyboard
            core.advanceFrame()
            core.runAllPrograms(screen)
            screen:UpdateLayout(size[1], size[2])
            local barX, barY = position(bar)
            local bottomInset = math.max(size[4], keyboard)
            assert(not screen.RespectSafeArea and screen.Height == size[2],
                "Detail pages retain the physical viewport during keyboard changes")
            assert(math.abs(barX) < 1 and math.abs(bar.ActualWidth - size[1]) < 1
                and math.abs(barY + bar.ActualHeight - size[2]) < 1 and bar.BackdropFilterBlurRadius > 0,
                "Detail bar frosted backgrounds must reach every physical bottom edge: " .. app.currentRoute)
            assert(math.abs(bar.ActualHeight - 58 - bottomInset) < 1
                and bar.PaddingLeft == size[5] and bar.PaddingRight == size[6],
                "Detail bar controls must avoid the home gesture, keyboard, and landscape cutouts")
        end
    end
    screen.Width, screen.Height = 390, 844
    screen.SafeAreaTop, screen.SafeAreaBottom = 0, 0
    screen.SafeAreaLeft, screen.SafeAreaRight = 0, 0
    core.advanceFrame()
    core.runAllPrograms(screen)
    screen:UpdateLayout(390, 844)
end

-- Guests land on the public timeline.
local app = newApp()
local home = app.screen
assert(home.Width == 390 and home.Height == 844 and home.ResizeMode == "NoResize")
assert(not app.client:getSession(), "Octopus starts as a guest")
assert(find(app, "Feedhome") and find(app, "Feedconnect") and find(app, "Feeddiscover"))
assert(app.pivot.SelectedIndex == 2, "Guests start on discover")
assert(find(app, "Postp-reef") and find(app, "Postp-quote"), "The discover timeline lists posts for guests")
assert(not app.media:path("photo:f-reef"), "Building the timeline must not fetch offscreen photos")
collectgarbage("collect")
collectgarbage("collect")
home:UpdateLayout(390, 844)
app:afterPaint()
drain(app)
assert(find(app, "Imagep-reef").Src == app.media:path("photo:f-reef"), "Visible photo posts load their image")
assert(find(app, "Imagep-reef").Source, "Downloaded photos must resolve a native image resource")
home:UpdateLayout(390, 844)
local reef, reefImage = find(app, "Photop-reef"), find(app, "Imagep-reef")
assert(math.abs(reef.AspectRatio - 156 / 104) < 0.01 and math.abs(reef.ActualWidth / reef.ActualHeight - 1.5) < 0.02,
    "A photo frame takes the photo's shape instead of leaving empty space beneath it")
assert(reefImage.Stretch == "UniformToFill" and math.abs(reefImage.ActualHeight - reef.ActualHeight) < 1,
    "A shaped frame is filled edge to edge")
assert(app.photos:aspect("f-reef") == reef.AspectRatio, "Rebuilt cards reserve the decoded shape")
local profilePhoto = assert(find(app, "Postp-reef"):findChild("AvatarPhoto", true),
    "Bundled demo portraits must appear beside Octopus posts")
assert(profilePhoto.Source and profilePhoto.Source.Width > 100, "Octopus portraits must resolve real image resources")
home:UpdateLayout(390, 844)
local card = find(app, "Postp-small")
assert(card.ActualWidth > 300 and card.ActualHeight > 100, "Cards need a usable size: "
    .. card.ActualWidth .. "x" .. card.ActualHeight)
assert(find(app, "Tagsoftware").ActualWidth > 40, "Tags need hit areas")
assert(find(app, "SignInhome") and find(app, "SignInconnect"), "Home and connect ask guests to sign in")
assert(find(app, "AppBarHome").IsEnabled == false and find(app, "AppBarSignIn"), "Guests get sign-in in the menu")

-- Both apps dock the frosted application bar, including changing native safe insets.
local bar = find(app, "AppBar")
home.SafeAreaTop, home.SafeAreaBottom = 59, 34
home.SafeAreaLeft, home.SafeAreaRight = 12, 18
core.advanceFrame()
core.runAllPrograms(home)
home:UpdateLayout(390, 844)
assert(not home.RespectSafeArea and home:getFirstChild().ActualY == 0, "Octopus content runs edge to edge")
assert(math.abs(bar.ActualHeight - 58 - 34) < 1 and math.abs(bar.ActualY + bar.ActualHeight - 844) < 1,
    "Octopus chrome must fill the area beneath its bottom bar")
-- The 38-point ring; its label below only shows while the bar is expanded.
assert(bar.ActualY + find(app, "AppBarHome").ActualY + 38 <= 810 + 0.5,
    "Octopus bottom controls must stay above the home gesture")
local status = find(app, "HomeStatusBar")
assert(status.Visible and math.abs(status.ActualY + status.ActualHeight - bar.ActualY) < 1,
    "A status strip with a notice docks above the bar")
app:notice("")
home:UpdateLayout(390, 844)
assert(not status.Visible, "An empty status strip leaves no frosted band above the bar")
app:notice("Offline mock · posts stay in this session")
home:UpdateLayout(390, 844)
home.SafeAreaTop, home.SafeAreaBottom = 0, 0
home.SafeAreaLeft, home.SafeAreaRight = 0, 0
core.advanceFrame()
core.runAllPrograms(home)
home:UpdateLayout(390, 844)
assert(math.abs(bar.ActualHeight - 58) < 1)

-- Going into a post turns the timeline out and then the post in, on a hinge at
-- the left edge as on Windows Phone; coming back does the reverse. Pages you
-- go into get a round back button before the title instead of a back command.
local PageTransition = require "orca.UIKit.PageTransition"
local function pumpUntil(app, done)
    while not done() do
        local message = assert(orca.system.peekMessage(), "The transition stopped before it finished")
        if not message:is("Window.Paint") then orca.system.dispatchMessage(app.screen, message) end
    end
end
app:openPost(assert(app.client:getPost("p-hello")))
assert(app.screen == home and PageTransition.page(home).TransitionMode == "ForwardOut", "The timeline turns out first")
pumpUntil(app, function() return app.screen ~= home end)
local postView = find(app, "PostView")
assert(PageTransition.page(app.screen) == postView and postView.TransitionMode == "ForwardIn", "Then the post turns in")
assert(postView.BackIcon:find("arrow-left", 1, true) and not app.screen:findChild("BackFromPost", true),
    "The post goes back from its title line")
drain(app)
assert(postView.TransitionMode == "None", "The turnstile settles")
app:goBack("/")
assert(postView.TransitionMode == "BackwardOut", "Going back turns the post out first")
pumpUntil(app, function() return app.screen == home end)
assert(PageTransition.page(home).TransitionMode == "BackwardIn", "Then the timeline turns back in")
assert((PageTransition.page(home).BackIcon or "") == "", "Root pages have no back button")
drain(app)
assert(PageTransition.page(home).TransitionMode == "None")

-- Guests can read a post and its comments, but writing asks them to sign in.
click(app, "Openp-hello")
assert(app.currentRoute == "/post" and find(app, "PostPage"))
assert(find(app, "Commentc-welcome") and find(app, "SignInToComment"))
assertDetailFooter(app)
local function stat(key) return find(app, "Stat" .. key):getTextContent() end
assert(stat("likes") == "2" and stat("reposts") == "1" and stat("comments") == "3", "Post pages show live counts")
assert(find(app, "PostTime"):getTextContent():match("^%d%d:%d%d · %d+ %a+ %d%d%d%d$"), "Posts show their full time")
click(app, "LikePost")
assert(app.currentRoute == "/auth" and find(app, "SignInPage"), "Liking as a guest opens sign in")
assert(find(app, "AuthStatus"):getTextContent() == "Sign in to like posts.")
click(app, "ContinueAsGuest")
assert(app.currentRoute == "/post", "Continuing as a guest returns to the post")
back(app)
assert(app.currentRoute == "/" and app.screen == home, "Back returns to the retained timeline")

-- Profiles and tags are public too.
click(app, "Authorp-reef")
assert(app.currentRoute == "/user" and find(app, "ProfileName"):getTextContent() == "Sam Rivera")
assert(find(app, "Countposts"):getTextContent() == "2" and find(app, "Countfollowers"):getTextContent() == "–")
assert(find(app, "Postp-tide") and find(app, "SignInToFollow"))
assertDetailFooter(app)
back(app)
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
back(app)
assert(find(app, "Post" .. published), "New posts top the timeline")
click(app, "Open" .. published)
assert(find(app, "CommentInput").Text == "Adding a source link later.", "Comment drafts survive navigation")
local thread = app.thread
require("Octopus/views/thread").send(app, thread)
drain(app)
assert(stat("comments") == "1")
local comments = app.client:listComments(published)
assert(#comments == 1 and comments[1].body == "Adding a source link later.")
local retainedComment = find(app, "Comment" .. comments[1].id)
click(app, "Reply" .. comments[1].id)
assert(isShown(find(app, "ReplyBar")))
find(app, "CommentInput").Text = "Done."
require("Octopus/views/thread").send(app, app.thread)
drain(app)
comments = app.client:listComments(published)
assert(#comments == 2 and comments[2].parentId == comments[1].id, "Replies thread under their comment")
assert(find(app, "Comment" .. comments[1].id) == retainedComment, "Adding a reply retains the parent comment view")

-- Like and repost from the post page and timeline.
click(app, "LikePost")
assert(stat("likes") == "1")
assert(find(app, "LikePost").Text == "unlike")
click(app, "LikePost")
assert(stat("likes") == "0")
back(app)
click(app, "Likep-small")
local stats = app.client:stats { "p-small" }
assert(stats["p-small"].likes == 2 and stats["p-small"].liked, "Timeline likes reach the service")
click(app, "Repostp-small")
assert(app.client:stats({ "p-small" })["p-small"].reposted)

-- Signed-in readers land on home, with reposts from the people they follow.
assert(app.pivot.SelectedIndex == 0 and app.feed == "home")
assert(find(app, "RepostedByp-small") and find(app, "RepostedByp-arms"), "Reposts show who reposted them")

-- Connect lists interactions with your posts, newest first.
app:selectFeed("connect")
drain(app)
assert(app.pivot.SelectedIndex == 1 and find(app, "Activity1") and find(app, "Activity7"))
click(app, "Activity1")
assert(app.currentRoute == "/post" and app.post.id == "p-hello", "Activity opens the post it is about")
back(app)
app:selectFeed("home")

-- Following adds people to the home timeline.
click(app, "Authorp-small")
assert(find(app, "Countfollowers"):getTextContent() == "0")
click(app, "FollowButton")
assert(find(app, "Countfollowers"):getTextContent() == "1" and app.client:followState("u-jules"))
assert(not app.feeds.home, "Following reloads the home timeline")
back(app)
assert(find(app, "Timelinehome"):findChild("Postp-quote", true), "Home includes people you just followed")

-- Your own profile: edit and sign out.
click(app, "AppBarMe")
assert(find(app, "ProfileEditor") and find(app, "EditName").Text == "Ada Reef")
find(app, "EditBio").Text = "Testing the phone app"
click(app, "SaveProfile")
assert(find(app, "ProfileBio"):getTextContent() == "Testing the phone app")
assert(find(app, "Countposts"):getTextContent() == "2", "Own profile post count: " .. find(app, "Countposts"):getTextContent())

-- A new profile photo replaces the bundled portrait, on the profile and posts.
local photoBytes = assert(app.client:getImage("f-reef"))
local previousPhoto = assert(find(app, "ProfileAvatar"):findChild("AvatarPhoto", true)).Src
app.media.pick = function() return nil, { code = "cancelled", message = "Cancelled" } end
click(app, "ChangePhoto")
assert(find(app, "ProfileAvatar"):findChild("AvatarPhoto", true).Src == previousPhoto,
    "Cancelling keeps the current photo")
app.media.pick = function() return { jpeg = photoBytes } end
click(app, "ChangePhoto")
local adaPhoto = app.media:path("avatar:" .. assert(app.client:getProfile("u-ada").avatarId))
assert(adaPhoto and find(app, "ProfileAvatar"):findChild("AvatarPhoto", true).Src == adaPhoto,
    "The uploaded photo shows on your profile")

-- Deleting needs a second tap.
click(app, "AppBarHome")
click(app, "Open" .. published)
assert(find(app, "PostAuthor"):findChild("AvatarPhoto", true).Src == adaPhoto, "Posts show the author's photo")
click(app, "DeletePost")
assert(app.client:getPost(published), "The first tap only asks for confirmation")
click(app, "DeletePost")
local _, err = app.client:getPost(published)
assert(err.status == 404 and app.currentRoute ~= "/post", "The second tap deletes the post")

click(app, "AppBarMe")
click(app, "SignOut")
assert(not app.client:getSession() and app.currentRoute == "/" and find(app, "SignInhome"))

-- Sign up creates the account and its public profile.
local fresh = newApp()
click(fresh, "AppBarMe")
click(fresh, "MeSignUp")
find(fresh, "DisplayName").Text = "Pat Example"
find(fresh, "AccountName").Text = "new-octopus"
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

-- All four composers use independent fields and drafts, including optional quote photos.
local writer = newApp()
assert(writer.client:signIn("demo@example.invalid", "password123"))
writer:show("/write")
drain(writer)
local fields = { "PostTitle", "PostContent", "PhotoCaption", "QuoteText", "QuoteSource", "QuoteThoughts",
    "LinkTitle", "LinkUrl", "LinkDescription", "ChoosePhoto" }
local function form(expected)
    for _, name in ipairs(fields) do
        assert((writer.screen:findChild(name, true) ~= nil) == (expected[name] == true), "Wrong field: " .. name)
    end
    assert(find(writer, "PostTags") and find(writer, "Publish"))
end
form { PostTitle = true, PostContent = true }
for _, size in ipairs { { 320, 640 }, { 390, 844 }, { 768, 1024 } } do
    writer.screen.Width, writer.screen.Height = size[1], size[2]
    writer.screen:UpdateLayout(size[1], size[2])
    for _, kind in ipairs { "text", "photo", "quote", "link" } do
        local tile = find(writer, "Kind" .. kind)
        assert(tile.ActualWidth > 100 and tile.ActualHeight >= 80, "Post types must have usable touch targets")
    end
    assert(find(writer, "PostTitle").ActualWidth > 250, "Composer fields adapt to the viewport")
end
find(writer, "PostTitle").Text = "A text draft"
find(writer, "PostContent").Text = "Text stays here."
find(writer, "PostTags").Text = "writing"
click(writer, "Kindlink")
form { LinkTitle = true, LinkUrl = true, LinkDescription = true }
assert(find(writer, "LinkTitle").Text == "" and find(writer, "LinkDescription").Text == ""
    and find(writer, "PostTags").Text == "", "A new type starts with its own empty draft")
find(writer, "LinkTitle").Text = "A link draft"
find(writer, "LinkUrl").Text = "javascript:alert(1)"
click(writer, "Publish")
assert(find(writer, "WriteStatus"):getTextContent() == "URL must start with http:// or https://")
find(writer, "LinkUrl").Text = "https://example.com"
find(writer, "LinkDescription").Text = "A useful read."
click(writer, "Kindphoto")
form { ChoosePhoto = true, PhotoCaption = true }
click(writer, "Publish")
assert(find(writer, "WriteStatus"):getTextContent() == "Choose a photo to share.")
find(writer, "PhotoCaption").Text = "A photo draft"
writer.media.pick = function() return nil, { code = "cancelled", message = "Cancelled" } end
click(writer, "ChoosePhoto")
assert(not isShown(find(writer, "PhotoPreview")) and find(writer, "PhotoCaption").Text == "A photo draft"
    and find(writer, "WriteStatus"):getTextContent() == "", "Cancelling preserves the form without an error")
writer.media.pick = function() return nil, { code = "unsupported", message = "Photo picker unavailable." } end
click(writer, "ChoosePhoto")
assert(find(writer, "WriteStatus"):getTextContent() == "Photo picker unavailable.")
local imageBytes = assert(writer.client:getImage("f-reef"))
writer.media.pick = function() return { jpeg = imageBytes } end
click(writer, "ChoosePhoto")
assert(isShown(find(writer, "PhotoPreview")) and find(writer, "RemovePhoto"))
local photoDraft = writer.composeDraft
click(writer, "Kindquote")
form { QuoteText = true, QuoteSource = true, QuoteThoughts = true, ChoosePhoto = true }
assert(find(writer, "QuoteText").Text == "" and not isShown(find(writer, "QuotePhotoPreview")),
    "Photo captions and selections never leak into quotes")
find(writer, "QuoteText").Text = "Stay curious."
find(writer, "QuoteSource").Text = "A friend"
find(writer, "QuoteThoughts").Text = "A good reminder."
click(writer, "ChoosePhoto")
assert(isShown(find(writer, "QuotePhotoPreview")))
click(writer, "RemovePhoto")
assert(not isShown(find(writer, "QuotePhotoPreview")) and find(writer, "QuoteText").Text == "Stay curious.")
click(writer, "ChoosePhoto")
click(writer, "Kindtext")
assert(find(writer, "PostTitle").Text == "A text draft" and find(writer, "PostContent").Text == "Text stays here."
    and find(writer, "PostTags").Text == "writing", "Every field survives switching types")
click(writer, "AppBarHome")
click(writer, "AppBarWrite")
assert(find(writer, "PostContent").Text == "Text stays here.", "Drafts survive leaving the composer")
click(writer, "Publish")
assert(writer.currentRoute == "/post" and writer.post.postType == "text" and not writer.post.imageId)
back(writer)
click(writer, "AppBarWrite")
assert(find(writer, "PostContent").Text == "", "Only the published type is cleared")
click(writer, "Kindlink")
assert(find(writer, "LinkTitle").Text == "A link draft" and find(writer, "LinkDescription").Text == "A useful read.")
click(writer, "Publish")
assert(writer.post.postType == "link" and writer.post.linkUrl == "https://example.com" and not writer.post.imageId)
back(writer)
click(writer, "AppBarWrite")
click(writer, "Kindphoto")
assert(writer.composeDraft == photoDraft and isShown(find(writer, "PhotoPreview")))
local upload = writer.client.uploadImage
writer.client.uploadImage = function() return nil, { code = "network", message = "Upload failed." } end
click(writer, "Publish")
assert(writer.currentRoute == "/write" and find(writer, "WriteStatus"):getTextContent() == "Upload failed. Your draft is safe."
    and writer.composeDraft.photo and find(writer, "PhotoCaption").Text == "A photo draft"
    and not find(writer, "Publish").IgnoreHitTest, "Upload failures keep the complete draft and allow retry")
writer.client.uploadImage = upload
click(writer, "Publish")
writer.screen:UpdateLayout(writer.screen.Width, writer.screen.Height)
writer:afterPaint()
drain(writer)
assert(writer.post.postType == "photo" and writer.post.content == "A photo draft" and find(writer, "Image" .. writer.post.id))
back(writer)
click(writer, "AppBarWrite")
click(writer, "Kindquote")
assert(find(writer, "QuoteText").Text == "Stay curious." and isShown(find(writer, "QuotePhotoPreview")))
click(writer, "Publish")
writer.screen:UpdateLayout(writer.screen.Width, writer.screen.Height)
writer:afterPaint()
drain(writer)
assert(writer.post.postType == "quote" and writer.post.quoteSource == "A friend" and writer.post.userText == "A good reminder."
    and writer.post.imageId and find(writer, "Image" .. writer.post.id), "Quote photos are published and displayed")

-- A pending publish belongs to its draft, even after leaving and returning to it.
back(writer)
click(writer, "AppBarWrite")
click(writer, "Kindtext")
find(writer, "PostTitle").Text = "The submitted title"
find(writer, "PostContent").Text = "The submitted body"
local inline, pending = writer.task, {}
writer.task = function(self, fn) pending[#pending + 1] = fn end
click(writer, "Publish")
assert(find(writer, "Publish").IgnoreHitTest and find(writer, "Publish").Opacity == 0.5)
find(writer, "PostContent").Text = "Newer edits"
click(writer, "Kindlink")
find(writer, "LinkTitle").Text = "A different draft"
click(writer, "Kindtext")
click(writer, "Publish")
assert(#pending == 1 and find(writer, "Publish").IgnoreHitTest, "Revisiting a pending draft cannot publish it twice")
click(writer, "Kindlink")
writer.task = inline
pending[1]()
drain(writer)
assert(writer.currentRoute == "/write" and writer.composeDraft.postType == "link"
    and find(writer, "LinkTitle").Text == "A different draft", "Finishing a publish preserves the current form")
click(writer, "Kindtext")
assert(find(writer, "PostContent").Text == "Newer edits" and not find(writer, "Publish").IgnoreHitTest,
    "Edits made while publishing remain in the draft")
local sent = assert(writer.client:listPosts { limit = 1 })[1]
assert(sent.title == "The submitted title" and sent.content == "The submitted body", "Publish uses a snapshot of the form")

-- Work queued by a previous session must not publish after signing in again.
local before = #assert(writer.client:listPosts { limit = 100 })
pending = {}
writer.task = function(self, fn) pending[#pending + 1] = fn end
click(writer, "Publish")
assert(#pending == 1)
writer.task = inline
assert(writer.client:signOut())
assert(writer.client:signIn("demo@example.invalid", "password123"))
writer:resetSessionViews()
writer:show("/write")
pending[1]()
drain(writer)
assert(#assert(writer.client:listPosts { limit = 100 }) == before and find(writer, "PostContent").Text == "",
    "Signing in again starts fresh forms and cancels the previous session's queued publish")

-- Suspended fetches leave navigation and existing content available.
local asyncInline, requests = orca.async, {}
local delayed = App { client = Octopus.mock { clock = function() return NOW end } }
local task = delayed.task
delayed.task = function(self, fn)
    orca.async = function(work) requests[#requests + 1] = coroutine.create(work) end
    task(self, fn)
    orca.async = asyncInline
end
delayed.activate_controller = function(self, controller) self.screen = controller.view; return controller end
delayed.repaint = function() end
local listPosts, failRefresh = delayed.client.listPosts, false
delayed.client.listPosts = function(self, query)
    coroutine.yield()
    if failRefresh then return nil, { message = "Temporarily offline" } end
    return listPosts(self, query)
end
local stats, failStats = delayed.client.stats, false
delayed.client.stats = function(self, ...)
    coroutine.yield()
    if failStats then return nil, { message = "Counts unavailable" } end
    return stats(self, ...)
end
local function step(request)
    local ok, err = coroutine.resume(request)
    assert(ok, err)
end
delayed:show("/")
local firstRequest = requests[1]
assert(delayed.screen and find(delayed, "Loadingdiscover").Visible and #requests == 1,
    "The shell and indicator must exist before the fetch begins")
delayed.screen:UpdateLayout(390, 844)
local loading = find(delayed, "Loadingdiscover")
local indicator = find(delayed, "LoadingdiscoverIndicator")
assert(loading.IgnoreHitTest and indicator.IgnoreHitTest
    and indicator.ActualWidth > 0 and indicator.ActualHeight > 0
    and math.abs(indicator.ActualX + indicator.ActualWidth / 2 - loading.ActualWidth / 2) < 1
    and math.abs(indicator.ActualY + indicator.ActualHeight / 2 - loading.ActualHeight / 2) < 1,
    "The loading indicator is centered without intercepting navigation")
step(firstRequest)
click(delayed, "AppBarSearch")
local searchScreen = delayed.screen
assert(delayed.currentRoute == "/search" and find(delayed, "SearchInput"), "Navigation works while a fetch is suspended")
step(firstRequest)
assert(delayed.screen == searchScreen and delayed.currentRoute == "/search",
    "A finished timeline fetch cannot navigate away from the current page")
delayed:show("/")
local retainedScreen, retainedPost = delayed.screen, find(delayed, "Postp-reef")
local firstStats = requests[#requests]
assert(delayed.feeds.discover.stats["p-reef"].pending and find(delayed, "Likep-reef").IgnoreHitTest,
    "Feed cards appear before counters, with unknown reader actions disabled")
step(firstStats)
assert(delayed.feeds.discover.stats["p-reef"].pending, "A yielding stats request never hides the feed")
step(firstStats)
assert(not delayed.feeds.discover.stats["p-reef"].pending and not find(delayed, "Likep-reef").IgnoreHitTest,
    "Counters enable reactions only after reader state arrives")
failRefresh = true
delayed:refresh()
local refreshRequest = requests[#requests]
step(refreshRequest)
assert(delayed.screen == retainedScreen and find(delayed, "Postp-reef") == retainedPost
    and find(delayed, "Loadingdiscover").Visible, "Refreshing keeps the existing feed visible")
step(refreshRequest)
assert(find(delayed, "Postp-reef") == retainedPost and not find(delayed, "Loadingdiscover").Visible
    and find(delayed, "HomeStatus"):getTextContent() == "Temporarily offline",
    "A refresh error preserves content and leaves refresh available")
failRefresh = false
delayed:refresh()
refreshRequest = requests[#requests]
step(refreshRequest); step(refreshRequest)
assert(find(delayed, "Postp-reef") == retainedPost and not find(delayed, "Loadingdiscover").Visible,
    "A successful feed refresh updates the collection without replacing retained posts")
local failedStats = requests[#requests]
failStats = true
step(failedStats); step(failedStats)
assert(find(delayed, "Postp-reef") == retainedPost and delayed.feeds.discover.stats["p-reef"].pending
    and find(delayed, "Likep-reef").IgnoreHitTest, "Counter failure preserves content and blocks unsafe toggles")
failStats = false
-- Post counts populate while the comments section is still waiting.
local listComments = delayed.client.listComments
delayed.client.listComments = function(self, id) coroutine.yield(); return listComments(self, id) end
local hello
for _, post in ipairs(listPosts(delayed.client, { limit = 100 })) do if post.id == "p-hello" then hello = post end end
local threadStart = #requests + 1
local summary = {}
for key, value in pairs(assert(hello)) do summary[key] = value end
summary.content = "Only the feed excerpt"
delayed:openPost(summary)
drain(delayed)
local threadRequest = requests[threadStart + 2]
assert(delayed.thread.post.content == "Only the feed excerpt")
step(requests[threadStart])
assert(delayed.thread.post.content == hello.content and delayed.post == delayed.thread.post,
    "Full detail replaces the feed summary in both the displayed view and current post")
step(requests[threadStart + 1]); step(requests[threadStart + 1])
step(threadRequest)
assert(find(delayed, "Statlikes"):getTextContent() == "2" and find(delayed, "CommentsLoading"),
    "Post counts populate before comments finish loading")
step(threadRequest)
local retainedComment = find(delayed, "Commentc-welcome")
assert(retainedComment and not delayed.screen:findChild("CommentsLoading", true)
    and find(delayed, "LikeCommentc-welcome").IgnoreHitTest,
    "Comments appear before their like counters return, with unsafe reactions disabled")
step(threadRequest)
assert(find(delayed, "Commentc-welcome") == retainedComment and not find(delayed, "LikeCommentc-welcome").IgnoreHitTest,
    "Comment counter enrichment updates the retained row and enables reactions")
-- Search posts populate before the independent people lookup finishes.
delayed:show("/search")
local searchPeople = delayed.client.searchPeople
delayed.client.searchPeople = function(self, query) coroutine.yield(); return searchPeople(self, query) end
find(delayed, "SearchInput").Text = "reef"
local partialIndex = #requests + 1
click(delayed, "RunSearch")
local partialSearch = requests[partialIndex]
step(partialSearch); step(partialSearch)
assert(delayed.searchState.results and delayed.searchState.results.posts and find(delayed, "Searching").Visible,
    "Search posts populate while people are still loading")
step(partialSearch)
assert(not find(delayed, "Searching").Visible)
-- A failed people lookup must not remove the successful post section.
delayed.client.searchPeople = function()
    coroutine.yield()
    return nil, { message = "People lookup unavailable" }
end
find(delayed, "SearchInput").Text = hello.title
partialIndex = #requests + 1
click(delayed, "RunSearch")
partialSearch = requests[partialIndex]
step(partialSearch); step(partialSearch)
local partialPost = find(delayed, "Post" .. hello.id)
step(partialSearch)
assert(find(delayed, "Post" .. hello.id) == partialPost
    and delayed.searchState.results.peopleError.message == "People lookup unavailable"
    and find(delayed, "RetryPeopleSearch") and not find(delayed, "Searching").Visible,
    "People lookup failure keeps the rendered posts and offers retry")
delayed.client.searchPeople = searchPeople
partialIndex = #requests + 1
click(delayed, "RetryPeopleSearch")
partialSearch = requests[partialIndex]
step(partialSearch); step(partialSearch)
assert(find(delayed, "Post" .. hello.id) and not delayed.searchState.results.peopleError
    and not delayed.screen:findChild("PeopleSearchError", true), "Retry clears the people error while retaining post results")
-- Clearing a search invalidates its in-flight response.
delayed:show("/search")
find(delayed, "SearchInput").Text = "reef"
local searchIndex = #requests + 1
click(delayed, "RunSearch")
local searchRequest = requests[searchIndex]
step(searchRequest)
assert(find(delayed, "Searching").Visible)
find(delayed, "SearchInput").Text = ""
click(delayed, "RunSearch")
step(searchRequest)
assert(not delayed.searchState.results and not find(delayed, "Searching").Visible,
    "A cleared query cannot be replaced by an older response")
-- Profile posts do not wait for metadata/count requests, and stale metadata
-- cannot overwrite a profile after navigation has changed.
local getProfile = delayed.client.getProfile
delayed.client.getProfile = function(self, id) coroutine.yield(); return getProfile(self, id) end
local profileStart = #requests + 1
delayed:openProfile("u-sam", "Sam Rivera")
drain(delayed)
local profileView = delayed.profileView
step(requests[profileStart])
step(requests[profileStart + 2])
assert(find(delayed, "ProfilePosts"):findChild("Postp-reef", true) and not profileView.profile,
    "Profile timelines paint while the metadata request is still suspended")
delayed:show("/search")
local afterProfile = delayed.screen
step(requests[profileStart])
assert(delayed.screen == afterProfile and not profileView.profile,
    "A late profile response cannot update a page after navigating away")
delayed.client.getProfile = getProfile
-- Late counter results from a signed-out reader cannot alter a new shell.
delayed:show("/")
delayed:refresh()
local staleFeed = requests[#requests]
step(staleFeed); step(staleFeed)
local staleStats = requests[#requests]
step(staleStats)
local previousEntries = delayed.feeds.discover.stats
delayed:resetSessionViews()
delayed:show("/")
step(staleStats)
assert(previousEntries["p-reef"].pending and delayed.feeds.discover.stats ~= previousEntries,
    "Session reset discards an already suspended enrichment result")
orca.async = asyncInline

-- Disk snapshots paint before the initial network request and survive failure.
local cacheClient = Octopus.mock { clock = function() return NOW end }
local freshPosts = assert(cacheClient:listPosts { limit = 20 })
local cachedItems = require("Octopus/views/posts").items { freshPosts[1], freshPosts[2] }
cacheClient.readFeedCache = function() return cachedItems, "cached-cursor" end
cacheClient.listPosts = function() return nil, { message = "Offline with cache" } end
local cachedApp = App { client = cacheClient }
cachedApp.activate_controller = function(self, controller) self.screen = controller.view; return controller end
cachedApp.repaint = function() end
cachedApp:show("/")
drain(cachedApp)
assert(find(cachedApp, "Post" .. cachedItems[1].post.id) and #cachedApp.feeds.discover.items == 2
    and not find(cachedApp, "Loadingdiscover").Visible,
    "An initial refresh failure keeps the cached discover page readable")
cacheClient.listPosts = function() return freshPosts end
cachedApp:refresh()
assert(#cachedApp.feeds.discover.items == #freshPosts,
    "A successful refresh replaces cached items without duplicating rows")

print("Octopus UI: guest discover/post/profile/search, home with reposts, connect, sign-in return, all four compose forms, independent drafts, photos, upload retry, pending publishes, comments and replies, likes, reposts, follows, profile editing, delete confirmation, sign-up, delayed fetches, retained refresh and errors passed")
