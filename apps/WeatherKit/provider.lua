-- The data provider every weather app shares: the profile's spots and one
-- report per spot (Data/Forecast.xml). Options inject `fetch(url)` and
-- `clock()` so tests run on recorded forecasts.
local Forecast = require "WeatherKit/forecast"
local Report = require "WeatherKit/report"
local Provider = {}

local function kind(name) return function(record) return record.values.Kind == name end end

function Provider.new(profile, options)
    options = options or {}
    local spotsById = {}
    for _, spot in ipairs(profile.spots) do spotsById[spot.id] = spot end
    return {
        profile = profile,
        computed = { scoreText = function(values) return ("%d"):format(math.floor(values.Score or 0)) end },
        filters = { nowRows = kind("now"), reasonRows = kind("reason"), hourRows = kind("hour"),
            dayRows = kind("day"), metricRows = kind("metric") },
        operations = {
            spots = function() return profile.spots end,
            report = function(params)
                local spot = spotsById[params.SpotId]
                if not spot then return nil, { code = "no_spot", message = "Choose a spot first." } end
                local clock = options.clock or os.time
                local forecast, err = Forecast.load(spot, profile.needsMarine, options.fetch, clock())
                if not forecast then return nil, { code = "network", message = err } end
                forecast.spot = spot
                return Report.rows(profile, spot, forecast)
            end,
        },
    }
end

return Provider
