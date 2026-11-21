-- The home-screen and lock-screen widget's snapshot (N3). Each build (the
-- full app and every edition) writes one small JSON file into its App Group
-- whenever the home place's fresh forecast is drawn, and after a background
-- refresh; the WidgetKit extension (Widget/WeatherWidget.swift) only reads
-- it. Nothing else is shared: the place's name, the temperature, the lead
-- activity's verdict now and one line.
--
--   Widget.snapshot(forecast, place, now)  the table written
--   Widget.encode(snapshot)                its JSON
--   Widget.write(forecast, place, options) hands it to options.notifier
--                                          (orca.notifications), if it can
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Report = require "Weather/report"
local Activities = require "Weather/activities"
local Widget = { VERSION = 1 }

-- The lead activity's verdict for the current hour and, for today, its
-- best window ("fishing best 06–08h"); without one, the sky in words.
function Widget.snapshot(forecast, place, now)
    local hour = forecast and forecast.hours and forecast.hours[forecast.now or 1]
    if not (hour and place) then return nil end
    local lead = Activities.lead()
    local ok, rated = pcall(Report.rate, lead, hour, forecast)
    if not ok then rated = nil end
    local found, best, span = pcall(Report.bestOfDay, lead, forecast, hour.date)
    local sky = Sky.describe(hour.code)
    return {
        version = Widget.VERSION,
        app = Activities.appName,
        edition = Activities.edition or "",
        place = place.title or "",
        temp = Units.temp(hour.temp),
        sky = sky,
        activity = lead.title,
        verdict = rated and rated.word or "",
        tone = rated and rated.verdict or "info",
        line = found and best and span and ("%s best %s"):format(lead.title, span) or sky,
        updated = math.floor(now or os.time()),
    }
end

local function quote(text)
    return '"' .. tostring(text):gsub('[%c"\\]', function(c)
        local named = { ['"'] = '\\"', ["\\"] = "\\\\", ["\n"] = "\\n", ["\r"] = "\\r", ["\t"] = "\\t" }
        return named[c] or ("\\u%04x"):format(c:byte())
    end) .. '"'
end

-- Flat tables of strings and numbers, keys sorted so equal snapshots match.
function Widget.encode(snapshot)
    local keys, parts = {}, {}
    for key in pairs(snapshot) do keys[#keys + 1] = key end
    table.sort(keys)
    for _, key in ipairs(keys) do
        local value = snapshot[key]
        parts[#parts + 1] = quote(key) .. ":" .. (type(value) == "number" and ("%.10g"):format(value) or quote(value))
    end
    return "{" .. table.concat(parts, ",") .. "}"
end

-- Writes the snapshot through `options.notifier.writeWidgetSnapshot`. Builds
-- without an App Group (no WIDGETS=1, or not provisioned) say why; that is
-- not an error for the app. Returns the JSON written, or nil and why.
function Widget.write(forecast, place, options)
    options = options or {}
    local notifier = options.notifier
    if not (notifier and notifier.writeWidgetSnapshot) then return nil, "no widget" end
    local snapshot = Widget.snapshot(forecast, place, options.now)
    if not snapshot then return nil, "no forecast" end
    local json = Widget.encode(snapshot)
    local ok, written, err = pcall(notifier.writeWidgetSnapshot, json)
    if not (ok and written) then return nil, ok and err or written end
    return json
end

return Widget
