-- PostCard and the post page over real data scopes: typed record fields,
-- variants, retained rows across refresh, actions through named handlers,
-- and comments in thread order. No network, renderer output or app shell.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
require("orca.renderer").init(390, 844, true)
fs.init("apps/octopus")
require("orca.core.startup").load_plugins()
local data = require "orca.data"
local OctopusProvider = require "Octopus/providers/octopus"

local function post(id, kind, content)
    return { id = id, postType = kind, title = "Title " .. id, content = content or "Body " .. id,
        authorId = "reader", authorName = "A Reader", createdAt = "2025-10-25T00:00:00Z", tags = {} }
end
local text, photo, quote, link = post("text", "text"), post("photo", "photo", ""), post("quote", "quote"), post("link", "link")
photo.imageId, quote.imageId, quote.quoteSource, quote.userText, link.linkUrl = "image", "quote-image", "A source", "My thoughts", "https://example.com/read"
text.tags = { "reef" }
local timeline = { { post = text }, { post = photo }, { post = quote }, { post = link },
    { post = text, repostedBy = { id = "booster", name = "A Booster" } } }
local comments = {
    { id = "one", postId = "text", parentId = "", authorId = "reader", authorName = "A Reader", body = "First", createdAt = text.createdAt },
    { id = "reply", postId = "text", parentId = "one", authorId = "reader", authorName = "A Reader", body = "Reply", createdAt = text.createdAt },
}
local liked, opened, profile
local client = {
    homeTimeline = function() return timeline end,
    stats = function(_, ids)
        local result = {}
        for _, id in ipairs(ids) do result[id] = { likes = 1, comments = 2, reposts = 0 } end
        return result
    end,
    toggleLike = function(_, id, _, current) liked = id; return not current and { id = "like-" .. id } or false end,
    getPost = function(_, id) for _, item in ipairs(timeline) do if item.post.id == id then return item.post end end end,
    listComments = function() return comments end,
}
local provider = OctopusProvider.new { client = client, media = {}, now = function() return 1761400000 end,
    openPost = function(raw) opened = raw.id end, openProfile = function(id) profile = id end,
    requireSession = function() return true end, notice = function() end, readerId = function() return "reader" end }
local store = data.Store.new { schema = data.schema("Octopus/Data/Octopus.Schema.xml"), scope = "octopus:test",
    computed = provider.computed }
local feeds = data.scope("Octopus/Data/Feeds.xml", { provider = provider, store = store })
local screen = assert(fs.loadObjectFromXmlString([[
<Screen Width="390" Height="844" ResizeMode="NoResize">
  <StackView Overflow="hidden scroll">
    <ItemsStack Name="PostItems" Spacing="12" JustifyContent="Start" ItemNamePrefix="Post"
                ItemsSource="{Binding DataContext/Home/Items}" ItemTemplate="Octopus/Templates/PostCard"/>
  </StackView>
</Screen>]]))
feeds:attach(screen)
local function settle(root)
    core.flushQueue(); core.advanceFrame(); core.runAllPrograms(root or screen); core.flushQueue()
end
local function row(id) return assert(screen:findChild("Post" .. id, true), "Missing row " .. id) end
local function field(node, name) return assert(node:findChild(name, true), name) end
feeds:member("Home"):refresh()
settle()
local items = screen:findChild("PostItems", true)
assert(items.ItemsSource and items.ItemTemplate, "The collection exposes native ItemsSource and ItemTemplate")
assert(field(row("text"), "PostBodyTitle").Text == "Title text", "Post fields reach the template through DataContext")
assert(field(row("text"), "PostBodyTitle").Visible and not field(row("text"), "PostBodyCaption").Visible)
assert(not field(row("photo"), "PostBodyCaption").Visible and not field(row("photo"), "PostBodyTitle").Visible,
    "A captionless photo has no text section")
assert(field(row("quote"), "PostBodyQuote").Visible and field(row("quote"), "PostBodyThoughts").Visible
    and field(row("quote"), "PostBodyQuoteSource").Text == "— A source", "Quote and photo sections compose independently")
assert(field(row("quote"), "Photo").Visible, "A quote with a background photo shows it")
assert(field(row("link"), "PostBodyLinkTitle").Text == "Title link" and not field(row("link"), "PostBodyTitle").Visible)
assert(field(row("text"), "Tags"):findChild("Tagreef", true), "Tags render through their own template")
-- One entry per post: the repost of a post already listed is not a second row.
local count = 0
for _ in items.children do count = count + 1 end
assert(count == 4, "A post and its repost never both appear")

-- Edits reach retained rows; reordering keeps the native row.
local retained = row("text")
text.title, photo.content = "Edited title", "A caption"
timeline = { { post = link }, { post = text }, { post = photo } }
feeds:member("Home"):refresh()
settle()
assert(row("text") == retained and field(retained, "PostBodyTitle").Text == "Edited title",
    "Reordering retains the native row and updates its fields")
assert(field(row("photo"), "PostBodyCaption").Visible and field(row("photo"), "PostBodyCaption").Text == "A caption")
assert(not screen:findChild("Postquote", true), "Refresh removes rows that left the feed")

-- Actions dispatch through the row's record.
field(retained, "Open"):send("Node.LeftButtonUp")
core.flushQueue()
assert(opened == "text", "Template post actions use the current record")
field(retained, "Like"):send("Node.LeftButtonUp")
core.flushQueue()
settle()
assert(liked == "text" and store:get("OctopusPost", "text").values.Likes == 2
    and field(retained, "Actions").StateClasses:find("liked", 1, true),
    "Likes update the shared record and the row's state classes")

-- The post page presents the same record at detail size, with comments.
local page = data.scope("Octopus/Data/Post.xml", { provider = provider, store = store })
page:expose("Post", store:get("OctopusPost", "text"))
local detail = assert(fs.loadObject("Octopus/Pages/Post"))
page:present(detail:findChild("PostDetail", true), "Post")
page:attach(detail)
screen:addChild(detail)
page:member("Detail"):set("PostId", "text")
page:member("Comments"):set("PostId", "text")
settle()
assert(field(detail, "PostDetail").DataContext
    and field(detail, "PostBodyTitle").Text == "Edited title", "The detail page presents the post record")
assert(field(detail, "Statlikes").Text == "1", "The detail page shows the counts its refresh returned")
local first, reply = field(detail, "Commentone"), field(detail, "Commentreply")
assert(reply.MarginLeft == 48 and first.MarginLeft == 0, "Replies indent under their comment")
comments[1].body = "Changed comment"
page:member("Comments"):refresh()
settle()
assert(field(detail, "Commentone") == first and field(first, "CommentBody").Text == "Changed comment",
    "Comment refresh keeps rows and updates their text")
field(reply, "Reply"):send("Node.LeftButtonUp")
core.flushQueue()
local draft = page:member("Draft"):values()
assert(draft.ReplyToId == "one" and draft.Replying, "Replying to a reply targets its top-level comment")
field(first, "Like"):send("Node.LeftButtonUp")
core.flushQueue()
settle()
assert(liked == "one" and field(first, "Like"):findChild("TextBlock", true).Text == "2",
    "Comment likes update the comment record")
field(first, "Avatar"):send("Node.LeftButtonUp")
core.flushQueue()
assert(profile == "reader", "A commenter's avatar opens their profile")
comments = {}
page:member("Comments"):refresh()
settle()
assert(first:getParent() == nil and field(detail, "NoComments").Visible, "Removing all comments shows the empty state")
page:dispose()
feeds:dispose()
print("PASS: Octopus XML templates, typed bindings, retained post/comment refresh, variants and actions")
