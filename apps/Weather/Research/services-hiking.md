# Hikers' Weather — existing services to reuse

Research date: November 2026. This builds on [hiking.md](hiking.md) (competitors,
complaints, HI1–HI8) and on what `editions/hiking/` already asks for: summit and
trailhead points, three models, Open-Meteo air quality and NWS alerts
(`sources.lua`). The question here is narrower. Which services that already
exist would fill the gaps hikers name before a go/no-go decision: avalanche
danger, fire and smoke, closures, measured snow, lightning, huts and water?

**Method and limits.** Web search, plus one fetched page (the avalanche.org API
guide on GitHub). This sandbox could not reach `avalanche.ca` or `slf.ch`, so
their endpoint details come from search snippets. Rows marked **verify** need
one real request and a licence check before code is written. Licence statements
are quoted from the provider where one was found. "None stated" means we found
none, which is not the same as permission.

## Summary

- **Avalanche danger is the biggest gap with a usable API.** In the US,
  avalanche.org has a public, keyless GeoJSON map layer with the 1–5 danger
  level per forecast zone. In the Alps, the EAWS bulletins come as CAAML v6
  JSON: SLF (Switzerland) is CC BY 4.0, and Euregio publishes keyless JSON with
  an unclear licence. We can show the danger *at the summit's elevation band*.
  Most weather apps only link out. Every source asks that the danger keep its
  meaning, expire on schedule and link to the full forecast.
- **US wildfire perimeters are public domain and keyless** (NIFC/WFIGS ArcGIS).
  One point-and-distance query answers "is there a fire within 30 km?". FIRMS
  satellite hotspots are open NASA data but need a free key. The smoke we
  already have comes from Open-Meteo air quality (HI8).
- **Measured snow beats modelled snow.** USDA SNOTEL (US, public domain,
  keyless REST), SLF IMIS (CC BY 4.0) and GeoSphere Austria (CC BY 4.0,
  keyless) give real station depths near curated peaks.
- **Closures have no single feed.** The NPS API covers national parks (free
  key, 1,000 requests an hour per key). USFS forest orders are scattered
  regional ArcGIS layers, so link to them.
- **Avoid Blitzortung.** Its terms bar commercial use and storm-warning use,
  and it has no public API. Real lightning observations have no free, light
  feed. ICON-D2 `lightning_potential` through Open-Meteo is the honest
  forecast option for central Europe.
- **Huts, shelters and water:** OSM data is ODbL and allowed commercially, but
  the public Overpass server is not for app traffic. Precompute them for the
  curated peaks instead.
- **Every Open-Meteo call stays under the PLAN.md §0 licence decision**, and so
  does the Open-Meteo elevation API.

## Services

Verdicts: **integrate** (a `sources` entry or curated data), **link-only** (a
row under "official word"), **avoid**.

| Service | Data | Endpoint | Key? | Licence / commercial | Coverage | Verdict |
| --- | --- | --- | --- | --- | --- | --- |
| avalanche.org public API | danger 1–5 per zone, zone name, centre, issued and expires times, link | `api.avalanche.org/v2/public/products/map-layer[/{center_id}]` (GeoJSON) | no | None stated. The guide asks that the meaning stay intact, that ratings publish and expire with the forecast, and that each zone link to its centre. Ask the National Avalanche Center in writing. | US forecast centres (about 20) | **integrate** (P0), after an email |
| Euregio avalanche.report (ALBINA) | CAAML v6 bulletins: danger by elevation band, problems, aspects | `static.avalanche.report/bulletins/latest/EUREGIO_en_CAAMLv6.json`; per-date XML/JSON | no | Disputed. "Open data" with no named licence on the provider side; a third party says CC BY 4.0. The code is AGPL-3.0 and does not cover the data. Ask. | Tyrol, South Tyrol, Trentino | **integrate** once the licence is confirmed, else link |
| avalanche.report EAWS aggregate | CAAML JSON of other EAWS members | `static.avalanche.report/eaws_bulletins/{date}/{date}-CH.json` (per country) | no | Each bulletin keeps its producer's licence (verify) | several EAWS countries | evaluate; prefer the producer |
| SLF bulletin (Switzerland) | CAAML bulletin; warning-region GeoJSON/KML; IMIS station data | `aws.slf.ch/api/bulletin/caaml`, `aws.slf.ch/api/warningregion/`, `measurement-api.slf.ch` | no | **CC BY 4.0**, with a visible source reference; data terms apply as well | Switzerland | **integrate** (verify the JSON format) |
| Météo-France BRA | 36 massif bulletins (XML) | DonneesPubliquesBRA via `portail-api.meteofrance.fr` | yes (free account) | Licence Ouverte 2.0, so commercial use is OK. Forum users report portal authentication errors. | French Alps, Pyrenees, Corsica | link-only now; integrate later |
| Varsom / NVE (Norway) | avalanche warning by coordinates | `api01.nve.no/hydrology/forecast/avalanche/…/AvalancheWarningByCoordinates/…` (**verify**, from memory) | no | NLOD/CC BY 4.0 believed; not found in search, **verify** | Norway | integrate after verification (Lofoten) |
| Avalanche Canada | areas GeoJSON, metadata, products JSON | `avcan-services-api.prod.avalanche.ca` (Postman docs) | no | Site terms: no reproduction without prior written consent; no API licence found | Canada | **link-only** unless consent is given |
| SAIS (Scotland) | daily avalanche reports | no public API found | — | — | Scottish Highlands | link-only |
| NIFC / WFIGS current perimeters | active US fire perimeters, name, acres, containment | ArcGIS `…/WFIGS_Interagency_Perimeters_Current/FeatureServer/0/query` (point + `distance`) (**verify** the current item URL on the NIFC Open Data site) | no | US government, public domain (catalog "us-pd"); the metadata says "otherRestrictions" and disclaims liability | US | **integrate** (P1) |
| WFIGS current incident locations | fire points including new starts | same ArcGIS org | no | as above | US | with perimeters |
| NASA FIRMS | VIIRS/MODIS hotspots, near real time | `firms.modaps.eosdis.nasa.gov/api/area/csv/{MAP_KEY}/{source}/{bbox}/{days}` | yes (free MAP_KEY); keyless whole-region CSV files | NASA open data, commercial use OK, acknowledgement requested; 5,000 transactions per 10 min | global | integrate later (P2, global fire) |
| Open-Meteo Air Quality | US AQI, PM2.5 (smoke forecast) | already in `sources.lua` | under §0 | Open-Meteo/CAMS terms (§0) | global | **in use** (HI8) |
| AirNow | measured US AQI | `airnowapi.org` | yes | Data Use Guidelines PDF (not read); about 500 requests an hour per key (third-party figure) | US | link-only; maybe later |
| PurpleAir | low-cost sensor PM2.5 | `api.purpleair.com` | yes | paid points model, attribution guide, enterprise pricing for commercial use | dense in the US west | **avoid** (cost and terms) |
| NPS API | park alerts: closures, cautions, dangers | `developer.nps.gov/api/v1/alerts?parkCode=` | yes (free) | US government data; 1,000 requests an hour per key, refreshed about every 2 h | US national parks | **integrate** (P1) with caching |
| USFS forest orders | closures and fire restrictions | regional ArcGIS layers (e.g. `apps.fs.usda.gov/fsgisx02/rest/services/r03/r03_ForestOrder_01`) | no | public domain; the signed order on the forest site is authoritative | per region, not national | link-only (per peak) |
| RIDB (recreation.gov) | facilities, campgrounds | `ridb.recreation.gov` | yes | public | US federal | not needed |
| USDA NRCS AWDB (SNOTEL) | measured snow depth (SNWD, in), SWE, temperature | `wcc.sc.egov.usda.gov/awdbRestApi/services/v1/data?stationTriplets=…&elements=SNWD` (Swagger) | no | US government, public domain | western US mountains | **integrate** (P1) |
| SLF IMIS stations | snow depth and new snow at about 2,000–3,000 m | `measurement-api.slf.ch` | no | CC BY 4.0 | Swiss Alps | integrate with SNOTEL (Zermatt) |
| GeoSphere Austria Data Hub | TAWES 10-min station data, incl. snow | `dataset.api.hub.geosphere.at/v1/…` | no | CC BY 4.0 for keyless data; "Data source: GeoSphere Austria" | Austria | later (no curated Austrian peak yet) |
| MeteoSwiss OGD | local forecasts for about 6,000 points, stations | STAC `data.geo.admin.ch/api/stac/v1/collections/ch.meteoschweiz.ogd-local-forecasting` | no | CC BY, "Source: MeteoSwiss"; files only, point queries promised "not before Q2 2026" | Switzerland | link-only; bulk files do not fit a phone |
| DWD open data | MOSMIX, warnings, lightning climatology | `opendata.dwd.de` | no | GeoNutzV / CC BY 4.0, commercial use OK with source | Germany | not needed; reached through Open-Meteo |
| Open-Meteo `lightning_potential` | ICON-D2 lightning potential index, 2 days | Open-Meteo forecast (`models=icon_d2`) | under §0 | §0 | central Europe (about 2 km) | **integrate** into HI3 (P2) |
| Blitzortung | volunteer lightning strikes | none public | — | Commercial use prohibited; storm-warning use barred; no API | global | **avoid** |
| GOES GLM (NOAA) | satellite lightning flashes | NOAA open data (netCDF on cloud buckets) | no | public domain | Americas | avoid on device (heavy); link only |
| MeteoAlarm | EU warnings (CAP) | redistributor hub feeds | ? | Terms not found (as in hiking.md) | Europe | evaluate with W10 |
| OSM via Overpass | huts, shelters, springs, drinking water near a peak | `overpass-api.de/api/interpreter` | no | Data ODbL: commercial use OK with "© OpenStreetMap contributors". The public server's policy counts all of an app's users together (about 10k queries a day) and points commercial use to self-hosted or paid servers. | global | **integrate as precomputed data** for curated peaks; no live queries |
| Open-Meteo / Open-Elevation elevation | DEM height of a point (Copernicus GLO-90) | `api.open-meteo.com/v1/elevation` | under §0 | Copernicus attribution | global | in use implicitly (`elevation=`); fine |

## What hikers say is missing

Earlier evidence (summit height, storm timing, wind, offline, paywalls) is in
[hiking.md § What people complain about](hiking.md#what-people-complain-about).
New items:

1. **Fire and smoke are checked in a separate app, repeatedly.** WTA added an
   AQI/smoke layer and closure alerts to its app. Its advice is to check "trip
   reports, trail conditions, wildfire danger and the smoke forecast up until
   you arrive at the trailhead". Colorado's COTREX app added wildfire alerts.
   CPW moved its updates from daily to hourly during heavy fire activity,
   which shows how fast this data goes stale.
   [WTA smoke/AQI layer](https://www.wta.org/news/signpost/website-smoke-aqi-wildfire-layer-2023),
   [WTA wildfire safety](https://www.wta.org/go-outside/trail-smarts/how-to/wildfire-season-and-campfire-safety),
   [COTREX wildfire alerts](https://cpw.state.co.us/news/05092024/colorado-trail-explorer-cotrex-app-adds-new-wildfire-alert-feature-outdoor),
   [COTREX hourly](https://cpw.state.co.us/news/07142025/colorado-trail-explorer-cotrex-app-gives-wildfire-alerts-outdoor-recreationists),
   [AllTrails wildfire overlay](https://www.alltrails.com/blog/the-new-wildfire-overlay-and-what-data-shows-about-wildfire-recovery)
2. **Closures confuse people.** The PCTA says trail closures "often begin and
   end many miles from a fire", so a clear sky does not mean an open trail.
   USFS says the signed order, not the map, is authoritative. The app must
   link the order and never say "open".
   [Outside: hiking in wildfire season](https://www.outsideonline.com/adventure-travel/destinations/north-america/hiking-wildfire-season-safety?scope=anon),
   [USFS Dixie closures map](https://www.fs.usda.gov/r04/dixie/data-tools/interactive-maps/map-forest-closures)
3. **Water sources come from volunteer spreadsheets and app comments.** Their
   own maintainers warn that a spring may have dried up since the last report.
   No open API gives live water status, so the app should show where water is
   (OSM) and say that whether it is flowing is unknown.
   [PCT water report](https://docs.google.com/spreadsheets/d/1gEyz3bw__aPvNXpqqHcs7KRwmwYrTH2L0DEMW3RbHes/pub?gid=3),
   [CDTC: when the water runs out](https://cdtcoalition.org/when-the-water-runs-out-hiking-through-deserts-and-drought),
   [Oregon Hikers water thread](https://www.oregonhikers.org/forum/viewtopic.php?p=234902)
4. **Avalanche information lives in a separate app per country.** SAIS's Be
   Avalanche Aware app is now iOS-only, and a reviewer complained about its
   location prompts. Walkhighlands forum users pair it with other tools.
   US and Alpine users likewise open their centre's site separately. We did
   not reach Reddit or UKHillwalking threads directly in this pass.
   [BMC on the SAIS app](https://www.thebmc.co.uk/News.aspx?id=8273),
   [SAIS mobile app](https://www.sais.gov.uk/mobile-app),
   [Walkhighlands forum](https://www.walkhighlands.co.uk/Forum/viewtopic.php?p=479062)
5. **Lightning is paywalled or patchy.** "My Lightning Tracker requires a
   subscription to access real-time strike data". BlitzortungLive reviewers
   note that "some lightning strikes don't seem to be detected". We will not
   imitate live strike maps. Better to say when storms are likely (HI3) and
   link out for live strikes.
   [My Lightning Tracker](https://apps.apple.com/us/app/id1175031987),
   [BlitzortungLive](https://apps.apple.com/us/app/blitzortunglive/id933558423),
   [Blitzortung terms](https://www.blitzortung.org/en/compendium.php)
6. **Modelled snow is distrusted** ("microspikes or not?", hiking.md §10).
   SNOTEL snow-depth series are "shorter/noisier than SWE", so show measured
   depth as one dated reading, not a trend.
   [TSTool AWDB reference](https://opencdss.state.co.us/tstool/latest/doc-user/datastore-ref/NRCS-AWDB/NRCS-AWDB/)

## Integration plan

Each item adds one optional `sources` entry in `editions/hiking/sources.lua`
(or its own module that `Sources.list` includes), puts its parse result under
`forecast.hiking.<id>`, and gives `Hiking.cards` one card. Rating stays
weather-only. A missing source lands in `forecast.missing[id]` and the hour is
still rated. Curated identifiers (centre, zone, station) go into
`editions/hiking/peaks.lua` next to the existing `avalanche = { name, where }`,
so no source needs a geocoding call. Each item gets an offline test in
`tests/test_weather_hiking.lua`. The test feeds a trimmed, recorded answer
(saved once by hand, licence permitting, under `tests/fixtures/hiking/`) to
`parse`, then checks the card text and tone, expiry, and the URL `url(spot)`
builds. No test touches the network. Attribution lines go on the settings
"about" rows next to Open-Meteo's (PLAN.md §0.3).

| ID | Feature | Pri | Size | Service | Notes |
| --- | --- | --- | --- | --- | --- |
| SV-HI1 | US avalanche danger card | P0 | M | avalanche.org map-layer per centre | blocked on the NAC's written OK |
| SV-HI2 | Alpine avalanche danger at the summit's elevation band | P0 | M | SLF CAAML (CC BY 4.0); Euregio JSON; Varsom | Euregio and Varsom licences to confirm |
| SV-HI3 | Wildfire near the route (US) | P1 | M | NIFC/WFIGS perimeters and incidents | public domain, keyless |
| SV-HI4 | Measured snow near the top | P1 | S–M | SNOTEL AWDB; SLF IMIS | complements HI5 |
| SV-HI5 | National park alerts and closures | P1 | S | NPS API (free key) | Yosemite now; key in `Forecast.endpoints` |
| SV-HI6 | Huts, shelters and water near the top | P2 | S | OSM, precomputed | ODbL attribution |
| SV-HI7 | Lightning potential in the storm clock (central Europe) | P2 | S | Open-Meteo ICON-D2 | within §0 |
| SV-HI8 | Official links where no open API exists | P2 | S | Avalanche Canada, BRA, SAIS, USFS orders, MeteoAlarm | links only |

**SV-HI1 — US avalanche danger (P0, M).** `peaks.lua`: give `avalanche` a
`center = "SAC"` (Tahoe) and an optional `zone` name. Source `hiking-avalanche`:
`url` returns `https://api.avalanche.org/v2/public/products/map-layer/<center>`
only when there is snow. That means `snow_depth` at the top > 0 or snowfall in
the last 72 h, so summer pays nothing. `parse` finds the feature whose polygon
contains `peak.top` (a ray-cast in Lua), else the named zone. It keeps `danger`
level/label, `start_date`/`end_date`, `link` and `travel_advice`.
Card `avalanche`: "3 considerable · Central Sierra Nevada", with detail
"Sierra Avalanche Center, until 18:00, read the full forecast". Tones are
1 good, 2 fair, 3+ poor, and "no rating" is info. An expired or missing rating
says "no current rating — check the centre" and never shows the last level.
The link always opens the centre's forecast (an API guideline). File:
`editions/hiking/avalanche.lua` (parse and point-in-polygon, pure).
Fixture: one centre's map-layer GeoJSON with two zones, one of them expired.
Attribution: "Avalanche danger from <centre> via avalanche.org".

**SV-HI2 — Alpine danger at summit height (P0, M).** `peaks.lua`: give
`avalanche` a `caaml = { provider = "slf" | "euregio" | "varsom", region = "<EAWS
micro-region id>" }`, taken once from regions.avalanches.org. `url`: SLF
`aws.slf.ch/api/bulletin/caaml/…` with `headers = { Accept = "application/json" }`
(verify the format), Euregio `…/latest/EUREGIO_en_CAAMLv6.json`, or Varsom by
the top's coordinates. `parse` picks the bulletin whose `regions[].regionID`
matches. From `dangerRatings[]` it takes the rating whose `elevation`
(`lowerBound`/`upperBound`, numbers or "treeline") contains `peak.top.elevation`,
and from `avalancheProblems[]` the problem types and aspects for that band.
Bulletins with an am/pm split use `validTimePeriod`. The card reads "3
considerable above 2,200 m · wind slab, N–E", and its detail has the
publication and next-update times plus the link. This is the "right height"
idea (hiking.md complaint 1) applied to avalanches. No app in our competitor
list does it. Shares the danger-scale wording and tones with SV-HI1 in
`avalanche.lua`. Fixtures: one SLF and one Euregio CAAML v6 JSON with two
elevation bands. Attribution: "Avalanche bulletin: WSL Institute for Snow and
Avalanche Research SLF" (CC BY 4.0). Name Euregio and Varsom too, once their
licences are confirmed. Until then they stay links.

**SV-HI3 — Wildfire near the route (P1, M).** Source `hiking-fire`, for US
peaks only (`peak.nws`). `url` is an ArcGIS `query` on WFIGS current perimeters
with `geometry=<lon>,<lat>&geometryType=esriGeometryPoint&inSR=4326&distance=30&units=esriSRUnit_Kilometer&outFields=poly_IncidentName,attr_IncidentSize,attr_PercentContained,attr_ModifiedOnDateTime_dt&returnGeometry=false&f=json`.
Do not filter on relative dates: NIFC warns this loads the service, and its
fall-off rules already drop stale fires. A second, optional query on incident
locations catches new starts that have no perimeter yet. The card "fire 12 km
away" (poor if under 10 km, fair under 30 km) gives name, size, containment and
update time. It says "closures can reach far beyond the fire; check the forest
order" with SV-HI8's link. It combines with HI8's smoke. Direction and distance
come from the perimeter's centroid (`returnCentroid=true`) to the trailhead,
using `Sky` compass helpers. Fixture: a query answer with one fire and one
empty answer. Attribution: "Fire data: NIFC / WFIGS". Global hotspots via FIRMS
can follow behind a key (P2).

**SV-HI4 — Measured snow near the top (P1, S–M).** `peaks.lua`: give each peak
a `station = { kind = "snotel" | "imis", id = "<triplet or code>", name, elevation }`.
Pick a station near the top and say its height. Source `hiking-snow`: the
AWDB `/services/v1/data?stationTriplets=<id>&elements=SNWD&duration=DAILY&beginDate=<-3 d>`
or the SLF measurement API. `parse` takes the newest non-null value and the
change over 3 days. HI5's trail-conditions card gains "measured 85 cm at Echo
Peak (2,330 m), yesterday; +20 cm in 3 days", next to the modelled depth.
Readings older than 2 days are dropped. Fixture: an AWDB answer with one null
day. Attribution: "Snow: USDA NRCS SNOTEL" / "SLF IMIS (CC BY 4.0)".

**SV-HI5 — Park alerts and closures (P1, S).** `peaks.lua`: give US park peaks
`nps = "yose"`. The `apiKey`-style key is `Forecast.endpoints.npsKey`
(`X-Api-Key` header via `headers`). Without a key, `url` returns nil. Answers
are saved with the forecast, and the page asks at most hourly: the 1,000
requests an hour is shared by every user of one key. `parse` keeps alerts by
`category` (Park Closure, Danger, Caution). Card "park closure" (poor) or
"caution" (fair) gives the title and links the alert URL. Fixture: an alerts
answer with a closure and an info item. Attribution: "National Park Service".

**SV-HI6 — Huts, shelters and water (P2, S).** No live calls. A maintainer runs
one Overpass query per curated peak by hand within 3 km of the route,
selecting `tourism=alpine_hut|wilderness_hut`, `amenity=shelter|drinking_water`
and `natural=spring`. The few nearest go into `peaks.lua` as `nearby = { { kind,
name, lat, lon, elevation } }`, with a date. A "what is on the way" detail or
card says "spring 1.2 km from the top — flow not known". Test: a pure distance
and wording function, with no fixture beyond the table. Attribution: "©
OpenStreetMap contributors (ODbL)". Found places (P1 search) get nothing until
a self-hosted or paid Overpass exists.

**SV-HI7 — Lightning potential (P2, S).** For peaks inside the ICON-D2 domain
(Garda, Zermatt), extend `hiking-points` (or a small source) with
`lightning_potential` from `models=icon_d2` for 48 h. `Trail` storm clock (HI3)
then names the first hour above a threshold as "lightning likely (model
estimate)", and falls back to CAPE and lifted index elsewhere. Fixture: a
recorded hourly answer with one spike. No new attribution beyond Open-Meteo and
DWD.

**SV-HI8 — Official links where we cannot integrate (P2, S).** Add
`official` entries in `peaks.lua` for Avalanche Canada, Météo-France BRA
(until a key and SV-HI2 parsing), SAIS, MWIS, the local USFS forest-orders page
and a live lightning map. These show in the existing "official word" detail.
The test only checks the rows. This keeps the app honest where terms say "ask
first".

## Open questions before code

1. Email the National Avalanche Center (avalanche.org), ALBINA/Euregio and NVE
   for written reuse terms, and ask Avalanche Canada for consent if wanted.
2. Get one real response each from SLF CAAML (is the format JSON?), WFIGS (the
   current item URL) and Varsom. This sandbox blocks those hosts, so do it
   where the network allows them.
3. Read the AirNow Data Use Guidelines before choosing measured AQI over the
   Open-Meteo forecast.
4. These sources inherit the edition's commercial status from PLAN.md §0. Only
   the Open-Meteo-based items (SV-HI7, HI8) depend on the Open-Meteo licence.
