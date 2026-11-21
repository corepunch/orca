-- Stargazers' Weather: will the night be dark and clear? The stargazing
-- activity's rules on hand-built hours, its astronomy against published
-- times, tonight's cards on a synthetic forecast, red night mode, then the
-- stargazing edition running on its own places (tests/weather_fixture.lua).
-- This file belongs to the stargazing edition.
local F = dofile("tests/weather_fixture.lua").init { edition = "stargazing" }
local check, verdict, says = F.check, F.verdict, F.says
-- The activities register their settings (red mode's among them).
local Stargazing = require("Weather/activities").byId.stargazing
local Moon = require "Weather/editions/stargazing/moon"
local Night = require "Weather/editions/stargazing/night"
local Seeing = require "Weather/editions/stargazing/seeing"
local Sources = require "Weather/editions/stargazing/sources"
local Aurora = require "Weather/editions/stargazing/aurora"
local RedMode = require "Weather/editions/stargazing/redmode"
local Planets = require "Weather/editions/stargazing/planets"
local Events = require "Weather/editions/stargazing/events"
local Forecast = require "Weather/forecast"
local Settings = require "Weather/settings"
local Sky = require "Weather/sky"

local function utc(text) -- "2026-11-18T23:00" → epoch
    return Sky.epochOf(text) + Sky.minutes(text) * 60
end
local function minutesApart(a, b) return math.abs(utc(a) - utc(b)) / 60 end

-- The rules without a place (estimates from the phase and the sunset).
local night = { time = "2026-11-18T23:00", isDay = false, cloud = 0, humidity = 50, epoch = 1794207720 + 11 * 3600 }
check(verdict(Stargazing, night).verdict == "good", "A clear night near new moon is good stargazing")
check(verdict(Stargazing, { isDay = true, cloud = 0 }).score <= 5, "Daylight is capped")
check(verdict(Stargazing, { time = "2026-11-18T18:00", isDay = false, cloud = 0 }).score <= 35, "Twilight is capped")
check(verdict(Stargazing, { time = "2026-11-18T23:00", isDay = false, cloud = 90, epoch = night.epoch }).verdict == "poor",
    "Clouds are poor")
local fullNight = { time = "2026-11-25T00:00", isDay = false, cloud = 0, humidity = 50, epoch = 1795531980 + 9 * 3600 }
check(verdict(Stargazing, fullNight).score < verdict(Stargazing, night).score, "A full moon overhead dims the sky")

-- ST2: moonrise and moonset against Open-Meteo's daily moonrise/moonset
-- (fetched 2026-11-20) for five places, within 3 minutes. Lofoten's moon
-- neither rises nor sets from 24 November.
local MOON = {
    { 51.87424, -3.4319305, 0, { { "2026-11-18", "13:40", nil }, { "2026-11-20", "14:03", "02:13" },
        { "2026-11-24", "15:28", "07:59" } } },
    { 39.089405, -120.04556, -28800, { { "2026-11-18", "13:20", "00:09" }, { "2026-11-22", "15:04", "04:38" } } },
    { -43.97188, 170.44006, 46800, { { "2026-11-19", "14:29", "02:54" }, { "2026-11-26", "23:16", "06:28" } } },
    { -22.88225, -68.227325, -10800, { { "2026-11-21", "17:05", "04:01" }, { "2026-11-25", "21:35", "07:14" } } },
    { 68.2322, 14.554535, 3600, { { "2026-11-21", "13:09", "04:00" }, { "2026-11-23", "11:44", "09:01" } } },
}
for _, place in ipairs(MOON) do
    for _, day in ipairs(place[4]) do
        local events = Moon.events(day[1], place[1], place[2], place[3])
        local rise, set = day[2] and day[1] .. "T" .. day[2], day[3] and day[1] .. "T" .. day[3]
        check((rise == nil) == (events.rise == nil) and (not rise or minutesApart(rise, events.rise) <= 3),
            ("Moonrise at %.1f° on %s: %s, published %s"):format(place[1], day[1], tostring(events.rise), tostring(rise)))
        check((set == nil) == (events.set == nil) and (not set or minutesApart(set, events.set) <= 3),
            ("Moonset at %.1f° on %s: %s, published %s"):format(place[1], day[1], tostring(events.set), tostring(set)))
    end
end
check(Moon.events("2026-11-25", 68.2322, 14.554535, 3600).always == "up", "Lofoten's moon stays up on 25 November")
-- Full moon 2026-11-24 14:53 UTC (USNO): fully lit, phase one half.
local full = Moon.phase(utc("2026-11-24T14:53"))
check(full.lit > 0.995 and math.abs(full.phase - 0.5) < 0.01 and full.name == "full moon", "The full moon of 24 November")
check(Moon.phase(utc("2026-11-20T12:00")).name == "first quarter" or Moon.phase(utc("2026-11-20T12:00")).name
    == "waxing gibbous", "20 November is past first quarter")

-- ST1: sunset and sunrise against Open-Meteo's daily times (within 2
-- minutes); full darkness against Sky.twilight; no full darkness in a
-- Welsh June; no night at all in a Lofoten June; no day in its December.
local brecon = { lat = 51.87424, lon = -3.4319305 }
local frame = Night.frame(brecon, utc("2026-11-18T15:00"), 0)
check(minutesApart(frame.sunset, "2026-11-18T16:19") <= 2 and minutesApart(frame.sunrise, "2026-11-19T07:39") <= 2,
    "Brecon's night runs from sunset " .. frame.sunset .. " to sunrise " .. frame.sunrise)
local evening, morning = Sky.twilight("2026-11-18", brecon.lat, brecon.lon, 0), Sky.twilight("2026-11-19", brecon.lat, brecon.lon, 0)
check(frame.level == "night" and minutesApart(Night.iso(frame.dark.from), evening.astronomical.dusk) <= 2
    and minutesApart(Night.iso(frame.dark.to), morning.astronomical.dawn) <= 2, "Full darkness from astronomical dusk to dawn")
check(Night.frame(brecon, utc("2026-11-19T02:00"), 0).from == frame.from, "At 2 am it is still the same night, across midnight")
local tekapo = Night.frame({ lat = -43.97188, lon = 170.44006 }, utc("2026-11-18T05:00"), 46800)
check(minutesApart(tekapo.sunset, "2026-11-18T20:47") <= 2 and minutesApart(tekapo.sunrise, "2026-11-19T05:59") <= 2,
    "Lake Tekapo's night: " .. tekapo.sunset .. "–" .. tekapo.sunrise)
local june = Night.frame(brecon, utc("2026-06-21T12:00"), 3600)
check(june.dark == nil and june.level == "astronomical" and #june.moonFree == 0, "No full darkness in a Welsh June")
local lofoten = { lat = 68.23, lon = 14.56 }
check(Night.frame(lofoten, utc("2026-06-21T12:00"), 7200).polar == "day", "No night in a Lofoten June")
local polar = Night.frame(lofoten, utc("2026-12-21T12:00"), 3600)
check(polar.polar == "night" and polar.sunrise == nil and polar.dark ~= nil, "No sunrise in a Lofoten December")

-- Hours at a place, with real sun and moon heights.
local function at(spot, time, fields)
    local hour = { time = time, date = time:sub(1, 10), epoch = utc(time), isDay = false, cloud = 0, cloudLow = 0,
        cloudMid = 0, cloudHigh = 0, temp = 8, dewPoint = 0, humidity = 60, wind = 3, gust = 5 }
    for key, value in pairs(fields or {}) do hour[key] = value end
    return hour
end
local function rate(spot, time, fields)
    return Stargazing.rate(at(spot, time, fields), { spot = spot, epoch = utc(time), offset = 0 }):result()
end

-- ST1 in the score: twilight caps, full darkness does not.
check(rate(brecon, "2026-11-18T16:10").score <= 20, "Civil twilight at Brecon is capped")
check(rate(brecon, "2026-11-18T17:00").score <= 40, "Nautical twilight is capped")
check(rate(brecon, "2026-11-19T04:00").verdict == "good", "4 am at Brecon, moon set, clear: good")
local summer = rate(brecon, "2026-06-21T00:00")
check(summer.verdict == "fair" and says(summer, "Not fully dark"), "A June midnight at Brecon is at best fair")

-- ST2 in the score: the waxing gibbous moon of 20 November is up in the
-- evening (sets 02:13) and down before dawn.
local moonUp, moonDown = rate(brecon, "2026-11-20T21:00"), rate(brecon, "2026-11-20T04:00")
check(says(moonUp, "moon") and not says(moonUp, "moon is down") and says(moonDown, "moon is down")
    and moonDown.score > moonUp.score, "The moon up costs points; set, it gives them")

-- ST4: thin high cloud and smoke are not "clear".
local veil = rate(brecon, "2026-11-19T04:00", { cloud = 70, cloudHigh = 70 })
check(veil.verdict ~= "good" and says(veil, "veil of high cloud") and not says(veil, "^Clear sky"), "A cirrus veil is not clear")
check(says(rate(brecon, "2026-11-19T04:00", { cloud = 20, cloudHigh = 20 }), "high cloud"), "Thin high cloud is named")
local smoke = rate(brecon, "2026-11-19T04:00", { aod = 0.5 })
check(says(smoke, "Smoke or dust") and smoke.score < rate(brecon, "2026-11-19T04:00").score, "Smoke lowers a clear night")
check(says(rate(brecon, "2026-11-19T04:00", { aod = 0.25 }), "Haze"), "Haze is named")

-- ST5: dew and frost on the optics from the dew-point gap.
check(Stargazing.dew(at(brecon, "2026-11-19T04:00", { temp = 8, dewPoint = 2 })) == nil, "Six degrees above the dew point: dry")
local dew = Stargazing.dew(at(brecon, "2026-11-19T04:00", { temp = 4, dewPoint = 2, wind = 1 }))
check(dew and dew.sure and not dew.frost, "Two degrees above the dew point on a clear, calm night: dew for sure")
check(Stargazing.dew(at(brecon, "2026-11-19T04:00", { temp = 5, dewPoint = 2, wind = 5 })).sure == false,
    "Three degrees in a breeze: dew likely")
check(Stargazing.dew(at(brecon, "2026-11-19T04:00", { temp = -1, dewPoint = -3 })).frost, "Below-zero dew point: frost")
check(says(rate(brecon, "2026-11-19T04:00", { temp = 4, dewPoint = 3.5 }), "heater"), "Dew is a reason")

-- ST11: fog now caps; calm, saturated air with a shallow layer is a risk.
check(rate(brecon, "2026-11-19T04:00", { code = 45 }).score <= 20, "Fog caps the score")
check(Stargazing.fog(at(brecon, "2026-11-19T04:00", { temp = 3, dewPoint = 2.5, wind = 1, boundary_layer_height = 80 }))
    == "risk", "Calm, saturated, shallow: fog may form")
check(Stargazing.fog(at(brecon, "2026-11-19T04:00", { temp = 3, dewPoint = 2.5, wind = 5, boundary_layer_height = 80 }))
    == nil, "A breeze keeps fog away")

-- ST7: other models.
local doubt = rate(brecon, "2026-11-19T04:00", { models = { a = 90, b = 80, c = 10 } })
check(says(doubt, "Most weather models expect cloud") and doubt.score < rate(brecon, "2026-11-19T04:00").score,
    "Clear here but cloudy in most models costs points")
check(says(rate(brecon, "2026-11-19T04:00", { cloud = 40, cloudLow = 40, models = { a = 0, b = 70 } }), "disagree"),
    "Models far apart: uncertain")
local agree = Sources.agreement({ models = { a = 5, b = 10, c = 60 } })
check(agree.count == 3 and agree.clear == 2 and agree.spread == 55, "Two of three models clear")

-- ST8: seeing and transparency estimates.
local calm = Seeing.seeing({ wind_speed_250hPa = 12, wind = 1, temperature_850hPa = 2, temperature_700hPa = -2,
    geopotential_height_850hPa = 1450, geopotential_height_700hPa = 3000 }, 300)
local jet = Seeing.seeing({ wind_speed_250hPa = 48, wind = 1 }, 300)
check(calm.value >= 4 and jet.value <= 3 and jet.why:find("jet stream"), "The jet stream spoils seeing")
check(math.abs(Seeing.lapse({ temperature_850hPa = 10, temperature_700hPa = -5, geopotential_height_850hPa = 1500,
    geopotential_height_700hPa = 3000 }, 0) - 1) < 1e-6, "Lapse rate: 15 °C over 1500 m is 1 °C per 100 m")
check(Seeing.lapse({ temperature_850hPa = 10, temperature_700hPa = 0, temperature_500hPa = -20 }, 2400) ~= nil,
    "A high site uses the layer above the ground")
check(Seeing.seeing({ wind = 1 }) == nil, "No upper winds, no seeing estimate")
check(Seeing.transparency({ cloudHigh = 0, aod = 0.05, total_column_integrated_water_vapour = 4 }).value == 5
    and Seeing.transparency({ cloudHigh = 60, aod = 0.5 }).value <= 2, "Transparency: clean and dry beats veiled and smoky")

-- ST9: the aurora from Kp, magnetic latitude and the dark.
check(math.abs(Aurora.magneticLatitude(69.65, 18.96) - 67) < 2 and math.abs(Aurora.magneticLatitude(brecon.lat, brecon.lon) - 54) < 2,
    "Magnetic latitude: Tromsø about 67°, Brecon about 54°")
check(Aurora.reach(69.65, 18.96, 2) == "overhead" and Aurora.reach(brecon.lat, brecon.lon, 5) == "low"
    and Aurora.reach(brecon.lat, brecon.lon, 2) == nil and Aurora.reach(-22.9, -68.2, 9) == nil, "Where Kp reaches")
local steps = Aurora.parseKp({ { time_tag = "2026-11-19T03:00:00", kp = 5.33, observed = "predicted", noaa_scale = "G1" },
    { time_tag = "2026-11-19T06:00:00", kp = 2, observed = "predicted" } })
check(#steps == 2 and Aurora.kpAt(steps, utc("2026-11-19T04:30")) == 5.33 and Aurora.kpAt(steps, utc("2026-11-20T04:30")) == nil,
    "Kp from NOAA's list of objects")
check(#Aurora.parseKp({ { "time_tag", "kp", "observed", "noaa_scale" }, { "2026-11-19 03:00:00", "4.67", "predicted", "G1" } }) == 1,
    "Kp from NOAA's older list of rows")
check(#Aurora.parseKp({ hourly = {} }) == 0, "An answer that is not Kp reads as none")
local auroraHour = at(brecon, "2026-11-19T04:00", { kp = 6 })
check(says(Stargazing.rate(auroraHour, { spot = brecon }):result(), "Aurora possible low in the north"),
    "Kp 6 at Brecon on a dark, clear night: aurora possible")
check(not says(rate(brecon, "2026-11-18T14:00", { kp = 6, isDay = true }), "Aurora"), "No aurora talk by day")

-- Sources read their answers onto the hours by local time.
local hours = { { time = "2026-11-18T22:00" }, { time = "2026-11-18T23:00" } }
Sources.air.parse({ hourly = { time = { "2026-11-18T22:00", "2026-11-18T23:00" }, aerosol_optical_depth = { 0.05, 0.31 },
    dust = { 0, 2 } } }, { hours = hours })
Sources.models.parse({ hourly = { time = { "2026-11-18T23:00" }, cloud_cover_ecmwf_ifs025 = { 10 },
    cloud_cover_gfs_seamless = { 80 }, cloud_cover_low = { 5 } } }, { hours = hours })
check(hours[1].aod == 0.05 and hours[2].aod == 0.31 and hours[2].dust == 2, "Aerosols land on their hours")
check(hours[2].models.ecmwf_ifs025 == 10 and hours[2].models.gfs_seamless == 80 and hours[2].models.low == nil
    and hours[1].models == nil, "Only the named models land on their hours")
local airUrl = Sources.air.url(brecon, Forecast)
check(airUrl:find("air%-quality%-api") and airUrl:find("aerosol_optical_depth"), "Aerosols come from the Air Quality API")
check(Sources.models.url(brecon, Forecast):find("models=ecmwf_ifs025,gfs_seamless,icon_seamless"), "Three models' clouds")

-- ST3, ST10 and the cards on a synthetic forecast at Brecon: clear all
-- week, damp from 03:00, the evening of 18 November. The waxing moon sets
-- at 00:56; low in the west before that, it spoils only the early evening.
local function hourOf(i) return i % 24 end
local weather = F.weather(brecon.lat, {
    cloud_cover = function() return 0 end, cloud_cover_low = function() return 0 end,
    cloud_cover_mid = function() return 0 end, cloud_cover_high = function() return 0 end,
    temperature_2m = function(i) return (hourOf(i) >= 3 and hourOf(i) < 6) and 4 or 10 end,
    dew_point_2m = function() return 3 end, weather_code = function() return 0 end,
})
weather.longitude = brecon.lon
local forecast = Forecast.parse(weather, nil, utc("2026-11-18T15:00"), brecon)
forecast.spot = brecon
local cards = {}
for _, card in ipairs(Stargazing.cards(forecast)) do cards[card.id] = card end
local tonight = Stargazing.judge(forecast, Night.tonight(forecast), forecast.epoch)
check(tonight.hours[1].hour.time == "2026-11-18T16:00" and tonight.hours[#tonight.hours].hour.time == "2026-11-19T07:00",
    "Tonight is one night, sunset to sunrise across midnight")
check(cards.tonight.value == "go" and tonight.run.last.time == "2026-11-19T05:00" and tonight.run.count >= 5,
    "Its clear run counts on past midnight: " .. cards.tonight.detail)
check(cards.tonight.detail:find("h in a row"), "Tonight says how many clear hours in a row")
check(cards.darkness.value:match("^18:%d%d–05:%d%d$"), "Full darkness " .. cards.darkness.value)
-- The moon sets at 00:56 (published) and the rest of the dark is moon-free.
check(cards.moon.value:match("^moon%-free 00:5%d–05:%d%d$") and cards.moon.detail:find("sets 00:5"),
    "Moon-free after moonset: " .. cards.moon.value .. " · " .. cards.moon.detail)
check(cards.dew.value == "heaters from 03:00", "Dew heaters from 03:00")
check(cards.fog == nil and cards.aurora == nil, "No fog or aurora card without a reason")
check(cards.sky and cards.sky.title:find("estimate"), "Seeing and transparency are labelled estimates")
check(Stargazing.focus(forecast) == forecast.now + 3, "The verdict shows the first dark hour, 18:00")
local chart = Stargazing.charts[2].value(forecast.hours[forecast.now], forecast)
check(chart > 0 and chart < 90, "The second chart is the moon's height")
-- A cloudy night: stay in, the reason, and the next good night (the
-- synthetic week has thunder all of 20 November, so not thursday's).
local cloudy = F.weather(brecon.lat, { cloud_cover_low = function(i) return i < 30 and 100 or 0 end,
    cloud_cover = function(i) return i < 30 and 100 or 0 end, cloud_cover_mid = function() return 0 end,
    cloud_cover_high = function() return 0 end })
cloudy.longitude = brecon.lon
local grey = Forecast.parse(cloudy, nil, utc("2026-11-18T15:00"), brecon)
grey.spot = brecon
local greyCards = Stargazing.cards(grey)
check(greyCards[1].value == "stay in" and greyCards[1].detail:find("Cloudy") and greyCards[1].detail:find("next good night: fri 20"),
    "A cloudy night says stay in, why, and the next good night: " .. greyCards[1].detail)
-- No night in a Lofoten June.
local midsummer = { hours = {}, spot = lofoten, epoch = utc("2026-06-21T10:00"), offset = 7200 }
check(Stargazing.cards(midsummer)[1].value == "no night", "Midsummer in Lofoten: no night")

-- Planets tonight on the Brecon forecast above: Venus (past inferior
-- conjunction on 24 October 2026) and Jupiter before dawn, Saturn
-- (opposition 4 October 2026) in the evening.
check(cards.planets and cards.planets.value:find("^Venus, Jupiter") and cards.planets.detail:find("Saturn 0%.%d · 1%d:%d%d%–")
    and cards.planets.detail:find("highest %d+° %a+ at %d%d:%d%d"),
    "Planets tonight at Brecon: " .. (cards.planets and cards.planets.value .. " · " .. cards.planets.detail or "none"))
check(cards.meteors == nil and cards.eclipse == nil, "No shower near its peak and no eclipse in the week of 18 November 2026")

-- SV-ST1 to SV-ST3: live space weather, on synthetic answers in the
-- services' documented shapes (tests/fixtures/stargazing/space-weather.lua);
-- the live services are never asked in tests.
local Space = dofile("tests/fixtures/stargazing/space-weather.lua")
local tromso = { id = "tromso", lat = 69.65, lon = 18.96 }
local atacama = { lat = -22.9, lon = -68.2 }
local NIGHT = utc("2026-11-18T22:00")
check(Sky.sunAltitude(NIGHT, tromso.lat, tromso.lon) < -18, "22:00 UTC is full night at Tromsø")
local MAG, PLASMA = { bz_gsm = "bz", bt = "bt" }, { speed = "speed", density = "density" }
local function south(ago) return ago < 40 and -8 or 2 end
local function fast() return 560 end

-- SV-ST1: solar wind now.
local southward = Aurora.solarWind(Aurora.rows(Space.mag(NIGHT, south), MAG), Aurora.rows(Space.plasma(NIGHT, fast), PLASMA), NIGHT)
check(southward.bz == -8 and southward.minutesSouth == 40 and southward.speed == 560 and southward.density == 4.5,
    "Bz −8 nT south for the last 40 minutes, 560 km/s")
local windLine, windLevel = Aurora.windText(southward)
check(windLevel == "watch" and windLine:find("Bz south −8 nT for 40 min: the next hour is the one to watch")
    and windLine:find("fast solar wind 560 km/s"), "Southward Bz held half an hour: " .. windLine)
local quietWind = Aurora.solarWind(Aurora.rows(Space.mag(NIGHT, function() return 3 end), MAG), {}, NIGHT)
check(select(2, Aurora.windText(quietWind)) == "quiet" and Aurora.windText(quietWind):find("Bz north"), "Northward Bz is quiet")
local gap = Aurora.solarWind(Aurora.rows(Space.mag(NIGHT, function(ago) return ago >= 20 and -6 or nil end), MAG),
    Aurora.rows(Space.plasma(NIGHT, function(ago) return ago >= 20 and 450 or nil end), PLASMA), NIGHT)
check(gap == nil, "A spacecraft gap: rows older than 15 minutes count as missing")
check(#Aurora.rows({ { time_tag = "2026-11-18 21:59:00.000", bz_gsm = -3.1, bt = 5 } }, MAG) == 1,
    "Solar wind from a list of objects too")

-- SV-ST2: the OVATION nowcast, cut to the cells around the place.
local cells = {}
for lat = 60, 80 do
    cells[lat] = {}
    for lon = 10, 30 do cells[lat][lon] = lat >= 72 and 60 or 25 end
end
Aurora.reset()
Aurora.source.parse(Space.kp(NIGHT, 3), { epoch = NIGHT, hours = {} })
check(Aurora.ovationSource.url(tromso) == Aurora.OVATION_URL and Aurora.magSource.url(tromso) == Aurora.MAG_URL,
    "Tromsø on a dark night asks for the nowcast and the solar wind")
local grid, read = Space.ovation(NIGHT, cells), { epoch = NIGHT, hours = {} }
Aurora.ovationSource.parse(grid, read)
check(#grid.coordinates > 300 and #grid.coordinates < 600, "Only the cells within 10° are kept: " .. #grid.coordinates)
local cast = Aurora.ovationAt(read.ovation, tromso.lat, tromso.lon, NIGHT)
check(cast.chance == 25 and cast.horizonChance == 60 and not cast.stale, "25% overhead, 60% low in the north")
check(Aurora.ovationAt(Aurora.ovation(Space.ovation(NIGHT, cells, 3 * 3600)), tromso.lat, tromso.lon, NIGHT).stale,
    "A nowcast observed three hours ago is stale")
check(Aurora.ovationSource.url(atacama) == nil and Aurora.alertsSource.url(atacama) == nil
    and Aurora.hp30Source.url(atacama) == nil, "The Atacama never asks for aurora data")
Aurora.source.parse(Space.kp(utc("2026-11-18T12:00"), 5), { epoch = utc("2026-11-18T12:00"), hours = {} })
check(Aurora.ovationSource.url(brecon) == nil and Aurora.magSource.url(brecon) == nil
    and Aurora.alertsSource.url(brecon) == Aurora.ALERTS_URL, "Brecon at noon: no nowcast by day, alerts still")

-- SV-ST3: NOAA's alerts and GFZ's Hp30.
local alerts = Aurora.parseAlerts(Space.alerts(NIGHT), NIGHT)
check(#alerts == 1 and alerts[1].kind == "watch" and alerts[1].g == 2,
    "A cancelled watch and a lapsed warning drop out; the G2 watch stays")
check(Aurora.alertLine(alerts, tromso.lat, tromso.lon, NIGHT, 3600) == "A G2 storm watch for tomorrow night",
    "The watch as a calm line")
check(Aurora.alertLine(alerts, atacama.lat, atacama.lon, NIGHT, 0) == nil, "A storm that cannot reach the place says nothing")
local RISING = { 2, 2.33, 2, 2.67, 3, 3.33, 4, 4.67, 5, 5.33, 5.67, 6 }
local activity = Aurora.activityNow(Aurora.parseHp30(Space.hp30(NIGHT, RISING)), NIGHT)
check(activity and activity.value == 6 and activity.rising, "Hp30 up from 4.7 to 6 in two hours: rising now")
check(Aurora.activityNow(Aurora.parseHp30(Space.hp30(NIGHT - 3 * 3600, RISING)), NIGHT) == nil, "Old Hp30 says nothing")
check(Aurora.hp30Url(NIGHT):find("start=2026%-11%-18T16:00:00Z&end=2026%-11%-18T22:00:00Z&index=Hp30"), "Six hours of Hp30")

-- The whole chain through Forecast.load at Tromsø: every answer read, the
-- nowcast replaces the Kp estimate, and the aurora card says it all.
local tromsoWeather = F.weather(tromso.lat, {
    cloud_cover = function() return 0 end, cloud_cover_low = function() return 0 end,
    cloud_cover_mid = function() return 0 end, cloud_cover_high = function() return 0 end,
    is_day = function(i) return Sky.sunAltitude(utc("2026-11-18T00:00") + i * 3600 + 1800, tromso.lat, tromso.lon) > 0 and 1 or 0 end,
})
tromsoWeather.longitude = tromso.lon
local function spaceNet(answer)
    local net = F.network()
    net.answers["latitude=69%.65"] = function() return tromsoWeather end
    net.answers["k%-index%-forecast"] = answer or function() return Space.kp(NIGHT, 4) end
    net.answers["mag%-2%-hour"] = answer or function() return Space.mag(NIGHT, south) end
    net.answers["plasma%-2%-hour"] = answer or function() return Space.plasma(NIGHT, fast) end
    net.answers["ovation_aurora"] = answer or function() return Space.ovation(NIGHT, cells) end
    net.answers["alerts%.json"] = answer or function() return Space.alerts(NIGHT) end
    net.answers["kp%.gfz%.de"] = answer or function() return Space.hp30(NIGHT, RISING) end
    return net
end
local function auroraAt(net)
    Aurora.reset()
    local loaded, _, raw = Forecast.load(tromso, false, net.fetch, NIGHT, { sources = Stargazing.sources })
    loaded.spot = tromso
    for _, card in ipairs(Stargazing.cards(loaded)) do if card.id == "aurora" then return card, loaded, raw end end
    return nil, loaded, raw
end
local liveCard, liveForecast, liveRaw = auroraAt(spaceNet())
check(next(liveForecast.missing) == nil and liveForecast.solarWind and liveForecast.ovation and liveForecast.hp30
    and #liveForecast.spaceAlerts == 1, "Every space-weather answer is read")
check(#liveRaw.sources.ovation.coordinates < 600, "Only the cells near Tromsø are saved with the forecast")
check(liveCard and liveCard.title == "aurora now" and liveCard.value == "possible now" and liveCard.tone == "fair",
    "The nowcast leads the aurora card: " .. tostring(liveCard and liveCard.value))
check(liveCard.detail:find("NOAA nowcast 25%% here, 60%% low in the north") and liveCard.detail:find("Bz south −8 nT for 40 min")
    and liveCard.detail:find("activity is rising now") and liveCard.detail:find("A G2 storm watch for tomorrow night")
    and liveCard.detail:find("Kp 4 forecast"), "The aurora card: " .. liveCard.detail)
local nowHour = liveForecast.hours[liveForecast.now]
check(says(Stargazing.rate(nowHour, liveForecast):result(), "Aurora nowcast: 25%% chance overhead"),
    "This hour's reason comes from the nowcast, not Kp")
check(not says(Stargazing.rate(liveForecast.hours[liveForecast.now + 2], liveForecast):result(), "nowcast"),
    "Two hours on, the Kp estimate again")
-- Every service silent: the card says the data is missing, never "no aurora".
local silentCard, silentForecast = auroraAt(spaceNet(function() return nil, "No connection." end))
check(silentForecast.missing.kp and silentForecast.missing.solarMag and silentForecast.missing.ovation,
    "Failed services are recorded as missing")
check(silentCard and silentCard.value == "no live data" and silentCard.detail:find("no answer from the Kp forecast, solar wind")
    and silentCard.detail:find("says nothing about the sky") and not (silentCard.detail .. silentCard.value):find("no aurora")
    and not silentCard.value:find("quiet"), "A silent service is not a quiet sky: " .. tostring(silentCard and silentCard.detail))

-- SV-ST4: planets against published events (USNO, NASA and Sky &
-- Telescope almanacs): the conjunctions' separations, opposition dates
-- (180° from the sun in longitude), greatest elongations and magnitudes.
local function planet(name, time) return Planets.position(name, utc(time)) end
local function apart(a, b) return Planets.separation(a.ra, a.dec, b.ra, b.dec) end
check(apart(planet("Jupiter", "2020-12-21T18:20"), planet("Saturn", "2020-12-21T18:20")) < 0.2,
    "The great conjunction of 21 December 2020: 0.1° apart")
check(apart(planet("Venus", "2016-08-27T22:00"), planet("Jupiter", "2016-08-27T22:00")) < 0.2,
    "Venus and Jupiter on 27 August 2016: 4′ apart")
for _, opposition in ipairs { { "Mars", "2020-10-13T23:20" }, { "Jupiter", "2023-11-03T05:00" }, { "Saturn", "2023-08-27T08:00" },
    { "Jupiter", "2026-01-10T08:00" }, { "Saturn", "2026-10-04T12:00" } } do
    local p = planet(opposition[1], opposition[2])
    check(math.abs((p.lon - p.sunLon) % 360 - 180) < 1, ("%s at opposition on %s"):format(opposition[1], opposition[2]:sub(1, 10)))
end
check(math.abs(planet("Venus", "2020-03-24T22:00").elongation - 46.1) < 0.5, "Venus 46.1° from the sun, 24 March 2020")
check(math.abs(planet("Mercury", "2020-02-10T14:00").elongation - 18.2) < 0.5, "Mercury 18.2° from the sun, 10 February 2020")
for _, bright in ipairs { { "Mars", "2020-10-13T23:20", -2.6 }, { "Jupiter", "2023-11-03T05:00", -2.9 },
    { "Saturn", "2023-08-27T08:00", 0.4 }, { "Venus", "2020-04-28T12:00", -4.7 } } do
    local mag = planet(bright[1], bright[2]).mag
    check(math.abs(mag - bright[3]) <= 0.25, ("%s magnitude %.2f, published %.1f"):format(bright[1], mag, bright[3]))
end
-- At opposition Jupiter culminates at local midnight (00:20 UTC at Brecon).
local january = Night.frame(brecon, utc("2026-01-10T12:00"), 0)
local jupiter
for _, entry in ipairs(Planets.night(january, brecon)) do if entry.name == "Jupiter" then jupiter = entry end end
check(jupiter and math.abs(jupiter.best.epoch - utc("2026-01-11T00:20")) <= 30 * 60 and jupiter.best.altitude > 55
    and Sky.compass(jupiter.best.azimuth) == "S", "Jupiter highest in the south around midnight at opposition")
for _, entry in ipairs(Planets.night(frame, brecon)) do
    if entry.rise then
        check(math.abs(Planets.altitude(entry.name, entry.rise, brecon.lat, brecon.lon) - Planets.HORIZON) < 0.2,
            entry.name .. " rises on the horizon")
    end
end

-- SV-ST5: meteor showers and lunar eclipses.
local function night(spot, time, offset) return { spot = spot, epoch = utc(time), offset = offset or 0, hours = {} } end
local function cardOf(forecast, id)
    for _, card in ipairs(Stargazing.cards(forecast)) do if card.id == id then return card end end
end
local perseids = cardOf(night(brecon, "2026-08-12T20:00", 3600), "meteors")
check(perseids and perseids.title == "Perseids" and perseids.value == "peak tonight" and perseids.detail:find("no moonlight")
    and tonumber(perseids.detail:match("up to about (%d+) an hour")) >= 60 and perseids.tone == "good",
    "The Perseids at new moon, 12 August 2026: " .. tostring(perseids and perseids.detail))
local quadrantids = cardOf(night(brecon, "2026-01-03T18:00"), "meteors")
check(quadrantids and quadrantids.title == "Quadrantids" and quadrantids.detail:find("full moon washes out"),
    "The Quadrantids under the full moon of 3 January 2026: " .. tostring(quadrantids and quadrantids.detail))
local qFrame = Night.frame(brecon, utc("2026-01-03T18:00"), 0)
check(Events.rate(Events.SHOWERS[1], qFrame, brecon, 0).rate < 0.6 * 80, "Moonlight cuts the Quadrantids' rate")
check(cardOf(night(brecon, "2026-08-10T20:00", 3600), "meteors").value == "peak in 2 nights", "Two nights before the peak")
check(cardOf(night(brecon, "2026-09-20T20:00", 3600), "meteors") == nil, "No shower card far from a peak")
local eclipse = cardOf(night(brecon, "2029-12-20T18:00"), "eclipse")
check(eclipse and eclipse.value == "total lunar eclipse tonight" and eclipse.detail:find("greatest at 22:42, the moon %d+° up"),
    "The total lunar eclipse of 20 December 2029 from Brecon: " .. tostring(eclipse and eclipse.detail))
local soon = cardOf(night(brecon, "2029-12-17T18:00"), "eclipse")
check(soon and soon.value == "total lunar eclipse thu 20" and soon.hub == false, "An eclipse later in the week: " .. tostring(soon and soon.value))
check(cardOf(night({ lat = -33.9, lon = 151.2 }, "2029-12-20T08:00", 39600), "eclipse") == nil,
    "Sydney sees no moon at 22:42 UTC: no eclipse card")
for _, entry in ipairs(Events.LUNAR_ECLIPSES) do
    local when = utc(entry.at)
    check(math.abs(Moon.phase(when).phase - 0.5) < 0.006 and math.abs(Moon.position(when).lat) < 1.3,
        "The eclipse of " .. entry.at .. " is a full moon near a node")
end

-- Credits on the settings' about.
local about = {}
for _, row in ipairs(require("Weather/provider").about()) do about[row[1]] = row[2] end
check(about["Space weather by NOAA SWPC"] and about["Hp30 index by GFZ Potsdam"]:find("CC BY 4.0")
    and about["Moon and planet positions after P. Schlyter"] and about["Meteor showers: IMO and IAU MDC"]
    and about["Eclipse Predictions by Fred Espenak, NASA/GSFC Emeritus"], "Every service is credited on the about")
Aurora.reset()

-- ST6: red night mode.
check(RedMode.classes("stargazing", true) == "stargazing night" and RedMode.classes("stargazing night", false) == "stargazing",
    "Night is a state class beside the edition's")
Settings.set("stargazing", "redmode", "on")
check(RedMode.on() and Stargazing.stylesheet() == RedMode.NIGHT, "Red mode on")
Settings.set("stargazing", "redmode", "off")
check(not RedMode.on() and Stargazing.stylesheet() == RedMode.DAY, "Red mode off")
Settings.set("stargazing", "redmode", "auto")
RedMode.follow({ spot = brecon, epoch = utc("2026-11-18T12:00") })
check(not RedMode.on(), "Auto: daylight at Brecon at noon")
check(RedMode.on(utc("2026-11-18T17:00")) and not RedMode.on(utc("2026-11-18T16:30")),
    "Auto switches at civil dusk (16:58 at Brecon), not at sunset")

-- The app at Lake Tahoe, 06:00 local (UTC−8) before a late-November
-- dawn: the hub turns red when its forecast is drawn, every new page is
-- born red, and settings switch it at once. The synthetic week here keeps
-- Tahoe's real clock, so the sky is computed for it.
local Places = require "Weather/places"
local store = F.storage("stargazing-red", Places)
RedMode.reset()
local net = F.network { placeClock = true }
local ui = F.launch { places = Places, storage = store, net = net, clock = function() return F.NOW + 8 * 3600 end }
local photo = F.checkHubPhoto(ui)
local wind, age = ui.find("HubWind"), ui.find("AgeText")
check(photo.Opacity == 0 and wind.ForegroundColor.B == 0 and age.ForegroundColor.B == 0,
    "Red night mode blacks out the photo and keeps the forecast labels red")
check(ui.cards()[1] == "Cardtahoe:card:tonight" and #ui.cards() <= 3,
    "The hub opens on tonight's cards: " .. table.concat(ui.cards(), " "))
check(store:load(RedMode.FILE).lat == Places[1].lat, "The last place is kept for the next launch")
ui.open(Places[1].id, "stargazing")
check(ui.app.hubPage.StateClasses == "stargazing night", "The hub follows the dark")
check(Stargazing.stylesheet() == RedMode.NIGHT, "New pages open in the red sheet")
check(ui.has("Card" .. Places[1].id .. ":stargazing:card:tonight") and ui.has("Card" .. Places[1].id .. ":stargazing:card:moon")
    and ui.has("Card" .. Places[1].id .. ":stargazing:card:darkness"), "The activity page shows all of tonight's cards")
ui.back()
ui.press("OpenSettings")
Settings.set("stargazing", "redmode", "off")
ui.app.activityScope:member("Settings"):refresh { force = true }
ui.settle()
check(ui.app.hubPage.StateClasses == "stargazing" and not ui.find("SettingsPage").StateClasses:find("night"),
    "Turning red mode off takes effect at once")
check(photo.Opacity > 0 and photo.Opacity < 1 and wind.ForegroundColor.B > 0 and age.ForegroundColor.B > 0,
    "Turning red mode off restores the sky photo and the daytime forecast colours")
Settings.set("stargazing", "redmode", "auto")
ui.back("SettingsPivot")
ui.app.hubScope:dispose()

F.checkEdition("stargazing")
F.done("weather-stargazing")
