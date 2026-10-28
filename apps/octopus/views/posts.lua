local C = require "Octopus/views/components"
local P = require "Octopus/views/presentation"
local Items = require "orca.UIKit.ItemsView"
local colors, space = C.colors, C.spacing
local Posts = {}

-- Type-specific content shared by timeline cards and the post page.
function Posts.body(app, post, full, data)
    local body = P.template("PostBody"):instantiate()
    if not data then
        data = P.record("Post"):instantiate()
        P.assign(data, P.post(app, post, full))
    end
    body.DataContext = data
    local frame = body:findChild("PhotoFrame", true)
    frame.Name = "Photo" .. post.id
    frame.AspectRatio, frame.Background = app.photos:aspect(post.imageId), colors.raised
    if post.imageId and (post.postType == "photo" or post.postType == "quote") then
        app.photos:watch(frame, post.imageId, "Image" .. post.id)
    end
    return body
end

function Posts.tags(app, post)
    if #post.tags == 0 then return nil end
    -- Let native text measurement size UTF-8 labels; byte counts are not glyph widths.
    local tags = C.row({ Name = "Tags" .. post.id, Spacing = space.inline, Height = 28 })
    for _, tag in ipairs(post.tags) do
        tags:addChild(C.button("#" .. tag, function() app:openTag(tag) end, {
            Name = "Tag" .. tag, Height = 28, FontSize = 14,
            TextWrapping = "NoWrap" }))
    end
    return tags
end

-- Like or repost, then call `done` to redraw whatever shows the counts.
function Posts.toggle(app, post, entry, kind, done)
    if not app:requireSession(kind == "like" and "Sign in to like posts." or "Sign in to repost.") then return end
    if entry.busy or entry.pending then return end
    entry.busy = true
    local session = app.sessionEpoch
    app:task(function()
        if app.sessionEpoch ~= session then entry.busy = false; return end
        local result, err
        if kind == "like" then result, err = app.client:toggleLike(post.id, "post", entry.liked)
        else result, err = app.client:toggleRepost(post.id, entry.reposted) end
        entry.busy = false
        if app.sessionEpoch ~= session then return end
        if result == nil then app:notice(err.message, true); return end
        local field, count = kind == "like" and "liked" or "reposted", kind == "like" and "likes" or "reposts"
        local before = entry[field] ~= nil
        entry[field] = result or nil
        entry[count] = math.max(0, (entry[count] or 0) + ((result and 1 or 0) - (before and 1 or 0)))
        if done then done() end
        app:repaint()
    end)
end

-- Layout belongs to PostCard/PostBody. This hook supplies actions and existing
-- avatar/photo services after the collection assigns the row's DataContext.
local function updateCard(app, entry, item, created)
    local post, row = item.post, entry.node
    row.Name = "Post" .. post.id
    if created then
        entry.controls = {}
        for _, name in ipairs { "Avatar", "Author", "Open", "RepostedBy", "Comment", "Repost", "Like", "Actions", "TagsHost" } do
            entry.controls[name] = assert(row:findChild(name, true))
        end
        entry.controls.Avatar:addChild(C.avatar(post.authorName, C.AVATAR, post.authorId, app))
    end
    local controls = entry.controls
    if not entry.body or entry.imageId ~= post.imageId then
        -- Keep decoded photos during ordinary refresh; a changed media identity
        -- gets a new frame so late downloads can only reach the detached body.
        C.clear(controls.Open)
        entry.body = Posts.body(app, post, false, entry.data)
        controls.Open:addChild(entry.body)
        entry.imageId = post.imageId
    end
    for name, control in pairs(controls) do
        if name ~= "TagsHost" then control.Name = name .. post.id end
    end
    local function author() app:openProfile(item.post.authorId, item.post.authorName) end
    controls.Avatar.Click, controls.Author.Click = author, author
    controls.Open.Click = function() app:openPost(item.post) end
    controls.Comment.Click = controls.Open.Click
    controls.RepostedBy.Click = function()
        if item.repostedBy then app:openProfile(item.repostedBy.id, item.repostedBy.name) end
    end
    for _, control in ipairs { controls.Like, controls.Repost } do
        control.IgnoreHitTest, control.Opacity = item.entry.pending == true, item.entry.pending and 0.5 or 1
    end
    local function refreshCounts()
        P.assign(entry.data, P.post(app, item.post, false, item.entry, item.repostedBy))
    end
    controls.Like.Click = function() Posts.toggle(app, item.post, item.entry, "like", refreshCounts) end
    controls.Repost.Click = function() Posts.toggle(app, item.post, item.entry, "repost", refreshCounts) end
    local tagsKey = table.concat(post.tags or {}, "\0")
    if entry.tagsKey ~= tagsKey then
        C.clear(controls.TagsHost)
        local tags = Posts.tags(app, post)
        if tags then controls.TagsHost:addChild(tags) end
        entry.tagsKey = tagsKey
    end
end

function Posts.card(app, post, entry, repostedBy)
    local data = P.record("Post"):instantiate()
    P.assign(data, P.post(app, post, false, entry, repostedBy))
    local row = P.template("PostCard"):instantiate()
    row.DataContext = data
    updateCard(app, { node = row, data = data }, {
        post = post, entry = entry or { pending = true }, repostedBy = repostedBy,
    }, true)
    return row
end

local collections = setmetatable({}, { __mode = "k" })
local function collection(app, list)
    local state = collections[list]
    if not state or state.node:getParent() ~= list then
        local node = C.stack { Name = "PostItems", Spacing = space.post }
        list:addChild(node)
        state = { node = node, values = {} }
        state.adapter = Items.new {
            node = node, template = P.template("PostCard"), record = P.record("Post"),
            key = function(item) return item.key end,
            fields = function(item) return P.post(app, item.post, false, item.entry, item.repostedBy) end,
            update = function(entry, item, created) updateCard(app, entry, item, created) end,
        }
        collections[list] = state
    end
    return state
end

local function values(items, stats)
    local result, occurrences = {}, {}
    for _, item in ipairs(items) do
        local post, repost = item.post, item.repostedBy
        stats[post.id] = stats[post.id] or item.entry or { pending = true }
        local identity = post.id .. (repost and ":repost:" .. repost.id or ":post")
        occurrences[identity] = (occurrences[identity] or 0) + 1
        result[#result + 1] = { key = identity .. ":" .. occurrences[identity], post = post,
            entry = stats[post.id], repostedBy = repost }
    end
    return result
end

function Posts.refresh(app, list, items, stats)
    local state = collection(app, list)
    state.values = values(items, stats)
    return state.adapter:refresh(state.values)
end

-- Timeline items are { post, repostedBy }; plain post lists are wrapped.
function Posts.items(posts)
    local items = {}
    for i, post in ipairs(posts) do items[i] = { post = post } end
    return items
end

-- Cards register photos for loading when their frame enters the viewport.
function Posts.append(app, list, items, stats)
    local state = collection(app, list)
    local combined = {}
    for _, value in ipairs(state.values) do combined[#combined + 1] = value end
    for _, item in ipairs(items) do combined[#combined + 1] = item end
    state.values = values(combined, stats)
    return state.adapter:refresh(state.values)
end

-- Fetch content first. Counts and reader reaction IDs arrive independently.
function Posts.fetch(app, loader)
    local items, cursor = loader()
    if not items then return nil, cursor end
    local stats = {}
    for _, item in ipairs(items) do
        local entry = { pending = true }
        for key, value in pairs(item.post.stats or {}) do entry[key] = value end
        entry.pending = true
        stats[item.post.id] = entry
    end
    return items, stats, cursor
end

-- Enrichment changes only row fields, preserving all loaded pages and their order.
function Posts.refreshStats(app, list, items, stats)
    local state = collections[list]
    if not state or state.node:getParent() ~= list then return end
    for _, value in ipairs(state.values) do
        if stats[value.post.id] then value.entry = stats[value.post.id] end
    end
    return state.adapter:refresh(state.values)
end

-- Keep entry identities stable: visible cards and detail controls share them.
-- Failed enrichment leaves unknown counts pending and reactions unavailable.
function Posts.enrich(app, items, entries, current, changed)
    local ids, posts, seen = {}, {}, {}
    for _, item in ipairs(items) do
        if not seen[item.post.id] then
            seen[item.post.id] = true
            ids[#ids + 1], posts[#posts + 1] = item.post.id, item.post
        end
    end
    if #ids == 0 then return end
    local session = app.sessionEpoch
    app:task(function()
        if app.sessionEpoch ~= session or not current() then return end
        local stats = app.client:stats(ids, nil, posts)
        if app.sessionEpoch ~= session or not current() then return end
        if not stats then return end
        for id, values in pairs(stats) do
            local entry = entries[id]
            if entry then
                for key in pairs(entry) do entry[key] = nil end
                for key, value in pairs(values) do entry[key] = value end
            end
        end
        changed()
        app:repaint()
    end)
end

return Posts
