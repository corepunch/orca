local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
local ui = require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/octopus")
require("orca.core.startup").load_plugins()
local data = require "orca.data"
local OctopusProvider = require "Octopus/providers/octopus"

-- Every post type, and a repost, through the real Home query and PostCard.
local items = {}
for _, kind in ipairs { "text", "photo", "quote", "link", "repost" } do
    items[#items + 1] = { post = { id = kind, postType = kind == "repost" and "link" or kind,
        title = "Хаос Русского мира", content = "Body text", tags = {},
        authorId = "reader", authorName = "Reader", createdAt = "2025-10-25T00:00:00Z",
        linkUrl = "https://example.com/read" },
        repostedBy = kind == "repost" and { id = "other", name = "Another Reader" } or nil }
end
local client = { homeTimeline = function() return items end, stats = function() return {} end }
local provider = OctopusProvider.new { client = client, media = {}, now = function() return 1761400000 end }
local store = data.Store.new { schema = data.schema("Octopus/Data/Octopus.Schema.xml"), scope = "octopus:test",
    computed = provider.computed }
local scope = data.scope("Octopus/Data/Feeds.xml", { provider = provider, store = store })
scope:member("Home"):refresh()

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
    local screen = assert(fs.loadObjectFromXmlString(([[
<Screen Width="%d" Height="844" ResizeMode="NoResize">
  <StackView Overflow="hidden scroll">
    <ItemsStack Name="PostItems" Spacing="12" JustifyContent="Start" ItemNamePrefix="Post"
                ItemsSource="{Binding DataContext/Home/Items}" ItemTemplate="Octopus/Templates/PostCard"/>
  </StackView>
</Screen>]]):format(width)))
    scope:attach(screen)
    core.flushQueue()
    core.advanceFrame()
    core.runAllPrograms(screen)
    screen:UpdateLayout(width, 844)
    for _, item in ipairs(items) do
        local kind, post = item.post.id, item.post
        local row = assert(screen:findChild("Post" .. kind, true), "Missing row " .. kind)
        local avatar = assert(row:findChild("Avatar", true))
        local body = assert(row:findChild("Open", true))
        local author = assert(row:findChild("PostAuthorName", true))
        local timestamp = assert(row:findChild("PostTimestamp", true))
        assert(math.abs(top(author, screen) - top(timestamp, screen)) < 0.01
            and author.FontSize == timestamp.FontSize and author.ActualHeight == timestamp.ActualHeight,
            "Author and timestamp share the same text line metrics")
        assert(left(timestamp, screen) >= left(author, screen) + author.ActualWidth,
            "The timestamp stays clear of the author at narrow widths")
        assert(math.abs(top(avatar, screen) - top(body, screen)) < 0.01,
            "Portraits start at the body, below the author and optional repost lines: " .. kind)
        local name = post.postType == "link" and "PostBodyLinkTitle"
            or post.postType == "quote" and "PostBodyQuote" or post.postType == "text" and "PostBodyTitle"
        if name then
            local leading = assert(row:findChild(name, true))
            assert(math.abs(top(avatar, screen) - top(leading, screen)) < 0.01,
                "Leading text starts at the portrait top: " .. kind)
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
scope:dispose()
print("PASS: Octopus text, photo, quote, link and repost alignment at 390 and 320 points")
