-- What the user reads: every number the app shows goes through here. The
-- rules stay metric inside (°C, m/s, metres, mm, hPa, km); only the text is
-- converted. `Units.use(choice)` picks the units (Settings does this from
-- the stored choice; `Units.defaults` gives the first one).
--
--   Units.temp(12)          "12°" or "54°"
--   Units.wind(5)           "5 m/s", "18 km/h", "11 mph", "10 kn" or "3 Bft"
--   Units.gust(5, 7.2)      "18 g 26 km/h"
--   Units.height(1.5)       "1.5 m" or "4.9 ft"
--   Units.rain(2)           "2.0 mm" or "0.08 in"
--   Units.pressure(1013)    "1013 hPa" or "29.91 inHg"
--   Units.distance(12)      "12 km" or "7 mi"
--   Units.clock("06:00")    "06:00" or "6:00 am" (also takes "2026-11-18T06:00")
--   Units.label("wind")     "m/s": the unit alone, for chart titles
--   Units.value("wind", 5)  5, 18, 11.2…: the converted number, for charts
local Units = {}

-- The choices for each kind, in the order the settings page offers them.
Units.CHOICES = {
    temp = { { "c", "°C" }, { "f", "°F" } },
    wind = { { "ms", "m/s" }, { "kmh", "km/h" }, { "mph", "mph" }, { "kn", "knots" }, { "bft", "Beaufort" } },
    height = { { "m", "metres" }, { "ft", "feet" } },
    rain = { { "mm", "mm" }, { "in", "inches" } },
    pressure = { { "hpa", "hPa" }, { "inhg", "inHg" } },
    distance = { { "km", "km" }, { "mi", "miles" } },
    clock = { { "24", "24-hour" }, { "12", "12-hour" } },
}

local METRIC = { temp = "c", wind = "ms", height = "m", rain = "mm", pressure = "hpa", distance = "km", clock = "24" }
local current = {}
for kind, unit in pairs(METRIC) do current[kind] = unit end

-- Regions that read the imperial units, and the UK's mix of both.
local US = { US = true, LR = true, MM = true, PR = true, GU = true, VI = true, AS = true, MP = true }
local UK = { GB = true }

-- The region code ("US") from a language tag or locale ("en-US",
-- "en_US.UTF-8"), or nil.
function Units.regionOf(tag)
    if type(tag) ~= "string" then return nil end
    tag = tag:gsub("[%.@].*$", "")
    for part in tag:gmatch("[%-_]([%a%d]+)") do
        if #part == 2 and part:match("^%a%a$") then return part:upper() end
    end
    return nil
end

local function core()
    local ok, module = pcall(require, "orca.core")
    return ok and type(module) == "table" and module or nil
end

-- The device's region: ORCA_REGION, the device's own region setting
-- (`core.getRegion`, which iOS and macOS keep apart from the language), the
-- preferred language's region ("en-US"), then LC_ALL, LC_MEASUREMENT or
-- LANG. nil when none says.
function Units.region()
    local override = os.getenv("ORCA_REGION")
    if override and override ~= "" then return override:upper() end
    local engine = core()
    local own = engine and engine.getRegion and engine.getRegion()
    if type(own) == "string" and own:match("^%a%a$") then return own:upper() end
    local region = Units.regionOf(engine and engine.getPreferredLanguage and engine.getPreferredLanguage())
    if region then return region end
    for _, name in ipairs { "LC_ALL", "LC_MEASUREMENT", "LANG" } do
        region = Units.regionOf(os.getenv(name))
        if region then return region end
    end
    return nil
end

-- What the device itself reads: `{ measurement = "metric" | "us" | "uk",
-- clock24 = true | false }`, or nil when the engine cannot say (older
-- builds) or ORCA_REGION decides.
function Units.device()
    local override = os.getenv("ORCA_REGION")
    if override and override ~= "" then return nil end
    local engine = core()
    if not (engine and engine.getMeasurementSystem and engine.uses24HourClock) then return nil end
    local measurement = engine.getMeasurementSystem()
    if measurement ~= "metric" and measurement ~= "us" and measurement ~= "uk" then return nil end
    return { measurement = measurement, clock24 = engine.uses24HourClock() == true }
end

-- The first choice: metric, °F, mph, feet, inches, inHg and miles in the
-- US, mph and miles in the UK, and knots for wind sports whatever the region.
-- `device` (Units.device()) is what the phone reads, and wins over the
-- region: a US phone set to metric stays metric, a 24-hour switch shows 24 h.
function Units.defaults(edition, region, device)
    local result = {}
    for kind, unit in pairs(METRIC) do result[kind] = unit end
    local system = device and device.measurement or (US[region] and "us") or (UK[region] and "uk") or "metric"
    if system == "us" then
        result.temp, result.wind, result.height, result.rain, result.pressure, result.distance, result.clock =
            "f", "mph", "ft", "in", "inhg", "mi", "12"
    elseif system == "uk" then
        result.wind, result.distance = "mph", "mi"
    end
    if device and device.clock24 ~= nil then result.clock = device.clock24 and "24" or "12" end
    if edition == "sailing" then result.wind = "kn" end
    return result
end

function Units.use(choice)
    for kind, unit in pairs(METRIC) do
        local wanted = choice and choice[kind]
        local valid = false
        for _, option in ipairs(Units.CHOICES[kind]) do valid = valid or option[1] == wanted end
        current[kind] = valid and wanted or unit
    end
end
function Units.current(kind) return current[kind] end

-- Beaufort force for a wind in m/s.
local BEAUFORT = { 0.5, 1.6, 3.4, 5.5, 8.0, 10.8, 13.9, 17.2, 20.8, 24.5, 28.5, 32.7 }
local function beaufort(ms)
    for force, limit in ipairs(BEAUFORT) do
        if ms < limit then return force - 1 end
    end
    return 12
end

local CONVERT = {
    temp = { c = function(v) return v end, f = function(v) return v * 9 / 5 + 32 end },
    wind = { ms = function(v) return v end, kmh = function(v) return v * 3.6 end, mph = function(v) return v * 2.236936 end,
        kn = function(v) return v * 1.943844 end, bft = beaufort },
    height = { m = function(v) return v end, ft = function(v) return v * 3.28084 end },
    rain = { mm = function(v) return v end, ["in"] = function(v) return v / 25.4 end },
    pressure = { hpa = function(v) return v end, inhg = function(v) return v * 0.02953 end },
    distance = { km = function(v) return v end, mi = function(v) return v * 0.621371 end },
}
local LABELS = {
    temp = { c = "°C", f = "°F" }, wind = { ms = "m/s", kmh = "km/h", mph = "mph", kn = "kn", bft = "Bft" },
    height = { m = "m", ft = "ft" }, rain = { mm = "mm", ["in"] = "in" }, pressure = { hpa = "hPa", inhg = "inHg" },
    distance = { km = "km", mi = "mi" }, percent = {}, clock = { ["24"] = "", ["12"] = "" },
}

-- The converted number for a kind ("wind"); unknown kinds pass through.
function Units.value(kind, value)
    if value == nil then return nil end
    local convert = CONVERT[kind] and CONVERT[kind][current[kind]]
    return convert and convert(value) or value
end

-- The unit alone ("km/h"), or "%" for percentages; "" for unknown kinds.
function Units.label(kind)
    if kind == "percent" then return "%" end
    local labels = LABELS[kind]
    return labels and labels[current[kind]] or ""
end

local function number(value, decimals)
    if decimals and decimals > 0 then return ("%." .. decimals .. "f"):format(value) end
    return tostring(math.floor(value + 0.5))
end

-- A value with its unit, or a dash when it is missing.
local function shown(kind, value, decimals, space)
    if value == nil then return "—" end
    return number(Units.value(kind, value), decimals) .. (space == false and "" or " ") .. Units.label(kind)
end

function Units.temp(c) return c == nil and "—" or number(Units.value("temp", c)) .. "°" end
-- A temperature difference ("4°"): no offset, only the scale.
function Units.tempDelta(dc) return dc == nil and "—" or number(current.temp == "f" and dc * 9 / 5 or dc) .. "°" end
function Units.wind(ms, decimals) return shown("wind", ms, current.wind ~= "bft" and decimals or nil) end
function Units.gust(ms, gust)
    if ms == nil then return "—" end
    if gust == nil then return Units.wind(ms) end
    return number(Units.value("wind", ms)) .. " g " .. number(Units.value("wind", gust)) .. " " .. Units.label("wind")
end
-- Waves and swell under 10 m keep a decimal; heights of hills do not.
function Units.height(m, decimals)
    if m == nil then return "—" end
    if decimals == nil then decimals = math.abs(Units.value("height", m)) < 10 and 1 or 0 end
    return shown("height", m, decimals)
end
function Units.rain(mm, decimals) return shown("rain", mm, decimals or (current.rain == "in" and 2 or 1)) end
-- inHg is about 34 times hPa, so it takes two more decimals.
function Units.pressure(hpa, decimals)
    return shown("pressure", hpa, (decimals or 0) + (current.pressure == "inhg" and 2 or 0))
end
function Units.distance(km) return shown("distance", km, km and Units.value("distance", km) < 10 and 1 or nil) end

-- "06:00" or "2026-11-18T06:00" as the clock reads it: "06:00" or "6:00 am".
function Units.clock(time)
    local h, m = (time or ""):match("(%d%d):(%d%d)")
    if not h then return "—" end
    if current.clock ~= "12" then return h .. ":" .. m end
    h = tonumber(h)
    return ("%d:%s %s"):format((h + 11) % 12 + 1, m, h < 12 and "am" or "pm")
end

-- An hour span ("06–08h", or "6–8 am" and "11 am–1 pm" on a 12-hour clock).
function Units.span(from, to)
    if from == to then return Units.clock(from) end
    local a, b = tonumber((from or ""):match("(%d%d):%d%d")), tonumber((to or ""):match("(%d%d):%d%d"))
    if not (a and b) then return "—" end
    if current.clock ~= "12" then return ("%02d–%02dh"):format(a, b) end
    local function twelve(h) return (h + 11) % 12 + 1 end
    local function half(h) return h < 12 and "am" or "pm" end
    if half(a) == half(b) then return ("%d–%d %s"):format(twelve(a), twelve(b), half(b)) end
    return ("%d %s–%d %s"):format(twelve(a), half(a), twelve(b), half(b))
end

return Units
