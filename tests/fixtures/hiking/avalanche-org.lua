-- Synthetic answer in the shape of avalanche.org's public map layer
-- (GET api.avalanche.org/v2/public/products/map-layer/SAC, GeoJSON), cut to
-- two zones: one around Mount Tallac with a current rating, one to the
-- south-east whose rating has expired. Not a recorded answer: the API's
-- terms are not settled (Research/services-hiking.md), so nothing real is
-- kept here.
return {
    type = "FeatureCollection",
    features = {
        { type = "Feature", id = 1,
            properties = { name = "Central Sierra Nevada", center = "Sierra Avalanche Center", center_id = "SAC",
                center_link = "https://www.sierraavalanchecenter.org/",
                link = "https://www.sierraavalanchecenter.org/forecasts/#/central-sierra-nevada",
                off_season = false, travel_advice = "Heightened avalanche conditions on wind-loaded slopes.",
                danger = "considerable", danger_level = 3, color = "#f7941e",
                start_date = "2026-11-17T19:00:00-08:00", end_date = "2026-11-18T10:00:00-08:00" },
            geometry = { type = "Polygon", coordinates = { {
                { -120.40, 38.60 }, { -119.90, 38.60 }, { -119.90, 39.30 }, { -120.40, 39.30 }, { -120.40, 38.60 },
            } } } },
        { type = "Feature", id = 2,
            properties = { name = "Mono", center = "Sierra Avalanche Center", center_id = "SAC",
                link = "https://www.sierraavalanchecenter.org/forecasts/#/mono",
                off_season = false, danger = "high", danger_level = 4,
                start_date = "2026-11-16T19:00:00-08:00", end_date = "2026-11-17T18:00:00-08:00" },
            geometry = { type = "MultiPolygon", coordinates = { { {
                { -119.40, 37.60 }, { -118.90, 37.60 }, { -118.90, 38.30 }, { -119.40, 38.30 }, { -119.40, 37.60 },
            } } } } },
    },
}
