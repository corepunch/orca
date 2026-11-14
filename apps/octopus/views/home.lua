-- After Twitter for Windows Phone: home (you and the people you follow, with
-- their reposts), connect (what happened to your posts) and discover. The
-- page is Pages/Home.xml over the Feeds scope; this module only owns which
-- feed is selected and when a feed loads.
local filesystem = require "orca.filesystem"
local Home = {}

Home.feeds = { "home", "connect", "discover" }
local QUERIES = { home = "Home", connect = "Connect", discover = "Discover" }

-- A refresh the reader asked for: the spinner shows over the kept rows.
function Home.load(self, name)
    local scope = self:feedsScope()
    if (name == "home" or name == "connect") and not self.client:getSession() then return end
    scope:member(QUERIES[name]):refresh()
    if name == "home" then scope:member("Suggestions"):refresh() end
end

function Home.select(self, name)
    self.feed = name
    if self.pivot then
        local index
        for i, feed in ipairs(self.feedOrder or Home.feeds) do if feed == name then index = i - 1 end end
        if self.pivot.SelectedIndex ~= index then self.pivot.SelectedIndex = index end
    end
    local scope = self:feedsScope()
    local query = scope:member(QUERIES[name])
    if (name == "home" or name == "connect") and not self.client:getSession() then return end
    -- Connect is always fresh; timelines keep their place until refreshed.
    if name == "connect" or not query.loaded then query:refresh() end
    local suggestions = scope:member("Suggestions")
    if name == "home" and not suggestions.loaded then suggestions:refresh() end
end

function Home.page(self)
    if self.shell and self.shell:has("Home") then
        self.status, self.statusBar = self.homeStatus, self.homeStatusBar
        local screen = self.shell:show("Home")
        Home.select(self, self.feed)
        return screen
    end
    local scope = self:feedsScope()
    local page = assert(filesystem.loadObject("Octopus/Pages/Home"), "Missing Octopus/Pages/Home")
    self.i18n:apply(page)
    if self.feedOrder then
        local pivot = page:findChild("Timeline", true)
        for _, name in ipairs(self.feedOrder) do
            local item = page:findChild("Feed" .. name, true)
            item:removeFromParent(); pivot:addChild(item)
        end
    end
    scope:attach(page)
    self.status, self.statusBar = page:findChild("HomeStatus", true), page:findChild("HomeStatusBar", true)
    self.homeStatus, self.homeStatusBar = self.status, self.statusBar
    self.pivot = page:findChild("Timeline", true)
    for _, name in ipairs(self.feedOrder or Home.feeds) do page:findChild("Feed" .. name, true).Header = self.i18n:t(name) end
    self.visibility:watch(page, scope)
    local screen = self:frame(page, "Home")
    Home.select(self, self.feed)
    return screen
end

return Home
