local ui = require "orca.UIKit"
local C = require "Octopus/views/components"
local Posts = require "Octopus/views/posts"
local colors = C.colors
local Home = {}

-- After Twitter for Windows Phone: home (you and the people you follow, with
-- their reposts), connect (what happened to your posts) and discover.
Home.feeds = { "home", "connect", "discover" }
local PAGE_SIZE = 20

local PROMPTS = {
    home = { "your timeline lives here", "Sign in to follow people and see their posts and reposts in one place." },
    connect = { "likes, reposts and replies", "Sign in to see who interacts with your posts and who follows you." },
}

local ACTIVITY = {
    like = { icon = "heart", color = "liked", text = " liked your post" },
    repost = { icon = "repeat-2", color = "reposted", text = " reposted your post" },
    comment = { icon = "message-circle", color = "accentText", text = " replied" },
    follow = { icon = "user-round", color = "accentText", text = " followed you" },
}

local function guestPrompt(self, name)
    local prompt = PROMPTS[name]
    return C.stack({ Spacing = 16 }, {
        C.empty(prompt[1], prompt[2]),
        C.primary("Sign in", function() self.authReturn = "/"; self:show("/auth") end, { Name = "SignIn" .. name }),
        C.button("Create an account", function() self.authReturn = "/"; self:show("/auth/signup") end, {
            Name = "SignUp" .. name }),
    })
end

-- One connect row: the person's tile, what they did and the post it touched.
local function activityRow(self, item, index)
    local style = ACTIVITY[item.kind]
    local row = ui.Grid { Columns = "52px 1fr", Spacing = 12, IgnoreHitTest = true }
    row:addChild(C.avatar(item.name, 52, item.userId, self))
    local line = ui.Grid { Columns = "18px 1fr auto", Spacing = 8, IgnoreHitTest = true }
    line:addChild(C.icon(style.icon, 16, colors[style.color], { MarginLeft = 0, MarginRight = 0 }))
    line:addChild(C.text(item.name .. style.text, { FontSize = 15, FontWeight = "Bold", TextOverflow = "Ellipsis" }))
    line:addChild(C.text(C.ago(item.at, self.clock()), { FontSize = 13, Foreground = colors.dim }))
    local details = C.stack({ Spacing = 4, JustifyContent = "Center", IgnoreHitTest = true }, { line })
    if item.body then details:addChild(C.text(C.excerpt(item.body, 140), { FontSize = 15, WordWrap = true })) end
    if item.post then
        local summary = item.post.title ~= "" and item.post.title or C.excerpt(item.post.content, 90)
        details:addChild(C.text(summary ~= "" and summary or "your photo", {
            FontSize = 13, Foreground = colors.dim, TextOverflow = "Ellipsis" }))
    end
    row:addChild(details)
    return C.area(row, function()
        if item.post then self:openPost(item.post) else self:openProfile(item.userId, item.name) end
    end, { Name = "Activity" .. index })
end

function Home.loadConnect(self, list)
    local session = self.sessionEpoch
    self.feeds.connect = { loading = true }
    C.clear(list):addChild(C.text("loading…", { Name = "Loading", FontSize = 22, Foreground = colors.muted }))
    self:task(function()
        local items, err = self.client:activity()
        self.feeds.connect.loading = false
        if self.sessionEpoch ~= session or self.timelines.connect ~= list then return end
        C.clear(list)
        if not items then
            list:addChild(C.empty("can't load activity", err.message .. " Use refresh to try again."))
            self:notice(err.message, true)
        elseif #items == 0 then
            list:addChild(C.empty("nothing yet", "When people like, repost or reply to your posts, or follow you, it shows up here."))
        else
            for index, item in ipairs(items) do list:addChild(activityRow(self, item, index)) end
        end
        self:repaint()
    end)
end

function Home.load(self, name, more)
    local list = self.timelines[name]
    if not list then return end
    if name == "connect" then return Home.loadConnect(self, list) end
    local feed = self.feeds[name] or { items = {}, stats = {} }
    self.feeds[name] = feed
    if feed.loading or (more and not feed.cursor) then return end
    feed.loading = true
    local session = self.sessionEpoch
    if not more then
        C.clear(list)
        list:addChild(C.text("loading…", { Name = "Loading", FontSize = 22, Foreground = colors.muted }))
    end
    self:task(function()
        local cursor = more and feed.cursor or nil
        local items, stats, nextCursor = Posts.fetch(self, function()
            if name == "home" then return self.client:homeTimeline { limit = PAGE_SIZE, before = cursor } end
            local posts, err = self.client:listPosts { limit = PAGE_SIZE, cursor = cursor }
            if not posts then return nil, err end
            return Posts.items(posts), #posts == PAGE_SIZE and posts[#posts].id or nil
        end)
        feed.loading = false
        if self.sessionEpoch ~= session or self.timelines[name] ~= list then return end
        if not more then C.clear(list) end
        local loadMore = list:findChild("LoadMore" .. name)
        if loadMore then loadMore:removeFromParent() end
        if not items then
            list:addChild(C.empty("can't load posts", stats.message .. " Use refresh to try again."))
            self:notice(stats.message, true)
            self:repaint()
            return
        end
        for id, entry in pairs(stats) do feed.stats[id] = entry end
        for _, item in ipairs(items) do feed.items[#feed.items + 1] = item end
        feed.cursor = nextCursor
        Posts.append(self, list, items, feed.stats)
        if #feed.items == 0 then
            if name == "home" then
                list:addChild(C.empty("your timeline is quiet", "Follow people from discover or search to see their posts here."))
            else
                list:addChild(C.empty("no posts yet", "Be the first to write something."))
            end
        elseif feed.cursor then
            list:addChild(C.button("load more posts", function() Home.load(self, name, true) end, {
                Name = "LoadMore" .. name, FontSize = 16 }))
        end
        self:notice(self.remote and "" or "Offline mock · posts stay in this session")
        self:repaint()
    end)
end

function Home.select(self, name)
    self.feed = name
    if self.pivot then
        local index
        for i, feed in ipairs(Home.feeds) do if feed == name then index = i - 1 end end
        if self.pivot.SelectedIndex ~= index then self.pivot.SelectedIndex = index end
    end
    -- Connect is always fresh; timelines keep their place until refreshed.
    if name == "connect" or not self.feeds[name] then Home.load(self, name) end
end

function Home.page(self)
    if self.shell and self.shell:has("Home") then
        self.status = self.homeStatus
        local screen = self.shell:show("Home")
        Home.select(self, self.feed)
        return screen
    end
    self.status = C.text("", { Name = "HomeStatus", FontSize = 11, Foreground = colors.dim, TextHorizontalAlignment = "Center" })
    self.homeStatus = self.status
    local signedIn = self.client:getSession() ~= nil
    self.pivot = ui.Pivot { Name = "Timeline", Title = "Octopus", Foreground = colors.ink,
        SelectionChanged = function(pivot) Home.select(self, Home.feeds[pivot.SelectedIndex + 1]) end,
    }
    self.timelines = {}
    for _, name in ipairs(Home.feeds) do
        local item = ui.PivotItem { Name = "Feed" .. name, Header = name }
        local list = C.stack({ Name = "Timeline" .. name, Spacing = name == "connect" and 20 or 26, Overflow = "hidden scroll" })
        if name ~= "discover" and not signedIn then list:addChild(guestPrompt(self, name))
        else self.timelines[name] = list end
        item:addChild(list)
        self.pivot:addChild(item)
    end
    local grid = ui.Grid { Rows = "1fr 20px", Background = colors.background }
    grid:addChild(self.pivot); grid:addChild(self.status)
    local screen = self:frame(grid, "Home")
    Home.select(self, self.feed)
    return screen
end

return Home
