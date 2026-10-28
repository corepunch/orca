local ui = require "orca.UIKit"
local Footer = require "orca.UIKit.Footer"
local system = require "orca.system"
local C = require "Octopus/views/components"
local Posts = require "Octopus/views/posts"
local colors = C.colors
local Thread = {}

local LABELS = { text = "post", photo = "photo", quote = "quote", link = "link" }

-- Count tiles under the post, like a tweet's detail page: big numbers over
-- small labels, between hairlines.
local function statTiles()
    local tiles = ui.Grid { Name = "PostStats", Columns = "1fr 1fr 1fr", Height = 64 }
    local labels = {}
    for _, key in ipairs { "likes", "reposts", "comments" } do
        labels[key] = { number = C.text("…", { Name = "Stat" .. key, FontSize = 28 }),
            label = C.text(key, { FontSize = 13, Foreground = colors.dim }) }
        tiles:addChild(C.stack({ Spacing = 0, JustifyContent = "Center" }, { labels[key].number, labels[key].label }))
    end
    local function rule() return ui.Grid { Height = 1, Background = colors.faint } end
    local frame = C.stack({ Spacing = 6 }, { rule(), tiles, rule() })
    return frame, function(entry)
        local singular = { likes = "like", reposts = "repost", comments = "comment" }
        for key, view in pairs(labels) do
            local count = entry[key]
            view.number:setTextContent(count == nil and "…" or tostring(count))
            view.label:setTextContent(count == 1 and singular[key] or key)
        end
    end
end

-- Comments with their replies indented beneath, as on the website.
local function renderComments(self, view, comments, likes)
    local list = C.clear(view.comments)
    local replies, top = {}, {}
    for _, comment in ipairs(comments) do
        if comment.parentId ~= "" then
            replies[comment.parentId] = replies[comment.parentId] or {}
            table.insert(replies[comment.parentId], comment)
        else top[#top + 1] = comment end
    end
    local function item(comment, nested)
        local entry = likes[comment.id] or { pending = true }
        likes[comment.id] = entry
        local row = ui.Grid { Name = "Comment" .. comment.id, Columns = "36px 1fr", Spacing = 12, MarginLeft = nested and 48 or 0 }
        local function author() self:openProfile(comment.authorId, comment.authorName) end
        row:addChild(C.area(C.avatar(comment.authorName, 36, comment.authorId, self), author, {
            Name = "CommentAvatar" .. comment.id, Width = 36, Height = 36 }))
        local header = ui.Grid { Columns = "1fr auto", Spacing = 8, IgnoreHitTest = true }
        header:addChild(C.text(comment.authorName, { FontSize = 14, FontWeight = "Bold", TextOverflow = "Ellipsis" }))
        header:addChild(C.text(C.ago(comment.createdAt, self.clock()), { FontSize = 12, Foreground = colors.dim }))
        local actions = C.row({ Spacing = 20, Height = 30 })
        local function fill()
            C.clear(actions)
            local tint = entry.liked and colors.liked or colors.dim
            actions:addChild(C.area(C.row({ Spacing = 6, IgnoreHitTest = true }, {
                C.icon("heart", 16, tint, { MarginLeft = 0, MarginRight = 0 }),
                C.text(entry.likes == nil and "…" or (entry.likes > 0 and tostring(entry.likes) or "like"), { FontSize = 13, Foreground = tint }),
            }), function()
                if not self:requireSession("Sign in to like comments.") or entry.busy or entry.pending then return end
                entry.busy = true
                local session, navigation = self.sessionEpoch, self.navigationEpoch
                self:task(function()
                    if self.sessionEpoch ~= session or self.navigationEpoch ~= navigation or self.thread ~= view then
                        entry.busy = false; return
                    end
                    local result, err = self.client:toggleLike(comment.id, "comment", entry.liked)
                    entry.busy = false
                    if self.sessionEpoch ~= session or self.navigationEpoch ~= navigation or self.thread ~= view then return end
                    if result == nil then self:notice(err.message, true); return end
                    entry.likes = math.max(0, entry.likes + (result and 1 or 0) - (entry.liked and 1 or 0))
                    entry.liked = result or nil
                    fill(); self:repaint()
                end)
            end, { Name = "LikeComment" .. comment.id, Height = 30, Width = 64 }))
            actions:addChild(C.button("reply", function()
                if not self:requireSession("Sign in to reply.") then return end
                -- Replies thread under the top-level comment, like the website.
                view.replyTo = nested and { id = comment.parentId, authorName = comment.authorName } or comment
                if view.replyLabel then
                    view.replyLabel:setTextContent("replying to " .. comment.authorName)
                    view.replyBar.Visible = true
                    view.input:setFocus()
                end
            end, { Name = "Reply" .. comment.id, Height = 30, Width = 48, FontSize = 13 }))
        end
        fill()
        row:addChild(C.stack({ Spacing = 4 }, {
            header, C.text(comment.body, { FontSize = 15, WordWrap = true }), actions,
        }))
        list:addChild(row)
        for _, reply in ipairs(replies[comment.id] or {}) do item(reply, true) end
    end
    for _, comment in ipairs(top) do item(comment, false) end
    -- Replies whose parent was deleted still deserve a place.
    for parent, orphans in pairs(replies) do
        local known = false
        for _, comment in ipairs(top) do if comment.id == parent then known = true end end
        if not known then for _, reply in ipairs(orphans) do item(reply, false) end end
    end
    if #comments == 0 then
        list:addChild(C.empty("no comments yet", self.client:getSession() and "Start the conversation below."
            or "Sign in to start the conversation."))
    end
end

function Thread.load(self, view)
    if view.loading then return end
    view.loading, view.request = true, {}
    view.entry.pending = true
    view.refreshStats()
    local request = view.request
    local loading = view.comments:findChild("CommentsLoading")
    if not loading then
        loading = C.loading { Name = "CommentsLoading", Height = 64 }
        view.comments:addChild(loading)
    end
    local post, session, navigation = view.post, self.sessionEpoch, self.navigationEpoch
    local function current()
        return self.thread == view and self.sessionEpoch == session and self.navigationEpoch == navigation
            and view.request == request
    end
    -- A feed can contain only an excerpt. Replace the displayed body as soon
    -- as the full document arrives; comments and reactions need not wait.
    self:task(function()
        if not current() then return end
        local fresh, err = self.client:getPost(post.id)
        if not current() then return end
        if fresh then
            view.post, self.post = fresh, fresh
            C.clear(view.postBody):addChild(Posts.body(self, fresh, true))
        else
            self:notice(err.message, true)
        end
        self:repaint()
    end)
    Posts.enrich(self, { { post = post } }, { [post.id] = view.entry }, current, view.refreshStats)
    self:task(function()
        if not current() then return end
        local comments, err = self.client:listComments(post.id)
        if not current() then return end
        view.loading = false
        loading:removeFromParent()
        if not comments then
            if not view.commentsLoaded then C.clear(view.comments):addChild(C.empty("can't load comments", err.message)) end
            self:notice(err.message, true); self:repaint(); return
        end
        view.commentsLoaded = true
        local likes, ids = {}, {}
        for _, comment in ipairs(comments) do
            ids[#ids + 1], likes[comment.id] = comment.id, { pending = true }
        end
        renderComments(self, view, comments, likes)
        self:repaint()
        if #ids == 0 then return end
        local resolved = self.client:stats(ids, "comment")
        if not current() or not resolved then return end
        renderComments(self, view, comments, resolved)
        self:repaint()
    end)
end

function Thread.send(self, view)
    if view.sending then return end
    local text = view.input.Text or ""
    if not text:match("%S") then return end
    view.sending = true
    self:notice("Posting…")
    local session, navigation = self.sessionEpoch, self.navigationEpoch
    self:task(function()
        if self.sessionEpoch ~= session or self.navigationEpoch ~= navigation or self.thread ~= view then
            view.sending = false; return
        end
        local comment, err = self.client:addComment(view.post.id, text, view.replyTo and view.replyTo.id)
        view.sending = false
        if self.thread ~= view or self.sessionEpoch ~= session or self.navigationEpoch ~= navigation then return end
        if not comment then self:notice(err.message .. " Your comment is still here.", true); return end
        self.commentDrafts[view.post.id] = nil
        view.input.Text, view.replyTo, view.replyBar.Visible = "", nil, false
        self.feeds = {}
        Thread.load(self, view)
    end)
end

function Thread.page(self)
    local post = self.post
    if not post then return self:home() end
    local session = self.client:getSession()
    local mine = session and session.user.id == post.authorId
    local view = { post = post, entry = { pending = true } }
    for key, value in pairs(post.stats or {}) do view.entry[key] = value end
    view.entry.pending = true
    self.thread = view
    -- The status line only takes room in the comment bar while it has text.
    self.status = C.text("", { Name = "PostStatus", FontSize = 11, Foreground = colors.dim,
        TextHorizontalAlignment = "Center", Visible = false })
    self.statusBar = self.status

    local author = ui.Grid { Columns = "56px 1fr", Spacing = 14, IgnoreHitTest = true }
    author:addChild(C.avatar(post.authorName, 56, post.authorId, self))
    author:addChild(C.stack({ Spacing = 2, JustifyContent = "Center", IgnoreHitTest = true }, {
        C.text(post.authorName, { FontSize = 22, TextOverflow = "Ellipsis" }),
        C.text(LABELS[post.postType] .. " · " .. C.ago(post.createdAt, self.clock()), { FontSize = 13, Foreground = colors.dim }),
    }))
    local stats, showStats = statTiles()
    view.comments = C.stack({ Name = "Comments", Spacing = 18 }, { C.loading { Name = "CommentsLoading", Height = 160 } })
    view.postBody = C.stack({ Name = "PostBody" }, { Posts.body(self, post, true) })
    local content = C.stack({ Name = "PostPage", Spacing = 16, Overflow = "hidden scroll" }, {
        C.area(author, function() self:openProfile(post.authorId, post.authorName) end, { Name = "PostAuthor", Height = 60 }),
        view.postBody,
    })
    local tags = Posts.tags(self, post)
    if tags then content:addChild(tags) end
    if post.postType == "link" and post.linkUrl then
        content:addChild(C.outline("Copy link", function()
            system.set_clipboard(post.linkUrl)
            self:notice("Link copied: " .. post.linkUrl)
        end, { Name = "CopyLink" }))
    end
    content:addChild(C.text(C.stamp(post.createdAt), { Name = "PostTime", FontSize = 13, Foreground = colors.dim }))
    content:addChild(stats)
    content:addChild(C.eyebrow("comments"))
    content:addChild(view.comments)

    local footer
    if session then
        view.input = C.input("CommentInput", "Write a comment", {
            Text = self.commentDrafts[post.id] or "", Submit = function() Thread.send(self, view) end,
            KeyUp = function() self.commentDrafts[post.id] = view.input.Text end,
        })
        view.replyLabel = C.text("", { FontSize = 13, Foreground = colors.accentText, TextVerticalAlignment = "Center" })
        view.replyBar = ui.Grid { Name = "ReplyBar", Columns = "1fr 36px", Height = 32, Visible = false }
        view.replyBar:addChild(view.replyLabel)
        view.replyBar:addChild(C.button("", function() view.replyTo, view.replyBar.Visible = nil, false end, {
            Name = "CancelReply", Width = 36, Height = 32 }))
        view.replyBar:findChild("CancelReply"):addChild(C.icon("x", 16, colors.muted))
        footer = C.toolbar("Bottom", { C.stack({ Spacing = 6 }, { self.status, view.replyBar, view.input }) },
            { Name = "CommentBar", Padding = "20 8" })
        self.leaving = function() self.commentDrafts[post.id] = view.input.Text end
    else
        footer = C.toolbar("Bottom", { C.stack({ Spacing = 0 }, { self.status,
            C.button("Sign in to comment", function() self.authReturn = "/post"; self:show("/auth") end, {
                Name = "SignInToComment", Height = 40 }) }) }, { Name = "CommentBar", Padding = "20 8" })
    end
    local body = ui.Node2D {}
    body:addChild(content); body:addChild(footer)

    local function back()
        self:goBack(self.postReturn and self.postReturn ~= "/post" and self.postReturn or "/")
    end
    local bar
    bar = C.appBar({
        { "LikePost", "heart", "like", function()
            Posts.toggle(self, post, view.entry, "like", function() view.refreshStats() end)
        end },
        { "RepostPost", "repeat-2", "repost", function()
            Posts.toggle(self, post, view.entry, "repost", function() view.refreshStats() end)
        end },
        { "CommentOnPost", "message-circle", "comment", function()
            if not self:requireSession("Sign in to comment.") then return end
            view.input:setFocus()
        end },
    }, {
        { "ViewAuthor", "view " .. post.authorName:lower(), function() self:openProfile(post.authorId, post.authorName) end },
        { "RefreshPost", "refresh", function() Thread.load(self, view) end },
        mine and { "DeletePost", "delete post", function()
            if view.confirmDelete ~= true then
                view.confirmDelete = true
                bar:findChild("DeletePost").Text = "tap again to delete"
                self:notice("Delete this post? Choose delete again to confirm. This cannot be undone.", true)
                return
            end
            self:task(function()
                local ok, err = self.client:deletePost(view.post)
                if not ok then self:notice(err.message, true); return end
                self:invalidate()
                self.post = nil
                back()
            end)
        end } or nil,
    })
    function view.refreshStats()
        showStats(view.entry)
        bar:findChild("LikePost").Text = view.entry.liked and "unlike" or "like"
        bar:findChild("RepostPost").Text = view.entry.reposted and "undo" or "repost"
        for _, name in ipairs { "LikePost", "RepostPost" } do
            local action = bar:findChild(name)
            action.IgnoreHitTest, action.Opacity = view.entry.pending == true, view.entry.pending and 0.5 or 1
        end
    end
    view.refreshStats()
    local screen = self:frame()
    Footer.layout(screen, C.page(LABELS[post.postType], body, { Name = "PostView", ExtendBodyBehindHeader = true, back = back }), bar,
        { Background = colors.background, Foreground = colors.ink })
    Thread.load(self, view)
    return screen
end

return Thread
