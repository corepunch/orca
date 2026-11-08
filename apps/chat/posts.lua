-- Octopus is a feature controller inside Chat's navigation, not a second app.
-- Its native XML views, data scopes and per-kind drafts remain the same ones
-- used by the website-compatible standalone client.
local ui = require "orca.UIKit"
local filesystem = require "orca.filesystem"
local Octopus = require "Octopus/api/octopus"
local OctopusApp = require "Octopus/App"
local Home = require "Octopus/views/home"
local Thread = require "Octopus/views/thread"
local People = require "Octopus/views/people"
local Compose = require "Octopus/views/compose"
local ChatColors = require "Chat/theme"
local PostColors = require "Octopus/theme"
local I18n = require "Chat/i18n"
local Posts = {}
Posts.__index = Posts

local ROUTES = { ["/"] = "/posts", ["/search"] = "/posts/search", ["/write"] = "/posts/write",
    ["/post"] = "/posts/post", ["/user"] = "/posts/user", ["/me"] = "/posts/me" }
local INTERNAL = {}
for path, route in pairs(ROUTES) do INTERNAL[route] = path end

local function userName(user)
    return user.name or user.user_metadata and user.user_metadata.display_name or user.email or user.id
end

-- Demo identities agree across the two offline providers, just as Appwrite
-- account IDs do in production. Fixture remapping never affects live data.
local function mockClient(chat)
    local session = chat.client:getSession()
    local ownId = session and session.user.id or "me"
    local ownName = session and userName(session.user) or "You"
    local ids = { ["u-ada"] = ownId, ["u-maya"] = "maya", ["u-jules"] = "jules", ["u-sam"] = "alex", ["u-noor"] = "noor" }
    local function copy(value)
        if type(value) ~= "table" then return ids[value] or value end
        local result = {}
        for key, item in pairs(value) do result[key] = copy(item) end
        return result
    end
    local fixtures = copy(require "Octopus/api/fixtures")
    fixtures.users[1].name, fixtures.users[4].name = ownName, "Alex Morgan"
    for _, rows in pairs(fixtures.collections) do
        for _, row in ipairs(rows) do
            if row.authorId == ownId then row.authorName = ownName end
            if row.authorId == "alex" then row.authorName = "Alex Morgan" end
        end
    end
    local client = Octopus.mock { clock = chat.client.clock, fixtures = fixtures }
    if chat.client:getSession() then assert(client:signIn("demo@example.invalid", "password123")) end
    return client
end

local function clientFor(chat)
    if chat.client.mode == "mock" then return mockClient(chat) end
    local config = require "Octopus/config"
    local chatConfig = require "Chat/config"
    return assert(Octopus.client { mode = chat.client.mode, projectId = chatConfig.projectId,
        databaseId = config.databaseId, bucketId = config.bucketId, feedCollectionId = config.feedCollectionId,
        functionId = config.functionId, clock = chat.client.clock,
        transport = function(request)
            local response, err = chat.client:publicPostsRequest(request)
            return response or { status = err and err.status or 502,
                body = err or { code = "network", message = "Could not load posts. Try again." } }
        end })
end

local function palette()
    -- Public posts use Chat's native appearance/accent while retaining their
    -- reaction colours. The standalone Octopus process still owns its theme.
    for _, key in ipairs { "appearance", "background", "surface", "raised", "chrome", "textbox", "textboxInk",
        "placeholder", "ink", "muted", "dim", "faint", "accent", "accentText", "onAccent", "soft", "error" } do
        PostColors[key] = ChatColors[key]
    end
end

local function visit(node, fn)
    fn(node)
    local child = node:getFirstChild()
    while child do visit(child, fn); child = child:getNext() end
end

function Posts.new(chat, options)
    options = options or {}
    palette()
    local client = options.client or clientFor(chat)
    local feature = setmetatable({ chat = chat, client = client, options = options }, Posts)
    feature:syncSession()
    local saveProfile = client.updateProfile
    feature.saveProfile = saveProfile
    client.updateProfile = function(current, values)
        local before, epoch = chat.client:getSession(), chat.sessionEpoch
        if not before then return nil, { code = "unauthenticated", message = "Sign in to continue." } end
        local profile, err = saveProfile(current, values)
        if not profile then return nil, err end
        local active = chat.client:getSession()
        if feature.disposed or chat.sessionEpoch ~= epoch or not active or active.user.id ~= before.user.id then
            return nil, { code = "stale", message = "The account changed while saving. Sign in again to continue." }
        end
        local shared
        shared, err = chat.client:updateProfile { display_name = profile.username }
        if not shared then
            return nil, { code = "profile_sync", status = err and err.status,
                message = "Your public profile was saved, but Chat could not update your name. Retry Save to finish syncing." }
        end
        feature:identityChanged()
        return profile
    end
    local app = OctopusApp { client = client, media = options.media, language = options.language or I18n.language }
    feature.app = app
    app.feedOrder = { "home", "discover", "connect" }
    -- Root ownership and all navigation remain with Chat. The small adapter
    -- supports Octopus's retained Home contract without creating another bar.
    app.shell = { pages = { Home = ui.Page {}, Search = ui.Page {}, Write = ui.Page {}, Me = ui.Page {} } }
    app.shell.has = function(_, key) return key == "Home" and feature.home ~= nil end
    app.shell.show = function(_, key, content) return feature:tab(key, content) end
    app.shell.replace = app.shell.show
    app.frame = function(_, content, tab)
        if tab then return feature:tab(tab, content) end
        local screen = chat:frame()
        screen.StyleSheet = "Chat/Styles/posts.css"
        screen.StateClasses = "chat-" .. ChatColors.appearance .. " octopus-" .. ChatColors.appearance .. " chat-posts"
        app.screen = screen
        return screen
    end
    app.tab = function(_, key, content) return feature:tab(key, content) end
    app.show = function(_, route, direction)
        if route == "/auth" or route == "/auth/signup" then
            local state = { route = ROUTES[app.currentRoute] or "/posts", post = app.post,
                profileId = app.profileId, profileName = app.profileName, query = app.searchState.query, feed = app.feed }
            chat.publicAuthResume = function(parent) parent:posts():resume(state) end
            return chat:show(route, direction)
        end
        return chat:show(ROUTES[route] or route, direction)
    end
    app.repaint = function() chat:repaint() end
    app.feedStatus = function(_, feed, query)
        if app.currentRoute == "/" and app.feed == feed and query.mode == nil then
            app:notice(query.error and query.error.message or "", query.error ~= nil)
        end
    end
    app.messagePerson = function(_, id, name) return chat:messagePerson(id, name) end
    app.sharePost = function(_, post) return chat:sharePost(post) end
    app.signOut = function() return chat:signOut() end
    local avatarChanged = app.avatarChanged
    app.avatarChanged = function(current, id)
        avatarChanged(current, id)
        if chat.client.avatarIds then chat.client.avatarIds[id] = nil end
        if chat.avatars then chat.avatars:invalidate(id) end
        feature:identityChanged()
    end
    return feature
end

function Posts:syncSession()
    if self.client.mode == "mock" then return end
    local session = self.chat.client:getSession()
    if not session then self.client.session = nil; return end
    local current = self.client.session
    if not current or current.user.id ~= session.user.id then current = { user = { id = session.user.id } } end
    current.user.name, current.user.email = userName(session.user), session.user.email
    self.client.session = current
end

-- Both retained identity pages must rebuild after edits in the public profile.
function Posts:identityChanged()
    local chat = self.chat
    chat.settingsScreen, chat.profileStatus = nil, nil
    chat.meContent, chat.meStatus = nil, nil
    local page = chat.shell and chat.shell.pages.Me
    if page then
        while page:getFirstChild() do page:getFirstChild():removeFromParent() end
    end
end

function Posts:avatarChanged(id)
    if self.client.avatarIds then self.client.avatarIds[id] = nil end
    if self.client.profiles then self.client.profiles[id] = nil end
    self.app:avatarChanged(id)
end

function Posts:decorate(content)
    content.StyleSheet = "Chat/Styles/posts.css"
    content.StateClasses = (content.className == "Screen" and "chat-" .. ChatColors.appearance .. " " or "")
        .. "chat-posts octopus-" .. ChatColors.appearance
    visit(content, function(node)
        if node.className == "Pivot" then node.Title = "Messenger" end
    end)
    local publish = content:findChild("Publish", true)
    if publish then
        publish:attachPropertyProgram("TextResourceID", 'IF({DataContext/Draft/Status/IsSubmitting}, "Publishing…", "Publish publicly")', "OneWay", true)
    end
    return content
end

function Posts:tab(key, content)
    local app, chat = self.app, self.chat
    if key == "Home" then
        if not self.home then
            self.home = assert(filesystem.loadObject("Chat/Pages/Posts"))
            self.home:findChild("PostsContent"):addChild(content)
            self.home:findChild("NewPublicPost", true).Click = function() chat:show("/posts/write", "Forward") end
            self.home:findChild("SearchPublicPosts", true).Click = function() chat:show("/posts/search", "Forward") end
            self:decorate(self.home)
            local pivot = content:findChild("Timeline", true)
            -- Service/query names stay unchanged; these are presentation labels.
            content:findChild("Feedhome", true).Header = I18n:t("following")
            content:findChild("Feedconnect", true).Header = I18n:t("activity")
            content:findChild("Feeddiscover", true).Header = I18n:t("discover")
            app.pivot = pivot
        end
        local screen = chat:frame(self.home, "Posts")
        screen.StyleSheet = "Chat/Styles/posts.css"
        app.screen = screen
        return screen
    end
    self:decorate(content)
    -- Search, profile editing and creation are focused tasks with header Back.
    local back = key == "Me" and "/me" or "/posts"
    visit(content, function(node)
        if node.className == "Pivot" then
            node.BackIcon = "Chat/assets/icons/arrow-left.svg"
            node.BackRequested = function() chat:goBack(back) end
        end
    end)
    local screen = app:frame()
    screen:addChild(content)
    app.screen = screen
    return screen
end

function Posts:page(route)
    self:syncSession()
    palette()
    local app = self.app
    local path = INTERNAL[route] or "/"
    app.currentRoute = path
    app.navigationEpoch = (app.navigationEpoch or 0) + 1
    local result
    if path == "/" then result = Home.page(app)
    elseif path == "/search" then result = People.search(app)
    elseif path == "/write" then result = Compose.page(app)
    elseif path == "/post" then result = Thread.page(app)
    elseif path == "/user" then result = People.profile(app)
    elseif path == "/me" then result = People.me(app) end
    if result then self:decorate(result) end
    self.chat.status = app.status
    local scope = path == "/" and app.scope or path == "/write" and app.composeScope
        or path == "/search" and app.searchScope or path == "/me" and app.meScope or app.transientScope
    if scope then scope:activate() end
    return result
end

function Posts:resume(state)
    local app = self.app
    app.post, app.profileId, app.profileName = state.post, state.profileId, state.profileName
    app.searchState.query, app.feed = state.query or "", state.feed or app.feed
    return self.chat:show(state.route or "/posts")
end

function Posts:openProfile(id, name)
    local app = self.app
    app.profileId, app.profileName = id, name
    app.profileReturn = nil
    if id == self.client:currentUserId() then return self.chat:show("/posts/me", "Forward") end
    return self.chat:show("/posts/user", "Forward")
end

function Posts:openPostId(id, title)
    local app = self.app
    app.post = { id = id, title = title or "", authorName = "", postType = "text", tags = {} }
    app.postReturn = "/"
    return self.chat:show("/posts/post", "Forward")
end

function Posts:leave()
    local app = self.app
    local leaving = app.leaving
    app.leaving = nil
    if leaving then leaving() end
    if app.transientScope then app.transientScope:dispose(); app.transientScope = nil end
    app:eachScope(function(scope) scope:deactivate() end)
end

function Posts:dispose()
    self.disposed = true
    self:leave()
    self.app.sessionEpoch = self.app.sessionEpoch + 1
    self.app:eachScope(function(scope) scope:dispose() end)
    if self.app.media and self.app.media.clear then self.app.media:clear() end
    self.client.session = nil
    self.client.updateProfile = self.saveProfile
end

function Posts:refresh() return self.app:refresh() end
function Posts:afterPaint() self.app:afterPaint() end
function Posts:updateAppearance()
    palette()
    if self.home then self:decorate(self.home) end
    if self.chat.currentRoute and self.chat.currentRoute:match("^/posts") and self.chat.screen then
        self:decorate(self.chat.screen)
    end
end

return Posts
