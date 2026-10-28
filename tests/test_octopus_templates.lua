local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
local ui = require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/octopus")
require("orca.core.startup").load_plugins()
local Posts = require "Octopus/views/posts"
local Thread = require "Octopus/views/thread"
local C = require "Octopus/views/components"

local watches = {}
local app = { sessionEpoch = 1, navigationEpoch = 1, commentDrafts = {},
    clock = function() return 1761400000 end,
    photos = { aspect = function() return 4 / 3 end,
        watch = function(_, frame, key) watches[key] = frame end } }
function app:avatarPhoto() end
function app:openProfile(id) self.profile = id end
function app:openPost(post) self.opened = post.id end
function app:openTag(tag) self.tag = tag end
function app:requireSession() return true end
function app:notice() end
function app:repaint() end
function app:task(fn) fn() end
function app:frame()
    self.screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
    return self.screen
end
local function post(id, kind, content)
    return { id = id, postType = kind, title = "Title " .. id, content = content or "Body " .. id,
        authorId = "reader", authorName = "A Reader", createdAt = "2025-10-25T00:00:00Z", tags = {} }
end
local text, photo, quote, link = post("text", "text"), post("photo", "photo", ""), post("quote", "quote"), post("link", "link")
photo.imageId, quote.imageId, quote.quoteSource, quote.userText, link.linkUrl = "image", "quote-image", "A source", "My thoughts", "https://example.com/read"
local stats = { text = { likes = 1 }, photo = {}, quote = {}, link = {} }
app.client = { toggleLike = function() return { id = "liked" } end }
local screen = app:frame()
local host = C.stack { Overflow = "hidden scroll", Spacing = 12 }
screen:addChild(host)
local items = Posts.items { text, photo, quote, link }
items[#items + 1] = { post = text, repostedBy = { id = "booster", name = "A Booster" } }
local rows = Posts.refresh(app, host, items, stats)
core.flushQueue()
screen:UpdateLayout(390, 844)
local function field(row, name) return assert(row.node:findChild("PostBody" .. name, true) or row.node:findChild(name, true), name) end
-- Avatars begin beside the post body, below author and optional repost rows.
for _, index in ipairs { 1, 5 } do
    local row = rows[index]
    local avatar, author, content = field(row, "Avatartext"), field(row, "Authortext"), field(row, "Opentext")
    local above = 24 + C.spacing.tight + (index == 5 and 20 + C.spacing.tight or 0)
    assert(author.Height == 24 and avatar.MarginTop == above,
        "Standard and repost cards reserve the author/repost rows above their avatar")
    assert(math.abs(avatar.ActualY - (content:getParent().ActualY + content.ActualY)) < 1,
        "Standard and repost avatars align vertically with their native laid-out post content")
end
local function top(node)
    local y = 0
    while node and node ~= screen do
        y = y + node.ActualY
        node = node:getParent()
    end
    return y
end
local function aligned(a, b, message)
    assert(math.abs(top(a) - top(b)) < 0.01, message)
end
for _, size in ipairs { { 390, 844 }, { 320, 700 } } do
    screen.Width, screen.Height = size[1], size[2]
    core.advanceFrame()
    core.runAllPrograms(screen)
    screen:UpdateLayout(size[1], size[2])
    for _, row in ipairs(rows) do
        aligned(row.controls.Avatar, row.controls.Open, "Portraits start at the post body for every post type")
    end
    aligned(rows[1].controls.Avatar, field(rows[1], "Title"), "Text titles align to the portrait")
    aligned(rows[3].controls.Avatar, watches["quote-image"], "Quote photos align to the portrait")
    local tile = field(rows[4], "LinkTile")
    aligned(rows[4].controls.Avatar, tile, "Link tiles align to the portrait")
    aligned(tile, field(rows[4], "LinkTitle"), "Link titles align to the tile")
    assert(tile.ActualWidth == rows[4].controls.Avatar.ActualWidth
        and tile.ActualHeight == rows[4].controls.Avatar.ActualHeight, "Link tiles match the portrait's square size")
end
screen.Width, screen.Height = 390, 844
assert(field(rows[1], "Title").Text == "Title text", "Post fields reach the XML template through DataContext")
assert(field(rows[1], "Title").Visible and not field(rows[1], "Caption").Visible)
assert(not field(rows[2], "Caption").Visible and not field(rows[2], "Title").Visible,
    "A captionless photo has no text section")
assert(field(rows[3], "Quote").Visible and field(rows[3], "Thoughts").Visible
    and field(rows[3], "QuoteSource").Text == "— A source", "Quote and photo sections compose independently")
assert(field(rows[4], "LinkTitle").Text == "Title link" and not field(rows[4], "Title").Visible)
assert(watches.image and watches["quote-image"], "Declarative photo frames use the existing viewport loader")
assert(host:findChild("PostItems").ItemsSource and host:findChild("PostItems").ItemTemplate,
    "The collection exposes native ItemsSource and ItemTemplate")
local repost = Posts.card(app, link, {}, { id = "reposter", name = "A Reposter" })
host:addChild(repost)
core.flushQueue()
core.runAllPrograms(screen)
screen:UpdateLayout(390, 844)
aligned(repost:findChild("Avatarlink", true), repost:findChild("PostBodyLinkTile", true),
    "A repost keeps its portrait aligned below the repost and author lines")
repost:removeFromParent()
local retained, context, frame = rows[1].node, rows[1].data, watches.image
text.title, photo.content, stats.text.likes = "Edited title", "A caption", 3
local changed = Posts.refresh(app, host, Posts.items { link, text, photo }, stats)
core.flushQueue()
assert(changed[2].node == retained and changed[2].data == context, "Reordering retains both native row and binding source")
assert(field(changed[2], "Title").Text == "Edited title", "Retained rows receive changed fields")
assert(field(changed[3], "Caption").Visible and field(changed[3], "Caption").Text == "A caption")
assert(watches.image == frame and rows[3].node:getParent() == nil, "Refresh retains photos and removes deleted rows")
field(changed[2], "Opentext"):send("Node.LeftButtonUp")
core.flushQueue()
assert(app.opened == "text", "Template post actions use the current record")
field(changed[2], "Liketext"):send("Node.LeftButtonUp")
core.flushQueue()
assert(stats.text.likes == 4 and context.Likes == "4", "Action counts update the retained binding source")
photo.imageId = "replacement"
local replaced = Posts.refresh(app, host, Posts.items { link, text, photo }, stats)
core.flushQueue()
assert(replaced[3].node == changed[3].node and watches.replacement and watches.replacement ~= frame
    and frame:getParent():getParent() == nil, "Changed media retains the card and detaches the old download target")
local full = Posts.body(app, text, true)
screen:addChild(full)
core.flushQueue()
core.runAllPrograms(screen)
assert(full:findChild("PostBodyTitle", true).FontSize == 30, "The detail page reuses the body template with full-size fields")

-- Drive comments through the real thread loader and refresh action.
local comments = {
    { id = "one", parentId = "", authorId = "reader", authorName = "A Reader", body = "First", createdAt = text.createdAt },
    { id = "reply", parentId = "one", authorId = "reader", authorName = "A Reader", body = "Reply", createdAt = text.createdAt },
}
app.post = text
app.client = {
    getSession = function() return { user = { id = "me" } } end,
    getPost = function() return text end,
    stats = function(_, ids, kind)
        local result = {}
        for _, id in ipairs(ids) do result[id] = { likes = 0, comments = 2, reposts = 0 } end
        return result
    end,
    listComments = function() return comments end,
    toggleLike = function() return { id = "liked" } end,
}
Thread.page(app)
core.flushQueue()
local thread, comment = app.thread, assert(app.screen:findChild("Commentone", true))
local reply = assert(app.screen:findChild("Commentreply", true))
assert(reply.MarginLeft == 48 and comment.MarginLeft == 0, "Thread indentation comes from the comment presentation record")
thread.input.Text = "Keep this draft"
comments[1].body = "Changed comment"
Thread.load(app, thread)
core.flushQueue()
assert(app.screen:findChild("Commentone", true) == comment and thread.input.Text == "Keep this draft",
    "Comment refresh keeps rows and the composer draft")
local body = comment:getFirstChild():getNext():getFirstChild():getNext()
assert(body.Text == "Changed comment", "Comment refresh updates XML text through the retained DataContext")
app.screen:findChild("Replyreply", true):send("Node.LeftButtonUp")
core.flushQueue()
assert(thread.replyTo.id == "one" and thread.replyBar.Visible, "Reply actions preserve the top-level thread parent")
app.screen:findChild("LikeCommentone", true):send("Node.LeftButtonUp")
core.flushQueue()
assert(comment.DataContext.Likes == "1", "Comment like actions update the binding source")
comments = {}
Thread.load(app, thread)
core.flushQueue()
assert(comment:getParent() == nil and app.screen:findChild("NoComments", true), "Removing all comments shows the empty state")
print("PASS: Octopus XML templates, typed bindings, retained post/comment refresh, variants and actions")
