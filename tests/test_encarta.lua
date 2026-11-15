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

core.flushQueue()
layout(1024, 768)
assert(library.Visible and not reader.Visible)
assert(control("ResultCount").Text == "13 ARTICLES" and count(wall) == 13 and wall.Visible and not shelf.Visible)
assert(control("CatalogTitle").Text == "Big questions")
local card = control("Open_mars", wall)
assert(card.ActualWidth > 200 and card.ActualHeight == 164, "four question cards per row in landscape")
assert(control("Question", card).Text == "Why is Mars red?")
assert(search.ActualHeight >= 44 and control("OpenFeatured").ActualHeight >= 44)
assert(control("FeaturedImage").ActualWidth > 900 and control("FeaturedImage").ActualHeight == 250)
local background = card.BackgroundColor
assert(background.B > 0.5 and background.R < 0.3, "space cards resolve the packaged category color")

assert(control("ShowWall").BackgroundColor.R > 0.9, "the question wall starts selected")
click("ShowShelf")
assert(control("ShowWall").BackgroundColor.R < 0.5 and control("ShowShelf").BackgroundColor.R > 0.9, "only the chosen view is highlighted")
assert(shelf.Visible and not wall.Visible and count(shelf) == 13)
assert(control("CatalogTitle").Text == "On the shelf")
local octopusCover, moonCover = control("Cover_octopus", shelf), control("Cover_moon", shelf)
assert(control("Art", octopusCover).Visible and not control("Icon", octopusCover).Visible, "art replaces the icon")
assert(control("Icon", moonCover).Visible and control("Volume", moonCover).Text == "Nº 02")
click("Category_space")
assert(count(shelf) == 3 and control("CatalogTitle").Text == "Space" and not control("Featured").Visible)
click("Category_all")
click("ShowWall")
assert(count(wall) == 13 and control("Featured").Visible)

search.Text = "  SPACE moon  "
core.flushQueue()
assert(control("ResultCount").Text == "1 ARTICLE" and not control("Featured").Visible)
assert(count(wall) == 1 and control("Open_moon", wall))
click("Open_moon", wall)
assert(not library.Visible and reader.Visible)
assert(control("ArticleTitle").Text == "The Moon" and not control("ArticleMedia").Visible)
assert(visibleSection().Text == "Light from the Sun")
assert(control("StepLabel").Text == "SPACE  ·  QUESTION 1 OF 2")
assert(not control("Previous").Visible and control("Next").Text == "Next question  ›")
layout(1024, 768)
assert(count(control("Steps")) == 4 and control("Pane").Opacity == 1)
layout(768, 1024)
assert(control("Pane").Opacity == 0 and control("ReaderScroll").ActualWidth > 600, "portrait gives the page the full width")
assert(control("ReadMore").ActualHeight >= 40)
layout(1024, 768)

local listen = control("ReadAloud")
local narration = app.libraryController.readAloud
local timings = require "encarta/Narration/Timings"
assert(listen.Visible and listen.ActualHeight >= 44 and listen.ActualWidth < 200)
click("ReadAloud")
assert(playing and listen.Text == "Pause" and control("StopReading").Visible)
local function hear(mark)
    position = (mark[1] + mark[2]) / 2000
    narration:update()
    core.flushQueue()
end
local first = timings["moon-first"][1].words
local title = visibleSection()
hear(first[1])
assert(title.HighlightedRange.X == first[1][3] and title.HighlightedRange.Y == first[1][4])
hear(first[2])
assert(title.HighlightedRange.X == first[2][3] and title.HighlightedRange.Y == first[2][4])

click("ReadAloud")
assert(not playing and listen.Text == "Resume" and title.HighlightedRange.Y == first[2][4])
click("ReadAloud")
assert(playing and opened == 1, "Resume reuses the same recording and position")
hear(timings["moon-first"][2].words[1])
assert(title.HighlightedRange.Y == 0)
hear(timings["moon-first"][3].words[1])
assert(visibleSection().Text == "A thin Moon, a round Moon", "the reader turns the page with the narration")
assert(app.libraryController.current.page == 2 and playing)

click("StopReading")
assert(not playing and closed == 1 and listen.Text == "Read aloud")
click("ReadAloud")
assert(playing and opened == 2 and position == 0)
click("ReadMore")
assert(not playing and closed == 2 and listen.Text == "Read aloud")
assert(visibleSection().Text == "A changing view" and app.libraryController.current.page == 1)
click("Next")
click("Next")
assert(control("FactPage").Visible and control("StepLabel").Text == "SPACE  ·  DID YOU KNOW?")
assert(control("Fact").Text:match("Footprints"))
click("Word1")
assert(control("Meaning", control("Words")).Visible)
click("Step4")
assert(control("QuizPage").Visible and not control("Next").Visible)
click("Answer2")
assert(control("Feedback").Text:match("^Not quite") and control("Answer2").BackgroundColor.B > control("Answer2").BackgroundColor.G)
click("Answer1")
assert(control("Feedback").Text:match("^You got it!") and control("Answer1").BackgroundColor.G > control("Answer1").BackgroundColor.B, "the right answer turns green")

control("ReaderScroll").ContentOffset = { X = 0, Y = -100 }
click("Open_water", control("Related"))
assert(control("ArticleTitle").Text == "The Water Cycle" and not control("Feedback").Visible)
assert(app.libraryController.current.page == 1)
click("Back")
assert(control("ArticleTitle").Text == "The Moon" and control("ReaderScroll").ContentOffset.Y == -100)
assert(control("QuizPage").Visible and app.libraryController.current.level == "more")
assert(control("Meaning", control("Words")).Visible and control("Feedback").Text:match("^You got it!"))
click("Back")
assert(library.Visible and not reader.Visible and control("ResultCount").Text == "1 ARTICLE")

search.Text = "no-such-article"
core.flushQueue()
assert(control("EmptyResults").Visible and control("ResultCount").Text == "0 ARTICLES")
click("ClearSearch")
assert(control("Featured").Visible and not control("EmptyResults").Visible)
click("OpenFeatured")
assert(control("ArticleTitle").Text == "Octopuses")
click("Step3")
assert(control("ArticleMedia").Visible and control("ArticleImage").Src == "encarta/Images/octopus.jpg")
click("ReadAloud")
hear(timings["octopus-first"][1].words[1])
assert(visibleSection() and app.libraryController.current.page == 1, "narration brings the reader back to the text")
local octopusTitle = visibleSection()
click("Back")
assert(not playing and octopusTitle.HighlightedRange.Y == 0 and not control("StopReading").Visible)

for _, article in ipairs(require "encarta/Content/Articles") do
    assert(article.question and article.fact, article.id)
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

print("PASS: Encarta question wall, bookshelf, paged reader, offline recordings and synchronized highlights")
