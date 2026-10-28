local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
local ui = require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/octopus")
require("orca.core.startup").load_plugins()
local Posts = require "Octopus/views/posts"
local app = { clock = function() return 1761400000 end,
    photos = { aspect = function() return 4 / 3 end, watch = function() end } }
function app:avatarPhoto() end

local function top(node, screen)
    local y = 0
    while node and node ~= screen do
        y = y + node.ActualY
        node = node:getParent()
    end
    return y
end

local function left(node, screen)
    local x = 0
    while node and node ~= screen do
        x = x + node.ActualX
        node = node:getParent()
    end
    return x
end

for _, width in ipairs { 390, 320 } do
    local screen = ui.Screen { Width = width, Height = 844, ResizeMode = "NoResize" }
    local host = ui.StackView { Spacing = 12 }
    screen:addChild(host)
    for _, kind in ipairs { "text", "photo", "quote", "link", "repost" } do
        local post = { id = kind, postType = kind == "repost" and "link" or kind,
            title = "Хаос Русского мира", content = "Body text", tags = {},
            authorId = "reader", authorName = "Reader", createdAt = "2025-10-25T00:00:00Z",
            linkUrl = "https://example.com/read" }
        local row = Posts.card(app, post, nil, kind == "repost" and { id = "other", name = "Another Reader" } or nil)
        host:addChild(row)
        core.flushQueue()
        core.runAllPrograms(screen)
        screen:UpdateLayout(width, 844)
        local avatar = assert(row:findChild("Avatar" .. kind, true))
        local body = assert(row:findChild("Open" .. kind, true))
        local author = assert(row:findChild("PostAuthorName", true))
        local timestamp = assert(row:findChild("PostTimestamp", true))
        assert(math.abs(top(author, screen) - top(timestamp, screen)) < 0.01
            and author.FontSize == timestamp.FontSize and author.ActualHeight == timestamp.ActualHeight,
            "Author and timestamp share the same text line metrics")
        assert(left(timestamp, screen) >= left(author, screen) + author.ActualWidth,
            "The timestamp stays clear of the author at narrow widths")
        assert(math.abs(top(avatar, screen) - top(body, screen)) < 0.01,
            "Portraits start at the body, below the author and optional repost lines")
        local name = post.postType == "link" and "PostBodyLinkTitle"
            or post.postType == "quote" and "PostBodyQuote" or post.postType == "text" and "PostBodyTitle"
        if name then
            local leading = assert(row:findChild(name, true))
            assert(math.abs(top(avatar, screen) - top(leading, screen)) < 0.01,
                "Leading text starts at the portrait top")
        end
        if post.postType == "link" then
            local tile = assert(row:findChild("PostBodyLinkTile", true))
            assert(tile.ActualWidth == avatar.ActualWidth and tile.ActualHeight == avatar.ActualHeight,
                "The link tile matches the portrait's square size")
            assert(math.abs(top(avatar, screen) - top(tile, screen)) < 0.01,
                "Link tiles align with portrait tops")
            local title = assert(row:findChild("PostBodyLinkTitle", true))
            local portraitGap = left(tile, screen) - left(avatar, screen) - avatar.ActualWidth
            local textGap = left(title, screen) - left(tile, screen) - tile.ActualWidth
            assert(portraitGap > 0 and math.abs(portraitGap - textGap) < 0.01,
                "Portrait-to-link and link-to-text gaps match across nested containers")
        end
    end
end
print("PASS: Octopus text, photo, quote, link and repost alignment at 390 and 320 points")
