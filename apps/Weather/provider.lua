-- The data provider: places, the place hub and one activity's report. A
-- place's forecast is fetched once and kept for ten minutes, so opening an
-- activity or going back reuses it. Options inject `fetch(url)` and
-- `clock()` so tests run on recorded forecasts.
local Forecast = require "Weather/forecast"
local Report = require "Weather/report"
local Activities = require "Weather/activities"
local Provider = {}

local CACHE_SECONDS = 10 * 60

local function kind(name) return function(record) return record.values.Kind == name end end

function Provider.new(places, options)
    options = options or {}
    local byId, cache = {}, {}
    for _, place in ipairs(places) do byId[place.id] = place end
    local clock = options.clock or os.time

    -- The place's forecast, from the cache while it is fresh.
    local function forecastFor(placeId, refresh)
        local place = byId[placeId]
        if not place then return nil, { code = "no_place", message = "Choose a place first." } end
        local cached = cache[placeId]
        local now = clock()
        if cached and not refresh and now - cached.at < CACHE_SECONDS then return cached.forecast, nil, place end
        local forecast, err = Forecast.load(place, Activities.needsMarine(place) and "optional", options.fetch, now)
        if not forecast then return nil, { code = "network", message = err } end
        forecast.spot = place
        cache[placeId] = { forecast = forecast, at = now }
        return forecast, nil, place
    end

    local provider = {
        places = places,
        computed = { scoreText = function(values) return ("%d"):format(math.floor(values.Score or 0)) end },
        filters = {
            nowRows = kind("now"), detailRows = kind("detail"), stripRows = kind("strip"), activityRows = kind("activity"),
            reasonRows = kind("reason"), hourRows = kind("hour"), dayRows = kind("day"), metricRows = kind("metric"),
        },
        handlers = {},
        operations = {
            places = function() return places end,
            -- A refresh (pull or poll) refetches; the first load may use the cache.
            hub = function(params, _, _, query)
                local forecast, err, place = forecastFor(params.PlaceId, query and query.state and query.state.HasLoaded)
                if not forecast then return nil, err end
                return Report.hub(place, forecast, Activities.at(place))
            end,
            activity = function(params)
                local activity = Activities.byId[params.ActivityId]
                if not activity then return nil, { code = "no_activity", message = "Unknown activity." } end
                local forecast, err, place = forecastFor(params.PlaceId)
                if not forecast then return nil, err end
                return Report.rows(activity, place, forecast)
            end,
        },
    }
    provider.forecastFor = forecastFor
    return provider
end

return Provider
