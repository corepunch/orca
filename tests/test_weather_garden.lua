-- Gardeners' Weather: good for garden work, and what does the garden need?
-- The garden activity's rules on hand-built hours, then the garden edition
-- running on its own places (tests/weather_fixture.lua). This file belongs to
-- the garden edition.
local F = dofile("tests/weather_fixture.lua").init { edition = "garden" }
local check, NOW, hour, verdict = F.check, F.NOW, F.hour, F.verdict
local Garden = require "Weather/activities/garden"

check(verdict(Garden, { time = "2026-11-18T11:00", temp = 16, wind = 2 }).verdict == "good",
    "A mild, calm, dry hour is good for the garden")
check(verdict(Garden, { temp = -2, soilTemp = -1 }).score < 45, "Frozen ground is poor")
check(verdict(Garden, { code = 63 }).score < verdict(Garden).score, "Rain lowers the score")
local frostAhead = { epoch = NOW, hours = { { epoch = NOW + 3600 * 5, temp = -1 } } }
local frosty = Garden.rate(hour { epoch = NOW }, frostAhead):result()
local warns = false
for _, reason in ipairs(frosty.reasons) do warns = warns or reason.text:find("Frost") ~= nil end
check(warns, "Frost in the next hours is a reason")
local metrics = Garden.metrics(hour { epoch = NOW }, { epoch = NOW, hours = {
    { epoch = NOW + 3600, temp = 0.5, et0 = 0.3, rain = 0 }, { epoch = NOW + 7200, temp = 3, et0 = 0.3, rain = 0 },
    { epoch = NOW + 3600 * 30, temp = -9, et0 = 5, rain = 0 } } })
check(metrics[1].value == "frost risk" and metrics[1].note == "lowest in 24 h: 1°", "Frost looks 24 hours ahead only")
check(metrics[2].value == "no", "Little water loss needs no watering: " .. metrics[2].note)

F.checkEdition("garden")
F.done("weather-garden")
