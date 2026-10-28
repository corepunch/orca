-- Offline service contracts for startup payloads; run with standalone Lua.
for _, name in ipairs { "json", "fixtures", "mock" } do
    local module = name
    package.preload["Octopus/api/" .. module] = function() return dofile("apps/octopus/api/" .. module .. ".lua") end
end
local Octopus = dofile("apps/octopus/api/octopus.lua")
local function reply(body, status) return { status = status or 200, body = body } end
local function query(request, method, attribute)
    for _, value in ipairs(request.query or {}) do
        if value.method == method and (not attribute or value.attribute == attribute) then return value end
    end
end
local function client(transport, options)
    options = options or {}
    options.transport, options.projectId = transport, "test"
    return assert(Octopus.client(options))
end
local function reader(service)
    service.session = { cookie = "a_session_test=private", user = { id = "alice", name = "Alice", email = "alice@example.invalid" } }
    return service
end
local function contains(values, wanted)
    for _, value in ipairs(values) do if value == wanted then return true end end
    return false
end

local feedRequests = {}
local summary = { ["$id"] = "post-one", ["$createdAt"] = "2099-01-01T00:00:00Z",
    sourceCreatedAt = "2026-10-28T09:00:00Z", authorId = "alice", authorName = "Alice", title = "Title",
    content = "Bounded excerpt", tags = { "reef" }, postType = "text", avatarId = "",
    statsUpdatedAt = "2026-10-28T09:01:00Z", likeCount = 10, commentCount = 3, repostCount = 4 }
local feed = client(function(request)
    feedRequests[#feedRequests + 1] = request
    assert(request.path:match("/feed%-summaries/documents$"), "Materialized feed must use its configured collection")
    assert(request.params.total == false, "Startup lists do not need exact totals")
    assert(query(request, "orderDesc", "sourceCreatedAt") and not query(request, "orderDesc", "$createdAt"))
    return reply { documents = { summary }, total = 1 }
end, { feedCollectionId = "feed-summaries" })
local posts = assert(feed:listPosts { limit = 20, cursor = "older-post", exclude = "exclude-post" })
assert(posts[1].id == "post-one" and posts[1].createdAt == summary.sourceCreatedAt and posts[1].summary)
assert(query(feedRequests[1], "cursorAfter").values[1] == "older-post"
    and query(feedRequests[1], "notEqual", "$id").values[1] == "exclude-post", "IDs and cursors must retain source identity")
assert(feed:avatars { "alice" }.alice == false and #feedRequests == 1, "Known absent avatars must skip profile lookup")
local snapshots = { posts[1] }
local counts = assert(feed:stats({ "post-one" }, nil, snapshots))
assert(counts["post-one"].likes == 10 and counts["post-one"].comments == 3 and counts["post-one"].reposts == 4
    and #feedRequests == 1, "Guests consume materialized counters without further requests")
assert(feed:timeline({ "alice" }, {}, { before = "2026-10-27T00:00:00Z" }))
assert(query(feedRequests[2], "lessThan", "sourceCreatedAt").values[1] == "2026-10-27T00:00:00Z")
local legacy = client(function(request)
    assert(request.path:match("/posts/documents$") and query(request, "orderDesc", "$createdAt"))
    return reply { documents = {} }
end)
assert(#assert(legacy:listPosts()) == 0, "Legacy post collections remain the default")

local viewerRequests = {}
local viewer = reader(client(function(request)
    viewerRequests[#viewerRequests + 1] = request
    assert(query(request, "equal", "userId").values[1] == "alice", "Materialized viewer lookups must only fetch the current reader")
    local selected = assert(query(request, "select")).values
    assert(#selected == 2 and contains(selected, "$id") and not contains(selected, "userId"))
    if request.path:match("/likes/documents$") then
        assert(contains(selected, "targetId"))
        return reply { documents = { { ["$id"] = "own-like", targetId = "post-one" } } }
    end
    assert(request.path:match("/reposts/documents$") and contains(selected, "postId"))
    return reply { documents = { { ["$id"] = "own-repost", postId = "post-one" } } }
end))
counts = assert(viewer:stats({ "post-one" }, nil, snapshots))
assert(#viewerRequests == 2 and counts["post-one"].liked == "own-like" and counts["post-one"].reposted == "own-repost"
    and counts["post-one"].likes == 10, "Viewer state must not increment materialized counts")

local mixed = reader(client(function(request)
    local selected = assert(query(request, "select")).values
    assert(not contains(selected, "body"), "Counter queries must exclude comment bodies")
    if request.path:match("/likes/documents$") then
        return reply { documents = { { ["$id"] = "snapshot-like", targetId = "post-one", userId = "alice" },
            { ["$id"] = "legacy-like", targetId = "legacy-post", userId = "alice" },
            { ["$id"] = "other-like", targetId = "legacy-post", userId = "bob" } } }
    elseif request.path:match("/comments/documents$") then
        assert(#selected == 2 and query(request, "equal", "postId").values[1] == "legacy-post",
            "Only missing counters need legacy comment queries")
        return reply { documents = { { ["$id"] = "comment", postId = "legacy-post" } } }
    end
    return reply { documents = { { ["$id"] = "snapshot-repost", postId = "post-one", userId = "alice" },
        { ["$id"] = "legacy-repost", postId = "legacy-post", userId = "alice" } } }
end))
counts = assert(mixed:stats({ "post-one", "legacy-post" }, nil, snapshots))
assert(counts["post-one"].likes == 10 and counts["post-one"].comments == 3 and counts["post-one"].reposts == 4)
assert(counts["legacy-post"].likes == 2 and counts["legacy-post"].comments == 1 and counts["legacy-post"].reposts == 1)
assert(counts["post-one"].liked == "snapshot-like" and counts["legacy-post"].reposted == "legacy-repost")

local likePages = 0
local popular = client(function(request)
    assert(not contains(assert(query(request, "select")).values, "body"))
    if not request.path:match("/likes/documents$") then return reply { documents = {} } end
    local cursor = query(request, "cursorAfter")
    local offset = cursor and tonumber(cursor.values[1]:match("(%d+)$")) or 0
    likePages = likePages + 1
    assert(query(request, "limit").values[1] == 100)
    local documents = {}
    for i = offset + 1, math.min(offset + 100, 1101) do
        documents[#documents + 1] = { ["$id"] = "like-" .. i, targetId = "popular-post", userId = "bob" }
    end
    return reply { documents = documents }
end)
assert(assert(popular:stats { "popular-post" })["popular-post"].likes == 1101 and likePages == 12,
    "Popular posts must paginate beyond ten pages without silently truncating counts")

for _, failRequest in ipairs { false, true } do
    local workers, started, completed = {}, 0, 0
    local concurrent = client(function(request)
        started = started + 1
        coroutine.yield()
        completed = completed + 1
        if failRequest and request.path:match("/likes/documents$") then error("Transport failed") end
        return reply { documents = {} }
    end, { schedule = function(job) workers[#workers + 1] = coroutine.create(job) end })
    local value, requestError
    local main = coroutine.create(function() value, requestError = concurrent:stats { "parallel-post" } end)
    assert(coroutine.resume(main))
    assert(#workers == 3)
    for _, worker in ipairs(workers) do assert(coroutine.resume(worker)) end
    assert(started == 3 and completed == 0, "All independent requests must start before any response completes")
    for _, worker in ipairs(workers) do assert(coroutine.resume(worker)) end
    assert(coroutine.resume(main))
    assert(coroutine.status(main) == "dead" and completed == 3, "Every request must settle even if another throws")
    assert(failRequest and not value and requestError.code == "network" or not failRequest and value["parallel-post"])
end

local saved, writes, reads = nil, 0, 0
local cached = client(function() error("Cache methods must not contact Appwrite") end, { cache = {
    readJSON = function(_, key, ttl) assert(key == "discover" and ttl > 0); reads = reads + 1; return saved end,
    writeJSON = function(_, key, value) assert(key == "discover"); writes = writes + 1; saved = value; return true end,
} })
cached:writeFeedCache("discover", { { post = posts[1] } }, "post-one")
local restored, cursor = cached:readFeedCache("discover")
assert(restored[1].post.id == "post-one" and cursor == "post-one" and writes == 1 and reads == 1)
assert(restored[1].post.avatarId == false and cached:avatars { "alice" }.alice == false,
    "Warm feeds must retain absent avatars without another profile request")
assert(not cached:readFeedCache("home")); cached:writeFeedCache("home", {}, nil)
reader(cached)
assert(not cached:readFeedCache("discover")); cached:writeFeedCache("discover", {}, nil)
assert(writes == 1 and reads == 1, "Reader-specific feeds must never use public disk snapshots")
cached.session = nil
for _, broken in ipairs { { items = { true } }, { items = { { post = { id = "missing-fields" } } } },
    { items = {}, cursor = {} } } do
    saved = broken
    assert(not cached:readFeedCache("discover"), "Malformed feed schemas must be cache misses")
end
for _, corrupt in ipairs { { stats = true }, { stats = { likes = "bad", comments = 0, reposts = 0 } }, { tags = { true } } } do
    local bad = {}; for key, value in pairs(posts[1]) do bad[key] = value end
    for key, value in pairs(corrupt) do bad[key] = value end
    saved = { items = { { post = bad } } }
    assert(not cached:readFeedCache("discover"), "Malformed optional fields must not reach UI constructors")
end

local jpg = "\255\216\255test"
for _, variant in ipairs { "avatar", "photo" } do
    local requests = {}
    local images = client(function(request)
        requests[#requests + 1] = request
        assert(request.binary and request.path:find("output=jpg&quality=80", 1, true))
        assert(request.path:find(variant == "avatar" and "width=192&height=192" or "width=780", 1, true))
        return reply(jpg)
    end)
    assert(images:getImage("image-file", variant) == jpg and #requests == 1, "Sized previews must avoid original downloads")
end
local attempts = {}
local fallback = client(function(request)
    attempts[#attempts + 1] = request.path
    if request.path:match("/view$") then return reply(jpg) end
    return reply({ message = "Preview unavailable" }, 400)
end)
assert(fallback:getImage("image-file", "avatar") == jpg and #attempts == 2 and attempts[2]:match("/view$"))
local missing = client(function() return reply({ message = "Gone" }, 404) end)
local _, err = missing:getImage("image-file", "photo")
assert(err.status == 404, "Missing originals must preserve service errors")
local unsupported = client(function() return reply("not an image") end)
_, err = unsupported:getImage("image-file", "photo")
assert(err.code == "invalid_image", "Invalid previews and originals must not enter image caches")

local transport, state = require("Octopus/api/mock").new { projectId = "test" }
local identity = client(transport)
local session = assert(identity:signUp("Display Name", "unique@example.invalid", "password123", "password123", "  My.Handle  "))
assert(session.user.id == "my.handle" and state.users["my.handle"].name == "Display Name",
    "Signup must use the canonical lowercase account name as its unique Appwrite ID")
assert(identity:updateProfile { username = "Updated Name", bio = "A short bio" })
assert(state.users["my.handle"].name == "Updated Name" and identity:getSession().user.name == "Updated Name")
local published = assert(identity:createPost { title = "A post", content = "Body" })
assert(published.authorName == "Updated Name" and published.authorId == "my.handle",
    "New posts must use the synchronized display name and stable readable account ID")
for _, rollbackFails in ipairs { false, true } do
    local failProfile, nameWrites = true, 0
    local recovering = client(function(request)
        if request.method == "PATCH" and request.path == "/account/name" then
            nameWrites = nameWrites + 1
            if rollbackFails and nameWrites == 2 then return reply({ message = "Unavailable" }, 503) end
        elseif request.method == "PATCH" and request.path:match("/profiles/documents/") and failProfile then
            return reply({ message = "Profile unavailable" }, 503)
        end
        return transport(request)
    end)
    assert(recovering:signIn("unique@example.invalid", "password123"))
    local result, saveError = recovering:updateProfile { username = "Recovered Name", bio = "Retry" }
    assert(not result and saveError.status == 503 and nameWrites == 2)
    assert(recovering:getSession().user.name == "Updated Name", "Failed profile saves must retain the published name")
    assert(state.users["my.handle"].name == (rollbackFails and "Recovered Name" or "Updated Name"))
    assert((saveError.code == "profile_sync") == rollbackFails, "Unrecoverable partial saves must explain retry recovery")
    failProfile = false
    assert(recovering:updateProfile { username = "Updated Name", bio = "Recovered" })
    assert(state.users["my.handle"].name == "Updated Name" and recovering:getSession().user.name == "Updated Name")
end
assert(identity:signOut())
_, err = identity:signUp("Another Person", "other@example.invalid", "password123", "password123", "MY.HANDLE")
assert(err.status == 409, "Canonical account names must collide even when email and display names differ")
for index, handle in ipairs { "", "has spaces", "_leading", string.rep("x", 37), "ümlaut" } do
    _, err = identity:signUp("Person", "valid@example.invalid", "password123", "password123", handle)
    assert(err.code == "validation", "Invalid account handles must fail before account creation")
end
print("PASS: projected startup queries, materialized and legacy counters, pagination, public cache contracts, image variants and readable account identity")
