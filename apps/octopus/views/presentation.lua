-- Plain service records become typed binding sources at the presentation boundary.
-- Templates choose their visible sections; photo, quote and link semantics stay
-- independent (a quote may also carry a photo).
local fs = require "orca.filesystem"
local Items = require "orca.UIKit.ItemsView"
local C = require "Octopus/views/components"
local P = {}
local prototypes, templates = {}, {}
local definitions = {
    Post = {
        string = { "Ink", "Dim", "Accent", "Muted", "OnAccent", "AccentText", "Caption", "Quote", "QuoteSource",
            "Thoughts", "LinkTitle", "Domain", "Title", "Content", "ContentColor", "AuthorName", "Age", "RepostedText",
            "Comments", "Reposts", "Likes", "RepostColor", "LikeColor" },
        bool = { "HasPhoto", "HasCaption", "IsQuote", "HasQuoteSource", "HasThoughts", "IsLink", "IsText", "HasContent", "IsRepost", "HasTags" },
        float = { "QuoteSize", "LinkSize", "TitleSize", "ContentSize", "AvatarTop" },
    },
    Comment = {
        string = { "Ink", "Dim", "AccentText", "AuthorName", "Age", "Body", "Likes", "LikeColor" },
        float = { "Indent" },
    },
}

function P.record(kind)
    if not prototypes[kind] then
        local fields = {}
        for datatype, names in pairs(assert(definitions[kind])) do
            for _, name in ipairs(names) do fields[name] = datatype end
        end
        prototypes[kind] = Items.record("Octopus" .. kind .. "Presentation", fields)
    end
    return prototypes[kind]
end

function P.template(name)
    if not templates[name] then templates[name] = assert(fs.loadObject("Octopus/Templates/" .. name)) end
    return templates[name]
end

function P.assign(record, fields)
    for key, value in pairs(fields) do record[key] = value end
end

function P.post(app, post, full, entry, repostedBy)
    local kind, colors = post.postType, C.colors
    local function count(value) return value == nil and "…" or (value > 0 and tostring(value) or "") end
    entry = entry or {}
    local text = full and C.plain(post.content) or C.excerpt(post.content)
    return {
        Ink = colors.ink, Dim = colors.dim, Accent = colors.accent, Muted = colors.muted,
        OnAccent = colors.onAccent, AccentText = colors.accentText,
        HasPhoto = kind == "photo" or (kind == "quote" and post.imageId ~= nil),
        HasCaption = kind == "photo" and post.content ~= "", Caption = text,
        IsQuote = kind == "quote", Quote = "“" .. C.plain(post.content) .. "”", QuoteSize = full and 26 or 21,
        HasQuoteSource = kind == "quote" and (post.quoteSource or "") ~= "", QuoteSource = "— " .. (post.quoteSource or ""),
        HasThoughts = kind == "quote" and (post.userText or "") ~= "", Thoughts = post.userText or "",
        IsLink = kind == "link", LinkTitle = post.title ~= "" and post.title or C.domain(post.linkUrl),
        LinkSize = full and 22 or 17, Domain = C.domain(post.linkUrl),
        IsText = kind ~= "photo" and kind ~= "quote" and kind ~= "link", Title = post.title or "", TitleSize = full and 30 or 20,
        HasContent = (kind == "link" and post.content ~= "") or (kind ~= "photo" and kind ~= "quote" and kind ~= "link"),
        Content = text, ContentSize = full and kind ~= "link" and 16 or 15, ContentColor = full and colors.ink or colors.muted,
        AuthorName = post.authorName or "", Age = C.ago(post.createdAt, app.clock()),
        IsRepost = repostedBy ~= nil, RepostedText = repostedBy and repostedBy.name .. " reposted" or "",
        AvatarTop = 24 + C.spacing.tight + (repostedBy and 20 + C.spacing.tight or 0), HasTags = #(post.tags or {}) > 0,
        Comments = count(entry.comments), Reposts = count(entry.reposts), Likes = count(entry.likes),
        RepostColor = entry.reposted and colors.reposted or colors.dim, LikeColor = entry.liked and colors.liked or colors.dim,
    }
end

function P.comment(app, item)
    local comment, entry, colors = item.comment, item.entry, C.colors
    return { Ink = colors.ink, Dim = colors.dim, AccentText = colors.accentText,
        AuthorName = comment.authorName, Age = C.ago(comment.createdAt, app.clock()), Body = comment.body,
        Indent = item.nested and 48 or 0, Likes = entry.likes == nil and "…" or (entry.likes > 0 and tostring(entry.likes) or "like"),
        LikeColor = entry.liked and colors.liked or colors.dim }
end

return P
