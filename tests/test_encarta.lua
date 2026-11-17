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
layout(1024, 768)
local reading = app.libraryController
local scroll = control("ReaderScroll")
local function section(n) return control("Section" .. n, sections) end
-- The current step is the one whose mark is painted sun yellow on the next
-- frame.
local function current()
    layout(view.Width, view.Height)
    local found
    for i = 1, count(control("Steps")) do
        local mark = control("Mark", control("Step" .. i)).BackgroundColor
        if mark.R > 0.9 and mark.G > 0.7 and mark.B < 0.4 then
            assert(not found, "only one step is current")
            found = i
        end
    end
    assert(found == reading.currentSection)
    return found
end
local function heading(n) return control("Title", section(n)) end
local function visibleTop(n) return reading:contentTop(section(n)) + scroll.ContentOffset.Y end
assert(count(sections) == 5 and section(5).Visible, "the whole article is one page")
assert(heading(1).Text == "Why does the Moon seem to change shape?")
assert(not view:findChild("StepLabel", true) and control("ArticleCategory").Text == "SPACE" and current() == 1)

-- The contents column and the title bar paint under the status bar but keep
-- their content below it; the page starts under the bar.
view.SafeAreaTop = 24
layout(1024, 768)
local bar, pane = control("ReaderBar"), control("Pane")
local function screenTop(node)
    local top = 0
    while node and node ~= view do top, node = top + node.ActualY, node.parent end
    return top
end
assert(screenTop(bar) == 0 and screenTop(pane) == 0, "the panels reach the top edge")
assert(screenTop(control("Back")) >= 24 and screenTop(control("ArticleTitle")) >= 24, "the title stays below the status bar")
assert(screenTop(control("Step1")) >= 24, "the contents stay below the status bar")
assert(scroll.AdjustedContentInsetTop >= bar.ActualHeight and scroll.ContentOffset.Y == 0, "the page rests below the title bar")
assert(screenTop(control("ArticleDeck")) >= bar.ActualHeight, "the first line of the page is not covered")
local last = bar.parent:getFirstChild()
while last:getNext() do last = last:getNext() end
assert(last == bar, "the frosted bar draws over the page and takes the taps on Back")
view.SafeAreaTop = 0
layout(1024, 768)
assert(not view:findChild("Pager", true) and not view:findChild("Next", true), "no pager on a continuous page")
local image, caption = questionImage()
assert(image.Src == "encarta/Images/moon-phases.jpg" and caption.Text:match("Northern Hemisphere"), "a question shows its own diagram")
assert(control("SectionImage", section(5)).Src == "encarta/Images/moon.jpg", "a later question can reuse the cover art")
assert(not control("SectionMedia", section(2)).Visible, "a later question does not inherit the cover art")
assert(count(control("Steps")) == 5 and control("Pane").Opacity == 1)
assert(scroll.ScrollHeight > scroll.ActualHeight * 2, "the article scrolls")

local home = visibleTop(1)
click("Step3")
assert(reading.scrollTarget and current() == 3, "the step lights at once and the page glides there")
reading:finishScroll()
assert(math.abs(visibleTop(3) - home) < 0.5 and current() == 3, "a step brings its question to the top")
scroll.ContentOffset = { X = 0, Y = 0 }
assert(current() == 1, "scrolling by hand moves the current question")
scroll.ContentOffset = { X = 0, Y = -(reading:contentTop(section(2)) - reading:contentTop(section(1))) }
assert(current() == 2)
scroll.ContentOffset = { X = 0, Y = reading:scrollLimit() }
assert(current() == 5, "the end of the article is its last question")
click("Step4")
reading:finishScroll()
if scroll.ContentOffset.Y == reading:scrollLimit() then
    assert(current() == 4, "a question that cannot reach the top stays chosen")
end
scroll.ContentOffset = { X = 0, Y = scroll.ContentOffset.Y + 40 }
assert(current() == reading:sectionAt(scroll.ContentOffset.Y), "scrolling by hand releases the chosen question")
click("Step1")
reading:finishScroll()
assert(scroll.ContentOffset.Y == 0 and current() == 1)

layout(768, 1024)
assert(control("Pane").Opacity == 0 and scroll.ActualWidth > 700, "portrait gives the page the full width")
local page = sections.ActualWidth
assert(image.ActualWidth > 200 and image.ActualWidth < page / 2, "the question artwork sits in a side column")
assert(math.abs(image.ActualWidth / image.ActualHeight - 1.5) < 0.02, "the question artwork keeps its proportions")
assert(heading(1).ActualWidth > page * 0.9, "the question runs across the page, above the artwork")
assert(control("Paragraphs", section(1)).ActualWidth < page * 0.7, "the answer shares its row with the artwork")
assert(control("Paragraphs", section(2)).ActualWidth > page * 0.9, "a text-only question spreads across the page")
layout(1024, 768)

local listen, listenIcon, listenLabel = control("ReadAloud"), control("ReadAloudIcon"), control("ReadAloudLabel")
local narration = reading.readAloud
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
local title = heading(1)
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
hear(timings.moon[7].words[1])
assert(heading(3).HighlightedRange.Y > 0 and current() == 3, "the steps follow the narration down the page")
reading:finishScroll()
assert(playing and math.abs(visibleTop(3) - home) < 0.5, "a new question glides to the top while reading")
hear(timings.moon[8].words[1])
local paragraph = control("Paragraphs", section(3))
local top = reading:contentTop(paragraph) + scroll.ContentOffset.Y
assert(top >= 0 and top < scroll.ActualHeight and current() == 3, "narration keeps its word in view")

click("StopReading")
assert(not playing and closed == 1 and listenLabel.Text == "Read aloud" and icon("volume-2"))
click("ReadAloud")
assert(playing and opened == 2 and position == 0)
click("Step2")
assert(not playing and closed == 2 and listenLabel.Text == "Read aloud", "jumping to a question stops the narration")
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
layout(1024, 768)
assert(scroll.ContentOffset.Y == 0 and current() == 1, "a new article starts at its first question")
click("ReadAloud")
hear(timings.octopus[1].words[1])
assert(playing and heading(1).HighlightedRange.Y > 0)
local octopusTitle = heading(1)
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
