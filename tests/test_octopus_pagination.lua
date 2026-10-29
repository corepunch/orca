-- Bounded feed paging over the real Feeds scope and Octopus provider with a
-- network double; no renderer, live service or UI.
local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
filesystem.init("apps/octopus")
local data = require "orca.data"
local OctopusProvider = require "Octopus/providers/octopus"

local requests, workers, fail, valid = {}, {}, false, true
local client = {}
function client:listPosts(options)
    requests[#requests + 1] = options
    if fail then return nil, { message = "Offline" } end
    local first = options.cursor and tonumber(options.cursor) or 0
    local posts = {}
    for i = first + 1, first + options.limit do posts[#posts + 1] = { id = tostring(i), userId = "u", title = "" } end
    return posts
end
function client:stats() return {} end
local provider = OctopusProvider.new { client = client, media = {}, now = function() return 0 end }
local store = data.Store.new { schema = data.schema("Octopus/Data/Octopus.Schema.xml"), scope = "octopus:test",
    computed = provider.computed }
local host = data.Scope.defaultHost()
host.run = function(fn) workers[#workers + 1] = fn end
local scope = data.scope("Octopus/Data/Feeds.xml", { provider = provider, store = store, host = host,
    valid = function() return valid end })
local function finish() local fn = table.remove(workers, 1); assert(fn, "Expected a request"); fn() end
-- Stats enrichment runs as its own task after each page.
local function settle() while #workers > 0 do finish() end end

local discover = scope:member("Discover")
discover:refresh(); settle()
assert(#requests == 1 and requests[1].limit == 20 and not requests[1].cursor and discover.items:count() == 20)
-- A 600-point viewport over 5000 points of content.
assert(not discover:prefetch(600, 5000, 0) and #workers == 0, "First paint must not drain the next page")
assert(not discover:prefetch(600, 5000, -3400) and #workers == 0, "Scrolling outside the threshold must not fetch")
assert(discover:prefetch(600, 5000, -3900) and not discover:prefetch(600, 5000, -3901),
    "Near-bottom prefetch must allow only one request")
settle()
assert(#requests == 2 and requests[2].cursor == "20" and discover.items:count() == 40)
assert(not discover:prefetch(600, 5000, -3900), "An unchanged viewport must not cascade through pages")
fail = true
discover:prefetch(600, 5000, -4000); settle()
assert(not discover:prefetch(600, 5000, -4100) and discover.error and discover.items:count() == 40,
    "Failure must pause automatic paging and keep the loaded rows")
fail = false
discover:loadMore(); settle()
assert(#requests == 4 and not discover.error and discover.items:count() == 60, "An explicit load more resumes")
-- Repeated posts across pages do not duplicate rows.
client.listPosts = function()
    local posts = {}
    for i = 51, 70 do posts[#posts + 1] = { id = tostring(i), userId = "u", title = "" } end
    return posts
end
discover:loadMore(); settle()
assert(discover.items:count() == 70, "Posts already listed are not added twice")
discover.cursor = nil
assert(not discover:prefetch(600, 5000, -4300), "An exhausted timeline must not fetch")
-- Work started for a previous reader is discarded.
local count = discover.items:count()
discover:refresh()
valid = false
finish()
valid = true
assert(discover.items:count() == count and discover.mode == "refresh",
    "A result for a replaced reader cannot change the timeline")
settle()

-- An empty intermediate page keeps a way to reach older valid posts.
function client:homeTimeline(options)
    if options.before then return { { post = { id = "older-valid", userId = "u", title = "" } } } end
    return {}, { version = 1 }
end
local home = scope:member("Home")
home:refresh(); settle()
assert(home.loaded and home.items:count() == 0 and home.cursor ~= nil and home.state.CanLoadMore,
    "An empty intermediate page must retain a way to reach older valid posts")
home:loadMore(); settle()
assert(home.items:count() == 1 and home.items:records()[1].values.PostId == "older-valid",
    "Older valid content follows the intermediate empty page")
scope:dispose()
print("Octopus pagination tests passed")
