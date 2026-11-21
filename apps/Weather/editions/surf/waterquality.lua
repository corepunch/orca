-- Official bathing-water status (SV-SU4), where a country publishes one,
-- before the app's own rain rule (SU8). One small adapter per country,
-- each an optional source whose `url` is nil unless the break names that
-- country's bathing-water ID in breaks.lua (`bathingWater = { ie = … }`).
-- Each keeps its list by bathing-water ID (`forecast.officialWater[country]`;
-- a source's parse does not know the spot), and `WaterQuality.status`
-- picks the break's: { status, cause, ends, source }, status "advice
-- against bathing", "risk" or "none listed". "none listed" is not "safe":
-- the card says only that no restriction is listed, and names who lists them.
--
-- Built: Ireland's EPA (CC BY 4.0). Its alert list was not fetched from
-- here (no network to data.epa.ie), so the reader accepts the documented
-- list either bare or under `alerts`/`items`/`data`, and a few spellings
-- of each field; verify against /bw/swagger/v1/swagger.json before a
-- break is given an Irish ID. England's EA pollution-risk forecast and
-- NSW Beachwatch are links only until their endpoints and terms are
-- confirmed (PLAN.md, Surfers' services).
local Breaks = require "Weather/editions/surf/breaks"
local WaterQuality = {}

local function first(t, names)
    for _, name in ipairs(names) do
        if t[name] ~= nil and t[name] ~= "" then return t[name] end
    end
end

local function list(json)
    if type(json) ~= "table" then return nil end
    if json[1] ~= nil or next(json) == nil then return json end
    for _, key in ipairs { "alerts", "items", "data", "results" } do
        if type(json[key]) == "table" then return json[key] end
    end
end

WaterQuality.ireland = {
    id = "surf-water-ie", optional = true, BASE = "https://data.epa.ie/bw/api/v1/alerts",
    NAME = "EPA Ireland",
    url = function(spot)
        local codes = Breaks.of(spot).bathingWater
        if not (codes and codes.ie) then return nil end
        return WaterQuality.ireland.BASE
    end,
    parse = function(json, forecast)
        local alerts = assert(list(json), "no bathing-water alert list")
        local name, by = WaterQuality.ireland.NAME, {}
        for _, alert in ipairs(alerts) do
            local code = type(alert) == "table" and first(alert, { "bathingWaterCode", "locationCode", "code", "bwCode" })
            if code then
                local kind = tostring(first(alert, { "alertType", "type", "restrictionType" }) or ""):lower()
                local against = kind:find("prohibit") or kind:find("advice")
                local status = { status = against and "advice against bathing" or "risk",
                    cause = first(alert, { "reason", "cause", "description", "title" }),
                    ends = first(alert, { "endDate", "validTo", "expires" }), source = name }
                if not (by[code] and by[code].status == "advice against bathing") then by[code] = status end
            end
        end
        forecast.officialWater = forecast.officialWater or {}
        forecast.officialWater.ie = { source = name, by = by }
    end,
}

WaterQuality.sources = { WaterQuality.ireland }

-- The break's official status, or nil where no country's list was read.
function WaterQuality.status(forecast)
    local codes = Breaks.of(forecast.spot).bathingWater
    for country, code in pairs(codes or {}) do
        local read = forecast.officialWater and forecast.officialWater[country]
        if read then return read.by[code] or { status = "none listed", source = read.source } end
    end
end

-- The official status as the water card, or nil without one.
function WaterQuality.card(forecast)
    local official = WaterQuality.status(forecast)
    if not official then return nil end
    if official.status == "none listed" then
        return { id = "water", title = "water quality", value = "no restriction listed", tone = "info", hub = false,
            detail = ("%s lists no bathing restriction here today; still stay out for 72 h after heavy rain")
                :format(official.source) }
    end
    local cause = official.cause and tostring(official.cause):lower() .. " · " or ""
    local ends = official.ends and ("until %s · "):format(tostring(official.ends):sub(1, 10)) or ""
    return { id = "water", title = "water quality", value = official.status, tone = "poor",
        detail = cause .. ends .. "official, from " .. official.source }
end

WaterQuality.attribution = { { "Bathing water: EPA Ireland (CC BY 4.0)",
    "Bathing-water alerts from data.epa.ie, Environmental Protection Agency, under CC BY 4.0." } }

return WaterQuality
