-- Stargazers' Weather: will the night be dark and clear? The stargazing
-- activity's rules on hand-built hours, its astronomy against published
-- times, tonight's cards on a synthetic forecast, red night mode, then the
-- stargazing edition running on its own places (tests/weather_fixture.lua).
-- This file belongs to the stargazing edition.
local F = dofile("tests/weather_fixture.lua").init { edition = "stargazing" }
local check, verdict, says = F.check, F.verdict, F.says
local Stargazing = require "Weather/activities/stargazing"
local Moon = require "Weather/editions/stargazing/moon"
local Night = require "Weather/editions/stargazing/night"
local Seeing = require "Weather/editions/stargazing/seeing"
local Sources = require "Weather/editions/stargazing/sources"
local Aurora = require "Weather/editions/stargazing/aurora"
local RedMode = require "Weather/editions/stargazing/redmode"
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

-- ST6: red night mode.
check(RedMode.classes("stargazing", true) == "stargazing night" and RedMode.classes("stargazing night", false) == "stargazing",
    "Night is a state class beside the edition's")
Settings.set("stargazing", "redmode", "on")
check(RedMode.on() and Stargazing.stylesheet == RedMode.NIGHT, "Red mode on")
Settings.set("stargazing", "redmode", "off")
check(not RedMode.on() and Stargazing.stylesheet == RedMode.DAY, "Red mode off")
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
store:remove(RedMode.FILE)
RedMode.reset()
local net = F.network()
net.answers["latitude=39%.09&longitude=%-120%.04"] = function()
    local answer = F.weather(39.09)
    answer.utc_offset_seconds, answer.longitude = -8 * 3600, -120.04
    return answer
end
local ui = F.launch { places = Places, storage = store, net = net, clock = function() return F.NOW + 8 * 3600 end }
check(ui.cards()[1] == "Cardtahoe:card:tonight" and #ui.cards() <= 3,
    "The hub opens on tonight's cards: " .. table.concat(ui.cards(), " "))
check(store:load(RedMode.FILE).lat == Places[1].lat, "The last place is kept for the next launch")
ui.open(Places[1].id, "stargazing")
check(ui.app.hubPage.StateClasses == "stargazing night", "The hub follows the dark")
check(Stargazing.stylesheet == RedMode.NIGHT, "New pages open in the red sheet")
check(ui.has("Card" .. Places[1].id .. ":stargazing:card:tonight") and ui.has("Card" .. Places[1].id .. ":stargazing:card:moon")
    and ui.has("Card" .. Places[1].id .. ":stargazing:card:darkness"), "The activity page shows all of tonight's cards")
ui.back()
ui.press("OpenSettings")
Settings.set("stargazing", "redmode", "off")
ui.app.activityScope:member("Settings"):refresh { force = true }
ui.settle()
check(ui.app.hubPage.StateClasses == "stargazing" and not ui.find("SettingsPage").StateClasses:find("night"),
    "Turning red mode off takes effect at once")
Settings.set("stargazing", "redmode", "auto")
ui.back("SettingsPivot")
ui.app.hubScope:dispose()

F.checkEdition("stargazing")
F.done("weather-stargazing")
