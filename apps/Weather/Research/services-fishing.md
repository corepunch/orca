# Existing data services for Fishers' Weather

Research for the `fishing` edition, November 2026. It builds on
[fishing.md](fishing.md) (competitors, complaints, stories 41–48 and 89–94)
and on what `editions/fishing/` already does: Open-Meteo forecast, marine
and flood data (`water.lua`, `river.lua`), NOAA CO-OPS tide predictions
(`tides.lua`), static rule links (`rules.lua`), species rules, the log, moon
and fronts.

## Summary

The biggest gap anglers name is the **state of the water**: real river
level and flow, and real water temperature. That gap is filled by public
gauge networks that are free, need no key, and allow commercial reuse.
USGS (US, public domain), the Environment Agency (England, OGL v3),
PEGELONLINE (German federal waterways, DL-DE Zero 2.0) and the OPW
(Ireland, CC BY 4.0) all answer in JSON or GeoJSON from a station near the
place. They replace the GloFAS grid estimate in `river.lua` where a gauge
exists. NOAA CO-OPS (already used for tides) and the GLERL Great Lakes
ERDDAP add measured water temperature on US coasts and the Great Lakes.
"Which fish are here" can come from GBIF occurrence facets, if we keep only
CC0 and CC BY records. iNaturalist and FishBase default to *non-commercial*
licences, so they are link-only. Stocking data exists as open Socrata
datasets in a few US states (NY, WA), so it is a curated per-place feature,
like `rules.lua`. No algal-bloom API was confirmed (EPA CyAN's REST API is
said to continue, but no documented endpoint was found), so blooms stay a
link. **Caveat:** this session could not reach the hosts directly (fetches
were blocked), so endpoints and parameters come from official docs found
by search and from client libraries. Each must be checked with one live
call before its fixture is recorded.

## Services

Verdicts: **integrate** = a `sources` entry; **link-only** = a row or card
pointing to the official page; **avoid** = not worth it now.

| Service | Data | Endpoint (base) | Key? | Licence / commercial | Coverage | Verdict |
| --- | --- | --- | --- | --- | --- | --- |
| USGS Water Data OGC API | latest and continuous discharge (00060), gage height (00065), water temp (00010) | `api.waterdata.usgs.gov/ogcapi/v0/collections/latest-continuous/items` and `…/continuous/items` (v0 vs v1: verify) | optional free api.data.gov key (`X-Api-Key`) for higher limits; anonymous limit not published, see `X-RateLimit-*` headers | public domain; credit "U.S. Geological Survey"; provisional data needs a disclaimer | ~US, thousands of gauges | **integrate** (P0) |
| USGS WaterServices `iv` (legacy) | the same | `waterservices.usgs.gov/nwis/iv` | no | public domain | US | **avoid**: degrading from Nov 2026, shut down 22 Feb 2027 |
| UK Environment Agency flood-monitoring | river/sea level, some flow, every 15 min | `environment.data.gov.uk/flood-monitoring/id/stations?lat=&long=&dist=`, `/id/stations/{id}/readings?_sorted&_limit=` | no | OGL v3, commercial OK; required statement (below); beta, no SLA | England, ~1,700+ stations | **integrate** (P1) |
| EA Hydrology / Water Quality Archive | checked flow history; spot water temp in WQ archive | `environment.data.gov.uk/hydrology`, `/water-quality` | no | OGL claimed; one temp archive says "all rights reserved" | England | **avoid** for now (history, not now) |
| PEGELONLINE (WSV) | water level W, discharge Q, water temp WT at some stations | `pegelonline.wsv.de/webservices/rest-api/v2/stations.json`, `/stations/{uuid}/W/measurements.json?start=P7D` | no, "no rate limit currently" | DL-DE Zero 2.0 (per a Hessen open-data record): any use incl. commercial | German federal waterways (Rhine, Elbe, Danube, coasts); not small rivers | **integrate** (P1) |
| OPW waterlevel.ie | latest level at every OPW gauge, 15 min | `waterlevel.ie/geojson/latest/`, stations `waterlevel.ie/geojson/` | no; OPW asks to be told (waterlevel@opw.ie) and no more than every 15 min | CC BY 4.0 | Ireland | **integrate** (P2) |
| Hub'Eau hydrométrie + température | real-time H/Q (`observations_tr`); continuous river temp (separate API) | `hubeau.eaufrance.fr/api/v2/hydrometrie/observations_tr`, `…/v1/temperature/chronique` | no | Licence Ouverte (Etalab) cited by the R client, not confirmed on Hub'Eau | France | **integrate later** (P2, confirm licence) |
| Swiss BAFU hydrodaten | level, flow, water temp | official data platform is on a *staging* host; `api.existenz.ch` is a private mirror | — | official: open; mirror: unofficial | Switzerland | **link-only** until BAFU's API is production |
| NOAA CO-OPS `water_temperature` | 6-min water temp at tide stations | `api.tidesandcurrents.noaa.gov/api/prod/datagetter?product=water_temperature&station=&date=recent&format=json` (`latest` may not apply) | no | US public domain | US coasts, Great Lakes NWLON | **integrate** (P0, same station as tides) |
| NOAA NDBC buoys | water temp WTMP, waves, wind | `ndbc.noaa.gov/data/realtime2/{id}.txt` (fixed-width text, `MM` = missing) | no | US public domain | US and partner buoys, offshore | **integrate later** (needs a text source; see SV-FI3) |
| NOAA GLERL GLSEA (ERDDAP) | daily lake-average surface temp, 1995–present; grid | `apps.glerl.noaa.gov/erddap/tabledap/glsea_avgtemps_3.json?…` | no | "may be used and redistributed for free", not for legal use | 5 Great Lakes (+ St Clair) | **integrate** (P1) |
| Copernicus Marine | SST analyses (OSTIA), ocean forecasts | Python toolbox / OPeNDAP, NetCDF/Zarr | free account | free incl. commercial with acknowledgement; guaranteed to mid-2028 | global | **avoid**: account plus NetCDF; Open-Meteo marine covers the sea surface |
| EMODnet | bathymetry, seabed habitats | WMS/WCS | no | CC BY 4.0 mostly | European seas | **link-only** (map, out of scope) |
| GBIF occurrence search | species recorded near a point; facets by `speciesKey` | `api.gbif.org/v1/occurrence/search?taxonKey=<Actinopterygii>&geoDistance=lat,lon,10km&facet=speciesKey&limit=0&license=CC0_1_0&license=CC_BY_4_0` (verify `geoDistance`) | no; "rapid queries may be rate limited" | per record: CC0, CC BY or CC BY-NC; filter out NC | global, uneven | **integrate** (P2) |
| iNaturalist API v1 | observed species counts near a point | `api.inaturalist.org/v1/observations/species_counts` | no; ~60 req/min | ToS; default CC BY-NC data | global, strong in NA/EU | **link-only** ("see on iNaturalist") |
| FishBase / SeaLifeBase | species facts | old API unmaintained; parquet snapshots (Hugging Face, Source Cooperative) | no | CC BY-NC 4.0 | global | **avoid** in-app; link per species |
| NY DEC stocking (data.ny.gov) | actual stocking 2011–last year (`e52k-ymww`); planned spring trout (`d9y2-n436`) | Socrata `data.ny.gov/resource/e52k-ymww.json?$where=…` | no (app token optional) | NY open data terms (free reuse; check wording) | New York | **integrate** as curated (P2) |
| WDFW fish plants (data.wa.gov) | trout and kokanee plants | Socrata `data.wa.gov/resource/6fex-3r7d.json` | no | WA open data terms (check) | Washington | **integrate** as curated (P2) |
| Other state stocking (PA, CO, MD, CT, NJ, CA) | schedules | web pages, PDFs, state apps; no API found | — | varies | per state | **link-only** in `rules.lua` |
| EPA CyAN | satellite cyanobacteria index, ~2,000 US lakes | "REST API unchanged" per EPA; endpoint not found; web app down since Mar 2026 | — | US public domain | large US lakes | **link-only** (How's My Waterway) |
| NOAA HAB forecasts | Lake Erie, Gulf bulletins | GeoTIFF/CSV at NCEI THREDDS; no API | no | public | regional | **link-only** |

## What anglers complain about (data gaps)

Earlier research found paywalls and trust issues. This search looked for
*data* that is missing.

- **No water temperature at all.** A bass app reviewer has to "switch to
  another app" for surface temperature
  ([search summary of App Store reviews](https://apps.apple.com/app/id1549715715)).
  Gauge apps say temperature exists "for a limited number of gauges"
  ([GaugeStage](https://apps.apple.com/app/id6477322714)). S.I.F.T. now
  *models* river temperature from air for rivers with no sensor, which
  shows how thin measured data is. Satellite lake temperatures miss small
  lakes and cloudy days (fishing.md §9).
- **Flow shown as gauge height, or gauges vanish.** "Most show gauge height
  instead [of flow], which is far less useful"
  ([River Levels & Flows reviews](https://justuseapp.com/en/app/1508808963/river-levels-flows/reviews)).
  A Rivercast user saw "a gauge that was there yesterday… gone today"; the
  developer blamed transmission outages and floods
  ([Rivercast](https://apps.apple.com/app/id545375951)). We must show the
  reading's age and say when a station is silent.
- **River data lives in separate apps.** Fly anglers use Riffle, HomeWater,
  FlyFishFinder, Current, GOFISHAB and River Data for USGS flows and
  temperature. In the UK they use UkFloodPro, Levee, PeakFlows and
  CatchNotes for EA levels
  ([Riffle](https://apps.apple.com/app/id1662402139),
  [HomeWater](https://apps.apple.com/us/app/homewater/id1227805310?l=en),
  [Current](https://apps.apple.com/app/id6760433517),
  [UkFloodPro](https://apps.apple.com/gb/app/ukfloodpro/id6474907587),
  [CatchNotes](https://apps.apple.com/gb/app/catchnotes/id6757416538)).
  Nobody combines gauges with an honest weather verdict and a private log.
  Current's free tier stops at three locations.
- **Stocking is scattered.** Each state publishes it differently: a PA app
  where "some waters are not listed", NJ QR codes on signs, MD daily web
  posts and a phone line, CO weekly on Fridays, CT more than 100 PDF maps
  ([PA](https://www.pa.gov/agencies/fishandboat/fishing/stocking/trout-stocking),
  [MD FAQ](https://dnr.maryland.gov/fisheries/pages/trout/faq.aspx),
  [CO](https://cpw.state.co.us/fishstocking),
  [CT](https://portal.ct.gov/DEEP/Fishing/General-Information/Trout-Stocking-Maps)).
  Of the apps found, only GOFISHAB (Alberta only) shows stocking
  ([GOFISHAB](https://apps.apple.com/us/app/gofishab/id6447429713)).
- **Algae: anglers are told to check, but not where.** The CDC says to
  check advisories before a visit
  ([CDC](https://www.cdc.gov/harmful-algal-blooms/prevention/index.html)).
  Wisconsin says most lakes are never tested, so "no report" is not "safe"
  ([WI DNR](https://dnr.wisconsin.gov/newsroom/release/96341)). Minnesota
  suggests waiting a week or two after a bloom before eating the fish
  ([MPCA](https://www.pca.state.mn.us/water/blue-green-algae-and-harmful-algal-blooms)).
  Dogs are most at risk
  ([OEHHA](https://www.oehha.ca.gov/fact-sheet/dog-owners-fact-sheet-habs)).
- **"Which fish are here."** Searches return only photo-ID apps. Lake
  survey data sits with state agencies, and no app lists what has been
  recorded at the place. Evidence is weak: no forum thread was found, so
  treat this as an inferred gap. Reddit itself was not reachable; all of
  the above are store listings, review aggregators and agency pages.

## Integration plan

All items follow the existing contracts. Each new source is
`{ id, optional = true, url = function(spot) … end, parse = function(json,
forecast) … end, headers }` in `Fishing.sources`. Its result is a field on
the forecast that a card or `Water`/`River` reads. Each returns nil from
`url` when the place has no station, so nothing is asked elsewhere. Tests go in
`tests/test_weather_fishing.lua`: an inline Lua table recorded from one
live answer (as with `Tides.parseNoaa`), trimmed to a few readings, plus
one replay check through `F.launch` with a fake fetch keyed by URL. Base
URLs go in `Forecast.endpoints` (`usgs`, `eaFlood`, `pegel`, `opw`,
`glerl`, `gbif`), so tests and a future proxy can swap them.

### SV-FI1 Measured river gauge for US places (USGS). P0, M

- **Files:** new `editions/fishing/gauges.lua` with a `Gauges.usgs` adapter.
  `river.lua` prefers `forecast.gauge` over GloFAS. `water.lua` adds a
  "measured at the gauge" step between the angler's reading and the model.
  The place gets `gauge = { usgs = "01646500", name = "Little Falls" }`,
  or the place editor finds the nearest one with `monitoring-locations`
  and a bbox.
- **Request:** `latest-continuous` for 00060, 00065 and 00010 (one call),
  plus `continuous` for the last 7 days of 00060/00065 for the trend.
  `headers` adds `X-Api-Key` only when one is configured.
- **Card:** "Potomac at Little Falls: 1,240 ft³/s, falling, 9.5 °C, 40 min
  ago (USGS, provisional)". The river state is high or low against the
  week's median, as now. The card turns "info" and says "gauge silent
  since …" when the newest reading is more than 6 h old.
- **Test:** a fixture of one GeoJSON `features` answer with three
  parameters and a missing temperature; check the card wording, the
  preference over GloFAS, and `missing.gauge` for an empty answer.
- **About:** "River data: U.S. Geological Survey (public domain).
  Recent readings are provisional and may be revised."
- **Note:** do not use `waterservices.usgs.gov`, which is retired on 22 Feb 2027.

### SV-FI2 Measured water temperature at NOAA tide stations. P0, S

- **Files:** `tides.lua` (same `tideStation.noaa` id) adds a
  `water_temperature` source with `date=recent` (or `begin_date` and a
  short `range`). `water.lua` uses it ahead of the marine SST.
- **Card or reason:** "Water 18.4 °C at Vaca Key, 2.1 km away, 12 min ago",
  with the source named, as tides already do.
- **Test:** fixture `{ data = { { t, v, f } } }`. Check unit conversion,
  that a stale value is dropped, and that a station without the sensor
  (`error.message`) falls back to the marine model.
- **About:** extend the NOAA line: "Tides and water temperature: NOAA
  CO-OPS (public domain)."

### SV-FI3 European gauges: England, Germany, Ireland. P1, M (L with France)

- **Files:** `gauges.lua` adapters `ea`, `pegel` and `opw`, all returning
  the same `{ station, distance, level, flow, temp, time, series }`.
  Places get `gauge = { ea = "…" }`, `{ pegel = "<uuid>" }` or
  `{ opw = "…" }`. The place editor offers the nearest station by
  `lat/long/dist` (EA) or the station list (Pegel, OPW).
- **What each gives:** EA has level, sometimes flow, and a station
  "typical range" for high or low. Pegel has W and Q, and WT where
  present. OPW has the latest level only, so the trend comes from the
  previous saved reading.
- **Test:** one fixture per adapter. The shared card test is parametrized
  over the normalized table.
- **About:** "This uses Environment Agency flood and river level data from
  the real-time data API (Beta)" (EA's required statement, OGL v3);
  "Water levels: WSV, PEGELONLINE (DL-DE Zero 2.0)"; "Water levels: Office
  of Public Works, waterlevel.ie (CC BY 4.0)".
- **Open questions:** email OPW before launch, as their page asks. Check
  the PEGELONLINE licence on the WSV site. Add Hub'Eau (France) after
  confirming Licence Ouverte. OPW GeoJSON is JSON, so it fits `parse`.

### SV-FI4 Great Lakes surface temperature (GLERL GLSEA). P1, S

- **Files:** `water.lua` gets a source with
  `url = spot.greatLake and glerl…`; places get
  `greatLake = "erie"`. The source asks `glsea_avgtemps_3` for the last
  10 days of that lake's daily average. That replaces the
  `soil_temperature_0cm` estimate for those lakes and gives the weekly
  trend (story 43). The answer is a lake average, so the card says so.
- **Test:** ERDDAP `table.rows` fixture. Check the column lookup by name,
  not position.
- **About:** "Great Lakes water temperature: NOAA GLERL, GLSEA."

### SV-FI5 "Recorded near here" species list (GBIF). P2, M

- **Files:** `species.lua` gains `Species.nearby(place)` and a source on
  the log page only, not the hub, to keep the call budget. The request is
  one facet-only query (`limit=0&facet=speciesKey&facetLimit=15`) for
  ray-finned fish within 10 km, keeping only `license=CC0_1_0` and
  `CC_BY_4_0`. Cache it for 30 days in the edition's storage. The
  vernacular name comes from `/v1/species/{key}` (one call per species,
  cached).
- **Card:** "Fish recorded within 10 km: pike, perch, roach… (GBIF
  records, not a survey)". It also suggests the species profile when one
  matches `Species.OPTIONS`.
- **Test:** fixture of a `facets[1].counts` answer and two species
  answers. Check the NC filter is in the URL and the honest wording.
- **About:** "Species records: GBIF.org, from datasets under CC0 and CC BY
  4.0." GBIF asks for a citation of the query. Use a short "GBIF.org
  occurrence search, <date>" and link the query.
- **iNaturalist:** link only ("see what others have seen here"). Its
  default data licence is CC BY-NC.

### SV-FI6 Stocking for NY and WA places (Socrata). P2, S

- **Files:** `rules.lua` grows `Rules.stocking(place)`. A curated place
  field `stocking = { ny = "Esopus Creek" }` or `{ wa = "Lake X" }` drives a
  source with `$where=waterbody='…'&$order=date DESC&$limit=5`.
  `$`-parameters must be URL-encoded. NY "actual" lags to the end of the
  year, so for spring trout use the planned dataset (`d9y2-n436`) and say
  "planned".
- **Card:** "Last stocked: rainbow trout, 2,000, 14 Apr (NY DEC)".
  Elsewhere the row links to the state's stocking page (PA, CO, MD, CT,
  NJ, CA).
- **Test:** two-row SODA fixture. Check the planned and actual wording.
- **About:** "Stocking: NY DEC and WDFW via data.ny.gov and data.wa.gov."
  Confirm each portal's terms of use first.

### SV-FI7 Algal-bloom check, link-only. P2, S

- **Files:** `rules.lua` adds a "blooms" row with per-region links: EPA
  How's My Waterway (US), the state HAB map, and EA or SEPA bathing-water
  pages (UK). Show it more prominently after hot, calm days, using the
  existing heat logic (story 47).
- **Wording:** calm, and honest that "no report" means untested, not safe.
  Add the dog warning and the advice to wait before eating fish.
- **Data:** no API. Revisit if EPA documents the CyAN REST endpoint. No
  fixture is needed beyond a row-text test.

### Foundation request (NDBC buoys)

NDBC realtime files are fixed-width text, not JSON. Ask under "Requests to
the foundation" for `format = "text"` on a source. Its `parse` would then
receive the body string, and the replay store would keep it as a string.
Until then, use NOAA CO-OPS and Open-Meteo marine for coastal water
temperature.

## Attribution summary for the settings "about"

Add rows to `Provider.ABOUT` only for the services a place has used, or
list them all once under "Fishers' Weather data":

- U.S. Geological Survey: river data (public domain; provisional).
- NOAA CO-OPS: tides and water temperature; NOAA GLERL: Great Lakes temperature.
- "This uses Environment Agency flood and river level data from the
  real-time data API (Beta)." (OGL v3)
- WSV PEGELONLINE (DL-DE Zero 2.0) and Office of Public Works, waterlevel.ie (CC BY 4.0).
- GBIF.org occurrence data (CC0 and CC BY 4.0 records).
- NY DEC and WDFW stocking lists.

Safety framing applies to all of them: these readings are not flood
warnings, and the EA feed is a beta with no service guarantee.
