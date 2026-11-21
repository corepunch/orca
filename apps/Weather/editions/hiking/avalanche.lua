-- Avalanche danger for the hiking edition (SV-HI1, SV-HI2), pure: reading
-- avalanche.org's map layer (US centres, GeoJSON) and EAWS CAAML v6 JSON
-- bulletins (SLF, Euregio), and wording the card.
--
-- Every provider asks the same three things: the danger keeps its official
-- meaning (the level and its own word, never a score of ours), a rating
-- expires with its forecast, and the card links the full forecast. An
-- expired, missing or unknown rating says "no current rating" and never
-- shows the last level; nothing here ever says "safe".
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Sources = require "Weather/editions/hiking/sources"
local Avalanche = {}

-- The five-level scale, North American and EAWS words: level 5 is
-- "extreme" in North America and "very high" in Europe.
Avalanche.WORDS = { us = { "low", "moderate", "considerable", "high", "extreme" },
    eaws = { "low", "moderate", "considerable", "high", "very high" } }
local CAAML = { low = 1, moderate = 2, considerable = 3, high = 4, very_high = 5 }
Avalanche.TONES = { "good", "fair", "poor", "poor", "poor" }

-- ISO times with an offset (or "Z", or none: read as UTC) → UTC epoch.
Avalanche.epoch = Sources.epoch

-- Ray casting: is the point inside one ring ({ { lon, lat }, … })?
local function inRing(lon, lat, ring)
    local inside, j = false, #ring
    for i = 1, #ring do
        local xi, yi, xj, yj = ring[i][1], ring[i][2], ring[j][1], ring[j][2]
        if (yi > lat) ~= (yj > lat) and lon < (xj - xi) * (lat - yi) / (yj - yi) + xi then inside = not inside end
        j = i
    end
    return inside
end

local function inPolygon(lon, lat, rings)
    if not (rings and rings[1] and inRing(lon, lat, rings[1])) then return false end
    for index = 2, #rings do
        if inRing(lon, lat, rings[index]) then return false end -- a hole
    end
    return true
end

-- Is `point` ({ lat, lon }) inside a GeoJSON Polygon or MultiPolygon?
function Avalanche.inside(point, geometry)
    if not (point and geometry and geometry.coordinates) then return false end
    if geometry.type == "Polygon" then return inPolygon(point.lon, point.lat, geometry.coordinates) end
    if geometry.type == "MultiPolygon" then
        for _, polygon in ipairs(geometry.coordinates) do
            if inPolygon(point.lon, point.lat, polygon) then return true end
        end
    end
    return false
end

-- avalanche.org's map layer for one centre: the zone whose polygon holds
-- the top, else the zone named in peaks.lua. Keeps the level and its word,
-- the times, the zone's forecast link and the travel advice.
-- A level below 1 (-1 "no rating") or an off-season zone has no level.
function Avalanche.mapLayer(json, peak)
    local features = assert(type(json) == "table" and type(json.features) == "table" and json.features, "no zones")
    local want = peak.avalanche or {}
    local found
    for _, feature in ipairs(features) do
        if Avalanche.inside(peak.top, feature.geometry) then found = feature; break end
    end
    if not found and want.zone then
        for _, feature in ipairs(features) do
            if (feature.properties or {}).name == want.zone then found = feature; break end
        end
    end
    if not found then return { kind = "us", centre = want[1], link = want[2], none = "zone" } end
    local p = found.properties or {}
    local level = tonumber(p.danger_level)
    if not level or level < 1 or level > 5 or p.off_season then level = nil end
    return { kind = "us", level = level, zone = p.name, centre = p.center or want[1],
        link = p.link or p.center_link or want[2], from = Avalanche.epoch(p.start_date), to = Avalanche.epoch(p.end_date),
        advice = p.travel_advice }
end

-- A CAAML elevation bound: metres, "treeline" (the peak's own treeline
-- when peaks.lua gives one) or nil.
local function bound(value, treeline)
    if value == nil then return nil end
    if tostring(value):lower() == "treeline" then return treeline or "treeline" end
    return tonumber(value)
end

-- Does an elevation band hold `elevation`? true, false, or "unsure" when
-- a bound is the treeline and its height is not known.
local function holds(band, elevation, treeline)
    local low, high = bound(band and band.lowerBound, treeline), bound(band and band.upperBound, treeline)
    if low == "treeline" or high == "treeline" then return "unsure" end
    return (low == nil or elevation >= low) and (high == nil or elevation < high)
end

local function bandText(band, treeline)
    local low, high = bound(band and band.lowerBound, treeline), bound(band and band.upperBound, treeline)
    local function text(b) return b == "treeline" and "the treeline" or Units.height(b, 0) end
    if low and high then return "between " .. text(low) .. " and " .. text(high) end
    if low then return "above " .. text(low) end
    if high then return "below " .. text(high) end
    return nil
end

local function period(item) return item.validTimePeriod or "all_day" end

-- The highest rating for one time of day that covers the top: certain
-- bands first, then (treeline height unknown) the higher of the unsure.
local function ratingAt(ratings, elevation, treeline, when)
    local best, unsure
    for _, rating in ipairs(ratings) do
        local p = period(rating)
        local level = CAAML[rating.mainValue]
        if level and (p == "all_day" or p == when) then
            local fit = holds(rating.elevation, elevation, treeline)
            if fit == true and (not best or level > best.level) then best = { level = level, rating = rating } end
            if fit == "unsure" and (not unsure or level > unsure.level) then unsure = { level = level, rating = rating } end
        end
    end
    local pick = best or unsure
    if not pick then return nil end
    return { level = pick.level, band = bandText(pick.rating.elevation, treeline), unsure = pick == unsure or nil }
end

local ASPECTS = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" }

-- Aspects as a hiker reads them: a run of three or more as "N–E", else a list.
function Avalanche.aspects(list)
    local on, count = {}, 0
    for _, aspect in ipairs(list or {}) do
        for index, name in ipairs(ASPECTS) do
            if name == tostring(aspect):upper() and not on[index] then on[index], count = true, count + 1 end
        end
    end
    if count == 0 then return nil end
    if count == 8 then return "all aspects" end
    -- Start just after a gap, so a run through the north reads "NW–NE".
    local start = 1
    for index = 1, 8 do
        if on[index] and not on[(index - 2) % 8 + 1] then start = index; break end
    end
    local runs, run = {}, nil
    for step = 0, 7 do
        local index = (start + step - 1) % 8 + 1
        if on[index] then
            if run then run[#run + 1] = ASPECTS[index] else run = { ASPECTS[index] } end
        elseif run then
            runs[#runs + 1], run = run, nil
        end
    end
    runs[#runs + 1] = run
    local names = {}
    for _, r in ipairs(runs) do
        if #r >= 3 then names[#names + 1] = r[1] .. "–" .. r[#r]
        else for _, name in ipairs(r) do names[#names + 1] = name end end
    end
    return table.concat(names, ", ")
end

-- The avalanche problems that reach the top's height at that time of day.
local function problemsAt(problems, elevation, treeline, when)
    local names, aspects, seen = {}, {}, {}
    for _, problem in ipairs(problems or {}) do
        local p = period(problem)
        local kind = problem.problemType
        if kind and kind ~= "favourable_situation" and (p == "all_day" or p == when)
            and holds(problem.elevation, elevation, treeline) ~= false then
            local word = kind:gsub("_", " ")
            if not seen[word] then seen[word], names[#names + 1] = true, word end
            for _, aspect in ipairs(problem.aspects or {}) do aspects[#aspects + 1] = aspect end
        end
    end
    return names, Avalanche.aspects(aspects)
end

-- An EAWS CAAML v6 JSON bulletin set: the bulletin for the peak's
-- micro-region (`avalanche.caaml.region`), with the rating and problems at
-- the top's height for the whole day, the morning ("earlier") and the
-- afternoon ("later").
function Avalanche.caaml(json, peak)
    local bulletins = assert(type(json) == "table" and type(json.bulletins) == "table" and json.bulletins, "no bulletins")
    local want = peak.avalanche or {}
    local config = want.caaml or {}
    local elevation = peak.top.elevation
    for _, bulletin in ipairs(bulletins) do
        for _, region in ipairs(bulletin.regions or {}) do
            if region.regionID == config.region then
                local result = { kind = "eaws", centre = want[1], link = want[2], region = region.name,
                    published = Avalanche.epoch(bulletin.publicationTime), nextUpdate = Avalanche.epoch(bulletin.nextUpdate),
                    from = Avalanche.epoch(bulletin.validTime and bulletin.validTime.startTime),
                    to = Avalanche.epoch(bulletin.validTime and bulletin.validTime.endTime), periods = {} }
                for _, when in ipairs { "all_day", "earlier", "later" } do
                    local rating = ratingAt(bulletin.dangerRatings or {}, elevation, config.treeline, when)
                    if rating then
                        rating.problems, rating.aspects = problemsAt(bulletin.avalancheProblems, elevation, config.treeline, when)
                        result.periods[when] = rating
                    end
                end
                return result
            end
        end
    end
    return { kind = "eaws", centre = want[1], link = want[2], none = "region" }
end

-- "until 18:00", or with the weekday when it is not today.
local function at(epoch, forecast)
    local offset = forecast.offset or 0
    local date, today = os.date("!%Y-%m-%d", epoch + offset), os.date("!%Y-%m-%d", (forecast.epoch or 0) + offset)
    local clock = Units.clock(os.date("!%H:%M", epoch + offset))
    return date == today and clock or Sky.weekday(date) .. " " .. clock
end
Avalanche.at = at

-- The rating in force now, or nil (expired, not yet valid, or none).
-- An am/pm bulletin uses the morning's before local noon, else the afternoon's.
function Avalanche.current(result, forecast)
    if not result or result.none then return nil end
    local now = forecast.epoch or 0
    if not result.to or now >= result.to then return nil end
    if result.from and now < result.from - 12 * 3600 then return nil end
    if result.kind == "us" then
        return result.level and { level = result.level, word = Avalanche.WORDS.us[result.level] } or nil
    end
    local hour = ((now + (forecast.offset or 0)) // 3600) % 24
    local when = hour < 12 and "earlier" or "later"
    local rating = result.periods[when] or result.periods.all_day
    if not rating then return nil end
    local other = result.periods[when == "earlier" and "later" or "earlier"]
    return { level = rating.level, word = Avalanche.WORDS.eaws[rating.level], band = rating.band, unsure = rating.unsure,
        problems = rating.problems, aspects = rating.aspects,
        later = when == "earlier" and other and other.level > rating.level and other or nil }
end

-- The card: the level with its own word and where it applies, the centre,
-- the times and the link. `missing` is the source's error, when it failed.
function Avalanche.card(result, peak, forecast, missing)
    local want = peak.avalanche
    local centre = result and result.centre or want[1]
    local link = result and result.link or want[2]
    local rating = Avalanche.current(result, forecast)
    if not rating then
        local why
        if missing then why = "the bulletin did not load"
        elseif result and result.none == "zone" then why = "the top is outside the centre's zones"
        elseif result and result.none == "region" then why = "no bulletin for this region"
        elseif result and result.to and (forecast.epoch or 0) >= result.to then why = "the last rating expired " .. at(result.to, forecast)
        elseif result then why = "no rating issued" end
        local parts = { centre .. ": check the centre before you go" }
        if why then table.insert(parts, 1, why) end
        parts[#parts + 1] = link
        return { id = "avalanche", title = "avalanche danger", value = "no current rating", tone = "info",
            detail = table.concat(parts, " · "), link = link }
    end
    local value = ("%d %s"):format(rating.level, rating.word)
    local parts = {}
    if result.kind == "us" then
        if result.zone then value = value .. " · " .. result.zone end
        parts[#parts + 1] = centre
        parts[#parts + 1] = "until " .. at(result.to, forecast)
    else
        if rating.band then value = value .. " " .. rating.band end
        if rating.problems and #rating.problems > 0 then
            parts[#parts + 1] = table.concat(rating.problems, ", ") .. (rating.aspects and " " .. rating.aspects or "")
        end
        if rating.unsure then parts[#parts + 1] = "the higher band shown: treeline height not known" end
        if rating.later then
            parts[#parts + 1] = ("rising to %d %s later in the day"):format(rating.later.level,
                Avalanche.WORDS.eaws[rating.later.level])
        end
        parts[#parts + 1] = centre
        if result.published then parts[#parts + 1] = "published " .. at(result.published, forecast) end
        parts[#parts + 1] = result.nextUpdate and "next update " .. at(result.nextUpdate, forecast)
            or "until " .. at(result.to, forecast)
    end
    parts[#parts + 1] = "read the full forecast: " .. link
    return { id = "avalanche", title = "avalanche danger", value = value, tone = Avalanche.TONES[rating.level],
        detail = table.concat(parts, " · "), link = link }
end

return Avalanche
