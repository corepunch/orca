local ui = require "orca.UIKit"
local Footer = require "orca.UIKit.Footer"
local C = require "Octopus/views/components"
local Posts = require "Octopus/views/posts"
local colors = C.colors
local People = {}

local function number(value)
    if value == nil then return "–" end
    if value >= 10000 then return string.format("%.0fk", value / 1000) end
    if value >= 1000 then return string.format("%.1fk", value / 1000) end
    return tostring(value)
end

-- Avatar, name, bio, counts and the person's posts. `own` adds the editor.
local function profileView(self, userId, name, own)
    local view = { userId = userId, own = own }
    view.avatar = ui.Grid { Name = "ProfileAvatar", Width = 96, Height = 96 }
    view.avatar:addChild(C.avatar(name or "?", 96, userId, self))
    view.name = C.text(name or "", { Name = "ProfileName", FontSize = 26, WordWrap = true })
    view.bio = C.text("", { Name = "ProfileBio", FontSize = 14, Foreground = colors.muted, WordWrap = true })
    local header = ui.Grid { Columns = "96px 1fr", Spacing = 16 }
    header:addChild(view.avatar)
    header:addChild(C.stack({ Spacing = 6, JustifyContent = "Center" }, { view.name, view.bio }))
    local counts = ui.Grid { Name = "ProfileCounts", Columns = "1fr 1fr 1fr", Height = 64 }
    view.counts = {}
    for _, key in ipairs { "posts", "followers", "following" } do
        view.counts[key] = C.text("–", { Name = "Count" .. key, FontSize = 30 })
        counts:addChild(C.stack({ Spacing = 0 }, { view.counts[key], C.text(key, { FontSize = 13, Foreground = colors.dim }) }))
    end
    view.actions = C.stack({ Name = "ProfileActions", Spacing = 12 })
    view.posts = C.stack({ Name = "ProfilePosts", Spacing = C.spacing.post }, { C.loading { Name = "ProfileLoading", Height = 160 } })
    view.content = C.stack({ Name = "ProfilePage", Spacing = 18, Overflow = "hidden scroll" }, {
        header, counts, view.actions, C.eyebrow("posts"), view.posts,
    })
    return view
end

local function followButton(self, view)
    C.clear(view.actions)
    if not self.client:getSession() then
        view.actions:addChild(C.primary("Sign in to follow", function()
            self.authReturn = "/user"; self:show("/auth")
        end, { Name = "SignInToFollow" }))
        return
    end
    local following = view.follow
    local button = (following and C.outline or C.primary)(following and "Unfollow" or "Follow", function()
        if view.busy then return end
        view.busy = true
        self:task(function()
            local result, err
            if view.follow then result, err = self.client:unfollow(view.userId, view.follow)
            else result, err = self.client:follow(view.userId) end
            view.busy = false
            if result == nil then self:notice(err.message, true); return end
            local before = view.follow ~= nil
            view.follow = result or nil
            if view.profile and view.profile.followers then
                view.profile.followers = view.profile.followers + (view.follow and 1 or 0) - (before and 1 or 0)
                view.counts.followers:setTextContent(number(view.profile.followers))
            end
            self.feeds.home = nil
            followButton(self, view)
            self:notice(view.follow and "You'll see their posts in your following timeline." or "Unfollowed.")
            self:repaint()
        end)
    end, { Name = "FollowButton" })
    view.actions:addChild(button)
end

local function loadProfile(self, view, after)
    if view.loading then return end
    view.loading = true
    local loading = view.posts:findChild("ProfileLoading")
    if not loading then
        loading = C.loading { Name = "ProfileLoading", Height = 64 }
        view.posts:addChild(loading)
    end
    local session, navigation = self.sessionEpoch, self.navigationEpoch
    local function current()
        if self.profileView == view and self.sessionEpoch == session and self.navigationEpoch == navigation then return true end
        view.loading = false
        return false
    end
    self:task(function()
        local profile, err = self.client:getProfile(view.userId)
        if not current() then return end
        if not profile then
            view.loading = false
            loading:removeFromParent()
            self:notice(err.message, true)
            if not view.profile then C.clear(view.posts):addChild(C.empty("can't load this profile", err.message)) end
            self:repaint()
            return
        end
        view.profile = profile
        if profile.exists or not view.name:getTextContent():match("%S") then
            view.name:setTextContent(profile.username)
            C.clear(view.avatar):addChild(C.avatar(profile.username, 96, view.userId, self))
        end
        view.bio:setTextContent(profile.bio ~= "" and profile.bio or (view.own and "Add a short bio below." or ""))
        for key, label in pairs(view.counts) do label:setTextContent(number(profile[key])) end
        if after then after(profile) end
        self:repaint()
        if not view.own then
            view.follow = self.client:followState(view.userId) or nil
            if not current() then return end
            followButton(self, view)
        end
        self:repaint()
        local posts, stats = Posts.fetch(self, function()
            return self.client:userTimeline(view.userId, { limit = 30 })
        end)
        view.loading = false
        if not current() then return end
        loading:removeFromParent()
        if not posts then
            if not view.postsLoaded then C.clear(view.posts):addChild(C.empty("can't load posts", stats.message)) end
            self:notice(stats.message, true)
        else
            C.clear(view.posts)
            view.postsLoaded = true
            if #posts == 0 then
                view.posts:addChild(C.empty("no posts yet", view.own and "Use write to share your first post." or nil))
            else Posts.append(self, view.posts, posts, stats) end
            self:notice("")
        end
        self:repaint()
    end)
end

function People.refresh(self)
    if self.profileView then loadProfile(self, self.profileView) end
end

function People.profile(self)
    local userId = self.profileId
    if not userId then return self:home() end
    local view = profileView(self, userId, self.profileName)
    self.profileView = view
    self.status = C.text("", { Name = "ProfileStatus", FontSize = 11, Foreground = colors.dim, TextHorizontalAlignment = "Center" })
    local body = ui.Grid { Rows = "1fr auto" }
    body:addChild(view.content); body:addChild(self.status)
    local bar = C.appBar({
        { "BackFromProfile", "arrow-left", "back", function()
            local route = self.profileReturn
            self:show(route and route ~= "/user" and route or "/")
        end },
        { "RefreshProfile", "refresh-cw", "refresh", function() loadProfile(self, view) end },
    })
    local screen = self:frame()
    local footer = Footer { Name = "AppBarFooter", Rows = "58px", Background = colors.chrome }
    footer:addChild(bar)
    Footer.layout(screen, C.page((self.profileName or "profile"):lower(), body), footer,
        { Background = colors.background, Foreground = colors.ink })
    followButton(self, view)
    loadProfile(self, view)
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
    if not session then
        local body = ui.Grid { Rows = "1fr auto" }
        body:addChild(welcome(self)); body:addChild(self.status)
        self.profileView = nil
        return self:tab("Me", C.page("me", body))
    end
    local view = profileView(self, session.user.id, session.user.name, true)
    self.profileView = view
    local nameInput = C.input("EditName", "Your name", { Text = session.user.name })
    local bioInput = C.input("EditBio", "A short bio", { Multiline = true, Height = 96 })
    local save
    save = C.primary("Save profile", function()
        if view.saving then return end
        view.saving = true
        save:setTextContent("Saving…")
        self:task(function()
            local profile, err = self.client:updateProfile { username = nameInput.Text, bio = bioInput.Text }
            view.saving = false
            save:setTextContent("Save profile")
            if self.profileView ~= view then return end
            if not profile then self:notice(err.message, true); return end
            view.name:setTextContent(profile.username)
            view.bio:setTextContent(profile.bio)
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
            if self.profileView ~= view then return end
            if not profile then self:notice(err.message, true); return end
            self.avatars:invalidate(session.user.id)
            C.clear(view.avatar):addChild(C.avatar(profile.username, 96, session.user.id, self))
            self:notice("Photo updated. It shows on the website and in Chat too.")
            self:repaint()
        end)
    end, { Name = "ChangePhoto" })
    view.actions:addChild(C.stack({ Name = "ProfileEditor", Spacing = 10 }, {
        C.eyebrow("edit profile", { MarginTop = 0 }), photo,
        C.text("name", { FontSize = 14, Foreground = colors.muted }), nameInput,
        C.text("bio", { FontSize = 14, Foreground = colors.muted }), bioInput, save, self.status,
    }))
    view.content:addChild(C.outline("Sign out", function() self:signOut() end, { Name = "SignOut", Foreground = colors.error,
        BorderColor = colors.error, MarginTop = 12 }))
    local screen = self:tab("Me", C.page("me", view.content))
    loadProfile(self, view, function(profile)
        nameInput.Text, bioInput.Text = profile.username, profile.bio
    end)
    return screen
end

-- Search -----------------------------------------------------------------------

local function personRow(self, person)
    local row = ui.Grid { Columns = C.AVATAR .. "px 1fr", Spacing = C.spacing.inline, IgnoreHitTest = true }
    row:addChild(C.avatar(person.username, C.AVATAR, person.userId, self))
    row:addChild(C.stack({ Spacing = 2, JustifyContent = "Center", IgnoreHitTest = true }, {
        C.text(person.username, { FontSize = 20, TextOverflow = "Ellipsis" }),
        person.bio ~= "" and C.text(C.excerpt(person.bio, 80), {
            FontSize = 13, Foreground = colors.dim, TextOverflow = "Ellipsis" }) or nil,
    }))
    return C.area(row, function() self:openProfile(person.userId, person.username) end, { Name = "Person" .. person.userId, Height = 56 })
end

local function trendingTags(self)
    local counts, tags = {}, {}
    local feed = self.feeds.discover
    for _, post in ipairs(feed and feed.posts or {}) do
        for _, tag in ipairs(post.tags) do
            if not counts[tag] then tags[#tags + 1] = tag end
            counts[tag] = (counts[tag] or 0) + 1
        end
    end
    table.sort(tags, function(a, b) return counts[a] > counts[b] or (counts[a] == counts[b] and a < b) end)
    if #tags == 0 then tags = { "design", "software", "reef" } end
    return { table.unpack(tags, 1, math.min(#tags, 6)) }
end

local function renderPeopleError(self, list, results)
    list:addChild(C.stack({ Name = "PeopleSearchError", Spacing = 8 }, {
        C.empty("can't load people", results.peopleError.message),
        C.button("retry search", function() People.runSearch(self, list) end, { Name = "RetryPeopleSearch" }),
    }))
end

local function renderResults(self, list)
    local state = self.searchState
    C.clear(list)
    if not state.results then
        list:addChild(C.eyebrow("trending tags", { MarginTop = 0 }))
        for _, tag in ipairs(trendingTags(self)) do
            list:addChild(C.button("#" .. tag, function() self:openTag(tag) end, {
                Name = "Trending" .. tag, FontSize = 22, Height = 44 }))
        end
        list:addChild(C.text("Search post titles and people, or start with # to browse a tag.", {
            FontSize = 13, Foreground = colors.dim, WordWrap = true, MarginTop = 8 }))
        return
    end
    local results = state.results
    if results.error then
        list:addChild(C.empty("search failed", results.error.message))
        return
    end
    if #results.people > 0 then
        list:addChild(C.eyebrow("people", { MarginTop = 0 }))
        for _, person in ipairs(results.people) do list:addChild(personRow(self, person)) end
    end
    if #results.posts > 0 then
        list:addChild(C.eyebrow(results.tag and ("#" .. results.tag) or "posts", { MarginTop = #results.people > 0 and 8 or 0 }))
        local posts = C.stack({ Name = "SearchPosts", Spacing = C.spacing.post })
        list:addChild(posts)
        Posts.append(self, posts, results.posts, results.stats)
    end
    if results.peopleError then renderPeopleError(self, list, results)
    elseif #results.people == 0 and #results.posts == 0 then
        list:addChild(C.empty("nothing found", "No posts or people match “" .. results.query .. "”. Try another word or tag."))
    end
end

function People.runSearch(self, list)
    local query = (self.searchState.query or ""):match("^%s*(.-)%s*$")
    local session = self.sessionEpoch
    self.searchEpoch = (self.searchEpoch or 0) + 1
    local epoch = self.searchEpoch
    local loading = self.searchLoading
    if query == "" then
        loading.Visible = false
        self.searchState.results = nil
        renderResults(self, list)
        return
    end
    loading.Visible = true
    self:task(function()
        local tag = query:match("^#(.+)$")
        local posts, stats = Posts.fetch(self, function()
            local found, err = self.client:listPosts(tag and { tag = tag, limit = 30 } or { search = query, limit = 20 })
            return found and Posts.items(found), err
        end)
        if self.searchEpoch ~= epoch or self.sessionEpoch ~= session then return end
        if posts and not tag then
            self.searchState.results = { query = query, posts = posts, stats = stats, people = {} }
            if self.searchList == list then renderResults(self, list) end
            self:repaint()
        end
        local people, peopleError = {}, nil
        if posts and not tag then people, peopleError = self.client:searchPeople(query) end
        if self.searchEpoch ~= epoch or self.sessionEpoch ~= session then return end
        loading.Visible = false
        self.searchState.results = not posts and { error = stats, query = query } or {
            query = query, tag = tag and tag:lower(), posts = posts, stats = stats,
            people = people or {}, peopleError = peopleError }
        if self.searchList == list then
            if posts and not people then
                -- The post section is already on screen; only add the failed section.
                renderPeopleError(self, list, self.searchState.results)
            else renderResults(self, list) end
        end
        self:repaint()
    end)
end

function People.search(self)
    local state = self.searchState
    self.status = C.text("", { FontSize = 11, Foreground = colors.dim })
    local list = C.stack({ Name = "SearchResults", Spacing = 14, Overflow = "hidden scroll" })
    self.searchList = list
    local results = ui.Grid()
    self.searchLoading = C.loading { Name = "Searching", Visible = false }
    results:addChild(list); results:addChild(self.searchLoading)
    local input
    input = C.input("SearchInput", "Posts, people or #tags", {
        Text = state.query, Submit = function() state.query = input.Text; People.runSearch(self, list) end,
        KeyUp = function() state.query = input.Text end,
    })
    self.leaving = function() state.query = input.Text end
    local bar = ui.Grid { Columns = "1fr 48px", Height = 48 }
    bar:addChild(input)
    local go = C.button("", function() state.query = input.Text; People.runSearch(self, list) end, {
        Name = "RunSearch", Width = 48, Height = 48, Background = colors.accent })
    go:addChild(C.icon("search", 20, colors.onAccent))
    bar:addChild(go)
    local body = ui.Grid { Rows = "48px 1fr auto", Spacing = 16 }
    body:addChild(bar); body:addChild(results); body:addChild(self.status)
    local screen = self:tab("Search", C.page("search", body))
    -- A tag opened from a post searches straight away.
    if state.results == nil and state.query:match("^#") then People.runSearch(self, list)
    else renderResults(self, list) end
    return screen
end

return People
