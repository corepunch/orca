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
local Avatars = require "orca.avatars"
local Safety = require "Octopus/views/safety"
local People = {}

-- The profile content for `userId`: header, counts, follow and posts.
local function profileContent(self, scope, userId, name, own)
    local store = scope.store
    local person = store:put("OctopusProfile", { userId = userId, username = name }, { partial = true,
        locals = { Own = own == true } })
    scope:expose("Profile", person)
    local page = assert(filesystem.loadObject("Octopus/Pages/Profile"), "Missing Octopus/Pages/Profile")
    self.i18n:apply(page)
    scope:attach(page)
    self.visibility:watch(page, scope)
    if not own and self.messagePerson then
        local action = assert(filesystem.loadObject("Chat/Pages/PostProfileActions"))
        action.Click = function() self:messagePerson(userId, name) end
        page:findChild("ProfileActions", true):addChild(action)
    end
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
    -- The label follows the block list loaded with the timelines.
    local name, bar = self.profileName, nil
    local function blockLabel()
        return self.i18n:t(self.client:isBlocked(userId) and "unblock %{name}" or "block %{name}", { name = name or "person" })
    end
    bar = C.appBar({
        { "RefreshProfile", "refresh-cw", self.i18n:t("refresh"), function() refreshProfile(scope) end },
    }, {
        { "ReportPerson", self.i18n:t("report"), function() Safety.open(self, { userId = userId, name = name }) end },
        { "BlockPerson", blockLabel(), function()
            local blocking = not self.client:isBlocked(userId)
            Safety.setBlocked(self, userId, name, blocking, function()
                if blocking then return back() end
                self:notice(self.i18n:t("%{name} is unblocked.", { name = name or "This person" }))
                if not scope.disposed then bar:findChild("BlockPerson").Text = blockLabel(); refreshProfile(scope) end
            end)
        end },
    })
    local screen = self:frame()
    Footer.layout(screen, C.page((self.profileName or self.i18n:t("profile")):lower(), body,
        { Name = "ProfileView", ExtendBodyBehindHeader = true, back = back }), bar,
        { Background = colors.background, Foreground = colors.ink })
    self.i18n:apply(screen)
    return screen
end

local function welcome(self)
    local logo = ui.Grid { Name = "WelcomeLogo", Width = 96, Height = 96, Background = colors.accent }
    logo:addChild(C.logo(64))
    return C.stack({ Name = "Welcome", Spacing = 18, Overflow = "hidden scroll" }, {
        logo,
        C.text(self.i18n:t("join the reef"), { FontSize = 32, WordWrap = true }),
        C.text(self.i18n:t("You're reading as a guest. Sign in to write posts, follow people, like, repost and reply."), {
            FontSize = 15, Foreground = colors.muted, WordWrap = true }),
        C.primary(self.i18n:t("Sign in"), function() self.authReturn = "/me"; self:show("/auth") end, { Name = "MeSignIn" }),
        C.outline(self.i18n:t("Create an account"), function() self.authReturn = "/me"; self:show("/auth/signup") end, { Name = "MeSignUp" }),
        C.text(self.i18n:t("Your account works on the Octopus website too."), { FontSize = 13, Foreground = colors.dim, WordWrap = true }),
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
        return self:tab("Me", C.page(self.i18n:t("me"), body, { ExtendBodyBehindHeader = true }))
    end
    if self.meScope then self.meScope:dispose() end
    local scope = self:octopusScope("Octopus/Data/Me.xml")
    self.meScope = scope
    local content = profileContent(self, scope, session.user.id, session.user.name, true)
    self.profileView = { userId = session.user.id, scope = scope, own = true }
    -- The form: an edit copy of the reader's profile (Data/Me.xml).
    local editor = assert(filesystem.loadObject("Octopus/Pages/ProfileEditor"), "Missing Octopus/Pages/ProfileEditor")
    scope:member("ProfileForm"):begin(scope:exposedRecord("Profile"))
    self.i18n:apply(editor)
    scope:attach(editor)
    content:findChild("ProfileActions", true):addChild(editor)
    editor:findChild("TakePhoto", true).Visible = People.canTakePhoto(self)
    self.status = editor:findChild("MeStatus", true)
    self.status:setTextContent("")
    return self:tab("Me", C.page(self.i18n:t("me"), content))
end

function People.canTakePhoto(self)
    local media = self.media
    return media ~= nil and media.canTake ~= nil and media:canTake() == true
end

-- Every account starts with a picture, as on Discord: one without a photo
-- gets a random character. True when it had none (a new account).
function People.assignRandom(self)
    local session = self.client:getSession()
    if not session then return false end
    local userId = session.user.id
    local avatars = self.client:avatars { userId }
    if not avatars or avatars[userId] ~= false then return false end
    local photo = Avatars.load(Avatars.random(self.randomRoll).id)
    if photo and self.client:setAvatar(photo) then self:avatarChanged(userId) end
    return true
end

-- The step after creating an account (Pages/PictureStep.xml over Data/Me.xml).
function People.picture(self)
    local session = self.client:getSession()
    if not session then return self:show("/") end
    if self.meScope then self.meScope:dispose(); self.meScope = nil end
    local scope = self:pageScope("Octopus/Data/Me.xml")
    local person = scope.store:put("OctopusProfile", { userId = session.user.id, username = session.user.name },
        { partial = true, locals = { Own = true } })
    scope:expose("Profile", person)
    local page = assert(filesystem.loadObject("Octopus/Pages/PictureStep"), "Missing Octopus/Pages/PictureStep")
    self.i18n:apply(page)
    scope:attach(page)
    self.visibility:watch(page, scope)
    page:findChild("TakePhoto", true).Visible = People.canTakePhoto(self)
    scope:member("Person"):set("UserId", session.user.id)
    self.status = page:findChild("PictureStatus", true)
    self.statusBar = nil
    return self:frame(C.page(self.i18n:t("your picture"), page))
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
    self.i18n:apply(page)
    scope:attach(page)
    self.visibility:watch(page, scope)
    self.status, self.statusBar = page:findChild("SearchStatus", true), page:findChild("SearchStatusBar", true)
    local input = page:findChild("SearchInput", true)
    input.Text = state.query or ""
    self.leaving = function() state.query = input.Text end
    scope:member("Trending"):refresh()
    local screen = self:tab("Search", C.page(self.i18n:t("search"), page, { ExtendBodyBehindHeader = true, class = "toolbar-page" }))
    -- A tag opened from a post searches straight away.
    if (state.query or ""):match("^#") then OctopusProvider.search(scope, state.query) end
    self.i18n:apply(screen)
    return screen
end

return People
