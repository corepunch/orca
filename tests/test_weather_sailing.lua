-- Sailors' Weather: is there a steady, usable wind? The sailing activity's
-- rules on hand-built hours, then the sailing edition running on its own
-- places (tests/weather_fixture.lua). This file belongs to the sailing
-- edition.
local F = dofile("tests/weather_fixture.lua").init { edition = "sailing" }
local check, hour, verdict = F.check, F.hour, F.verdict
local Sailing = require "Weather/activities/sailing"

local shore = { facing = 270 }
local breeze = { wind = 8, gust = 9.5, windDir = 0 }
check(verdict(Sailing, breeze, shore).verdict == "good", "A steady cross-shore 8 m/s is good sailing")
check(verdict(Sailing, { wind = 8, gust = 9.5, windDir = 90 }, shore).score <= 30, "Offshore wind is capped")
check(verdict(Sailing, { wind = 1.5, gust = 2 }, shore).verdict == "poor", "No wind is poor")
check(verdict(Sailing, { wind = 18, gust = 25 }, shore).score <= 20, "A gale is capped")
check(verdict(Sailing, { wind = 8, gust = 14, windDir = 0 }, shore).score < verdict(Sailing, breeze, shore).score,
    "Gusty wind scores lower than steady wind")
check(Sailing.windTo({ windDir = 90 }, shore) == "offshore" and Sailing.windTo({ windDir = 270 }, shore) == "onshore",
    "Wind relative to the shore")

F.checkEdition("sailing")
-- Sailors read knots first, with gusts as "18 g 26" (SA1).
local Units = require "Weather/units"
check(Units.current("wind") == "kn" and Sailing.note(hour { wind = 9.26, gust = 13.4, windDir = 0 }, { spot = { facing = 270 } })
    == "18 g 26 kn · cross-shore N", "The sailing edition starts in knots")
F.done("weather-sailing")
