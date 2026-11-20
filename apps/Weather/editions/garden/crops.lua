-- A small table of common crops and the soil warmth they need at sowing
-- depth (about 5 cm), in °C, as a daily mean. Rules of thumb from
-- university extension and RHS sowing tables: seed sown colder than this
-- sits and rots more often than it grows. Not a plant database: the user
-- never enters a crop. Ordered from the hardiest.
return {
    sowing = {
        { name = "peas", detail = "peas and broad beans", min = 5 },
        { name = "lettuce", detail = "lettuce, spinach and radish", min = 5 },
        { name = "carrots", detail = "carrots and parsnips", min = 7 },
        { name = "beetroot", detail = "beetroot and chard", min = 8 },
        { name = "beans", detail = "french and runner beans", min = 12 },
        { name = "sweetcorn", detail = "sweetcorn", min = 12 },
        { name = "squash", detail = "courgettes, squash and pumpkins", min = 15 },
    },
    -- Lawns, at about 10 cm: crabgrass seed sprouts once the soil holds
    -- about 13 °C (55 °F) for several days, so pre-emergent goes down just
    -- before; cool-season grass seed takes between about 10 and 21 °C.
    lawn = { preEmergent = 13, preEmergentDays = 3, seedMin = 10, seedMax = 21 },
    -- Tender plants go out once nights stay above this (and frost-free).
    plantOutNight = 10,
}
