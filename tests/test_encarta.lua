require("orca").init()
local core = require "orca.core"
local fs = require "orca.filesystem"
require "orca.UIKit"
require "orca.renderer"

local project = fs.init("apps/encarta")
local position, playing, opened, closed = 0, false, 0, 0
local app = require(project.StartupViewController)(function(bytes)
    assert(#bytes > 100 and bytes:sub(1, 4) == "RIFF" and bytes:sub(9, 12) == "WAVE", "read packaged WAV audio")
    position, opened = 0, opened + 1
    return {
        play = function() playing = true; return true end,
        pause = function() playing = false end,
        status = function() return position, playing end,
        close = function() playing = false; closed = closed + 1 end,
    }
end)
app:activate_route("/")
local view = app.screen
view.ResizeMode = "NoResize"

local function control(name, root)
    return assert((root or view):findChild(name, true), name)
end

local function click(name, root)
    control(name, root):send("Node.LeftButtonUp")
    core.flushQueue()
end

local function layout(width, height)
    view.Width, view.Height = width, height
    core.advanceFrame()
    core.runAllPrograms(view)
    view:UpdateLayout(width, height)
    core.flushQueue()
end

local library = control("LibraryScroll")
local reader = control("Reader")
local wall = control("Topics")
local shelf = control("Shelf")
local sections = control("Sections")
local search = control("Search")
local function count(node) local n = 0 for _ in node.children do n = n + 1 end return n end
local function visibleSection()
    for section in sections.children do if section.Visible then return control("Title", section) end end
end
local function questionImage()
    for section in sections.children do
        if section.Visible and control("SectionMedia", section).Visible then
            return control("SectionImage", section), control("SectionCaption", section)
        end
    end
end

core.flushQueue()
layout(1024, 768)
assert(library.Visible and not reader.Visible)
assert(control("ResultCount").Text == "15 ARTICLES" and count(wall) == 15 and wall.Visible and not shelf.Visible)
assert(control("CatalogTitle").Text == "Big questions")
local card = control("Open_mars", wall)
assert(card.ActualWidth > 200 and math.abs(card.ActualWidth / card.ActualHeight - 1.4) < 0.02, "four landscape question cards per row")
assert(control("Question", card).Text == "Why is Mars red?")
local pyramidCover = control("Art", control("Open_pyramids", wall))
local earthCover = control("Art", control("Open_earth-layers", wall))
assert(pyramidCover.Visible and pyramidCover.Src == "encarta/Images/Thumbs/pyramids.jpg")
assert(earthCover.Visible and earthCover.Src == "encarta/Images/Thumbs/earth-cover.jpg"
    and earthCover.ActualHeight == control("Open_earth-layers", wall).ActualHeight, "art fills the question card")
assert(search.ActualHeight >= 44 and control("OpenFeatured").ActualHeight >= 44)
assert(control("FeaturedImage").ActualWidth > 900 and control("FeaturedImage").ActualHeight == 176)
local background = card.BackgroundColor
assert(background.B > 0.5 and background.R < 0.3, "space cards resolve the packaged category color")

assert(control("ShowWall").BackgroundColor.R > 0.9, "the question wall starts selected")
click("ShowShelf")
assert(control("ShowWall").BackgroundColor.R < 0.5 and control("ShowShelf").BackgroundColor.R > 0.9, "only the chosen view is highlighted")
assert(shelf.Visible and not wall.Visible and count(shelf) == 15)
assert(control("CatalogTitle").Text == "On the shelf")
local octopusCover, moonCover = control("Cover_octopus", shelf), control("Cover_moon", shelf)
assert(control("Art", octopusCover).Visible and not control("Icon", octopusCover).Visible, "art replaces the icon")
layout(1024, 768)
local moonArt = control("Art", moonCover)
assert(moonArt.Src == "encarta/Images/Thumbs/moon.jpg" and control("Title", moonCover).Text == "The Moon")
assert(moonArt.ActualHeight > moonArt.ActualWidth and moonArt.ActualHeight == moonCover.ActualHeight, "art fills the portrait cover")
assert(control("Art", control("Cover_volcanoes", shelf)).Src == "encarta/Images/Thumbs/volcanoes.jpg", "a dedicated cover takes priority")
click("Category_space")
assert(count(shelf) == 3 and control("CatalogTitle").Text == "Space" and not control("Featured").Visible)
click("Category_all")
click("ShowWall")
assert(count(wall) == 15 and control("Featured").Visible)

search.Text = "  SPACE moon  "
core.flushQueue()
assert(control("ResultCount").Text == "1 ARTICLE" and not control("Featured").Visible)
assert(count(wall) == 1 and control("Open_moon", wall))
click("Open_moon", wall)
assert(not library.Visible and reader.Visible)
assert(control("ArticleTitle").Text == "The Moon")
assert(visibleSection().Text == "Why does the Moon seem to change shape?")
local image, caption = questionImage()
assert(image.Src == "encarta/Images/moon-phases.jpg" and caption.Text:match("Northern Hemisphere"), "a question shows its own diagram")
assert(control("StepLabel").Text == "SPACE  ·  QUESTION 1 OF 5")
assert(not control("Previous").Visible and control("TextBlock", control("Next")).Text == "Next question")
layout(1024, 768)
assert(count(control("Steps")) == 5 and control("Pane").Opacity == 1)
layout(768, 1024)
assert(control("Pane").Opacity == 0 and control("ReaderScroll").ActualWidth > 600, "portrait gives the page the full width")
local page = control("ReaderScroll").ActualWidth
assert(image.ActualWidth > 200 and image.ActualWidth < page / 2, "the question artwork sits in a side column")
assert(math.abs(image.ActualWidth / image.ActualHeight - 1.5) < 0.02, "the question artwork keeps its proportions")
local function visibleBody()
    for section in sections.children do
        if section.Visible then return control("Title", section), control("Paragraphs", section) end
    end
end
local heading, paragraphs = visibleBody()
assert(heading.ActualWidth > page * 0.9, "the question runs across the page, above the artwork")
assert(paragraphs.ActualWidth < page * 0.7, "the answer shares its row with the artwork")
layout(1024, 768)

local listen, listenIcon, listenLabel = control("ReadAloud"), control("ReadAloudIcon"), control("ReadAloudLabel")
local narration = app.libraryController.readAloud
local timings = require "encarta/Narration/Timings"
local function icon(name) return listenIcon.Source == "encarta/Icons/" .. name .. ".svg" end
assert(listen.Visible and listen.ActualHeight >= 44 and listen.ActualWidth < 200 and icon("volume-2"))
click("ReadAloud")
assert(playing and listenLabel.Text == "Pause" and icon("pause") and control("StopReading").Visible)
local function hear(mark)
    position = (mark[1] + mark[2]) / 2000
    narration:update()
    core.flushQueue()
end
local first = timings.moon[1].words
local title = visibleSection()
hear(first[1])
assert(title.HighlightedRange.X == first[1][3] and title.HighlightedRange.Y == first[1][4])
hear(first[2])
assert(title.HighlightedRange.X == first[2][3] and title.HighlightedRange.Y == first[2][4])

click("ReadAloud")
assert(not playing and listenLabel.Text == "Resume" and icon("play") and title.HighlightedRange.Y == first[2][4])
click("ReadAloud")
assert(playing and opened == 1, "Resume reuses the same recording and position")
hear(timings.moon[2].words[1])
assert(title.HighlightedRange.Y == 0)
hear(timings.moon[4].words[1])
assert(visibleSection().Text == "How far away is the Moon?", "the reader turns the page with the narration")
assert(app.libraryController.current.page == 2 and playing)
assert(not questionImage(), "a later question does not inherit the cover art")

click("StopReading")
assert(not playing and closed == 1 and listenLabel.Text == "Read aloud" and icon("volume-2"))
click("ReadAloud")
assert(playing and opened == 2 and position == 0)
click("Step5")
assert(not playing and closed == 2 and listenLabel.Text == "Read aloud")
assert(visibleSection().Text == "How can you track the Moon?" and not control("Next").Visible)
assert(questionImage().Src == "encarta/Images/moon.jpg", "a later question can reuse the cover art")
control("ReaderScroll").ContentOffset = { X = 0, Y = -100 }
click("Previous")
assert(visibleSection().Text == "Who has walked on the Moon?" and control("ReaderScroll").ContentOffset.Y == 0)
click("Back")
assert(library.Visible and not reader.Visible and control("ResultCount").Text == "1 ARTICLE")

search.Text = "no-such-article"
core.flushQueue()
assert(control("EmptyResults").Visible and control("ResultCount").Text == "0 ARTICLES")
assert(control("ClearSearch").Visible, "Clear appears once there is something to clear")
click("ClearSearch")
assert(control("Featured").Visible and not control("EmptyResults").Visible and not control("ClearSearch").Visible)
click("OpenFeatured")
assert(control("ArticleTitle").Text == "Octopuses")
assert(questionImage().Src == "encarta/Images/octopus.jpg")
click("Step2")
assert(not questionImage(), "a text-only question does not inherit the previous image")
layout(768, 1024)
heading, paragraphs = visibleBody()
page = control("ReaderScroll").ActualWidth
assert(heading.ActualWidth > page * 0.9 and paragraphs.ActualWidth > page * 0.9,
    "a text-only question spreads across the page without an empty picture column")
layout(1024, 768)
click("ReadAloud")
hear(timings.octopus[1].words[1])
assert(app.libraryController.current.page == 1, "narration brings the reader back to the text")
local octopusTitle = visibleSection()
click("Back")
assert(not playing and octopusTitle.HighlightedRange.Y == 0 and not control("StopReading").Visible)

click("Open_pyramids", wall)
assert(control("ArticleTitle").Text == "The Pyramids" and questionImage().Src == "encarta/Images/pyramids.jpg")
assert(not control("ReadAloud").Visible, "articles without a recording hide Read aloud")
click("Back")
click("Open_earth-layers", wall)
assert(control("ArticleTitle").Text == "Inside Earth" and questionImage().Src == "encarta/Images/earth-layers.jpg")
click("Back")

local function checkArtwork(path)
    local bytes = assert(fs.readTextFile(path), path)
    assert(bytes:sub(1, 2) == string.char(0xff, 0xd8), path .. " must be a bundled JPEG")
end
for _, article in ipairs(require "encarta/Content/Articles") do
    assert(article.question and article.image.caption ~= "", article.id)
    checkArtwork(article.image.path)
    if article.cover then checkArtwork(article.cover) end
    for _, section in ipairs(article.sections) do
        if section.image then checkArtwork(section.image.path) end
    end
end

local media = require "orca.media"
local invalid, err = media.openAudio("not an audio file")
assert(not invalid and err.code)
for name in pairs(timings) do
    local bytes = assert(fs.readTextFile("encarta/Narration/" .. name .. ".wav"))
    local audio, failure = media.openAudio(bytes)
    if not audio then assert(failure.code == "unsupported") else
        local time, active = audio:status()
        assert(time == 0 and not active)
        audio:close()
    end
end

print("PASS: Encarta question wall, illustrated bookshelf and reader, offline recordings and synchronized highlights")
