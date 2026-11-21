-- Sailors' facts per spot (SA8), keyed by place id, kept here because
-- places.lua is shared. `safe` is the wind directions (where the wind comes
-- FROM, degrees) a rider can use there, clockwise from `from` to `to`;
-- outside it the wind blows off the beach and a fallen kiter drifts away
-- from help. `note` is what a local would add; `links` are pages to check
-- by hand (Weather/editions/sailing/links.lua).
--
-- These are estimates from each beach's shore line, not local rules: check
-- with the spot's school or club. A spot missing here gets a sector from its
-- `facing` (Spots.sector).
local Sky = require "Weather/sky"
local Spots = {}

Spots.byId = {
    crissy = { safe = { from = 240, to = 60 },
        note = "the summer westerly is cross-shore; south winds blow off the city, gusty and offshore" },
    islamorada = { safe = { from = 45, to = 225 }, note = "the easterly trades blow along the shore" },
    cabarete = { safe = { from = 280, to = 100 }, note = "the east-northeast trade is cross-onshore" },
    tarifa = { safe = { from = 95, to = 325 },
        note = "the Poniente (west) is cross-onshore; a strong Levante (east) runs cross-offshore: experts only" },
    spo = { safe = { from = 180, to = 360 }, note = "a wide beach; at low tide the water is far out" },
    bodensee = { safe = { from = 90, to = 270 }, note = "a north shore: a north wind carries you across the lake",
        links = { { "OpenWindMap stations", "openwindmap.org" } } },
    garda = { safe = { from = 135, to = 225 },
        note = "the afternoon Ora blows up the lake from the south; kites launch by boat at the north end",
        links = { { "OpenWindMap stations", "openwindmap.org" } } },
    mamry = { safe = { from = 180, to = 360 }, note = "an east shore" },
}

-- How far either side of straight onshore a derived sector reaches: onshore,
-- cross-onshore and cross-shore, but not cross-offshore.
local HALF = 100

-- The safe sector for a place: the one the user set in the place editor
-- (`safeFrom`, `safeTo`), else the table's, else one around its shore's
-- facing, else nil (nothing known).
function Spots.sector(place)
    local known = place and Spots.byId[place.id]
    local base, note
    if known and known.safe then base, note = known.safe, known.note
    elseif place and place.facing then
        base = { from = (place.facing - HALF) % 360, to = (place.facing + HALF) % 360, derived = true }
    end
    if place and (place.safeFrom or place.safeTo) then
        return { from = place.safeFrom or (base and base.from) or 0, to = place.safeTo or (base and base.to) or 0,
            own = true }, note
    end
    return base, note
end

-- Whether a wind from `direction` falls in a sector (clockwise from..to).
function Spots.inside(sector, direction)
    if not (sector and direction) then return true end
    local span = (sector.to - sector.from) % 360
    if span == 0 then return true end
    return (direction - sector.from) % 360 <= span
end

-- "S–N": the sector as compass points.
function Spots.text(sector)
    if not sector then return "" end
    return Sky.compass(sector.from) .. "–" .. Sky.compass(sector.to % 360)
end

return Spots
