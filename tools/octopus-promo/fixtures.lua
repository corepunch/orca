-- Fictional, deterministic, offline-only App Store screenshot content for
-- Octopus. People and portraits match tools/chat-promo/fixtures.lua, so the
-- two apps' store pages share one cast. Ages are seconds before `now`.
local MIN, HOUR, DAY = 60, 3600, 86400
local F = {}
F.now = 1793180460 -- 2026-10-28 09:41 UTC; independent of the host time zone.

-- { id, name, candid portrait number, bio }
local PEOPLE = {
    { "u-sam", "Sam Taylor", 24, "Notes from a small studio. Coffee, type and long walks." },
    { "u-alex", "Alex Morgan", 2, "Trains, maps and the occasional very long thread." },
    { "u-maya", "Maya Chen", 5, "Interface designer. Collects tide tables and typefaces." },
    { "u-jules", "Jules Park", 16, "Photographs mountains, mostly by accident." },
    { "u-nina", "Nina Patel", 13, "Reads too much. Recommends all of it." },
    { "u-leo", "Leo Martin", 26, "Writes about small software and the people who make it." },
    { "u-ella", "Ella Brooks", 17, "Links worth your evening. No hot takes." },
    { "u-ben", "Ben Wilson", 18, "Marine biologist. Part-time octopus whisperer." },
    { "u-sofia", "Sofia Rossi", 3, "Runs a book club and a very small bakery." },
    { "u-omar", "Omar Hassan", 8, "Field notes from the shoreline." },
    { "u-amelia", "Amelia Reed", 1, "Quotes I keep coming back to." },
    { "u-noah", "Noah Clarke", 4, "Old phones, new ideas." },
}

F.users, F.avatars = {}, {}
for _, p in ipairs(PEOPLE) do
    local avatar = string.format("profile-%02d", p[3])
    F.avatars[#F.avatars + 1] = avatar
    F.users[#F.users + 1] = { id = p[1], avatar = avatar, name = p[2], bio = p[4], password = "password123",
        email = p[1] == "u-sam" and "demo@example.invalid" or p[1]:sub(3) .. "@example.invalid" }
end

local function avatar(id)
    for _, p in ipairs(PEOPLE) do if p[1] == id then return string.format("f-avatar-profile-%02d", p[3]) end end
    error("Unknown promo person " .. id)
end

-- Extra photos served by the mock's storage; Capture.lua loads them.
F.files = { ["f-lake"] = { path = "Octopus/promo/assets/lake.jpg", owner = "u-jules" } }

local function post(id, age, kind, author, fields)
    local name
    for _, p in ipairs(PEOPLE) do if p[1] == author then name = p[2] end end
    local row = { id = id, age = age, postType = kind, title = "", authorId = author, authorName = name, content = "" }
    for k, v in pairs(fields) do row[k] = v end
    return row
end

local album = { avatar("u-nina"), avatar("u-leo"), avatar("u-amelia"), avatar("u-omar") }

local posts = {
    -- Home: Sam, the people Sam follows, and what they reposted.
    post("p-lake", 18 * MIN, "photo", "u-jules", { imageId = "f-lake", imageIds = { "f-lake" },
        imageWidths = { 1280 }, imageHeights = { 960 }, tags = { "outdoors", "weekend" },
        content = "Finally made it up to the lake. Worth every single switchback." }),
    post("p-eames", 52 * MIN, "quote", "u-maya", { quoteSource = "Charles Eames", tags = { "design" },
        content = "The details are not the details. They make the design.",
        userText = "Pinned above my desk since my first job." }),
    post("p-small", 2 * HOUR, "text", "u-leo", { title = "Small software is a feature", tags = { "software", "craft" },
        content = "The apps I love do one thing and leave room to breathe. They open fast, they forget nothing "
            .. "important, and they never ask me to **upgrade my workflow**.\n\nWrite less. Ship the part people use." }),
    post("p-bookclub", 3 * HOUR, "photo", "u-sofia", { imageId = album[1], imageIds = album,
        imageWidths = { 0, 0, 0, 0 }, imageHeights = { 0, 0, 0, 0 }, tags = { "bookclub" },
        content = "Book club, month twelve. Same table, a few new faces." }),
    post("p-rss", 5 * HOUR, "link", "u-ella", { title = "The quiet return of the feed reader",
        linkUrl = "https://example.com/feed-reader", tags = { "web", "reading" },
        content = "Chronological, calm and entirely yours. Why people are going back to following, not scrolling." }),
    post("p-hello", 9 * HOUR, "text", "u-sam", { title = "Hello, Octopus", tags = { "hello" },
        content = "First post from my phone. Long posts still read best on the website, "
            .. "but quick notes feel right at home here." }),
    post("p-nina", 14 * HOUR, "quote", "u-nina", { quoteSource = "Mary Oliver", tags = { "reading" },
        content = "Attention is the beginning of devotion.",
        userText = "Finished the essays on the train. Recommending them to everyone." }),
    post("p-studio", DAY + 2 * HOUR, "text", "u-sam", { title = "Studio notes, week 43", tags = { "studio" },
        content = "New desk lamp, old notebook. Three sketches for the poster and one I actually like." }),
    post("p-maya-grid", DAY + 6 * HOUR, "text", "u-maya", { title = "Pivot, don't panic", tags = { "design", "metro" },
        content = "Big lowercase headers, flat colour, generous margins. Old phone design still teaches clarity: "
            .. "let type do the work and get the chrome out of the way." }),
    post("p-maya-type", 2 * DAY, "link", "u-maya", { title = "A field guide to grotesque typefaces",
        linkUrl = "https://example.com/grotesques", tags = { "type" },
        content = "Twelve faces, one paragraph each, and why the boring ones are the good ones." }),
    -- Discover: people Sam doesn't follow yet.
    post("p-reef", 25 * MIN, "text", "u-ben", { title = "Three octopuses, one rock", tags = { "reef", "fieldnotes" },
        content = "Low tide this morning. Three octopuses were sharing the same rock, and only one of them "
            .. "was pretending not to notice me." }),
    post("p-trains", 70 * MIN, "text", "u-alex", { title = "Window seat, every time", tags = { "travel" },
        content = "Six hours, two changes, one excellent sandwich. Slow trains are the last place nobody "
            .. "expects a reply from you." }),
    post("p-kay", 2 * HOUR + 20 * MIN, "quote", "u-amelia", { quoteSource = "Alan Kay", tags = { "software" },
        content = "Simple things should be simple, complex things should be possible." }),
    post("p-arms", 4 * HOUR, "link", "u-omar", { title = "How octopuses taste with their arms",
        linkUrl = "https://example.com/octopus-arms", tags = { "science", "reef" },
        content = "Most of an octopus's neurons live in its arms, and the suckers can taste what they touch." }),
    post("p-phones", 6 * HOUR, "text", "u-noah", { title = "The phone that scrolled sideways",
        tags = { "design", "metro" },
        content = "Panoramas, pivots and live tiles: an interface that treated the screen as a window onto "
            .. "something wider. It still feels ahead of its time." }),
    post("p-tide", 8 * HOUR, "text", "u-omar", { title = "Field notes #12", tags = { "fieldnotes" },
        content = "Counted 41 hermit crabs, two sea hares and one very offended blenny. Water at 14 °C." }),
}

local follows, likes, reposts, comments = {}, {}, {}, {}
for _, id in ipairs { "u-maya", "u-jules", "u-nina", "u-leo", "u-ella", "u-sofia" } do
    follows[#follows + 1] = { followerId = "u-sam", followingId = id, age = 20 * DAY }
end
for _, id in ipairs { "u-jules", "u-leo", "u-nina", "u-ella" } do
    follows[#follows + 1] = { followerId = "u-maya", followingId = id, age = 30 * DAY }
end
for _, id in ipairs { "u-jules", "u-leo", "u-nina", "u-ella", "u-sofia", "u-alex", "u-noah", "u-amelia" } do
    follows[#follows + 1] = { followerId = id, followingId = "u-maya", age = 25 * DAY }
end
-- Connect: who followed Sam and what happened to Sam's posts.
for i, id in ipairs { "u-alex", "u-ben", "u-noah", "u-maya", "u-nina" } do
    follows[#follows + 1] = { followerId = id, followingId = "u-sam", age = (i * 47) * MIN }
end
local function like(target, user, age) likes[#likes + 1] = { targetId = target, targetType = "post", userId = user, age = age } end
local function repost(target, user, age) reposts[#reposts + 1] = { postId = target, userId = user, age = age } end
for i, id in ipairs { "u-maya", "u-jules", "u-alex", "u-nina", "u-leo", "u-ella", "u-sofia", "u-omar" } do
    like("p-hello", id, (8 * HOUR) - i * 20 * MIN)
end
for i, id in ipairs { "u-maya", "u-sam", "u-nina", "u-alex", "u-ella", "u-leo", "u-ben", "u-sofia", "u-amelia", "u-noah", "u-omar" } do
    like("p-lake", id, 16 * MIN - i * 60)
end
for i, id in ipairs { "u-sam", "u-jules", "u-leo", "u-noah", "u-amelia" } do like("p-eames", id, 45 * MIN - i * 60) end
for i, id in ipairs { "u-maya", "u-ella", "u-sam", "u-alex" } do like("p-small", id, HOUR + i * 60) end
for i, id in ipairs { "u-jules", "u-maya", "u-sam", "u-nina", "u-noah", "u-ella" } do like("p-reef", id, 20 * MIN - i * 60) end
for i, id in ipairs { "u-nina", "u-maya" } do like("p-studio", id, DAY - i * HOUR) end
like("p-bookclub", "u-sam", 2 * HOUR)
like("p-rss", "u-sam", 4 * HOUR)
repost("p-lake", "u-maya", 12 * MIN)
repost("p-lake", "u-alex", 10 * MIN)
repost("p-hello", "u-leo", 7 * HOUR)
repost("p-hello", "u-alex", 6 * HOUR)
repost("p-small", "u-maya", 90 * MIN)
repost("p-trains", "u-nina", 40 * MIN) -- shows "Nina Patel reposted" on home
repost("p-kay", "u-leo", 2 * HOUR)
repost("p-reef", "u-ella", 15 * MIN)

local function comment(id, postId, author, body, age, parent)
    local name
    for _, p in ipairs(PEOPLE) do if p[1] == author then name = p[2] end end
    comments[#comments + 1] = { id = id, postId = postId, authorId = author, authorName = name,
        parentId = parent or "", body = body, age = age }
end
comment("c-lake-1", "p-lake", "u-maya", "That colour can't be real. Adding it to the list.", 15 * MIN)
comment("c-lake-2", "p-lake", "u-jules", "It's even greener in person. Go early, before the crowds.", 13 * MIN, "c-lake-1")
comment("c-lake-3", "p-lake", "u-sam", "Saving this for the long weekend.", 11 * MIN)
comment("c-lake-4", "p-lake", "u-alex", "Which trailhead did you start from?", 8 * MIN)
comment("c-lake-5", "p-lake", "u-jules", "The one past the old station. Window seat on the way up.", 6 * MIN, "c-lake-4")
comment("c-hello-1", "p-hello", "u-maya", "Welcome aboard! The pivot headers look great on a phone.", 8 * HOUR)
comment("c-hello-2", "p-hello", "u-nina", "Hello from the website side.", 7 * HOUR + 30 * MIN)
comment("c-hello-3", "p-hello", "u-alex", "Quick notes are the best notes.", 5 * HOUR)
comment("c-studio-1", "p-studio", "u-maya", "Post the one you like!", DAY)
comment("c-reef-1", "p-reef", "u-jules", "Three! Were they getting along?", 18 * MIN)
comment("c-reef-2", "p-reef", "u-ben", "Mostly. One kept changing colour at the others.", 16 * MIN, "c-reef-1")

F.collections = { posts = posts, follows = follows, likes = likes, reposts = reposts, comments = comments }
return F
