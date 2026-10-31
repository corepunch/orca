-- A post page: Pages/Post.xml over the Post scope (Data/Post.xml). This
-- module owns the page chrome (application bar, back) and the comment draft
-- kept per post while you browse elsewhere.
local Footer = require "orca.UIKit.Footer"
local filesystem = require "orca.filesystem"
local C = require "Octopus/views/components"
local colors = C.colors
local Thread = {}
local OctopusProvider = require "Octopus/providers/octopus"

-- Reloads the post, its counts and comments in place.
function Thread.load(self, view)
    for _, name in ipairs { "Detail", "Comments" } do view.scope:member(name):refresh() end
end

-- Posts the comment bar, as its Submit does.
function Thread.send(self, view)
    local edit = view.scope:member("Draft")
    if view.input then edit.draft.Text = view.input.Text end
    return edit:submit()
end

-- A draft, a reply target or a comment in flight holds the page (a theme
-- change waits rather than rebuild it).
function Thread.busy(view)
    local edit = view.scope:member("Draft")
    local values = edit:values()
    return edit.submitting or values.Replying or (values.Text or "") ~= ""
end

function Thread.page(self)
    local post = self.post
    if not post then return self:home() end
    local scope = self:pageScope("Octopus/Data/Post.xml")
    local record = scope.store:put("OctopusPost", OctopusProvider.preparePost(post), { partial = true })
    scope:expose("Post", record)
    local page = assert(filesystem.loadObject("Octopus/Pages/Post"), "Missing Octopus/Pages/Post")
    -- Present the post before the first evaluation of the page's bindings.
    scope:present(page:findChild("PostDetail", true), "Post")
    scope:attach(page)
    self.visibility:watch(page, scope)
    local edit = scope:member("Draft")
    local view = { scope = scope, post = post, input = page:findChild("CommentInput", true) }
    self.thread = view
    edit.draft.Text = self.commentDrafts[post.id] or ""
    self.leaving = function() self.commentDrafts[post.id] = edit:values().Text end
    -- The status line only takes room in the comment bar while it has text.
    self.status = page:findChild("PostStatus", true)
    self.statusBar = self.status

    local function back()
        self:goBack(self.postReturn and self.postReturn ~= "/post" and self.postReturn or "/")
    end
    local function run(name)
        return function() scope:member(name):execute { item = scope:exposedRecord("Post") } end
    end
    local mine = record.values.IsMine
    local bar
    bar = C.appBar({
        { "LikePost", "heart", "like", run("Like") },
        { "RepostPost", "repeat-2", "repost", run("Repost") },
        { "CommentOnPost", "message-circle", "comment", function()
            if self:requireSession("Sign in to comment.") then view.input:setFocus() end
        end },
    }, {
        { "ViewAuthor", "view " .. (record.values.AuthorName or "author"):lower(), run("OpenAuthor") },
        { "RefreshPost", "refresh", function() Thread.load(self, view) end },
        mine and { "DeletePost", "delete post", function()
            scope:member("DeletePost"):execute({}, function()
                local command = scope:member("DeletePost")
                if command.confirming and not scope.disposed then bar:findChild("DeletePost").Text = "tap again to delete" end
            end)
        end } or nil,
    })
    -- Reactions on the bar follow the post record, as cards do.
    scope:present(bar, "Post")
    bar:findChild("LikePost"):attachPropertyProgram("ApplicationBarIconButton.Text", "{DataContext/LikeText}", "OneWay", true)
    bar:findChild("RepostPost"):attachPropertyProgram("ApplicationBarIconButton.Text", "{DataContext/RepostText}", "OneWay", true)
    for _, name in ipairs { "LikePost", "RepostPost" } do
        bar:findChild(name):attachPropertyProgram("ApplicationBarIconButton.IsEnabled", "{DataContext/CanReact}", "OneWay", true)
    end
    local screen = self:frame()
    Footer.layout(screen, C.page(record.values.KindLabel, page, { Name = "PostView", ExtendBodyBehindHeader = true,
        back = back }), bar, { Background = colors.background, Foreground = colors.ink })
    scope:member("Detail"):set("PostId", post.id)
    scope:member("Comments"):set("PostId", post.id)
    return screen
end

return Thread
