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
local colors = C.colors


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
        -- Readers start on their own timeline; guests on discover.
        self.feed = self.client and self.client:getSession() and "home" or "discover"
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
