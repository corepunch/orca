local ui = require "orca.UIKit"
local orca = require "orca"
local Application = require "orca.core.application"
local Octopus = require "Octopus/api/octopus"
local system = require "orca.system"
local renderer = require "orca.renderer"
local C = require "Octopus/views/components"
local Shell = require "Octopus/views/shell"
local Home = require "Octopus/views/home"
local Thread = require "Octopus/views/thread"
local People = require "Octopus/views/people"
local Compose = require "Octopus/views/compose"
local Identity = require "Octopus/views/identity"
local AvatarLoader = require "orca.UIKit.AvatarLoader"
local PageTransition = require "orca.UIKit.PageTransition"
local data = require "orca.data"
local Visibility = require "orca.data.visibility"
local OctopusProvider = require "Octopus/providers/octopus"
local colors = C.colors


local App = Application:extend {
    layout = function(self, ctx) return ctx.slots.inner end,

    __init = function(self, options)
        Application.__init(self)
        options = options or {}
        local config = require "Octopus/config"
        local backend = config.backend or "appwrite"
        local cache
        if backend == "appwrite" and not options.client then
            cache = require("Octopus/cache").new {
                namespace = table.concat({ config.endpoint, config.projectId, config.databaseId, config.bucketId,
                    config.feedCollectionId or "posts" }, "|"),
            }
            config = setmetatable({ cache = cache, schedule = function(fn) orca.async(fn) end }, { __index = config })
        end
        if options.client then self.client = options.client
        elseif backend == "appwrite" then self.client, self.configurationError = Octopus.appwrite(config)
        elseif backend == "mock" then self.client = Octopus.mock()
        else self.configurationError = { message = "Choose OCTOPUS_BACKEND=appwrite or mock." } end
        self.clock = self.client and self.client.clock or os.time
        self.media = options.media or require("Octopus/media").new { cache = cache }
        self.publicCache = cache
        self.photos = require("Octopus/photo_loader").new(self)
        self.remote = self.client ~= nil and self.client.mode ~= "mock"
        self:resetSessionViews()
    end,

    -- Everything tied to the reader. Guests and each signed-in account get a
    -- fresh shell, timelines and drafts.
    resetSessionViews = function(self)
        self.sessionEpoch = (self.sessionEpoch or 0) + 1
        -- Each reader (a guest or an account) gets its own records.
        self:eachScope(function(scope) scope:dispose() end)
        self.reader, self.scope, self.transientScope, self.searchScope, self.meScope = nil, nil, nil, nil, nil
        self.visibility = Visibility.new()
        if self.media and self.media.clear then self.media:clear() end
        -- Persist only public guest images; authenticated media stays temporary.
        if self.media then
            if self.client and self.client:getSession() then self.media.cache = nil
            else self.media.cache = self.publicCache end
        end
        if self.avatars then self.avatars:clear() end
        self.avatarPaths = {}
        self.avatars = AvatarLoader.new(self, {
            lookup = function(ids) return self.client:avatars(ids) end,
            fetch = function(id) return self.client:getImage(id, "avatar") end,
            cached = function(id) return self.media:path("avatar:" .. id) end,
            store = function(bytes, id) return self.media:store(bytes, "avatar:" .. id) end,
            apply = C.photo,
            schedule = function(fn) self:task(fn) end,
            repaint = function() if self.screen then self.screen:post("Window.Paint", renderer.getSize()) end end,
        })
        self.shell, self.status, self.statusBar, self.pivot = nil, nil, nil, nil
        self.searchState, self.composeDraft, self.commentDrafts = { query = "" }, { postType = "text" }, {}
        self.composeDrafts = { text = self.composeDraft }
        self.thread, self.profileView, self.composeView = nil, nil, nil
        -- Readers start on their own timeline; guests on discover.
        self.feed = self.client and self.client:getSession() and "home" or "discover"
    end,

    -- One provider and store per reader (a guest or an account): every page
    -- scope shares them, so a post or person has one record across pages.
    readerData = function(self)
        if self.reader and self.reader.epoch == self.sessionEpoch then return self.reader end
        local session = self.client:getSession()
        local user = session and session.user.id or "guest"
        local provider = OctopusProvider.new {
            client = self.client, media = self.media, now = function() return self.clock() end,
            openPost = function(post) self:openPost(post) end,
            openProfile = function(id, name) self:openProfile(id, name) end,
            openTag = function(tag) self:openTag(tag) end,
            requireSession = function(reason) return self:requireSession(reason) end,
            notice = function(text, isError) self:notice(text, isError) end,
            selectFeed = function(name) Home.select(self, name) end,
            signIn = function(route) self.authReturn = route; self:show("/auth") end,
            signUp = function(route) self.authReturn = route; self:show("/auth/signup") end,
            invalidateFeeds = function(names) self:invalidateFeeds(names) end,
            repaint = function() self:repaint() end,
        }
        local store = data.Store.new { schema = data.schema("Octopus/Data/Octopus.Schema.xml"),
            scope = "octopus:" .. user, computed = provider.computed }
        self.reader = { epoch = self.sessionEpoch, user = user, provider = provider, store = store,
            record = store:put("OctopusReader", { id = user, signedIn = session ~= nil }), scopes = {} }
        return self.reader
    end,

    -- A data scope for a page, over the reader's store. Results for a
    -- previous reader never apply.
    octopusScope = function(self, declaration)
        local reader = self:readerData()
        local epoch, user = reader.epoch, reader.user
        local host = data.Scope.defaultHost()
        host.run = function(fn) self:task(fn) end
        host.report = function(err) io.stderr:write("octopus: " .. tostring(err) .. "\n") end
        local scope = data.scope(declaration, { provider = reader.provider, store = reader.store, host = host,
            valid = function()
                local active = self.client:getSession()
                return self.sessionEpoch == epoch and (active and active.user.id or "guest") == user
            end })
        scope:expose("Reader", reader.record)
        reader.scopes[scope] = true
        return scope
    end,

    -- The timelines (Data/Feeds.xml), retained with the home tab.
    feedsScope = function(self)
        if self.scope and not self.scope.disposed then return self.scope end
        self.scope = self:octopusScope("Octopus/Data/Feeds.xml")
        self.provider = self:readerData().provider
        for name, feed in pairs { Home = "home", Connect = "connect", Discover = "discover" } do
            self.scope:member(name):listen(function(query) self:feedStatus(feed, query) end)
        end
        return self.scope
    end,

    -- Pushed pages (a profile, later a post) get a scope that ends with the
    -- page; tabs keep theirs while retained.
    pageScope = function(self, declaration)
        if self.transientScope then self.transientScope:dispose() end
        self.transientScope = self:octopusScope(declaration)
        return self.transientScope
    end,

    -- A new profile photo: Lua-built tiles and every scope's records reload it.
    avatarChanged = function(self, userId)
        self.avatars:invalidate(userId)
        self:eachScope(function(scope)
            local avatars = scope:member("Avatars")
            if avatars then avatars:invalidate(userId) end
        end)
    end,

    -- Every live scope of this reader, for changes that cross pages.
    eachScope = function(self, fn)
        for scope in pairs(self.reader and self.reader.scopes or {}) do
            if not scope.disposed then fn(scope) end
        end
    end,

    -- The home page's status line follows the selected feed's last request.
    feedStatus = function(self, feed, query)
        if self.currentRoute ~= "/" or self.feed ~= feed or query.mode ~= nil then return end
        if query.error then self:notice(query.error.message, true)
        elseif query.loaded then self:notice(self.remote and "" or "Offline mock · posts stay in this session") end
    end,

    -- A post changed elsewhere (a comment, a publish): reload feeds that
    -- loaded; others load fresh on their next visit.
    invalidateFeeds = function(self, names)
        self:eachScope(function(scope)
            for _, name in ipairs(names or { "Home", "Discover", "Connect", "ProfilePosts", "SearchPosts" }) do
                local query = scope:member(name)
                if query and query.loaded then query:invalidate() end
            end
        end)
    end,

    -- Run work that may wait on the network in an ORCA coroutine.
    task = function(self, fn)
        local session, navigation = self.sessionEpoch, self.navigationEpoch
        orca.async(function()
            local ok, err = pcall(fn)
            if not ok then
                io.stderr:write("octopus: " .. tostring(err) .. "\n")
                if self.sessionEpoch == session and self.navigationEpoch == navigation then
                    self:notice("Something went wrong. Please try again.", true)
                end
            end
            self:repaint()
        end)
    end,

    repaint = function(self)
        if self.screen then pcall(function() self.screen:post("Window.Paint", renderer.getSize()) end) end
    end,

    avatarPhoto = function(self, tile, userId)
        if self.client and self.media then self.avatars:watch(tile, userId) end
    end,

    afterPaint = function(self)
        if self.visibility then self.visibility:update() end
        if self.avatars then self.avatars:loadVisible() end
        if self.photos then self.photos:loadVisible() end
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
            class = "octopus-" .. colors.appearance,
        }
        if content then screen:addChild(content) end
        return screen
    end,

    -- Show a root page whose content is rebuilt from state on each visit.
    tab = function(self, key, content)
        if self.shell and self.shell:has(key) then return self.shell:replace(key, content) end
        return self:frame(content, key)
    end,

    -- `direction` navigates as on Windows Phone: "Forward" going into a
    -- post or profile, "Backward" coming back out. The page in view goes out
    -- first and the next page comes in once it has gone.
    show = function(self, route, direction)
        local request = {}
        self.pendingShow = request
        local leaving = PageTransition.leave(self.screen, direction, function()
            if self.pendingShow == request then self:navigate(route, direction) end
        end)
        if not leaving then return self:navigate(route, direction) end
    end,

    navigate = function(self, route, direction)
        -- Pages with inputs keep their drafts when you navigate away.
        local leaving = self.leaving
        self.leaving = nil
        if leaving then leaving() end
        -- A pushed page's scope ends with it; late results cannot reach it.
        if self.transientScope then self.transientScope:dispose(); self.transientScope = nil end
        self.navigationEpoch = (self.navigationEpoch or 0) + 1
        local epoch = self.navigationEpoch
        local controller = self:dispatch(route or "/")
        if self.navigationEpoch ~= epoch then return end
        local result = self:activate_controller(controller)
        if result then PageTransition.enter(self.screen, direction) end
        return result
    end,

    goBack = function(self, route) return self:show(route, "Backward") end,

    -- A page's status line. Where it has its own strip (self.statusBar), the
    -- strip only shows while there is something to say.
    notice = function(self, text, isError)
        if self.status then
            self.status:setTextContent(text or "")
            self.status.Foreground = isError and colors.error or colors.dim
        end
        if self.statusBar then self.statusBar.Visible = (text or "") ~= "" end
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
        self:show("/post", "Forward")
    end,

    openProfile = function(self, userId, name)
        if self.client:getSession() and userId == self.client:currentUserId() then return self:show("/me") end
        self.profileId, self.profileName = userId, name
        self.profileReturn = self.currentRoute ~= "/user" and self.currentRoute or self.profileReturn
        self:show("/user", "Forward")
    end,

    openTag = function(self, tag)
        self.searchState.query, self.searchState.results = "#" .. tag, nil
        self:show("/search")
    end,

    -- Drop cached timelines and pages so the next visit reloads them.
    invalidate = function(self)
        self.searchState.results = nil
        self:invalidateFeeds()
        -- The home page is bound to the feeds; other root pages rebuild.
        if self.shell then
            for _, page in ipairs(Shell.pages) do
                if page.key ~= "Home" then C.clear(self.shell.pages[page.key]) end
            end
        end
    end,

    refresh = function(self)
        local route = self.currentRoute or "/"
        if route == "/" then Home.load(self, self.feed); return end
        if route == "/me" or route == "/user" then People.refresh(self); return end
        if route == "/search" then People.runSearch(self); return end
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

    selectFeed = function(self, name) Home.select(self, name) end,

    home = function(self)
        if not self.client then
            return self:frame(C.page("connect", C.stack({ Spacing = 16 }, {
                C.text(self.configurationError.message, { WordWrap = true, Foreground = colors.error }),
                C.text("Check APPWRITE_ENDPOINT and APPWRITE_PROJECT_ID, then restart.", { WordWrap = true }),
            })))
        end
        return Home.page(self)
    end,

    ["/"] = function(self) self.currentRoute = "/"; return self:home() end,
    ["/post"] = function(self) self.currentRoute = "/post"; return Thread.page(self) end,
    ["/user"] = function(self) self.currentRoute = "/user"; return People.profile(self) end,
    ["/me"] = function(self) self.currentRoute = "/me"; return People.me(self) end,
    ["/search"] = function(self) self.currentRoute = "/search"; return People.search(self) end,
    ["/write"] = function(self) self.currentRoute = "/write"; return Compose.page(self) end,
    ["/auth"] = function(self) self.currentRoute = "/auth"; return Identity.auth(self, "signin") end,
    ["/auth/signup"] = function(self) self.currentRoute = "/auth/signup"; return Identity.auth(self, "signup") end,

    updateAppearance = function(self)
        -- Keep active forms and in-flight submissions intact. Chrome is
        -- scoped to this palette, so deferring cannot create a mixed theme.
        local route = self.currentRoute
        if route == "/auth" or route == "/auth/signup" then return false end
        if (route == "/me" or route == "/write") and self.client and self.client:getSession() then return false end
        local thread = route == "/post" and self.thread
        if thread and (thread.sending or thread.replyTo or (thread.input and (thread.input.Text or "") ~= "")) then return false end
        if not colors.refresh() then return false end
        -- Rebuild retained pages too; navigation captures input drafts and
        -- invalidates pending view work without replacing the client/session.
        self.pendingShow = nil
        PageTransition.enter(self.screen)
        self.shell = nil
        self:navigate(route or "/")
        return true
    end,

    run = function(self)
        self.screen:post("Window.Paint", renderer.getSize())
        for message in system.getMessage do
            if message:is("Window.Closed") then return end
            if message:is("Window.Paint") then self:updateAppearance() end
            local ok, result = pcall(system.dispatchMessage, self.screen, message)
            if not ok then io.stderr:write(tostring(result) .. "\n")
            elseif result and not message:is("Window.Paint") then self.screen:post("Window.Paint", renderer.getSize()) end
            if ok and message:is("Window.Paint") then self:afterPaint() end
            system.translateMessage(message)
        end
    end,
}

return App
