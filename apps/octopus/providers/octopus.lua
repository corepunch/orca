-- The Octopus provider: timeline operations, stats enrichment, reactions,
-- computed presentation fields and photo/avatar media behind the names that
-- Data/*.xml declares. The service client keeps its API; views never call it.
--
-- context: client, media (files), clock(), now(), photos (decoded ratios),
-- openPost(raw), openProfile(id, name), openTag(tag), requireSession(reason),
-- notice(text, isError), signIn(route), signUp(route), selectFeed(name).
local Format = require "Octopus/format"
local Schema = require "orca.data.schema"
local Provider = {}

local LABELS = { text = "post", photo = "photo", quote = "quote", link = "link" }
local TALLEST, WIDEST, GUESS = 4 / 5, 1.91, 4 / 3
local ACTIVITY = {
    like = { icon = "heart", text = " liked your post" },
    repost = { icon = "repeat-2", text = " reposted your post" },
    comment = { icon = "message-circle", text = " replied" },
    follow = { icon = "user-round", text = " followed you" },
}
local PAGE_SIZE = 20

local function number(value)
    if value == nil then return "–" end
    if value >= 10000 then return string.format("%.0fk", value / 1000) end
    if value >= 1000 then return string.format("%.1fk", value / 1000) end
    return tostring(value)
end
local function count(value) return value == nil and "…" or (value > 0 and tostring(value) or "") end
local function tile(value) return value == nil and "…" or tostring(value) end
local function label(value, singular, plural) return value == 1 and singular or plural end
local function clamp(ratio) return math.max(TALLEST, math.min(WIDEST, ratio)) end

-- Persisted dimensions first, then the decoded shape, then a guess.
local function aspect(width, height, learned)
    if type(width) == "number" and type(height) == "number" and width > 0 and height > 0
        and width <= 65535 and height <= 65535 then
        return clamp(width / height)
    end
    if learned and learned > 0 then return clamp(learned) end
    return GUESS
end
Provider.aspect = aspect

-- Runs a search: both queries take the trimmed text; the same text again
-- refreshes. An empty query returns the page to its suggestions.
function Provider.search(scope, text)
    text = (text or ""):match("^%s*(.-)%s*$")
    scope.store:put("OctopusSearch", { id = "search", query = text })
    for _, name in ipairs { "SearchPosts", "SearchPeople" } do
        local query = scope:member(name)
        if not query:set("Query", text) and text ~= "" then query:refresh() end
    end
end

function Provider.new(context)
    local client = context.client
    local computed = {
        hasPhoto = function(v) return v.PostType == "photo" or (v.PostType == "quote" and v.ImageId ~= nil) end,
        isQuote = function(v) return v.PostType == "quote" end,
        isLink = function(v) return v.PostType == "link" end,
        isText = function(v) return v.PostType ~= "photo" and v.PostType ~= "quote" and v.PostType ~= "link" end,
        kindLabel = function(v) return LABELS[v.PostType] or "post" end,
        hasCaption = function(v) return v.PostType == "photo" and (v.Content or "") ~= "" end,
        excerpt = function(v) return Format.excerpt(v.Content) end,
        plain = function(v) return Format.plain(v.Content) end,
        quote = function(v) return "“" .. Format.plain(v.Content) .. "”" end,
        hasQuoteSource = function(v) return v.PostType == "quote" and (v.QuoteSource or "") ~= "" end,
        quoteSource = function(v) return "— " .. (v.QuoteSource or "") end,
        hasThoughts = function(v) return v.PostType == "quote" and (v.UserText or "") ~= "" end,
        thoughts = function(v) return v.UserText or "" end,
        linkTitle = function(v) return (v.Title or "") ~= "" and v.Title or Format.domain(v.LinkUrl) end,
        domain = function(v) return Format.domain(v.LinkUrl) end,
        hasContent = function(v)
            return (v.PostType == "link" and (v.Content or "") ~= "")
                or (v.PostType ~= "photo" and v.PostType ~= "quote" and v.PostType ~= "link")
        end,
        age = function(v) return Format.ago(v.CreatedAt, context.now()) end,
        stamp = function(v) return Format.stamp(v.CreatedAt) end,
        hasTags = function(_, record) return #(record.children.Tags or {}) > 0 end,
        likesText = function(v) return count(v.Likes) end,
        repostsText = function(v) return count(v.Reposts) end,
        commentsText = function(v) return count(v.Comments) end,
        likeCount = function(v) return tile(v.Likes) end,
        repostCount = function(v) return tile(v.Reposts) end,
        commentCount = function(v) return tile(v.Comments) end,
        likesLabel = function(v) return label(v.Likes, "like", "likes") end,
        repostsLabel = function(v) return label(v.Reposts, "repost", "reposts") end,
        commentsLabel = function(v) return label(v.Comments, "comment", "comments") end,
        liked = function(v) return v.LikeId ~= nil end,
        reposted = function(v) return v.RepostId ~= nil end,
        likeText = function(v) return v.LikeId ~= nil and "unlike" or "like" end,
        repostText = function(v) return v.RepostId ~= nil and "undo" or "repost" end,
        canReact = function(v) return v.StatsReady and not v.Busy end,
        locked = function(v) return not v.CanReact end,
        postState = function(v)
            local classes = {}
            if v.Liked then classes[#classes + 1] = "liked" end
            if v.Reposted then classes[#classes + 1] = "reposted" end
            if not v.CanReact then classes[#classes + 1] = "pending" end
            return table.concat(classes, " ")
        end,
        aspect = function(v) return aspect(v.ImageWidth, v.ImageHeight, v.PhotoRatio) end,
        photoShown = function(v) return (v.PhotoPath or "") ~= "" end,
        photoFrame = function(v) return (v.PhotoPath or "") ~= "" and "loaded" or "" end,
        photoLoading = function(v) return v.PhotoState == "loading" or v.PhotoState == "preview" end,
        photoFailed = function(v) return v.PhotoState == "error" end,
        photoPending = function(v) return (v.PhotoPath or "") == "" and v.PhotoState ~= "error" end,
        initials = function(v) return Format.initials(v.AuthorName or v.Person) end,
        tone = function(v) return "tone" .. Format.tone(v.AuthorId or v.UserId or "", 6) end,
        hasAvatar = function(v) return (v.AvatarPath or "") ~= "" end,
        hashTag = function(v) return "#" .. (v.Label or "") end,
        isRepost = function(v) return v.RepostedById ~= nil end,
        repostedText = function(v) return v.RepostedByName and v.RepostedByName .. " reposted" or "" end,
        -- Below the author line and, on reposts, the repost line (24 + 4 + 20 + 4).
        avatarTop = function(v) return 28 + (v.RepostedById ~= nil and 24 or 0) end,
        activityLine = function(v) return v.Person .. (ACTIVITY[v.Kind] or ACTIVITY.follow).text end,
        activityIcon = function(v) return "Octopus/assets/icons/" .. (ACTIVITY[v.Kind] or ACTIVITY.follow).icon .. ".svg" end,
        activityAge = function(v) return Format.ago(v.At, context.now()) end,
        activityHasBody = function(v) return v.Body ~= nil end,
        activityExcerpt = function(v) return Format.excerpt(v.Body, 140) end,
        activityHasPost = function(v) return v.PostId ~= nil end,
        activitySummary = function(v) return v.PostSummary ~= nil and v.PostSummary ~= "" and v.PostSummary or "your photo" end,
        isGuest = function(v) return not v.SignedIn end,
        personName = function(v) return v.DisplayName or "" end,
        searchHeading = function(v)
            local tag = (v.Query or ""):match("^%s*#(.-)%s*$")
            return tag and "#" .. tag:lower() or "posts"
        end,
        searchMissing = function(v)
            return "No posts or people match “" .. (v.Query or "") .. "”. Try another word or tag."
        end,
        bioText = function(v)
            if (v.Bio or "") ~= "" then return v.Bio end
            return v.Own and "Add a short bio below." or ""
        end,
        bioExcerpt = function(v) return Format.excerpt(v.Bio or "", 80) end,
        hasBio = function(v) return (v.Bio or "") ~= "" end,
        postsText = function(v) return number(v.PostCount) end,
        followersText = function(v) return number(v.Followers) end,
        followingText = function(v) return number(v.FollowingCount) end,
        isFollowing = function(v) return v.FollowId ~= nil and v.FollowId ~= "" end,
        followText = function(v) return (v.FollowId or "") ~= "" and "Unfollow" or "Follow" end,
        followStyle = function(v) return (v.FollowId or "") ~= "" and "octopus-outline" or "octopus-primary" end,
        canFollow = function(v) return v.FollowKnown and not v.Own end,
        personInitials = function(v) return Format.initials(v.DisplayName or "?") end,
        personTone = function(v) return "tone" .. Format.tone(v.UserId or "", 6) end,
    }

    local function postOf(record)
        if not record then return nil end
        if record.entity.name == "OctopusPost" then return record end
        return record.related and record.related.Post
            or record.values.PostId and record.store:get("OctopusPost", record.values.PostId)
    end

    -- Timeline rows: one entry per post per feed, the first occurrence wins
    -- (a post and its repost never both appear). Stats load separately.
    local function entries(query, items)
        local store, rows, seen = query.scope.store, {}, {}
        store:batch(function()
            for _, item in ipairs(items or {}) do
                local post = item.post
                if post and not seen[post.id] then
                    seen[post.id] = true
                    store:put("OctopusPost", post, { locals = { StatsReady = false } })
                    rows[#rows + 1] = store:put("OctopusEntry", { key = query.name .. ":" .. post.id, postId = post.id,
                        repostedBy = item.repostedBy })
                end
            end
        end)
        return rows
    end
    local function wrap(posts)
        local items = {}
        for i, post in ipairs(posts or {}) do items[i] = { post = post } end
        return items
    end

    local operations = {
        home = function(_, page, _, query)
            local items, cursor = client:homeTimeline { limit = PAGE_SIZE, before = page.cursor }
            if not items then return nil, cursor end
            return { items = entries(query, items), cursor = cursor }
        end,
        discover = function(_, page, _, query)
            local posts, err = client:listPosts { limit = PAGE_SIZE, cursor = page.cursor }
            if not posts then return nil, err end
            return { items = entries(query, wrap(posts)), cursor = #posts == PAGE_SIZE and posts[#posts].id or nil }
        end,
        profilePosts = function(params, _, _, query)
            local items, err = client:userTimeline(params.AuthorId, { limit = 30 })
            if not items then return nil, err end
            return entries(query, items)
        end,
        searchPosts = function(params, _, _, query)
            local text = (params.Query or ""):match("^%s*(.-)%s*$")
            local tag = text:match("^#(.+)$")
            local posts, err = client:listPosts(tag and { tag = tag, limit = 30 } or { search = text, limit = 20 })
            if not posts then return nil, err end
            return entries(query, wrap(posts))
        end,
        connect = function()
            local items, err = client:activity()
            if not items then return nil, err end
            local rows = {}
            for index, item in ipairs(items) do
                local post = item.post
                rows[index] = { key = table.concat({ item.kind, item.userId, item.at or "", post and post.id or "" }, ":"),
                    position = tostring(index), kind = item.kind, userId = item.userId, name = item.name,
                    at = item.at, body = item.body, post = post,
                    summary = post and (post.title ~= "" and post.title or Format.excerpt(post.content, 90)) or nil }
            end
            return rows
        end,
        openPost = function(params)
            local item = params.item
            if item.entity.name == "OctopusActivity" then
                if item.raw.post then context.openPost(item.raw.post)
                else context.openProfile(item.values.UserId, item.values.Person) end
                return true
            end
            context.openPost(postOf(item).raw)
            return true
        end,
        openAuthor = function(params)
            local post = postOf(params.item)
            context.openProfile(post.values.AuthorId, post.values.AuthorName)
            return true
        end,
        openReposter = function(params)
            local entry = params.item
            if entry.values.RepostedById then context.openProfile(entry.values.RepostedById, entry.values.RepostedByName) end
            return true
        end,
        openTag = function(params)
            context.openTag(params.item.values.Label)
            return true
        end,
    }

    -- People -------------------------------------------------------------

    operations.person = function(params, _, _, query)
        local profile, err = client:getProfile(params.UserId)
        if not profile then return nil, err end
        -- A missing profile keeps the name the page was opened with.
        if not profile.exists then profile.username = nil end
        return { query.scope.store:put("OctopusProfile", profile, { partial = true }) }
    end
    operations.followState = function(params, _, _, query)
        local state, err = client:followState(params.UserId)
        if state == nil then return nil, err end
        return { query.scope.store:put("OctopusProfile", { userId = params.UserId }, { partial = true,
            locals = { FollowId = state or "", FollowKnown = true } }) }
    end
    operations.follow = function(_, _, command)
        local person = command.scope:exposedRecord("Profile")
        if not person or not context.requireSession("Sign in to follow people.") then return true end
        if person.values.FollowBusy then return true end
        local store, scope = person.store, command.scope
        local token = scope:token()
        store:update(person, { FollowBusy = true })
        local following = (person.values.FollowId or "") ~= ""
        local result, err
        if following then result, err = client:unfollow(person.key, person.values.FollowId)
        else result, err = client:follow(person.key) end
        if not scope:valid(token) then return nil, { code = "stale" } end
        store:update(person, { FollowBusy = false })
        if result == nil then context.notice(err.message, true); return nil, err end
        local now = result and true or false
        store:update(person, { FollowId = result or "" })
        if person.values.Followers then
            store:put("OctopusProfile", { userId = person.key,
                followers = person.values.Followers + (now and 1 or 0) - (following and 1 or 0) }, { partial = true })
        end
        context.invalidateFeeds { "Home" }
        context.notice(now and "You'll see their posts in your following timeline." or "Unfollowed.")
        return true
    end
    operations.searchPeople = function(params, _, _, query)
        local text = (params.Query or ""):match("^%s*(.-)%s*$")
        if text:match("^#") then return {} end
        local people, err = client:searchPeople(text)
        if not people then return nil, err end
        local rows = {}
        for _, person in ipairs(people) do
            rows[#rows + 1] = query.scope.store:put("OctopusProfile", person, { partial = true })
        end
        return rows
    end
    -- The most used tags among posts this reader has loaded.
    operations.trending = function(_, _, _, query)
        local counts, tags = {}, {}
        for _, post in pairs(query.scope.store.records.OctopusPost or {}) do
            for _, tag in ipairs(post.children.Tags or {}) do
                local label = tag.values.Label
                if not counts[label] then tags[#tags + 1] = label end
                counts[label] = (counts[label] or 0) + 1
            end
        end
        table.sort(tags, function(a, b) return counts[a] > counts[b] or (counts[a] == counts[b] and a < b) end)
        if #tags == 0 then tags = { "design", "software", "reef" } end
        local rows = {}
        for index = 1, math.min(#tags, 6) do rows[index] = { value = tags[index] } end
        return rows
    end
    operations.openPerson = function(params)
        context.openProfile(params.item.key, params.item.values.DisplayName)
        return true
    end

    -- Like or repost; counts adjust from the service's answer.
    local function react(kind)
        return function(params, _, command)
            local post = postOf(params.item)
            if not post then return true end
            if not context.requireSession(kind == "like" and "Sign in to like posts." or "Sign in to repost.") then return true end
            if post.values.Busy or not post.values.StatsReady then return true end
            local store, scope = post.store, command.scope
            local token = scope:token()
            store:update(post, { Busy = true })
            local result, err
            if kind == "like" then result, err = client:toggleLike(post.key, "post", post.values.LikeId)
            else result, err = client:toggleRepost(post.key, post.values.RepostId) end
            if not scope:valid(token) then return nil, { code = "stale" } end
            store:update(post, { Busy = false })
            if result == nil then context.notice(err.message, true); return nil, err end
            local id, total = kind == "like" and "LikeId" or "RepostId", kind == "like" and "Likes" or "Reposts"
            local before = post.values[id] ~= nil
            local stats = {}
            stats[kind == "like" and "liked" or "reposted"] = result or Schema.null
            stats[kind == "like" and "likes" or "reposts"] = math.max(0, (post.values[total] or 0)
                + ((result and 1 or 0) - (before and 1 or 0)))
            store:put("OctopusPost", { id = post.key, stats = stats }, { partial = true })
            return true
        end
    end
    operations.like, operations.repost = react("like"), react("repost")

    -- Counts and reaction IDs arrive after the content; until then actions
    -- stay unavailable and counts read "…".
    local function enrich(query, rows)
        local store, ids, posts, seen = query.scope.store, {}, {}, {}
        for _, row in ipairs(rows) do
            local post = postOf(row)
            if post and not seen[post.key] then
                seen[post.key] = true
                ids[#ids + 1], posts[#posts + 1] = post.key, post.raw
            end
        end
        if #ids == 0 then return end
        local scope, token = query.scope, query.scope:token()
        scope:run(function()
            local stats = client:stats(ids, nil, posts)
            if not stats or not scope:valid(token) then return end
            store:batch(function()
                for id, values in pairs(stats) do
                    if store:get("OctopusPost", id) then
                        store:put("OctopusPost", { id = id, stats = { likes = values.likes, reposts = values.reposts,
                            comments = values.comments, liked = values.liked or Schema.null,
                            reposted = values.reposted or Schema.null } },
                            { partial = true, locals = { StatsReady = true } })
                    end
                end
            end)
            if context.repaint then context.repaint() end
        end)
    end
    local hooks = {
        enrich = function(query, rows) enrich(query, rows) end,
        discoverLoaded = function(query, rows, mode)
            if mode ~= "more" and client.writeFeedCache then
                local items = {}
                for _, row in ipairs(rows) do
                    local post = postOf(row)
                    if post then items[#items + 1] = { post = post.raw, repostedBy = row.raw.repostedBy } end
                end
                client:writeFeedCache("discover", items, query.cursor)
            end
            enrich(query, rows)
        end,
    }
    local caches = {
        discover = function(_, _, query)
            if not client.readFeedCache then return nil end
            local items, cursor = client:readFeedCache("discover")
            if not items or #items == 0 then return nil end
            return entries(query, items), cursor
        end,
    }

    local media = {
        photoFetch = function(file, _, _, preview)
            return client:getImage(file, "photo", function(bytes) return preview(bytes) end)
        end,
        photoCached = function(file) return context.media:path("photo:" .. file) end,
        photoStore = function(bytes, file) return context.media:store(bytes, "photo:" .. file) end,
        photoPreview = function(bytes) return context.media:storePreview(bytes) end,
        photoDiscard = function(file) context.media:discard("photo:" .. file) end,
        photoValidate = function(path) return require("orca.data.media").decodes(path) end,
        avatarLookup = function(ids) return client:avatars(ids) end,
        avatarFetch = function(file) return client:getImage(file, "avatar") end,
        avatarCached = function(file) return context.media:path("avatar:" .. file) end,
        avatarStore = function(bytes, file) return context.media:store(bytes, "avatar:" .. file) end,
    }
    local handlers = {
        SelectFeed = function(_, params)
            local feeds = { "home", "connect", "discover" }
            context.selectFeed(feeds[(params.sender.SelectedIndex or 0) + 1])
        end,
        SignIn = function() context.signIn("/") end,
        SignUp = function() context.signUp("/") end,
        SignInToFollow = function() context.signIn("/user") end,
        RunSearch = function(scope, params)
            local field = params.sender and (params.sender.Name == "SearchInput" and params.sender
                or params.sender:getParent():findChild("SearchInput"))
            Provider.search(scope, field and field.Text or "")
        end,
    }
    return { computed = computed, operations = operations, hooks = hooks, caches = caches, media = media,
        handlers = handlers, postOf = postOf, repaint = context.repaint }
end

return Provider
