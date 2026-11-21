# Gardeners' Weather — existing services to reuse

The edition already computes most of "the sky and soil" itself from Open-Meteo:
ground frost, water balance, soil warmth, job windows, Hutton blight, frost dates
and a USDA zone from 30 years of ERA5 (`editions/garden/climate.lua`). So the
outside services worth adding are the few that bring something the app **cannot
compute**: the official warning the gardener's neighbours also hear (NWS, Met
Office), the official drought category, Europe's pollen forecast, and the
phenology "is spring early here?" view from USA-NPN. Growing degree days (GA10)
need no new service at all: they come from the archive endpoint the app already
calls. Plant databases (Trefle, Perenual, OpenFarm, Growstuff) are dead,
keyed, or carry licences that do not fit, and they break the "no data entry"
rule in [garden.md](garden.md#what-we-must-avoid). Hosepipe bans have no
machine-readable feed anywhere; a link is the honest answer.

Research date: November 2026. Hosts outside the session allowlist (open-meteo.com,
droughtmonitor.unl.edu, phzmapi.org, cran.r-project.org, metoffice.github.io)
could not be fetched, so their details come from search results and secondary
pages and are marked *verify*. Check every licence again before shipping.

## Services

Verdicts: **integrate** (a `source` in the app), **compute** (derive from data the
app already fetches), **link** (a card link to the official page), **avoid**.

| Service | Data | Endpoint | Key | Licence / commercial | Coverage | Verdict |
| --- | --- | --- | --- | --- | --- | --- |
| Open-Meteo Historical (already used) | Daily max/min since 1940 (ERA5/ERA5-Land) | `Forecast.url("archive", …)` | No (key on commercial host) | Data CC BY 4.0; free API is non-commercial, <10k calls/day; commercial needs a subscription, which `Forecast.url` already switches to | Global | **compute** GDD (GA10) |
| Open-Meteo Air Quality: pollen | Hourly alder, birch, grass, mugwort, olive, ragweed pollen (grains/m³), 4-day | `Forecast.url("airQuality", "…&hourly=grass_pollen,birch_pollen")` | No | As above, plus required CAMS attribution; Copernicus licence allows commercial reuse with "Generated using Copernicus Atmosphere Monitoring Service information" | **Europe only** (CAMS Europe, ~11 km) | **integrate** (UK persona) |
| NWS API alerts | Frost Advisory, Freeze Warning, Hard Freeze, Heat, Wind, Red Flag; CAP/GeoJSON | `https://api.weather.gov/alerts/active?point=<lat>,<lon>` | No; a User-Agent is required (the app sends one) | US government work, public domain | US | **integrate** |
| Met Office NSWWS warnings | Yellow/amber/red warnings (wind, snow, ice, rain, heat) by region | Public Atom feed / regional RSS ([RSS guide](https://weather.metoffice.gov.uk/guides/rss), [public API docs](https://metoffice.github.io/nswws-public-api)) *verify* | No for the feed | [NSWWS Open Licence](https://www.metoffice.gov.uk/api/assets/file/NSWWSOpenLicencepdf); [style guide](https://www.metoffice.gov.uk/api/assets/file/03115NSWWSStyleGuideupdate_v5pdf): show **unmodified**, call them "Met Office Weather Warnings", no other warning sources beside them. The Met Office issues no frost warnings | UK | **integrate** (P1, after the licence is read) |
| Met Office Weather DataHub | Site-specific spot and blended probabilistic forecasts | `datahub.metoffice.gov.uk`, `apikey` header | Yes | Free tier about 360 calls/day and one location ([Home Assistant docs](https://home-assistant.io/integrations/metoffice)); paid above that. DataPoint was retired in 2025 ([FAQ](https://www.metoffice.gov.uk/services/data/datapoint/datapoint-retirement-faqs)) | UK, global | **avoid** for now: needs a key, and the app is multi-place |
| US Drought Monitor (NDMC/USDA/NOAA) | Weekly drought category D0–D4 | REST `usdmdataservices.unl.edu/api/<area>/<stat>?aoi=<FIPS>&startdate&enddate&statisticsType` with `Accept: application/json` ([docs](https://droughtmonitor.unl.edu/DmData/DataDownload/WebServiceInfo.aspx)); point polygon query on the [`USDM_current` FeatureServer](https://services5.arcgis.com/0OTVzJS4K09zlixn/ArcGIS/rest/services/USDM_current/FeatureServer) *verify owner*; GeoJSON via [Iowa Mesonet](https://mesonet.agron.iastate.edu/geojson/usdm.py?help) | No | Public, credit "U.S. Drought Monitor, NDMC/USDA/NOAA" *verify wording* | US | **integrate** (point query; the REST API needs a county FIPS, which the app does not have) |
| European Drought Observatory (JRC/CEMS) | Combined Drought Indicator (CDI v4) | WMS only ([service](https://drought.emergency.copernicus.eu/data/wms-service)) | No | "Freely downloaded"; no stated WMS/API licence found | Europe | **link**: maps, no point JSON |
| UK hosepipe bans | Temporary use bans by water company | None. Human pages only: [Water UK](https://www.water.org.uk/water-supply/drought-and-hosepipe-bans-your-questions-answered), each company (e.g. [Thames Water](https://www.thameswater.co.uk/hosepipe-ban)), EA weekly [drought reports](https://www.gov.uk/government/publications/dry-weather-and-drought-in-england-july-2025-to-march-2026-summary-reports/dry-weather-and-drought-in-england-25-to-31-july-2025) | — | — | England, Wales | **link**, only in dry summers |
| US water restrictions | Utility watering days | No national feed | — | — | US | **link** to the user's utility (none known) |
| EA Hydrology / flood monitoring | River flows and levels | `environment.data.gov.uk/hydrology` | No | OGL, commercial OK | England | **avoid**: no soil moisture or drought status for a plot |
| USA-NPN Spring Index, AGDD, Pheno Forecasts | Spring leaf-out and bloom (lilac model), its anomaly, accumulated GDD, 6-day pest stage forecasts (emerald ash borer, tent caterpillar, lilac borer, winter moth…) | Geoserver WCS `geoserver.usanpn.org/geoserver/wcs?service=WCS&version=2.0.1&request=GetCoverage&coverageId=si-x:average_leaf_ncep&subset=…`; AGDD `…/stations/getTimeSeries.json?latitude&longitude&start_date&end_date&layer=gdd:agdd_50f` (from the [rnpn client](https://github.com/usa-npn/rnpn/blob/master/R/npn_geoserver.R)) | No | Federally funded, open-data policy ([USGS](https://pubs.usgs.gov/publication/ofr20181007)); the [terms](https://usanpn.org/terms) require attribution and name a separate "For-Profit Company Policy" *read before shipping* | Contiguous US | **integrate** (P2), with the pest thresholds otherwise computed locally |
| USDA hardiness: phzmapi.org / Frostline | Zone by ZIP code (JSON) | `https://phzmapi.org/<zip>.json` ([Frostline](https://github.com/waldoj/frostline)) | No | Code MIT; data from PRISM/OSU, which [retains ownership](https://prism.oregonstate.edu/phzm) under its [terms](https://prism.oregonstate.edu/terms/); "not the official map". The "US Grow Zones API" is only in directories | US ZIPs only | **avoid**: needs a ZIP, the licence is unclear, and `climate.lua` already gives a zone; **link** to [the official map](https://planthardiness.ars.usda.gov/) |
| AHDB BlightWatch (Hutton criteria) | Postcode blight alerts by email/SMS | No public API found | Registration | AHDB service | GB | **link** for allotment potato growers; the app's own Hutton check (GA6) stays |
| RHS | Advice, H-ratings | No API | — | Copyright | UK | **link** to advice pages only |
| iNaturalist API | Observations (phenology annotations) | `api.inaturalist.org/v1/observations` | No | Default CC BY-NC; [terms](https://www.inaturalist.org/terms) restrict commercial AI use | Global | **avoid** |
| GBIF occurrence API | Aggregated sightings | `api.gbif.org/v1/occurrence/search?license=CC0_1_0` | No | Per record; [licence data is messy](https://data-blog.gbif.org/post/gbif-occurrence-license-processing/), BY-NC non-commercial | Global | **avoid**: sightings are not forecasts |
| UK Pollinator Monitoring Scheme, BeeWatch | Bee counts | Downloads from EIDC, 1–2 years late ([JNCC](https://jncc.gov.uk/our-work/pollinator-monitoring-scheme)) | — | OGL on some records, inconsistent | UK | **avoid**: a "bee-friendly hours" estimate is better computed (warm, dry, light wind) |
| Trefle | Plant database | `trefle.io` | Token | Free with credit ([terms](https://trefle.io/terms)) | Global | **avoid**: dormant, data dump archived, SDKs say "project is over" |
| Perenual | Plant care database (10k+ species) | `perenual.com/api` | Key | Pricing and commercial terms not found | Global | **avoid** |
| OpenFarm | Growing guides | — | — | MIT | — | **avoid**: [shut down 21 April 2025](https://farm.bot/blogs/news/sunsetting-openfarm) |
| Growstuff | Crowd planting/harvest data | API v0; v1 a draft ([wiki](https://github-wiki-see.page/m/Growstuff/growstuff/wiki/API-Version-1)) | — | [CC BY-SA 3.0](https://growstuff-prod.herokuapp.com/support): share-alike would bind the app's own data | Sparse | **avoid** |
| CABI Plantwise / PlantwisePlus | Pest factsheets | Professional portal | Login | CABI copyright | Global South focus | **avoid** |

## Complaint evidence (data gaps)

Most complaints are already in [garden.md](garden.md#what-people-complain-about).
New ones that the services above answer:

- **The official frost warning stops when the gardener still needs it.**
  NWS offices end Frost Advisories after a killing freeze, or on a fixed date
  about 10 days after the median first freeze; outside the season "the NWS will
  not issue any Frost or Freeze headlines" ([NWS Charleston WV](https://www.weather.gov/rlx/frost-freeze-information),
  [NWS Binghamton](https://www.weather.gov/bgm/frostfreeze)). So the app must show
  its own frost estimate *with* the official one, and say why one is silent.
- **The app's low is not the yard's low.** "Frost does not happen based on your
  city. It happens based on your yard" ([Ambient Weather](https://ambientweather.com/blog/late-frost-in-spring-how-to-protect-your-garden-from-frost-damage)).
  A reader's app said 45 °F on a frosty morning ([Review-Journal](https://www.reviewjournal.com/?p=2509865)).
  Growers ask for warnings "even when temperatures stay above freezing"
  ([PalmTalk](https://www.palmtalk.org/forum/topic/82250-frost-warning-app));
  UK growers fall back on their own judgment with late frosts into June
  ([Kitchen Garden forum](https://forum.kitchengarden.co.uk/viewtopic.php?p=101499)).
- **Zones and averages are not dates.** Average last frost carries about a 30 %
  chance of a later frost; hard frosts came at the end of April against an
  average of 15 April; zone 6 gardens saw June frost
  ([garden.org](https://garden.org/thread/view/157869/Asheville-NC-planting-dates-are-completely-wrong/),
  [Garden Betty](https://gardenbetty.com/frost-dates/)). The 2023 USDA map moved
  half the US a half-zone warmer ([USDA ARS](https://www.ars.usda.gov/news-events/news/research-news/2023/usda-unveils-updated-plant-hardiness-zone-map/)).
  This supports odds-based frost dates (GA9) and a "spring is N days early" view
  (NPN) over a bare zone.
- **Planners hide where their dates come from.** Seed to Spoon takes dates from
  the "nearest weather station" and tells users to correct them by hand
  ([help centre](https://seedtospoonhelp.freshdesk.com/support/solutions/articles/62000107853-how-to-change-the-spring-and-fall-frost-dates-used-for-planting-calculations)).
- **Drought matters more than the zone for some.** "Drought in mid-summer was
  a bigger concern than frost timing" ([Permies](https://permies.com/t/234860/USDA-zones)).
  In 2025 and 2026, bans came in by company, on different dates
  (Yorkshire Water 11 July 2025, Thames Water July 2026); 14 companies were still in
  drought status in November 2025 ([gov.uk](https://www.gov.uk/government/publications/drought-prospects-for-spring-2026/3-public-water-supply-drought-risks-impacts-and-actions-drought-prospects-for-spring-2026),
  [Thames Water](https://www.thameswater.co.uk/news/2026/jul/thames-water-announces-temporary-use-ban-following-record-hot-weather-and-exceptionally-high-demand)).
  The only lists are human pages, which is user story 39's "(uncertain)".
- **Not found:** no App Store or Reddit complaints asking for pollen or pest-stage
  data in a garden app turned up. Pollen is a judgement call: gardeners with hay
  fever mow and weed, which is why it sits at P1, not P0.

## Integration plan

Every item is an `optional = true` source or pure computation. `url` returns nil
outside its coverage, so a UK place never asks NWS and a US place never asks for
CAMS pollen. Hosts other than Open-Meteo need an entry in `Forecast.endpoints`
(as `noaaTides` has), which is a shared file: raise it under "Requests to the
foundation" in PLAN.md rather than editing `forecast.lua`. Each item gets offline
checks in `tests/test_weather_garden.lua` with a small recorded answer
(`ui.net.answers["<host pattern>"]`, as the archive test does), and a missing
answer must leave the cards unchanged (`forecast.missing[id]`).

| ID | Item | Pri | Size | Files | `sources` / `cards` | Attribution |
| --- | --- | --- | --- | --- | --- | --- |
| SV-GA1 | **Official frost and heat warnings, US.** NWS active alerts at the place's point; keep Frost Advisory, Freeze Warning, Hard Freeze Warning/Watch, Excessive Heat, High Wind. Card "Frost advisory tonight (NWS)" ranked above the app's own frost card; if NWS is silent past its season, the frost card says "NWS frost alerts end for the season; this is our estimate" | P0 | S | `editions/garden/warnings.lua`, `activities/garden.lua` | `{ id = "nws", optional = true, headers = { Accept = "application/geo+json" }, url = US bbox or nil, parse → forecast.official }`; card `official`, `rank` 0 | "Alerts: National Weather Service" (public domain) |
| SV-GA2 | **Growing degree days (GA10, un-skipped).** Base 10 °C/50 °F, `max(0, (min(max,30)+max(min,10))/2 − 10)`, from 1 January (from 1 April for the UK view) using the archive up to 5 days ago, then the forecast's past and coming days. Answers Dan's lawn question (crabgrass preventer between about 200 and 250 °F-GDD from 1 January, a common extension rule; *cite one extension source in the file*) and "sweetcorn about N days ahead of normal" against the 30-year mean already pulled by `climate.lua` | P1 | M | `editions/garden/degree.lua`, extend `climate.lua` to keep daily means | second archive source `gdd`, cached once a day like `climate`; card `season` ("18 days ahead of an average year") | "Weather data by Open-Meteo.com" (already shown) |
| SV-GA3 | **Pollen for the gardener, Europe.** Grass and birch (spring), mugwort (late summer) as low/medium/high on the "best time today" card: "grass pollen high until 18:00, mow after" | P1 | S | `editions/garden/jobs.lua`, `activities/garden.lua` | `{ id = "pollen", url = Europe bbox → Forecast.url("airQuality", …"&hourly=grass_pollen,birch_pollen,mugwort_pollen&forecast_days=3"), parse → hour.grass_pollen … }`; a reason on `mow`, no own card | "Generated using Copernicus Atmosphere Monitoring Service information 2026" plus Open-Meteo |
| SV-GA4 | **Drought category, US.** Point-in-polygon on the current USDM map; D1+ adds to the water card: "Moderate drought (D1) here: check your utility's watering days". Weekly, so cache 24 h | P1 | M | `editions/garden/water.lua` | `{ id = "drought", url = US bbox → FeatureServer query `geometry=<lon>,<lat>&geometryType=esriGeometryPoint&outFields=DM&f=json`, parse → forecast.drought }` | "U.S. Drought Monitor (NDMC, USDA, NOAA)" |
| SV-GA5 | **Hosepipe-ban link, UK.** No feed, so no claim. When the water card says "water" in June–September and the last 30 days ran short, add one action, "Is there a hosepipe ban?", to Water UK's page. Never says "there is a ban" | P1 | S | `editions/garden/water.lua`, `activities/garden.lua` (`act` opens the URL) | card `action`, no source | Link text names Water UK |
| SV-GA6 | **Met Office Weather Warnings, UK.** Show amber/red wind, snow, ice and heat warnings for the place's region **unmodified** in their own card; the app's frost estimate stays separate and never borrows the Met Office name | P1 | M | `editions/garden/warnings.lua` | `{ id = "metoffice", url = UK bbox → Atom feed }`; region match by polygon if the feed carries one, else skip | "Met Office Weather Warnings", per the NSWWS style guide. **Read the NSWWS Open Licence first** |
| SV-GA7 | **Spring early or late, US (NPN).** Spring Index leaf-out and bloom for the year with its anomaly: "Spring arrived 12 days early here (lilac leaf-out)". Pest-stage hints (tent caterpillar, winter moth) from published NPN GDD thresholds, computed from SV-GA2, labelled "estimate, after USA-NPN" | P2 | M | `editions/garden/degree.lua` | `{ id = "npn", url = CONUS bbox → WCS GetCoverage point subset, parse GML value }`; card `season` gains a line | "USA National Phenology Network, usanpn.org". **Clear the For-Profit policy first** |
| SV-GA8 | **Reference links, no data.** On the hub's zone and blight lines: the official USDA map (US), RHS hardiness ratings (UK), BlightWatch sign-up (GB potatoes). Shows the app knows the authoritative source without copying it | P2 | S | `activities/garden.lua` | card `actions` | Link names only |

Order: SV-GA1, then SV-GA2 and SV-GA3 (keyless, already-licensed paths), then
SV-GA4 and SV-GA5 for the water story. SV-GA6 and SV-GA7 wait on a licence
reading, not on code.

Not planned: plant databases (data entry, and dead or unclear licences), Met
Office DataHub (key, one free place), citizen-science sightings (non-commercial
licences, not forecasts) and phzmapi.org (ZIP only, PRISM ownership; the ERA5
zone is already shown with its method).
