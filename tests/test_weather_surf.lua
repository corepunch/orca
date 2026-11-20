-- Surfers' Weather: is it worth paddling out? The surf activity's rules on
-- hand-built hours, then the surf edition running on its own places
-- (tests/weather_fixture.lua). This file belongs to the surf edition.
local F = dofile("tests/weather_fixture.lua").init { edition = "surf" }
local check, verdict = F.check, F.verdict
local Surf = require "Weather/activities/surf"

local beach = { facing = 270 }
local clean = { swellHeight = 1.5, swellPeriod = 12, swellDir = 280, windDir = 90, wind = 4 }
check(verdict(Surf, clean, beach).verdict == "good", "Offshore wind on a long-period swell is good surf")
check(verdict(Surf, { swellHeight = 1.5, swellPeriod = 12, swellDir = 280, windDir = 270, wind = 9 }, beach).verdict ~= "good",
    "The same swell blown out onshore is not")
check(verdict(Surf, { swellHeight = 0.2, swellPeriod = 6, swellDir = 280 }, beach).verdict == "poor", "Flat is poor")
check(verdict(Surf, { swellHeight = 1.5, swellPeriod = 12, swellDir = 280, isDay = false }, beach).score <= 30,
    "Dark caps the score")
check(verdict(Surf, { swellHeight = 1.5, swellPeriod = 12, swellDir = 90, windDir = 90, wind = 4 }, beach).score
    < verdict(Surf, clean, beach).score, "Swell pointed away from the beach scores lower")

F.checkEdition("surf")
F.done("weather-surf")
