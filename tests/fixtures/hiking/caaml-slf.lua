-- Synthetic EAWS CAAML v6 JSON in the shape SLF publishes (one bulletin
-- per group of micro-regions), cut to two bulletins. The region ids are
-- made up for the test (CH-TEST-*): the real ones come from
-- regions.avalanches.org. Two elevation bands: considerable above 2200 m,
-- moderate below.
return {
    bulletins = {
        { bulletinID = "test-1", lang = "en",
            publicationTime = "2026-11-17T16:00:00Z", nextUpdate = "2026-11-18T07:00:00Z",
            validTime = { startTime = "2026-11-17T16:00:00Z", endTime = "2026-11-18T16:00:00Z" },
            regions = { { regionID = "CH-TEST-1", name = "Zermatt test region" } },
            dangerRatings = {
                { mainValue = "considerable", elevation = { lowerBound = "2200" }, validTimePeriod = "all_day" },
                { mainValue = "moderate", elevation = { upperBound = "2200" }, validTimePeriod = "all_day" },
            },
            avalancheProblems = {
                { problemType = "wind_slab", elevation = { lowerBound = "2200" }, aspects = { "N", "NE", "E" },
                    validTimePeriod = "all_day" },
                { problemType = "persistent_weak_layers", elevation = { lowerBound = "2600" }, aspects = { "NW", "N" },
                    validTimePeriod = "all_day" },
                { problemType = "gliding_snow", elevation = { upperBound = "2000" }, aspects = { "S" },
                    validTimePeriod = "all_day" },
            } },
        { bulletinID = "test-2", lang = "en",
            publicationTime = "2026-11-17T16:00:00Z",
            validTime = { startTime = "2026-11-17T16:00:00Z", endTime = "2026-11-18T16:00:00Z" },
            regions = { { regionID = "CH-TEST-2", name = "Elsewhere" } },
            dangerRatings = { { mainValue = "low" } } },
    },
}
