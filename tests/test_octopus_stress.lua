-- Real paged feed records and PostCard trees, with deterministic offline data.
-- OCTOPUS_STRESS_ROWS can increase the default without making normal tests huge.
local orca = require "orca"
orca.init()
local fs, core = require "orca.filesystem", require "orca.core"
require("orca.renderer").init(390, 844, true)
fs.init("apps/octopus")
require("orca.core.startup").load_plugins()
local data = require "orca.data"
local Visibility = require "orca.data.visibility"
local Provider = require "Octopus/providers/octopus"

local total = tonumber(os.getenv("OCTOPUS_STRESS_ROWS")) or 1000
assert(total >= 40 and total % 20 == 0, "Use a multiple of 20, at least 40")
local requests, enriched = 0, 0
local client = {}
function client:listPosts(options)
    requests = requests + 1
    local first, posts = tonumber(options.cursor) or 0, {}
    for index = first + 1, math.min(total, first + options.limit) do
        posts[#posts + 1] = { id = tostring(index), postType = "text", title = "Post " .. index,
            content = string.rep("Long feed content with Unicode: café, 日本語. ", index % 5 + 1),
            authorId = "author", authorName = "A Reader", createdAt = "2025-10-25T00:00:00Z", tags = {} }
    end
    return posts
end
function client:stats(ids)
    assert(#ids <= 20, "Enrichment must stay page-sized as history grows")
    enriched = enriched + #ids
    local result = {}
    for _, id in ipairs(ids) do result[id] = { likes = 1, comments = 0, reposts = 0 } end
    return result
end
local provider = Provider.new { client = client, media = {}, now = function() return 1761400000 end }
local store = data.Store.new { schema = data.schema("Octopus/Data/Octopus.Schema.xml"),
    scope = "octopus:stress", computed = provider.computed }
local scope = data.scope("Octopus/Data/Feeds.xml", { provider = provider, store = store })
local screen = assert(fs.loadObjectFromXmlString([[
<Screen Width="390" Height="844" ResizeMode="NoResize">
  <StackView Name="Feed" Overflow="hidden scroll" JustifyContent="Start">
    <ItemsStack Name="PostItems" Spacing="12" JustifyContent="Start" ItemNamePrefix="Post"
                ItemsSource="{Binding DataContext/Discover/Items}" ItemTemplate="Octopus/Templates/PostCard"/>
  </StackView>
</Screen>]]))
scope:attach(screen)
local function settle()
    core.flushQueue(); core.advanceFrame(); core.runAllPrograms(screen); core.flushQueue()
end
local query = scope:member("Discover")
local started = os.clock()
query:refresh(); settle()
local first = assert(screen:findChild("Post1", true))
for _ = 2, total / 20 do assert(query:loadMore()); settle() end
screen:UpdateLayout(390, 844)
local loaded = os.clock() - started
assert(query.items:count() == total and requests == total / 20 and enriched == total,
    "Paging must neither duplicate rows nor re-enrich all loaded history")
assert(screen:findChild("Post1", true) == first, "Appending pages must retain existing native rows")
assert(screen:findChild("Post" .. total, true):findChild("PostBodyTitle", true).Text == "Post " .. total)
local feed = screen:findChild("Feed", true)
assert(feed.ScrollHeight > 844 * 2, "The fixture must produce a long laid-out feed")
local visibility = Visibility.new()
visibility:watch(screen, scope)
local function visible(id)
    for _, record in ipairs(visibility:records(visibility.pages[1])) do
        if record.entity.name == "OctopusPost" and record.key == tostring(id) then return true end
    end
    return false
end
assert(visible(1) and not visible(total), "First paint only offers nearby rows to media")
started = os.clock()
for _ = 1, 30 do visibility:records(visibility.pages[1]) end
local scan = (os.clock() - started) / 30
feed.ContentOffset = { X = 0, Y = -(feed.ScrollHeight - feed.ActualHeight) }
assert(visible(total) and not visible(1), ("Scrolling to the end discovers the last post (height=%s viewport=%s offset=%s lastY=%s lastHeight=%s)")
    :format(feed.ScrollHeight, feed.ActualHeight, feed.ContentOffset.Y,
        screen:findChild("Post" .. total, true).ActualY, screen:findChild("Post" .. total, true).ActualHeight))
started = os.clock()
for _ = 1, 30 do visibility:records(visibility.pages[1]) end
local endScan = (os.clock() - started) / 30
store:put("OctopusPost", { id = "1", title = "Edited first post" }, { partial = true })
settle()
assert(first:findChild("PostBodyTitle", true).Text == "Edited first post",
    "Offscreen retained rows must still receive shared-record updates")
print(("Octopus stress: %d native cards, load %.3fs, visibility start %.3fms/frame, end %.3fms/frame")
    :format(total, loaded, scan * 1000, endScan * 1000))
scope:dispose()
