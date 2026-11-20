-- Fishers' Weather: will the fish bite? The fishing activity's rules on hand-
-- built hours, then the fishing edition running on its own places
-- (tests/weather_fixture.lua). This file belongs to the fishing edition.
local F = dofile("tests/weather_fixture.lua").init { edition = "fishing" }
local check, verdict = F.check, F.verdict
local Fishing = require "Weather/activities/fishing"

local dawn = verdict(Fishing)
check(dawn.verdict == "good", "Dawn, light breeze and slowly falling pressure is good fishing: " .. dawn.score)
check(dawn.reasons[1].text:find("Dawn"), "The strongest reason leads: " .. dawn.reasons[1].text)
check(verdict(Fishing, { code = 95 }).verdict == "poor", "Thunder means poor fishing")
check(verdict(Fishing, { pressureTrend = 2.5, time = "2026-11-18T12:00", cloud = 0, uv = 7 }).verdict == "poor",
    "Rising pressure under a bright noon sun is poor")
check(verdict(Fishing, { wind = 13, gust = 20 }).score <= 35, "Gusts cap the score")

F.checkEdition("fishing")
F.done("weather-fishing")
