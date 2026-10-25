local ui = require "orca.UIKit"
local C = require "Octopus/views/components"
local colors = C.colors
local Posts = {}

local PHOTO_HEIGHT = 196

-- Type-specific content shared by timeline cards and the post page.
function Posts.body(app, post, full)
    local body = C.stack({ Spacing = 8, IgnoreHitTest = true })
    local kind = post.postType
    if kind == "photo" then
        local frame = ui.Grid { Name = "Photo" .. post.id, Height = full and 260 or PHOTO_HEIGHT,
            Background = colors.raised, IgnoreHitTest = true }
        frame:addChild(C.icon("image", 28, colors.dim))
        body:addChild(frame)
        if post.content ~= "" then
            body:addChild(C.text(full and C.plain(post.content) or C.excerpt(post.content), { FontSize = 15, WordWrap = true }))
        end
    elseif kind == "quote" then
        -- A quote sits beside an accent rule, like a Metro pull quote.
        local quote = ui.Grid { Columns = "4px 1fr", Spacing = 14, IgnoreHitTest = true }
        quote:addChild(ui.Grid { Background = colors.accent, IgnoreHitTest = true })
        local words = C.stack({ Spacing = 6, IgnoreHitTest = true }, {
            C.text("“" .. C.plain(post.content) .. "”", { FontSize = full and 26 or 21, WordWrap = true }),
        })
        if post.quoteSource and post.quoteSource ~= "" then
            words:addChild(C.text("— " .. post.quoteSource, { FontSize = 14, Foreground = colors.muted, WordWrap = true }))
        end
        quote:addChild(words)
        body:addChild(quote)
        if post.userText and post.userText ~= "" then
            body:addChild(C.text(post.userText, { FontSize = 15, WordWrap = true }))
        end
    elseif kind == "link" then
        local link = ui.Grid { Columns = "40px 1fr", Spacing = 12, IgnoreHitTest = true }
        local badge = ui.Grid { Width = 40, Height = 40, Background = colors.accent, IgnoreHitTest = true }
        badge:addChild(C.icon("link", 20, colors.onAccent))
        link:addChild(badge)
        link:addChild(C.stack({ Spacing = 2, IgnoreHitTest = true }, {
            C.text(post.title ~= "" and post.title or C.domain(post.linkUrl), {
                FontSize = full and 22 or 17, Foreground = colors.accentText, WordWrap = true }),
            C.text(C.domain(post.linkUrl), { FontSize = 13, Foreground = colors.dim, TextOverflow = "Ellipsis" }),
        }))
        body:addChild(link)
        if post.content ~= "" then
            body:addChild(C.text(full and C.plain(post.content) or C.excerpt(post.content), {
                FontSize = 15, Foreground = full and colors.ink or colors.muted, WordWrap = true }))
        end
    else
        body:addChild(C.text(post.title, { FontSize = full and 30 or 20, WordWrap = true }))
        body:addChild(C.text(full and C.plain(post.content) or C.excerpt(post.content), {
            FontSize = full and 16 or 15, Foreground = full and colors.ink or colors.muted, WordWrap = true }))
    end
    return body
end

function Posts.tags(app, post)
    if #post.tags == 0 then return nil end
    local tags = C.row({ Spacing = 14, Height = 28 })
    for _, tag in ipairs(post.tags) do
        tags:addChild(C.button("#" .. tag, function() app:openTag(tag) end, {
            Name = "Tag" .. tag, Height = 28, FontSize = 14, Width = math.max(36, #tag * 8 + 18) }))
    end
    return tags
end

-- Comment, repost and like with counts. Liked and reposted states take the
-- Twitter colours so they read at a glance.
function Posts.fillActions(app, row, post, entry)
    C.clear(row)
    local function refill() Posts.fillActions(app, row, post, entry) end
    local function action(name, icon, count, active, color, handler)
        local tint = active and color or colors.dim
        local button = C.area(C.row({ Spacing = 8, IgnoreHitTest = true }, {
            C.icon(icon, 18, tint, { MarginLeft = 0, MarginRight = 0 }),
            C.text(count > 0 and tostring(count) or "", { FontSize = 13, Foreground = tint }),
        }), handler, { Name = name .. post.id, Height = 36 })
        row:addChild(button)
    end
    action("Comment", "message-circle", entry.comments or 0, false, colors.ink, function() app:openPost(post) end)
    action("Repost", "repeat-2", entry.reposts or 0, entry.reposted ~= nil, colors.reposted,
        function() Posts.toggle(app, post, entry, "repost", refill) end)
    action("Like", "heart", entry.likes or 0, entry.liked ~= nil, colors.liked,
        function() Posts.toggle(app, post, entry, "like", refill) end)
end

-- Like or repost, then call `done` to redraw whatever shows the counts.
function Posts.toggle(app, post, entry, kind, done)
    if not app:requireSession(kind == "like" and "Sign in to like posts." or "Sign in to repost.") then return end
    if entry.busy then return end
    entry.busy = true
    app:task(function()
        local result, err
        if kind == "like" then result, err = app.client:toggleLike(post.id, "post", entry.liked)
        else result, err = app.client:toggleRepost(post.id, entry.reposted) end
        entry.busy = false
        if result == nil then app:notice(err.message, true); return end
        local field, count = kind == "like" and "liked" or "reposted", kind == "like" and "likes" or "reposts"
        local before = entry[field] ~= nil
        entry[field] = result or nil
        entry[count] = math.max(0, (entry[count] or 0) + ((result and 1 or 0) - (before and 1 or 0)))
        if done then done() end
        app:repaint()
    end)
end

-- One timeline card: avatar tile, author line, the post and its actions.
function Posts.card(app, post, entry)
    local card = ui.Grid { Name = "Post" .. post.id, Columns = "52px 1fr", Spacing = 12 }
    local avatar = C.area(C.avatar(post.authorName, 52, post.authorId), function() app:openProfile(post.authorId, post.authorName) end, {
        Name = "Avatar" .. post.id, Width = 52, Height = 52 })
    card:addChild(avatar)
    local header = ui.Grid { Columns = "1fr auto", Spacing = 8, IgnoreHitTest = true }
    header:addChild(C.text(post.authorName, { FontSize = 16, FontWeight = "Bold", TextOverflow = "Ellipsis" }))
    header:addChild(C.text(C.ago(post.createdAt, app.clock()), { FontSize = 13, Foreground = colors.dim }))
    local column = C.stack({ Spacing = 6 }, {
        C.area(header, function() app:openProfile(post.authorId, post.authorName) end, { Name = "Author" .. post.id, Height = 24 }),
        C.area(Posts.body(app, post), function() app:openPost(post) end, { Name = "Open" .. post.id }),
    })
    local tags = Posts.tags(app, post)
    if tags then column:addChild(tags) end
    local actions = ui.Grid { Name = "Actions" .. post.id, Columns = "1fr 1fr 1fr", Height = 36 }
    Posts.fillActions(app, actions, post, entry or { likes = 0, comments = 0, reposts = 0 })
    column:addChild(actions)
    card:addChild(column)
    return card
end

-- Add cards to a list, then fetch their photos in the background.
function Posts.append(app, list, posts, stats)
    for _, post in ipairs(posts) do
        stats[post.id] = stats[post.id] or { likes = 0, comments = 0, reposts = 0 }
        list:addChild(Posts.card(app, post, stats[post.id]))
    end
    Posts.loadPhotos(app, list, posts)
end

function Posts.loadPhotos(app, root, posts)
    local pending = {}
    for _, post in ipairs(posts) do
        if post.postType == "photo" and post.imageId then pending[#pending + 1] = post end
    end
    if #pending == 0 then return end
    local session = app.sessionEpoch
    app:task(function()
        for _, post in ipairs(pending) do
            if app.sessionEpoch ~= session then return end
            local path = app.media:path(post.imageId)
            if not path then
                local bytes = app.client:getImage(post.imageId)
                path = bytes and app.media:store(bytes, post.imageId)
            end
            local frame = root:findChild("Photo" .. post.id, true)
            if frame and path then
                C.clear(frame)
                frame:addChild(ui.ImageView { Name = "Image" .. post.id, Src = path, Stretch = "Uniform", IgnoreHitTest = true })
                app:repaint()
            end
        end
    end)
end

-- Load a page of posts with their counts. Returns posts, stats or nil, error.
function Posts.fetch(app, loader)
    local posts, err = loader()
    if not posts then return nil, err end
    local ids = {}
    for _, post in ipairs(posts) do ids[#ids + 1] = post.id end
    local stats = app.client:stats(ids)
    return posts, stats or {}
end

return Posts
