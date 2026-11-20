-- Red night mode: red type on black from dusk to dawn, so eyes that took
-- twenty minutes to adapt to the dark stay adapted. Every screen is born
-- in the right sheet (the activity's `stylesheet` asks RedMode), so a page
-- that opens at night never flashes white; the app dresses the screens
-- already open (`pageClasses`, a "night" class) whenever a page is drawn:
-- the half-hourly poll at the latest, and the settings after a change.
-- "auto" switches at civil dusk and dawn (the sun 6° down) at the place
-- last shown; "on" and "off" fix it.
local Sky = require "Weather/sky"
local Settings = require "Weather/settings"
local Storage = require "Weather/storage"
local RedMode = {
    DAY = "Weather/editions/stargazing/style.css",
    NIGHT = "Weather/editions/stargazing/night.css",
    DUSK = Sky.TWILIGHT.civil,
    -- The last place shown, so the first screen at launch is already right.
    FILE = "stargazing-place",
}

local state = {}

local function savedSpot()
    local ok, spot = pcall(Storage.load, RedMode.FILE)
    return ok and type(spot) == "table" and tonumber(spot.lat) and tonumber(spot.lon) and spot or nil
end

-- The time the forecast was made, moved on by the time since: the app's
-- own clock (tests fix it), not only the device's.
function RedMode.now()
    if state.epoch then return state.epoch + math.max(0, os.time() - state.seen) end
    return os.time()
end

-- Whether red mode is on now: the setting, else the sun at the last place.
function RedMode.on(epoch)
    local mode = Settings.get("stargazing", "redmode")
    if mode == "on" then return true end
    if mode == "off" then return false end
    local spot = state.spot or savedSpot()
    if not spot then return false end
    return Sky.sunAltitude(epoch or RedMode.now(), spot.lat, spot.lon) < RedMode.DUSK
end

function RedMode.stylesheet() return RedMode.on() and RedMode.NIGHT or RedMode.DAY end

-- A page's state classes with "night" added or taken away.
function RedMode.classes(classes, on)
    local list = {}
    for name in (classes or ""):gmatch("%S+") do if name ~= "night" then list[#list + 1] = name end end
    if on then list[#list + 1] = "night" end
    return table.concat(list, " ")
end

-- A forecast was drawn for a place: remember where and when, then follow.
function RedMode.follow(forecast)
    local spot = forecast and forecast.spot
    if not (spot and spot.lat and spot.lon and forecast.epoch) then return end
    local known = state.spot or savedSpot()
    if not known or known.lat ~= spot.lat or known.lon ~= spot.lon then
        pcall(Storage.save, RedMode.FILE, { lat = spot.lat, lon = spot.lon })
    end
    state.spot, state.epoch, state.seen = { lat = spot.lat, lon = spot.lon }, forecast.epoch, os.time()
end

-- Forgets the place (tests).
function RedMode.reset() state = {} end

return RedMode
