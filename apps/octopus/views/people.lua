-- Profiles, the reader's own page and search. Content is XML over a data
-- scope (Pages/Profile.xml with Data/Profile.xml, Pages/Search.xml with
-- Data/Search.xml); this module owns the page chrome and which person or
-- query a page shows.
local ui = require "orca.UIKit"
local Footer = require "orca.UIKit.Footer"
local filesystem = require "orca.filesystem"
local C = require "Octopus/views/components"
local OctopusProvider = require "Octopus/providers/octopus"
local colors = C.colors
local People = {}

-- The profile content for `userId`: header, counts, follow and posts.
local function profileContent(self, scope, userId, name, own)
    local store = scope.store
    local person = store:put("OctopusProfile", { userId = userId, username = name }, { partial = true,
        locals = { Own = own == true } })
    scope:expose("Profile", person)
    local page = assert(filesystem.loadObject("Octopus/Pages/Profile"), "Missing Octopus/Pages/Profile")
    scope:attach(page)
    self.visibility:watch(page, scope)
    -- Metadata, follow state and posts load independently.
    scope:member("ProfilePosts"):set("AuthorId", userId)
    scope:member("Person"):set("UserId", userId)
    if not own and self.client:getSession() then scope:member("FollowState"):set("UserId", userId) end
    return page
end

local function refreshProfile(scope)
    for _, name in ipairs { "Person", "FollowState", "ProfilePosts" } do
        local query = scope:member(name)
        if query.loaded or query.error then query:refresh() end
    end
end

function People.refresh(self)
    local scope = self.currentRoute == "/me" and self.meScope or self.transientScope
    if scope and not scope.disposed and scope:member("Person") then refreshProfile(scope) end
end

function People.profile(self)
    local userId = self.profileId
    if not userId then return self:home() end
    local scope = self:pageScope("Octopus/Data/Profile.xml")
    local content = profileContent(self, scope, userId, self.profileName)
    self.profileView = { userId = userId, scope = scope }
    self.status = C.text("", { Name = "ProfileStatus", FontSize = 11, Foreground = colors.dim, TextHorizontalAlignment = "Center" })
    local body = ui.Node2D {}
    self.statusBar = C.toolbar("Bottom", { self.status }, { Name = "ProfileStatusBar", Visible = false })
    body:addChild(content); body:addChild(self.statusBar)
    local function back()
        local route = self.profileReturn
        self:goBack(route and route ~= "/user" and route or "/")
    end
    local bar = C.appBar({
        { "RefreshProfile", "refresh-cw", "refresh", function() refreshProfile(scope) end },
    })
    local screen = self:frame()
    Footer.layout(screen, C.page((self.profileName or "profile"):lower(), body,
        { Name = "ProfileView", ExtendBodyBehindHeader = true, back = back }), bar,
        { Background = colors.background, Foreground = colors.ink })
    return screen
end

local function welcome(self)
    local logo = ui.Grid { Name = "WelcomeLogo", Width = 96, Height = 96, Background = colors.accent }
    logo:addChild(C.logo(64))
    return C.stack({ Name = "Welcome", Spacing = 18, Overflow = "hidden scroll" }, {
        logo,
        C.text("join the reef", { FontSize = 32, WordWrap = true }),
        C.text("You're reading as a guest. Sign in to write posts, follow people, like, repost and reply.", {
            FontSize = 15, Foreground = colors.muted, WordWrap = true }),
        C.primary("Sign in", function() self.authReturn = "/me"; self:show("/auth") end, { Name = "MeSignIn" }),
        C.outline("Create an account", function() self.authReturn = "/me"; self:show("/auth/signup") end, { Name = "MeSignUp" }),
        C.text("Your account works on the Octopus website too.", { FontSize = 13, Foreground = colors.dim, WordWrap = true }),
    })
end

function People.me(self)
    local session = self.client:getSession()
    self.status = C.text("", { Name = "MeStatus", FontSize = 13, Foreground = colors.dim, WordWrap = true })
    self.statusBar = nil
    if not session then
        self.statusBar = C.toolbar("Bottom", { self.status }, { Name = "MeStatusBar", Visible = false })
        local body = ui.Node2D {}
        body:addChild(welcome(self)); body:addChild(self.statusBar)
        self.profileView = nil
        return self:tab("Me", C.page("me", body, { ExtendBodyBehindHeader = true }))
    end
    if self.meScope then self.meScope:dispose() end
    local scope = self:octopusScope("Octopus/Data/Profile.xml")
    self.meScope = scope
    local content = profileContent(self, scope, session.user.id, session.user.name, true)
    local person = scope:exposedRecord("Profile")
    local view = { userId = session.user.id, scope = scope, own = true }
    self.profileView = view
    local nameInput = C.input("EditName", "Your name", { Text = session.user.name })
    local bioInput = C.input("EditBio", "A short bio", { Multiline = true, Height = 96 })
    -- The editor starts from the loaded profile once.
    local filled = false
    scope:member("Person"):listen(function(query)
        if filled or not query.loaded then return end
        filled = true
        nameInput.Text, bioInput.Text = person.values.DisplayName or nameInput.Text, person.values.Bio or ""
    end)
    local save
    save = C.primary("Save profile", function()
        if view.saving then return end
        view.saving = true
        save:setTextContent("Saving…")
        self:task(function()
            local profile, err = self.client:updateProfile { username = nameInput.Text, bio = bioInput.Text }
            view.saving = false
            save:setTextContent("Save profile")
            if self.profileView ~= view or scope.disposed then return end
            if not profile then self:notice(err.message, true); return end
            scope.store:put("OctopusProfile", { userId = view.userId, username = profile.username, bio = profile.bio },
                { partial = true })
            self:notice("Profile saved. New posts use your account name.")
            self:repaint()
        end)
    end, { Name = "SaveProfile" })
    local photo
    photo = C.outline("Change photo", function()
        if view.uploading then return end
        local picked, err = self.media:pick()
        if not picked then
            if not (type(err) == "table" and err.code == "cancelled") then
                self:notice(type(err) == "table" and err.message or tostring(err), true)
            end
            return
        end
        view.uploading = true
        photo:setTextContent("Uploading photo…")
        self:task(function()
            local profile
            profile, err = self.client:setAvatar(picked)
            view.uploading = false
            photo:setTextContent("Change photo")
            if self.profileView ~= view or scope.disposed then return end
            if not profile then self:notice(err.message, true); return end
            self:avatarChanged(session.user.id)
            self:notice("Photo updated. It shows on the website and in Chat too.")
            self:repaint()
        end)
    end, { Name = "ChangePhoto" })
    content:findChild("ProfileActions", true):addChild(C.stack({ Name = "ProfileEditor", Spacing = 10 }, {
        C.eyebrow("edit profile", { MarginTop = 0 }), photo,
        C.text("name", { FontSize = 14, Foreground = colors.muted }), nameInput,
        C.text("bio", { FontSize = 14, Foreground = colors.muted }), bioInput, save, self.status,
    }))
    content:addChild(C.outline("Sign out", function() self:signOut() end, { Name = "SignOut", Foreground = colors.error,
        BorderColor = colors.error, MarginTop = 12 }))
    return self:tab("Me", C.page("me", content))
end

-- Search -----------------------------------------------------------------------

function People.runSearch(self)
    local scope = self.searchScope
    if not scope or scope.disposed then return end
    OctopusProvider.search(scope, self.searchState.query)
end

function People.search(self)
    local state = self.searchState
    if not self.searchScope or self.searchScope.disposed then self.searchScope = self:octopusScope("Octopus/Data/Search.xml") end
    local scope = self.searchScope
    local page = assert(filesystem.loadObject("Octopus/Pages/Search"), "Missing Octopus/Pages/Search")
    scope:attach(page)
    self.visibility:watch(page, scope)
    self.status, self.statusBar = page:findChild("SearchStatus", true), page:findChild("SearchStatusBar", true)
    local input = page:findChild("SearchInput", true)
    input.Text = state.query or ""
    self.leaving = function() state.query = input.Text end
    scope:member("Trending"):refresh()
    local screen = self:tab("Search", C.page("search", page, { ExtendBodyBehindHeader = true, class = "toolbar-page" }))
    -- A tag opened from a post searches straight away.
    if (state.query or ""):match("^#") then OctopusProvider.search(scope, state.query) end
    return screen
end

return People
