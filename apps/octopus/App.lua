local ui = require "orca.UIKit"
local orca = require "orca"
local Application = require "orca.core.application"
local Octopus = require "Octopus/api/octopus"
local system = require "orca.system"
local renderer = require "orca.renderer"
local C = require "Octopus/views/components"
local Shell = require "Octopus/views/shell"
local Posts = require "Octopus/views/posts"
local Thread = require "Octopus/views/thread"
local People = require "Octopus/views/people"
local Compose = require "Octopus/views/compose"
local Identity = require "Octopus/views/identity"
local colors = C.colors

local FEEDS = { "discover", "following" }
local PAGE_SIZE = 20

local App = Application:extend {
    layout = function(self, ctx) return ctx.slots.inner end,

    __init = function(self, options)
        Application.__init(self)
        options = options or {}
        local config = require "Octopus/config"
        local backend = config.backend or "appwrite"
        if options.client then self.client = options.client
        elseif backend == "appwrite" then self.client, self.configurationError = Octopus.appwrite(config)
        elseif backend == "mock" then self.client = Octopus.mock()
        else self.configurationError = { message = "Choose OCTOPUS_BACKEND=appwrite or mock." } end
        self.clock = self.client and self.client.clock or os.time
        self.media = options.media or require("Octopus/media").new()
        self.remote = self.client ~= nil and self.client.mode ~= "mock"
        self:resetSessionViews()
    end,

    -- Everything tied to the reader. Guests and each signed-in account get a
    -- fresh shell, timelines and drafts.
    resetSessionViews = function(self)
        self.sessionEpoch = (self.sessionEpoch or 0) + 1
        self.shell, self.status, self.feeds, self.timelines, self.pivot = nil, nil, {}, {}, nil
        self.searchState, self.composeDraft, self.commentDrafts = { query = "" }, { postType = "text" }, {}
        self.thread, self.profileView, self.composeView = nil, nil, nil
        self.feed = "discover"
    end,

    -- Run work that may wait on the network in an ORCA coroutine.
    task = function(self, fn)
        orca.async(function()
            local ok, err = pcall(fn)
            if not ok then
                io.stderr:write("octopus: " .. tostring(err) .. "\n")
                self:notice("Something went wrong. Please try again.", true)
            end
        end)
    end,

    repaint = function(self)
        if self.screen then pcall(function() self.screen:post("Window.Paint", renderer.getSize()) end) end
    end,

    frame = function(self, content, tab)
        if tab then
            if not self.shell then
                local menu = { { "AppBarRefresh", "refresh", function() self:refresh() end } }
                if self.client and self.client:getSession() then
                    menu[#menu + 1] = { "AppBarSignOut", "sign out", function() self:signOut() end }
                else
                    menu[#menu + 1] = { "AppBarSignIn", "sign in", function() self:show("/auth") end }
                    menu[#menu + 1] = { "AppBarSignUp", "create account", function() self:show("/auth/signup") end }
                end
                self.shell = Shell.new(self:frame(), function(route) self:show(route) end, menu)
            end
            return self.shell:show(tab, content)
        end
        local screen = ui.Screen {
            Width = 390, Height = 844, ResizeMode = PLATFORM == "ios" and "CanResize" or "NoResize",
            ClearColor = colors.background, StyleSheet = "Octopus/Styles/app.css",
        }
        if content then screen:addChild(content) end
        return screen
    end,

    -- Show a root page whose content is rebuilt from state on each visit.
    tab = function(self, key, content)
        if self.shell and self.shell:has(key) then return self.shell:replace(key, content) end
        return self:frame(content, key)
    end,

    show = function(self, route)
        -- Pages with inputs keep their drafts when you navigate away.
        local leaving = self.leaving
        self.leaving = nil
        if leaving then leaving() end
        self.navigationEpoch = (self.navigationEpoch or 0) + 1
        local epoch = self.navigationEpoch
        local controller = self:dispatch(route or "/")
        if self.navigationEpoch == epoch then return self:activate_controller(controller) end
    end,

    notice = function(self, text, isError)
        if self.status then
            self.status:setTextContent(text or "")
            self.status.Foreground = isError and colors.error or colors.dim
        end
    end,

    -- Guests can read everything; writing asks them to sign in and come back.
    requireSession = function(self, reason)
        if self.client and self.client:getSession() then return true end
        self.authReason, self.authReturn = reason, self.currentRoute
        self:show("/auth")
        return false
    end,

    openPost = function(self, post)
        self.post, self.postReturn = post, self.currentRoute
        self:show("/post")
    end,

    openProfile = function(self, userId, name)
        if self.client:getSession() and userId == self.client:currentUserId() then return self:show("/me") end
        self.profileId, self.profileName = userId, name
        self.profileReturn = self.currentRoute ~= "/user" and self.currentRoute or self.profileReturn
        self:show("/user")
    end,

    openTag = function(self, tag)
        self.searchState.query, self.searchState.results = "#" .. tag, nil
        self:show("/search")
    end,

    -- Drop cached timelines and pages so the next visit reloads them.
    invalidate = function(self)
        self.feeds, self.searchState.results = {}, nil
        if self.shell then for _, page in ipairs(Shell.pages) do C.clear(self.shell.pages[page.key]) end end
    end,

    refresh = function(self)
        local route = self.currentRoute or "/"
        self:invalidate()
        self:show(route)
    end,

    signOut = function(self)
        self:task(function()
            local ok, err = self.client:signOut()
            self:resetSessionViews(); self:show("/")
            if not ok then self:notice("Signed out on this device. " .. err.message, true) end
        end)
    end,

    -- Home --------------------------------------------------------------------

    loadFeed = function(self, name, more)
        local feed = self.feeds[name] or { posts = {}, stats = {} }
        self.feeds[name] = feed
        local list = self.timelines[name]
        if not list or feed.loading or (more and feed.done) then return end
        feed.loading, feed.error = true, nil
        local session = self.sessionEpoch
        if not more then
            C.clear(list)
            list:addChild(C.text("loading…", { Name = "Loading", FontSize = 22, Foreground = colors.muted }))
        end
        self:task(function()
            local cursor = more and feed.posts[#feed.posts] and feed.posts[#feed.posts].id or nil
            local posts, stats = Posts.fetch(self, function()
                if name == "following" then return self.client:homeTimeline { limit = PAGE_SIZE, cursor = cursor } end
                return self.client:listPosts { limit = PAGE_SIZE, cursor = cursor }
            end)
            feed.loading = false
            if self.sessionEpoch ~= session or self.timelines[name] ~= list then return end
            if not more then C.clear(list) end
            local loadMore = list:findChild("LoadMore" .. name)
            if loadMore then loadMore:removeFromParent() end
            if not posts then
                feed.error = stats
                list:addChild(C.empty("can't load posts", stats.message .. " Use refresh to try again."))
                self:notice(stats.message, true)
                self:repaint()
                return
            end
            for id, entry in pairs(stats) do feed.stats[id] = entry end
            for _, post in ipairs(posts) do feed.posts[#feed.posts + 1] = post end
            feed.done = #posts < PAGE_SIZE
            Posts.append(self, list, posts, feed.stats)
            if #feed.posts == 0 then
                if name == "following" then
                    list:addChild(C.empty("your timeline is quiet",
                        "Follow people from their profile or search to see their posts here."))
                else
                    list:addChild(C.empty("no posts yet", "Be the first to write something."))
                end
            elseif not feed.done then
                list:addChild(C.button("load more posts", function() self:loadFeed(name, true) end, {
                    Name = "LoadMore" .. name, FontSize = 16 }))
            end
            self:notice(self.remote and "" or "Offline mock · posts stay in this session")
            self:repaint()
        end)
    end,

    selectFeed = function(self, name)
        self.feed = name
        if name == "following" and not self.client:getSession() then return end
        if not self.feeds[name] then self:loadFeed(name) end
    end,

    home = function(self)
        if not self.client then
            return self:frame(C.page("connect", C.stack({ Spacing = 16 }, {
                C.text(self.configurationError.message, { WordWrap = true, Foreground = colors.error }),
                C.text("Check APPWRITE_ENDPOINT and APPWRITE_PROJECT_ID, then restart.", { WordWrap = true }),
            })))
        end
        if self.shell and self.shell:has("Home") then
            self.status = self.homeStatus
            local screen = self.shell:show("Home")
            self:selectFeed(self.feed)
            return screen
        end
        self.status = C.text("", { Name = "HomeStatus", FontSize = 11, Foreground = colors.dim, TextHorizontalAlignment = "Center" })
        self.homeStatus = self.status
        self.pivot = ui.Pivot { Name = "Timeline", Title = "Octopus", Foreground = colors.ink,
            SelectedIndex = self.feed == "following" and 1 or 0,
            SelectionChanged = function(pivot) self:selectFeed(FEEDS[pivot.SelectedIndex + 1]) end,
        }
        self.timelines = {}
        for _, name in ipairs(FEEDS) do
            local item = ui.PivotItem { Name = "Feed" .. name, Header = name }
            local list = C.stack({ Name = "Timeline" .. name, Spacing = 26, Overflow = "hidden scroll" })
            if name == "following" and not self.client:getSession() then
                list:addChild(C.stack({ Spacing = 16 }, {
                    C.empty("posts from people you follow", "Sign in to follow writers and build your own timeline."),
                    C.primary("Sign in", function() self.authReturn = "/"; self:show("/auth") end, { Name = "FollowingSignIn" }),
                    C.button("Create an account", function() self.authReturn = "/"; self:show("/auth/signup") end, {
                        Name = "FollowingSignUp" }),
                }))
            else
                self.timelines[name] = list
            end
            item:addChild(list)
            self.pivot:addChild(item)
        end
        local grid = ui.Grid { Rows = "1fr 20px", Background = colors.background }
        grid:addChild(self.pivot); grid:addChild(self.status)
        local screen = self:frame(grid, "Home")
        self:selectFeed(self.feed)
        return screen
    end,

    ["/"] = function(self) self.currentRoute = "/"; return self:home() end,
    ["/post"] = function(self) self.currentRoute = "/post"; return Thread.page(self) end,
    ["/user"] = function(self) self.currentRoute = "/user"; return People.profile(self) end,
    ["/me"] = function(self) self.currentRoute = "/me"; return People.me(self) end,
    ["/search"] = function(self) self.currentRoute = "/search"; return People.search(self) end,
    ["/write"] = function(self) self.currentRoute = "/write"; return Compose.page(self) end,
    ["/auth"] = function(self) self.currentRoute = "/auth"; return Identity.auth(self, "signin") end,
    ["/auth/signup"] = function(self) self.currentRoute = "/auth/signup"; return Identity.auth(self, "signup") end,

    run = function(self)
        self.screen:post("Window.Paint", renderer.getSize())
        for message in system.getMessage do
            if message:is("Window.Closed") then return end
            local ok, result = pcall(system.dispatchMessage, self.screen, message)
            if not ok then io.stderr:write(tostring(result) .. "\n")
            elseif result and not message:is("Window.Paint") then self.screen:post("Window.Paint", renderer.getSize()) end
            system.translateMessage(message)
        end
    end,
}

return App
