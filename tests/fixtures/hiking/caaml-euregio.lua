-- Synthetic EAWS CAAML v6 JSON in the shape of Euregio's
-- EUREGIO_en_CAAMLv6.json, one bulletin with a morning/afternoon split
-- (`validTimePeriod` "earlier"/"later") and treeline bounds. The region id
-- is made up (IT-32-TN-TEST). Not recorded: Euregio's data licence is open.
return {
    bulletins = {
        { bulletinID = "test-euregio",
            publicationTime = "2026-11-17T16:00:00Z", nextUpdate = "2026-11-18T16:00:00Z",
            validTime = { startTime = "2026-11-17T16:00:00Z", endTime = "2026-11-18T16:00:00Z" },
            regions = { { regionID = "IT-32-TN-TEST", name = "Trentino test region" } },
            dangerRatings = {
                { mainValue = "moderate", elevation = { lowerBound = "treeline" }, validTimePeriod = "earlier" },
                { mainValue = "low", elevation = { upperBound = "treeline" }, validTimePeriod = "earlier" },
                { mainValue = "considerable", elevation = { lowerBound = "treeline" }, validTimePeriod = "later" },
                { mainValue = "moderate", elevation = { upperBound = "treeline" }, validTimePeriod = "later" },
            },
            avalancheProblems = {
                { problemType = "wet_snow", elevation = { lowerBound = "treeline" }, aspects = { "E", "SE", "S", "SW" },
                    validTimePeriod = "later" },
                { problemType = "new_snow", aspects = { "N" }, validTimePeriod = "earlier" },
            } },
    },
}
