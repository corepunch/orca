-- Sailors' Weather: is there a steady, usable wind for this rider at this
-- spot? The sailing activity's rules on hand-built hours, its extra
-- requests (models, tides, nearby spots) on recorded answers, then the
-- sailing edition running on its own places (tests/weather_fixture.lua).
-- No network: every answer is built here. This file belongs to the sailing
-- edition.
local F = dofile("tests/weather_fixture.lua").init { edition = "sailing" }
local check, hour, verdict, says = F.check, F.hour, F.verdict, F.says
local Settings = require "Weather/settings"
local Units = require "Weather/units"
local Forecast = require "Weather/forecast"
local Sailing = require("Weather/activities").byId.sailing
local Profile, Spots, Models, Tides, Rank = Sailing.Profile, Sailing.Spots, Sailing.Models, Sailing.Tides, Sailing.Rank
local KN = Profile.KN

-- Knots and a fresh profile (kite, intermediate, 12–28 kn, 75 kg, 9 / 12).
local function fresh() Settings.init { edition = "sailing", region = "NL" } end
fresh()

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

-- SA1: knots first, gusts as "18 g 26", in every reason a rider reads.
check(Units.current("wind") == "kn", "Sailors' Weather starts in knots")
check(Sailing.note(hour { wind = 9.26, gust = 10.3, windDir = 0 }, { spot = shore }) == "18 g 20 kn · cross-shore N · 9 m",
    "An hour reads wind g gusts in knots, the shore and the kite to rig")
check(says(verdict(Sailing, { wind = 9.26, gust = 10.3, windDir = 0 }, shore), "^18 kn: in your 12–28 kn range"),
    "The wind band is told in knots")
for _, metric in ipairs(Sailing.metrics(hour { wind = 9.26, gust = 13.4, windDir = 0 }, { spot = shore })) do
    check(not tostring(metric.value):find("m/s") and not tostring(metric.note or ""):find("m/s"),
        "No metric falls back to m/s: " .. metric.label)
end

-- SA2: the rider profile sets the band and the gear.
local rider = Profile.current()
check(rider.sport == "kite" and rider.level == "intermediate" and math.abs(rider.min - 12 * KN) < 1e-6
    and math.abs(rider.max - 28 * KN) < 1e-6 and rider.weight == 75, "The default rider is an intermediate kiter")
check(Profile.rig(rider, 18 * KN) == "the 9 m", "75 kg in 18 kn rigs the 9 m (75 × 2.2 ÷ 18 ≈ 9)")
check(Profile.rig(rider, 13 * KN) == "the 12 m", "13 kn rigs the 12 m")
local _, why, fit = Profile.rig(rider, 30 * KN)
check(fit == "over" and why:find("smallest") and why:find("rule of thumb"), "30 kn on a 9 / 12 quiver is a handful")
local function keys()
    local list = {}
    for _, setting in ipairs(Sailing.settings) do list[setting.key] = setting end
    return list
end
check(keys().kites and keys().weight and keys()["min-kite"] and keys().range, "A kiter sets weight, kites and range")
Settings.set("sailing", "kites", "7 / 9 / 12")
check(Profile.rig(Profile.current(), 25 * KN) == "the 7 m", "A bigger quiver rigs the 7 m in 25 kn")
Settings.set("sailing", "level", "beginner")
check(verdict(Sailing, { wind = 12, gust = 13, windDir = 0 }, shore).score <= 25,
    "23 kn is too strong for a learner kiter (12–20 kn)")
Settings.set("sailing", "level", "expert")
check(verdict(Sailing, { wind = 12, gust = 13, windDir = 0 }, shore).verdict == "good", "and good for an expert")
check(verdict(Sailing, { wind = 18, gust = 20, windDir = 0 }, shore).score <= 20, "A gale caps even an expert")
Settings.set("sailing", "min-kite", 14)
Settings.set("sailing", "sport", "dinghy")
check(not keys().kites and not keys().weight and keys()["min-dinghy"], "A dinghy sailor sees no kites or weight")
check(math.abs(Profile.current().max - 24 * KN) < 1e-6 and Profile.current().board == false,
    "An expert dinghy sailor's band is 6–24 kn")
local offshore = verdict(Sailing, { wind = 5, gust = 6, windDir = 90 }, shore)
check(offshore.score > 30 and says(offshore, "hard to come back"), "Offshore wind is a caution for a dinghy, not a ban")
Settings.set("sailing", "sport", "keelboat")
check(Profile.rig(Profile.current(), 17 * KN) == "one reef", "A keelboat takes one reef in 17 kn")
Settings.set("sailing", "min-keelboat", 10)
check(says(verdict(Sailing, { wind = 3.6, gust = 4, windDir = 0 }, shore), "you need 10 kn"), "The rider's own minimum")
Settings.set("sailing", "sport", "kite")
check(math.abs(Profile.current().min - 14 * KN) < 1e-6, "Each sport keeps its own range")
fresh()

-- SA3: gusts, lulls, squalls, shifts and fog.
local steady = verdict(Sailing, { wind = 8, gust = 9.5, windDir = 0 }, shore)
local squall = verdict(Sailing, { wind = 8, gust = 9.5, windDir = 0, cape = 1800, rainChance = 60 }, shore)
check(squall.score <= 35 and says(squall, "squalls"), "Unstable air with showers caps the hour: squalls")
check(says(verdict(Sailing, { wind = 8, gust = 9.5, windDir = 0, cape = 700 }, shore), "gusts may run higher"),
    "Some instability warns that gusts may run higher")
check(verdict(Sailing, { wind = 8, gust = 9.5, windDir = 0, pressureTrend = -7 }, shore).score <= 25,
    "Pressure falling very fast caps the hour")
check(says(verdict(Sailing, { wind = 7, gust = 11, windDir = 0 }, shore), "Lulls may drop"), "Lulls under the minimum")
check(verdict(Sailing, { wind = 8, gust = 9.5, windDir = 0, visibility = 600 }, shore).score <= 30, "Fog caps the hour")
local turning = { hours = {
    hour { time = "2026-11-18T10:00", wind = 8, gust = 9.5, windDir = 0 },
    hour { time = "2026-11-18T11:00", wind = 8, gust = 9.5, windDir = 30 },
    hour { time = "2026-11-18T12:00", wind = 8, gust = 9.5, windDir = 80 },
}, spot = shore }
local turned = Sailing.rate(turning.hours[1], turning):result()
check(turned.score < steady.score and says(turned, "turns from N to E"), "A 80° shift within three hours is flagged")
local squallWhy
for _, metric in ipairs(Sailing.metrics(hour { wind = 8, gust = 10, cape = 900 }, { spot = shore })) do
    if metric.label == "gusts" then squallWhy = metric.note end
end
check(squallWhy and squallWhy:find("could reach") and squallWhy:find("estimate"), "Gusts 'could reach' in unstable air")

-- SA8: a safe sector per spot, kept in the edition's files.
local crissy = { id = "crissy", facing = 0 }
local sector = Spots.sector(crissy)
check(sector.from == 240 and Spots.inside(sector, 270) and Spots.inside(sector, 10) and not Spots.inside(sector, 150),
    "Crissy Field is safe from WSW through north")
local outside = verdict(Sailing, { wind = 8, gust = 9.5, windDir = 110 }, crissy)
check(outside.score <= 35 and says(outside, "outside this spot's safe"), "A kiter is kept out of an unsafe sector")
local derived = Spots.sector { facing = 270 }
check(derived.derived and Spots.inside(derived, 270) and not Spots.inside(derived, 90), "A sector from the shore's facing")
for id in pairs(Spots.byId) do
    local found = false
    for _, place in ipairs(require "Weather/places") do found = found or place.id == id end
    check(found, "Spot data names a sailing place: " .. id)
end

-- SA4: model agreement from one multi-model request.
local function multi(models)
    local series, n = F.series, 7 * 24
    local hourly = { time = series(n, F.iso) }
    for id, wind in pairs(models) do
        hourly["wind_speed_10m_" .. id] = series(n, function() return wind end)
        hourly["wind_gusts_10m_" .. id] = series(n, function() return wind * 1.2 end)
        hourly["wind_direction_10m_" .. id] = series(n, function() return 0 end)
    end
    return { hourly = hourly }
end
local spotSF = { id = "crissy", lat = 37.81, lon = -122.46, facing = 0 }
local url = Models.source.url(spotSF, Forecast)
check(url:find("models=") and url:find("gfs_seamless") and url:find("gem_seamless") and not url:find("hrrr")
    and not url:find("nam_"), "Models are asked for by family, never a retiring model")
check(not Models.source.url({ lat = 36, lon = -5.6 }, Forecast):find("gem_"), "Regional models only where they cover")
local function parsed(extra)
    local f = Forecast.parse(F.weather(37.81, extra), F.marine(), F.NOW, spotSF)
    f.spot = spotSF
    return f
end
local f = parsed()
Models.source.parse(multi { ecmwf_ifs025 = 8, gfs_seamless = 8.5, icon_seamless = 7.6, mystery_model = 8.2 }, f)
local now = f.hours[f.now]
check(now.agreement.word == "sure" and #now.models == 4, "Close models are sure, any model the answer has counts")
Models.source.parse(multi { ecmwf_ifs025 = 6, gfs_seamless = 9, icon_seamless = 7 }, f)
check(f.hours[f.now].agreement.word == "likely", "6 kn apart is likely")
Models.source.parse(multi { ecmwf_ifs025 = 4, gfs_seamless = 11, icon_seamless = 7 }, f)
local unsure = f.hours[f.now]
check(unsure.agreement.word == "uncertain" and Sailing.note(unsure, f):find("uncertain$"), "14 kn apart is uncertain")
check(says(Sailing.rate(unsure, f):result(), "anywhere from 8–21 kn"), "Disagreement is told as a range")
check(Sailing.strip(unsure.agreement) == "ECMWF 8 · GFS 21 · ICON 14 kn", "The model strip")

-- SA5: a sea-breeze hint on a sunny day with warm land and a light morning.
local sunny = parsed {
    cloud_cover = function() return 10 end,
    temperature_2m = function(i) return (i % 24 >= 11 and i % 24 <= 16) and 21 or 14 end,
    wind_speed_10m = function(i) return (i % 24 >= 14) and 7 or 2.5 end,
    wind_direction_10m = function(i) return (i % 24 >= 14) and 10 or 180 end,
}
local hint = Sailing.seaBreeze(sunny, "2026-11-18")
check(hint and hint.word == "likely" and hint.from == "2026-11-18T14:00", "A sea breeze is likely from 14:00")
check(not Sailing.seaBreeze(parsed(), "2026-11-18"), "No sea breeze on a cloudy, cool day")
check(not Sailing.seaBreeze((function() local l = parsed(); l.spot = { lake = true, facing = 0 }; return l end)(),
    "2026-11-18"), "No sea breeze on a lake")

-- SA6: an approximate tide from the marine model.
local function tideAnswer()
    local series, n = F.series, 7 * 24
    return { hourly_units = { ocean_current_velocity = "km/h" }, hourly = {
        time = series(n, F.iso),
        sea_level_height_msl = series(n, function(i) return math.cos(2 * math.pi * (i - 9) / 12.4) end),
        ocean_current_velocity = series(n, function() return 3.6 end),
        ocean_current_direction = series(n, function() return 0 end),
    } }
end
check(Tides.source.url(spotSF, Forecast):find("sea_level_height_msl")
    and Tides.source.url({ lat = 47.6, lon = 9.4, lake = true }, Forecast) == nil, "Tides are asked for at sea, not on lakes")
local tidal = parsed { wind_speed_10m = function() return 8 end, wind_direction_10m = function() return 10 end }
Tides.source.parse(tideAnswer(), tidal)
local turns = Tides.turns(tidal, "2026-11-18")
local highs = {}
for _, t in ipairs(turns) do if t.kind == "high" then highs[#highs + 1] = t.time end end
check(#turns >= 3 and highs[1] == "2026-11-18T09:00", "High water about 09:00, from the turns of the sea level")
check(math.abs(tidal.hours[tidal.now].current - 1) < 1e-6, "The stream is read in its answer's units")
check(Tides.windAgainst(tidal.hours[tidal.now]) and says(Sailing.rate(tidal.hours[tidal.now], tidal):result(),
    "Wind against tide"), "A north wind against a north-going stream: chop")
local tideCard
for _, card in ipairs(Sailing.cards(tidal, spotSF)) do if card.id == "tide" then tideCard = card end end
check(tideCard and tideCard.title:find("approximate") and tideCard.detail:find("not for navigation")
    and tideCard.value == "high about 09:00", "The tide card says high water and that it is approximate")

-- SA7: rank the spots within range for the weekend.
local garda = { id = "garda", title = "Lake Garda", lat = 45.87, lon = 10.87, lake = true, facing = 180, activities = { "sailing" } }
local constance = { id = "bodensee", title = "Lake Constance", lat = 47.60, lon = 9.40, lake = true, facing = 180,
    activities = { "sailing" } }
local spo = { id = "spo", title = "Sankt Peter-Ording", lat = 54.30, lon = 8.60, facing = 270, activities = { "sailing" } }
Rank.places = { garda, constance, spo }
local near = Rank.near(constance, 300)
check(#near == 1 and near[1].place.id == "garda", "Within 300 km of Lake Constance: Lake Garda")
local spotsUrl = Rank.source(function() return 900 end).url(constance)
check(spotsUrl:find("latitude=45.87,54.3") and spotsUrl:find("longitude=10.87,8.6"), "One request for every nearby spot")
local home = Forecast.parse(F.weather(47.6), nil, F.NOW, constance)
home.spot = constance
local function windy(lat, ms, dir)
    local answer = F.weather(lat, { wind_speed_10m = function() return ms end, wind_gusts_10m = function() return ms * 1.15 end,
        wind_direction_10m = function() return dir end })
    answer.longitude = lat == 45.87 and 10.87 or 8.6
    return answer
end
Rank.source(function() return 900 end).parse({ windy(45.87, 8, 180), windy(54.3, 4, 270) }, home)
check(#home.spots == 2 and home.spots[1].place.id == "garda", "Each answer is matched to its spot")
check(#Rank.dates(home) == 2, "A Wednesday compares Saturday and Sunday")
local weekend
for _, card in ipairs(Sailing.cards(home, constance)) do if card.id == "weekend" then weekend = card end end
check(weekend and weekend.value == "Lake Garda" and weekend.tone == "good" and weekend.detail:find("^good sat"),
    "Lake Garda wins the weekend")
Rank.places = nil

-- Wave 3: a safe sector the rider sets comes before the table, and the
-- weekend compares my saved spots (Weather/compare.lua).
do
    local MyPlaces = require "Weather/myplaces"
    local Places = require "Weather/places"
    MyPlaces.init { curated = Places }
    local Compare = require "Weather/compare"
    check(#Compare.places("sailing") == #Places, "Before any is saved, the weekend compares the suggested spots")
    MyPlaces.add("bodensee")
    local draft = MyPlaces.fromResult({ id = 2878044, name = "Lindau", latitude = 47.55, longitude = 9.68, feature_code = "LK",
        country_code = "DE", country = "Germany", admin1 = "Bavaria" })
    local lindau = MyPlaces.resolveDraft(draft)
    check(lindau.lake and table.concat(lindau.activities, ",") == "sailing" and Spots.sector(lindau) == nil,
        "A found lake is for sailing; no shore, no sector yet")
    MyPlaces.apply(draft, "facing", 180)
    MyPlaces.apply(draft, "safeFrom", 135)
    local id = MyPlaces.add(draft)
    local sector = Spots.sector(MyPlaces.find(id))
    check(sector.own and sector.from == 135 and sector.to == 280, "The rider's start of the sector, the shore's end")
    MyPlaces.set("bodensee", "safeTo", 300)
    local constance = Spots.sector(MyPlaces.find("bodensee"))
    check(constance.from == 90 and constance.to == 300 and select(2, Spots.sector(MyPlaces.find("bodensee"))):find("north shore"),
        "A curated spot keeps its table under the rider's change")
    local near = Rank.near(MyPlaces.find(id), 300)
    check(#near == 1 and near[1].place.id == "bodensee", "The weekend compares my saved spots only")
    MyPlaces.init { curated = Places }
end

-- The cards: what to rig, any danger, and how sure.
local storm = parsed { weather_code = function(i) return i == 9 and 95 or 2 end,
    wind_speed_10m = function() return 9.26 end, wind_gusts_10m = function() return 10 end,
    wind_direction_10m = function() return 270 end }
Models.source.parse(multi { ecmwf_ifs025 = 9, gfs_seamless = 9.5, icon_seamless = 9.2 }, storm)
local byId = {}
for _, card in ipairs(Sailing.cards(storm, spotSF)) do byId[card.id] = card end
check(byId.rig and byId.rig.value == "the 9 m" and byId.rig.detail:find("rule of thumb"), "The rig card: the 9 m")
check(byId.hazard and byId.hazard.value == "thunder from 09:00" and byId.hazard.tone == "poor", "Thunder ahead is a card")
check(byId.models and byId.models.value == "sure", "How sure the forecast is")

-- The edition on its own places, with every extra request answered.
local net = F.network()
net.answers["models="] = function() return multi { ecmwf_ifs025 = 3, gfs_seamless = 3.2, icon_seamless = 2.9 } end
net.answers["sea_level_height_msl"] = function() return tideAnswer() end
net.answers["latitude=[%-%d%.]+,"] = function(u)
    local list = {}
    for lat in u:match("latitude=([^&]+)"):gmatch("[^,]+") do list[#list + 1] = windy(tonumber(lat), 3, 0) end
    return list
end
F.checkEdition("sailing")
local Places = require "Weather/places"
local ui = F.launch { places = Places, storageName = "sailing-cards", net = net }
local place = Places[1]
check(ui.has("Card" .. place.id .. ":card:rig"), "The hub leads with what to rig")
ui.open(place.id, "sailing")
check(ui.has("Card" .. place.id .. ":sailing:card:rig") and ui.has("Card" .. place.id .. ":sailing:card:models"),
    "The activity's now view shows the rig and the models")
local asked = {}
for _, u in ipairs(net.requests) do
    asked.models = asked.models or u:find("models=") ~= nil
    asked.tides = asked.tides or u:find("sea_level_height_msl") ~= nil
end
check(asked.models and asked.tides, "The edition asks for the models and the tide")
ui.back()
-- The profile on the settings page follows the sport.
ui.press("OpenSettings")
local function setting(id) return "Settingsettings:sailing:" .. id end
check(ui.text(setting("sport"), "SettingValue") == "kitesurf" and ui.text(setting("min-kite"), "SettingValue") == "12 kn"
    and ui.has(setting("kites")) and ui.has(setting("weight")), "A kiter's profile lists the kites")
ui.find(setting("sport")):findChild("SettingChange", true):send("Node.LeftButtonUp")
ui.settle()
check(ui.text(setting("sport"), "SettingValue") == "windsurf" and ui.has(setting("sails")) and not ui.has(setting("kites"))
    and ui.has(setting("min-windsurf")), "Switching to windsurf lists the sails instead")
ui.back("SettingsPivot")
ui.app.hubScope:dispose()
fresh()
F.done("weather-sailing")
