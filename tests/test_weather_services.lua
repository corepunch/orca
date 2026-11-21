-- Weather's existing-service integrations (apps/Weather/services): the
-- official-warning registry (SV-W1) with its coverage states, each provider
-- (NWS, Environment Canada, MET Norway, DWD via Bright Sky) on a synthetic
-- answer in its documented shape, the cards everyday outdoors makes from
-- them, the DWD radar line (SV-W5) and the credits on settings' "about".
-- No live network: every answer comes from F.network().answers.
local F = dofile("tests/weather_fixture.lua").init()
local check, NOW = F.check, F.NOW
local Forecast = require "Weather/forecast"
local Units = require "Weather/units"
Units.use {}
local Util = require "Weather/services/util"
local Warnings = require "Weather/services/warnings"
local Radar = require "Weather/services/radar"
local Activities = require "Weather/activities"
local Outdoors = Activities.byId.outdoors
local Provider = require "Weather/provider"

local HOUR = 3600
local function iso(epoch) return os.date("!%Y-%m-%dT%H:%M:%SZ", epoch) end
local function forecastAt(spot)
    local forecast = Forecast.parse(F.weather(spot.lat), nil, NOW, spot)
    forecast.spot = spot
    return forecast
end

-- Times ----------------------------------------------------------------------
check(Util.isoEpoch("2026-11-18T14:00:00-08:00") == NOW + 16 * HOUR and Util.isoEpoch("2026-11-18T06:00:00Z") == NOW
    and Util.isoEpoch("2026-11-18T07:00:00+0100") == NOW and Util.isoEpoch("2026-11-18T06:00:30.000Z") == NOW + 30
    and Util.isoEpoch("2026-11-18T06:00") == nil and Util.isoEpoch(nil) == nil, "Time stamps with every kind of zone")

-- Coverage: covered, link or none ---------------------------------------------
local TAHOE = { lat = 39.09, lon = -120.04 }
local HALIFAX = { lat = 44.65, lon = -63.57 }
local TORONTO = { lat = 43.65, lon = -79.38, country = "CA" }
local OSLO = { lat = 59.91, lon = 10.75, country = "NO" }
local BERLIN = { id = "berlin", title = "Berlin", region = "Germany", lat = 52.52, lon = 13.4, country = "DE", activities = {} }
local PARIS = { lat = 48.86, lon = 2.35, country = "FR" }
local LONDON = { id = "london", title = "London", region = "England", lat = 51.51, lon = -0.13, activities = {} }
local SYDNEY = { lat = -33.87, lon = 151.21 }
local NAIROBI = { lat = -1.29, lon = 36.82, country = "KE" }
local function covers(spot)
    local state, entry = Warnings.coverage(spot)
    return state .. ":" .. (entry and entry.id or "-")
end
check(covers(TAHOE) == "covered:nws" and covers(HALIFAX) == "covered:eccc" and covers(TORONTO) == "covered:eccc"
    and covers(OSLO) == "covered:metno" and covers(BERLIN) == "covered:dwd", "Five services are read")
check(covers(PARIS) == "link:meteoalarm" and covers(LONDON) == "link:metoffice" and covers(SYDNEY) == "link:bom"
    and covers({ lat = 35.68, lon = 139.7 }) == "link:jma", "Elsewhere a link to the official warnings")
check(covers({ lat = 59.91, lon = 10.75 }) == "link:meteoalarm", "Without its country, Norway is not guessed from a box")
check(covers(NAIROBI) == "none:wmo" and covers({}) == "none:-", "No service known: the WMO's list, or nothing at all")

-- Each provider asks only where coverage picks it.
local nws, eccc, metno, dwd = Warnings.source("nws"), Warnings.source("eccc"), Warnings.source("metno"), Warnings.source("dwd")
check(nws == Outdoors.alertSource and nws.url(TAHOE):find("^https://api%.weather%.gov/alerts/active%?point=39%.0900")
    and nws.url(BERLIN) == nil, "The NWS for US places only")
check(eccc.url(TORONTO):find("^https://api%.weather%.gc%.ca/collections/weather%-alerts/items%?f=json&bbox=%-79%.430,43%.600")
    and eccc.url(TAHOE) == nil, "Environment Canada around Canadian places")
check(metno.url(OSLO):find("^https://api%.met%.no/weatherapi/metalerts/2%.0/current%.json%?lat=59%.9100&lon=10%.7500")
    and metno.url(PARIS) == nil, "MET Norway by point in Norway")
check(dwd.url(BERLIN) == "https://api.brightsky.dev/alerts?lat=52.5200&lon=13.4000" and dwd.url(PARIS) == nil,
    "DWD via Bright Sky in Germany")
local ids = {}
for _, source in ipairs(Outdoors.sources) do ids[#ids + 1] = source.id; check(source.optional, source.id .. " is optional") end
check(table.concat(ids, ",") == "outdoors-air,warnings-nws,warnings-eccc,warnings-metno,warnings-dwd,radar-dwd",
    "The full app's sources: air, warnings and radar")

-- An answer without its list fails the source, never "no warnings".
for _, source in ipairs { nws, eccc, metno } do
    check(not pcall(source.parse, { title = "busy" }, forecastAt(TAHOE)), source.id .. ": no list, no answer")
end
check(not pcall(dwd.parse, { error = "busy" }, forecastAt(BERLIN)), "warnings-dwd: no list, no answer")

-- Environment Canada: the polygon decides; ended alerts go. -------------------
local function square(lon, lat, size)
    return { type = "Polygon", coordinates = { { { lon - size, lat - size }, { lon + size, lat - size },
        { lon + size, lat + size }, { lon - size, lat + size }, { lon - size, lat - size } } } }
end
local ECCC = { type = "FeatureCollection", features = {
    { geometry = square(-79.38, 43.65, 0.3), properties = { alert_name_en = "Snowfall Warning",
        alert_name_fr = "Avertissement de neige", alert_type = "warning", status_en = "issued",
        publication_datetime = iso(NOW - HOUR), event_end_datetime = iso(NOW + 18 * HOUR) } },
    -- the same alert for a second area that also holds the spot
    { geometry = { type = "MultiPolygon", coordinates = { square(-79.4, 43.6, 0.5).coordinates } },
        properties = { alert_name_en = "Snowfall Warning", alert_type = "warning", status_en = "issued",
            publication_datetime = iso(NOW - HOUR), event_end_datetime = iso(NOW + 18 * HOUR) } },
    { geometry = square(-78.5, 44.2, 0.2), properties = { alert_name_en = "Wind Warning", alert_type = "warning",
        status_en = "issued", event_end_datetime = iso(NOW + 6 * HOUR) } },
    { geometry = square(-79.38, 43.65, 0.3), properties = { alert_name_en = "Fog Advisory", alert_type = "advisory",
        status_en = "ended", event_end_datetime = iso(NOW + 2 * HOUR) } },
    { geometry = square(-79.38, 43.65, 0.3), properties = { alert_name_en = "Special Weather Statement",
        alert_type = "statement", status_en = "issued", expiration_datetime = iso(NOW + 30 * HOUR) } },
} }
local toronto = forecastAt(TORONTO)
eccc.parse(ECCC, toronto)
check(toronto.warningsFrom == "eccc" and #toronto.warnings == 4, "Every live Canadian alert is read")
local cards = Outdoors.warningCards(toronto, NOW, TORONTO)
check(#cards == 2 and cards[1].value == "snowfall warning" and cards[1].tone == "poor"
    and cards[1].detail == "until thu 19 00:00 · issued wed 18 05:00 · Environment Canada"
    and cards[2].value == "special weather statement" and cards[2].tone == "fair",
    "Only the alerts whose area holds the place, once each, with the issuer and issue time")
local saved = Util.language
Util.language = function() return "fr" end
eccc.parse(ECCC, toronto)
check(toronto.warnings[1].event == "Avertissement de neige" and toronto.warnings[4].event == "Special Weather Statement",
    "French readers get the French name where there is one")
Util.language = saved

-- MET Norway ---------------------------------------------------------------------
local oslo = forecastAt(OSLO)
metno.parse({ type = "FeatureCollection", features = {
    { when = { interval = { iso(NOW - HOUR), iso(NOW + 20 * HOUR) } }, properties = { eventAwarenessName = "Gale",
        awareness_level = "3; orange; Severe", web = "https://www.met.no/en/weather-and-climate/warnings" } },
    { when = { interval = { iso(NOW + HOUR), iso(NOW + 8 * HOUR) } }, properties = { title = "Ice, yellow level",
        severity = "Moderate" } },
    { when = { interval = { iso(NOW - 9 * HOUR), iso(NOW - HOUR) } }, properties = { eventAwarenessName = "Rain",
        severity = "Moderate" } },
} }, oslo)
cards = Outdoors.warningCards(oslo, NOW, OSLO)
check(#cards == 2 and cards[1].value == "gale" and cards[1].tone == "poor" and cards[1].detail == "until thu 19 02:00 · MET Norway"
    and cards[2].value == "ice, yellow level" and cards[2].tone == "fair", "Norway's warnings by level; expired ones go")

-- DWD through Bright Sky -----------------------------------------------------------
local DWD = { alerts = {
    { status = "actual", severity = "moderate", event_en = "frost", event_de = "FROST",
        effective = "2026-11-18T05:30:00+00:00", expires = "2026-11-18T10:00:00+00:00" },
    { status = "actual", severity = "severe", event_en = "heavy snowfall", headline_en = "Official WARNING of HEAVY SNOWFALL",
        effective = "2026-11-18T04:00:00+00:00", expires = "2026-11-19T06:00:00+00:00" },
    { status = "test", severity = "extreme", event_en = "test" },
}, location = { warn_cell_id = 111000000, name = "Berlin" } }
local berlin = forecastAt(BERLIN)
dwd.parse(DWD, berlin)
cards = Outdoors.warningCards(berlin, NOW, BERLIN)
check(#cards == 2 and cards[1].value == "heavy snowfall" and cards[1].tone == "poor"
    and cards[1].detail == "until thu 19 06:00 · issued wed 18 04:00 · Deutscher Wetterdienst"
    and cards[2].value == "frost" and cards[2].tone == "fair", "German warnings from DWD; test messages skipped")
dwd.parse({ alerts = {}, location = { name = "Berlin" } }, berlin)
check(#Outdoors.warningCards(berlin, NOW, BERLIN) == 0, "A quiet answer adds nothing: no \"no warnings\" card")

-- Where nothing is read, a calm card -------------------------------------------------
local london = forecastAt(LONDON)
cards = Outdoors.warningCards(london, NOW, LONDON)
check(#cards == 1 and cards[1].id == "warnings" and cards[1].value == "see the Met Office"
    and cards[1].detail:find("metoffice.gov.uk", 1, true) and cards[1].link:find("^https://") and cards[1].tone == "info",
    "In the UK: see the Met Office")
check(Outdoors.warningCards(forecastAt(PARIS), NOW, PARIS)[1].value == "see MeteoAlarm", "In France: see MeteoAlarm")
local nairobi = Outdoors.warningCards(forecastAt(NAIROBI), NOW, NAIROBI)
check(nairobi[1].value == "not checked here" and nairobi[1].hub == false and nairobi[1].detail:find("severeweather.wmo.int", 1, true),
    "No known service: say so on the activity page only")
local failed = forecastAt(TAHOE)
failed.missing["warnings-nws"] = "The forecast service is not answering."
cards = Outdoors.warningCards(failed, NOW, TAHOE)
check(cards[1].value == "not available right now" and cards[1].detail:find("weather.gov", 1, true),
    "A service that did not answer says so")
check(#Outdoors.warningCards(forecastAt(TAHOE), NOW, TAHOE) == 0, "Not asked yet: nothing")
check(#Warnings.cards(forecastAt(LONDON), NOW) == 0, "The note is the caller's choice (editions keep their own)")
local answers = Outdoors.cards(london, LONDON, NOW)
check(answers[#answers].id == "warnings", "The link comes after the other answers")

-- Radar: where rain is now around a German place (SV-W5) ----------------------------
-- A 31 x 31 km grid, the place at its centre; `wet` lists { row, col }.
local function frame(at, wet)
    local rows = {}
    for r = 1, 31 do
        rows[r] = {}
        for c = 1, 31 do rows[r][c] = 0 end
    end
    for _, cell in ipairs(wet) do rows[cell[1]][cell[2]] = 40 end
    return { timestamp = iso(at), source = "RADOLAN::RV", precipitation_5 = rows }
end
local function radar(frames) return { radar = frames, bbox = { 100, 200, 130, 230 }, latlon_position = { x = 215, y = 115 } } end
check(Radar.source.url(BERLIN) == "https://api.brightsky.dev/radar?lat=52.5200&lon=13.4000&distance=15000&format=plain"
    and Radar.source.url(LONDON) == nil and Radar.source.url(TAHOE) == nil, "Radar only where DWD's composite reaches")
local rainy = forecastAt(BERLIN)
Radar.source.parse(radar { frame(NOW - 30 * 60, { { 16, 28 } }), frame(NOW - 5 * 60, { { 16, 24 } }),
    frame(NOW + 30 * 60, { { 16, 16 } }) }, rainy)
check(Radar.line(rainy, NOW) == "radar: rain about 8 km away, moving closer", "Rain 8 km off and closing in")
check(Radar.line(rainy, NOW + 2 * HOUR) == nil, "An old frame says nothing")
local dry = forecastAt(BERLIN)
Radar.source.parse(radar { frame(NOW - 10 * 60, {}) }, dry)
check(Radar.line(dry, NOW) == "radar: dry within 15 km", "Dry around the place")
local wet = forecastAt(BERLIN)
Radar.source.parse(radar { frame(NOW - 10 * 60, { { 16, 16 } }) }, wet)
check(Radar.line(wet, NOW) == "radar: rain here now", "Rain over the place")
check(not pcall(Radar.source.parse, { error = "busy" }, dry), "No frames, no line")
check(Radar.line(forecastAt(BERLIN), NOW) == nil, "Without radar, nothing")
rainy.quarters = {}
for i = 1, 8 do rainy.quarters[i] = { epoch = NOW + i * 900, precipitation = 0, snowfall = 0 } end
local nextCard
for _, card in ipairs(Outdoors.cards(rainy, BERLIN, NOW)) do if card.id == "next-hour" then nextCard = card end end
check(nextCard.value == "dry for the next 2 hours"
    and nextCard.detail == "no rain in the next two hours · radar: rain about 8 km away, moving closer",
    "The radar line sits beside the model's next two hours, never as minutes")

-- On the hub: Berlin with its country asks DWD and radar --------------------------------
local net = F.network()
net.answers["brightsky%.dev/alerts"] = function() return DWD end
net.answers["brightsky%.dev/radar"] = function()
    return radar { frame(NOW - 30 * 60, { { 16, 28 } }), frame(NOW - 5 * 60, { { 16, 24 } }) }
end
local ui = F.launch { places = { BERLIN, LONDON }, net = net, storageName = "services" }
local asked = table.concat(net.requests, "\n")
check(asked:find("api.brightsky.dev/alerts?lat=52.5200", 1, true) and asked:find("api.brightsky.dev/radar?lat=52.5200", 1, true)
    and not asked:find("weather.gov", 1, true), "Berlin asks DWD and its radar, not the NWS")
check(ui.text("Answerberlin:answer:warning-1", "AnswerValue") == "heavy snowfall"
    and ui.text("Answerberlin:answer:warning-2", "AnswerValue") == "frost", "The hub leads with Germany's warnings")
-- The synthetic week has no quarter hours, so the radar line stands alone.
check(ui.text("Answerberlin:answer:next-hour", "AnswerTitle") == "rain nearby"
    and ui.text("Answerberlin:answer:next-hour", "AnswerValue") == "rain about 8 km away, moving closer",
    "and where rain is now, from the radar")
ui.choose("london")
check(ui.text("Answerlondon:answer:warnings", "AnswerValue") == "see the Met Office", "London points at the Met Office")
ui.app.hubScope:dispose()

-- Credit ---------------------------------------------------------------------------------
local about = {}
for _, line in ipairs(Provider.about()) do about[line[1]] = line[2] end
check(about["Warnings in Canada by Environment and Climate Change Canada"]:find("Environment and Climate Change Canada")
    and about["Warnings in Norway by MET Norway"]:find("CC BY 4.0")
    and about["Warnings in Germany by Deutscher Wetterdienst, via Bright Sky"]
    and about["Warnings in the US by the National Weather Service"]
    and about["Radar: Deutscher Wetterdienst via Bright Sky"], "Settings' about credits every service")

F.done("weather-services")
