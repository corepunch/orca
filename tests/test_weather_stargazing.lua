-- Stargazers' Weather: will the night be dark and clear? The stargazing
-- activity's rules on hand-built hours, then the stargazing edition running
-- on its own places (tests/weather_fixture.lua). This file belongs to the
-- stargazing edition.
local F = dofile("tests/weather_fixture.lua").init { edition = "stargazing" }
local check, verdict = F.check, F.verdict
local Stargazing = require "Weather/activities/stargazing"

local night = { time = "2026-11-18T23:00", isDay = false, cloud = 0, humidity = 50, epoch = 1794207720 + 11 * 3600 }
check(verdict(Stargazing, night).verdict == "good", "A clear night near new moon is good stargazing")
check(verdict(Stargazing, { isDay = true, cloud = 0 }).score <= 5, "Daylight is capped")
check(verdict(Stargazing, { time = "2026-11-18T18:00", isDay = false, cloud = 0 }).score <= 35, "Twilight is capped")
check(verdict(Stargazing, { time = "2026-11-18T23:00", isDay = false, cloud = 90, epoch = night.epoch }).verdict == "poor",
    "Clouds are poor")
local fullNight = { time = "2026-11-25T00:00", isDay = false, cloud = 0, humidity = 50, epoch = 1795531980 + 9 * 3600 }
check(verdict(Stargazing, fullNight).score < verdict(Stargazing, night).score, "A full moon overhead dims the sky")

F.checkEdition("stargazing")
F.done("weather-stargazing")
