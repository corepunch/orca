-- Bounded feed loading with UI/network doubles; standalone Lua, no live service.
local function node(props)
    local self = props or {}
    self.children = {}
    function self:addChild(child)
        self.children[#self.children + 1] = child
        child.parent = self
        return child
    end
    function self:getFirstChild() return self.children[1] end
    function self:getNext()
        if not self.parent then return nil end
        for i, child in ipairs(self.parent.children) do
            if child == self then return self.parent.children[i + 1] end
        end
    end
    function self:findChild(name)
        for _, child in ipairs(self.children) do if child.Name == name then return child end end
    end
    function self:removeFromParent()
        for i, child in ipairs(self.parent.children) do
            if child == self then table.remove(self.parent.children, i); break end
        end
    end
    return self
end
local C = { colors = {} }
function C.loading(props) return node(props) end
function C.button(_, callback, props) local n = node(props); n.click = callback; return n end
function C.empty(_, _, props) return node(props) end
function C.clear(n) n.children = {}; return n end
package.loaded["orca.UIKit"] = {}
package.loaded["Octopus/views/components"] = C
local rendered = {}
local Posts = {}
function Posts.items(posts)
    local items = {}
    for _, post in ipairs(posts) do items[#items + 1] = { post = post } end
    return items
end
function Posts.fetch(_, fetch)
    local items, cursor = fetch()
    if not items then return nil, cursor end
    return items, {}, cursor
end
function Posts.append(_, _, items)
    for _, item in ipairs(items) do rendered[#rendered + 1] = item.post.id end
end
function Posts.refresh(_, list, items)
    rendered = {}
    if not list:findChild("PostItems") then list:addChild(node { Name = "PostItems" }) end
    for _, item in ipairs(items) do rendered[#rendered + 1] = item.post.id end
end
function Posts.enrich() end
package.loaded["Octopus/views/posts"] = Posts
local Home = dofile("apps/octopus/views/home.lua")
local requests, workers, fail = {}, {}, false
local app = { currentRoute = "/", feed = "discover", sessionEpoch = 1,
    feeds = {}, timelines = { discover = node { ActualHeight = 600, ScrollHeight = 5000, ContentOffset = { Y = 0 } } },
    timelineLoading = { discover = {} }, client = {} }
function app:task(fn) workers[#workers + 1] = fn end
function app:repaint() end
function app:notice() end
function app.client:listPosts(options)
    requests[#requests + 1] = options
    if fail then return nil, { message = "Offline" } end
    local first = options.cursor and tonumber(options.cursor) or 0
    local posts = {}
    for i = first + 1, first + options.limit do posts[#posts + 1] = { id = tostring(i) } end
    return posts
end
local function finish() local fn = table.remove(workers, 1); assert(fn); fn() end
local list = app.timelines.discover
Home.load(app, "discover")
assert(#workers == 1 and #requests == 0)
finish()
assert(#requests == 1 and requests[1].limit == 20 and not requests[1].cursor and #rendered == 20)
Home.prefetch(app)
assert(#workers == 0, "First paint must not drain the next page")
list.ContentOffset.Y = -3400
Home.prefetch(app)
assert(#workers == 0, "Scrolling outside the threshold must not fetch")
list.ContentOffset.Y = -3900
Home.prefetch(app); Home.prefetch(app)
assert(#workers == 1, "Near-bottom prefetch must allow only one request")
finish()
assert(#requests == 2 and requests[2].cursor == "20" and #rendered == 40)
Home.prefetch(app)
assert(#workers == 0, "An unchanged viewport must not cascade through pages")
list.ContentOffset.Y = -4000
fail = true
Home.prefetch(app); finish()
Home.prefetch(app)
assert(#workers == 0 and app.feeds.discover.autoPaused, "Failure must pause automatic retries")
fail = false
list:findChild("LoadMorediscover").click(); finish()
assert(#requests == 4 and not app.feeds.discover.autoPaused and #rendered == 60)
-- Repeated posts across streams/pages should not duplicate cards.
app.client.listPosts = function()
    local posts = {}
    for i = 51, 70 do posts[#posts + 1] = { id = tostring(i) } end
    return posts
end
Home.load(app, "discover", true); finish()
assert(#rendered == 70 and #app.feeds.discover.items == 70)
app.currentRoute = "/compose"
list.ContentOffset.Y = -4300
Home.prefetch(app)
assert(#workers == 0, "Hidden timelines must not fetch")
app.currentRoute = "/"
app.feeds.discover.cursor = nil
Home.prefetch(app)
assert(#workers == 0, "An exhausted timeline must not fetch")
-- Discard work when the account changes before its coroutine starts.
app.feeds.discover.cursor = "70"
Home.load(app, "discover", true)
local count = #rendered
app.sessionEpoch = 2
finish()
assert(#rendered == count and not app.feeds.discover.loading)
-- A bounded batch of deleted repost targets can be empty but still advance.
app.timelines.home, app.timelineLoading.home = node(), {}
function app.client:homeTimeline() return {}, { version = 1 } end
Home.load(app, "home"); finish()
assert(app.timelines.home:findChild("LoadMorehome") and app.timelines.home:findChild("EmptyTimelinehome"),
    "An empty intermediate page must retain a way to reach older valid posts")
function app.client:homeTimeline() return { { post = { id = "older-valid" } } } end
Home.load(app, "home", true); finish()
assert(not app.timelines.home:findChild("EmptyTimelinehome") and #app.feeds.home.items == 1,
    "Older valid content must replace the intermediate empty state")
print("Octopus pagination tests passed")
