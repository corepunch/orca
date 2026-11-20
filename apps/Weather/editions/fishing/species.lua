-- Species profiles (FI9): the target fish changes a few rules. Each is an
-- angler's rule of thumb for water temperature, light and wind, worded as
-- experience, not science. Temperature rules need a water temperature; with
-- none known they say nothing rather than guess from the air.
local Units = require "Weather/units"
local Species = {}

Species.OPTIONS = {
    { "any", "any fish" }, { "bass", "bass" }, { "walleye", "walleye" }, { "pike", "pike" }, { "trout", "trout" },
    { "carp", "carp" }, { "seabass", "sea bass" },
}

local function band(card, water, low, high, points, name)
    if water and water >= low and water <= high then
        card:add(points, ("Water %s: %s anglers find their fish most active from %s to %s."):format(Units.temp(water), name,
            Units.temp(low), Units.temp(high)))
        return true
    end
end

-- `ctx`: { hour, water (°C or nil), coast, wave (m or nil), twilight
-- (minutes), bright (sunny glare) }.
local RULES = {
    bass = function(card, ctx)
        if not band(card, ctx.water, 18, 27, 6, "bass") and ctx.water and ctx.water < 10 then
            card:add(-6, "Bass slow right down in water under " .. Units.temp(10) .. ".")
        end
    end,
    walleye = function(card, ctx)
        band(card, ctx.water, 10, 20, 4, "walleye")
        if ctx.twilight <= 60 then card:add(5, "Low light: walleye anglers' best time.") end
        if ctx.bright then card:add(-4, "Walleye shun bright light: try deeper water.") end
        local wind = ctx.hour.wind or 0
        if wind >= 4 and wind <= 8 then card:add(4, "A \"walleye chop\" on the water: anglers love it.") end
    end,
    pike = function(card, ctx)
        if not band(card, ctx.water, 8, 20, 4, "pike") then
            if ctx.water and ctx.water < 8 then card:add(3, "Pike keep feeding in cold water.")
            elseif ctx.water and ctx.water > 24 then card:add(-6, "Warm water stresses pike: go early, release quickly.") end
        end
    end,
    trout = function(card, ctx)
        if not band(card, ctx.water, 8, 16, 6, "trout") and ctx.water and ctx.water > 20 then
            card:add(-12, "Water over " .. Units.temp(20) .. ": trout are stressed. Many anglers stop fishing for them.")
        end
        if ctx.bright then card:add(-4, "Trout are shy in bright light.") end
    end,
    carp = function(card, ctx)
        if not band(card, ctx.water, 18, 26, 6, "carp") and ctx.water and ctx.water < 10 then
            card:add(-8, "Carp barely feed in water under " .. Units.temp(10) .. ".")
        end
        if not ctx.hour.isDay and ctx.twilight > 120 then card:add(8, "Carp anglers often fish on into the night.") end
    end,
    seabass = function(card, ctx)
        if not ctx.coast then return end
        band(card, ctx.water, 12, 20, 4, "sea bass")
        if ctx.wave and ctx.wave >= 0.5 and ctx.wave <= 1.5 then
            card:add(5, "A bit of white water: sea bass hunt in the surf.")
        elseif ctx.wave and ctx.wave < 0.3 and ctx.bright then
            card:add(-4, "Flat, clear and bright: sea bass stay wary.")
        end
    end,
}

function Species.apply(card, species, ctx)
    local rule = RULES[species]
    if rule then rule(card, ctx) end
end

return Species
