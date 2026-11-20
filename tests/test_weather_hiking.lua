-- Hikers' Weather: is the trail safe and pleasant? The hiking activity's
-- rules on hand-built hours, then the hiking edition running on its own
-- places (tests/weather_fixture.lua). This file belongs to the hiking
-- edition.
local F = dofile("tests/weather_fixture.lua").init { edition = "hiking" }
local check, hour, verdict, forecast = F.check, F.hour, F.verdict, F.forecast
local Hiking = require "Weather/activities/hiking"

check(verdict(Hiking, { feels = 14, cloud = 10 }).verdict == "good", "Mild, clear and calm is good hiking")
check(verdict(Hiking, { code = 96 }).verdict == "poor", "Thunder is poor hiking")
check(verdict(Hiking, { gust = 25 }).score <= 30, "Knock-down gusts are capped")
check(verdict(Hiking, { visibility = 500 }).score < verdict(Hiking).score, "Fog lowers the score")
forecast.elevation = 3000
local icy = Hiking.rate(hour { freezingLevel = 2500 }, forecast):result()
forecast.elevation = nil
local saysIce = false
for _, reason in ipairs(icy.reasons) do saysIce = saysIce or reason.text:find("ice") ~= nil end
check(saysIce, "A freezing level below the spot warns of ice")

F.checkEdition("hiking")
F.done("weather-hiking")
