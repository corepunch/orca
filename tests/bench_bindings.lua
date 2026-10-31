-- Offline benchmark for issue #436. Uses the real Octopus datasource and row
-- templates, with deterministic data and no network/image downloads. This is
-- deliberately not a timing assertion in CI: compare identical runs locally.
local orca = require "orca"
orca.init()
local fs, core = require "orca.filesystem", require "orca.core"
require("orca.renderer").init(390, 844, true)
fs.init("apps/octopus")
require("orca.core.startup").load_plugins()
local data = require "orca.data"
local Provider = require "Octopus/providers/octopus"
local function positive(name, fallback)
    local value = tonumber(os.getenv(name) or fallback)
    assert(value and value >= 1 and value % 1 == 0, name .. " must be a positive integer")
    return value
end
local count = positive("BENCH_POSTS", 200)
local frames = positive("BENCH_FRAMES", 200)
local trials = positive("BENCH_TRIALS", 5)
local kinds, timeline = { "text", "photo", "quote", "link" }, {}
for i = 1, count do
    local kind = kinds[(i - 1) % #kinds + 1]
    local post = { id = "p" .. i, postType = kind, title = "Title " .. i,
        content = kind == "photo" and "" or "Body " .. i,
        authorId = "reader", authorName = "A Reader", createdAt = "2025-10-25T00:00:00Z",
        tags = { "reef" } }
    if kind == "photo" or kind == "quote" then post.imageId = "img" .. i end
    if kind == "quote" then post.quoteSource, post.userText = "A source", "My thoughts" end
    if kind == "link" then post.linkUrl = "https://example.com/" .. i end
    timeline[#timeline + 1] = { post = post }
end
local client = {
    homeTimeline = function() return timeline end,
    stats = function(_, ids)
        local result = {}
        for _, id in ipairs(ids) do result[id] = { likes = 1, comments = 2, reposts = 0 } end
        return result
    end,
}
local provider = Provider.new { client = client, media = {}, now = function() return 1761400000 end,
    requireSession = function() return true end, notice = function() end,
    readerId = function() return "reader" end }
local store = data.Store.new { schema = data.schema("Octopus/Data/Octopus.Schema.xml"),
    scope = "octopus:bench", computed = provider.computed }
local feeds = data.scope("Octopus/Data/Feeds.xml", { provider = provider, store = store, pageSize = count })
local screen = assert(fs.loadObjectFromXmlString([[
<Screen Width="390" Height="844" ResizeMode="NoResize">
  <StackView Overflow="hidden scroll">
    <ItemsStack Name="PostItems" ItemNamePrefix="Post" ItemsSource="{Binding DataContext/Home/Items}"
                ItemTemplate="Octopus/Templates/PostCard"/>
  </StackView>
</Screen>]]))
feeds:attach(screen)
local function pass() core.advanceFrame(); core.runAllPrograms(screen) end
feeds:member("Home"):refresh()
for _ = 1, 8 do core.flushQueue(); pass(); core.flushQueue() end
local items = assert(screen:findChild("PostItems", true))
local rows, nodes = 0, 0
local function visit(node)
    nodes = nodes + 1
    local child = node:getFirstChild()
    while child do visit(child); child = child:getNext() end
end
local child = items:getFirstChild()
while child do rows = rows + 1; child = child:getNext() end
visit(screen)
assert(rows == count, ("Expected %d rows, got %d"):format(count, rows))
local samples = {}
for trial = 1, trials do
    collectgarbage("collect")
    -- Exclude collection and fixture construction from the measured pass.
    collectgarbage("stop")
    local start = os.clock()
    for _ = 1, frames do pass() end
    local elapsed = (os.clock() - start) * 1000 / frames
    collectgarbage("restart")
    samples[trial] = elapsed
    print(("BENCH posts=%d rows=%d nodes=%d frames=%d trial=%d binding-pass=%.3f ms/frame")
        :format(count, rows, nodes, frames, trial, elapsed))
end
table.sort(samples)
print(("BENCH SUMMARY posts=%d median=%.3f min=%.3f max=%.3f ms/frame")
    :format(count, samples[math.ceil(trials / 2)], samples[1], samples[#samples]))
feeds:dispose()
