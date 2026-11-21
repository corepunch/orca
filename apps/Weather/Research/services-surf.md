# Surfers' Weather — existing services to integrate

Builds on [surf.md](surf.md) (competitors, complaints, stories). Researched
2026-11-21 with web search. This session could not fetch
`environment.data.gov.uk`, `mapservices.weather.noaa.gov` or NDBC directly.
Endpoints marked *verify* come from search results or third-party code, not
from a live request.

## Summary

The forecast side is done (Open-Meteo marine, GFS Wave outlook, tides,
breaks, log). What surfers still open a second app for is **what is
happening now** and **whether the water is safe**: a live buoy reading
("has the swell arrived?"), the measured water temperature, rip-current and
high-surf warnings, and official water-quality alerts after rain or sewage
spills. Most of this already exists as free public data. US federal data
(NWS, NDBC, CO-OPS) is public domain and keyless. Ireland's EPA (CC BY 4.0),
England's Environment Agency (OGL) and NSW Beachwatch (CC BY 3.0 AU) publish
official bathing-water status. The best first step is to reuse the app's
existing `api.weather.gov` alert source for beach hazards, then add a buoy
card, measured water temperature and official water-quality alerts. Cams,
shark alerts and the Surfers Against Sewage sewage alerts have no usable open
API: link to them, do not embed. Avoid Stormglass, Windy Webcams and
Surfline/MSW for data. Copernicus Marine is free but needs an account and
its Python toolbox, which does not suit an on-device app.

**Foundation constraint found.** `Forecast.networkFetch` (forecast.lua)
decodes every source answer as JSON. NDBC `realtime2` files are plain text,
so the plan uses JSON routes (ERDDAP, CO-OPS `format=json`). A source option
such as `format = "text"` would allow the fresher NDBC files. Ask for it
under "Requests to the foundation" in PLAN.md if SV-SU2 needs it.

## Services

| Service | Data | Endpoint | Key? | Licence / commercial | Coverage | Verdict |
| --- | --- | --- | --- | --- | --- | --- |
| **NWS alerts** (api.weather.gov) | Rip Current Statement, Beach Hazards Statement, High Surf Advisory/Warning, Coastal Flood | `https://api.weather.gov/alerts/active?point=LAT,LON` (GeoJSON; already used in `activities/outdoors.lua`) | No; User-Agent required, the app already sends one | US government work, public domain; commercial OK. Rate limit not published ("generous"), retry after ~5 s ([NWS API][nws-api]) | US coasts, Great Lakes, territories | **Integrate (P0)** |
| **NWS Surf Zone Forecast** (SRF) | Daily rip-current risk (low/moderate/high), surf height, water temp, UV, per beach zone, 2 days | Text product: `api.weather.gov/products/types/SRF/locations/{WFO}` (*verify*); also GIS GeoJSON per office, e.g. `weather.gov/source/okx/beachforecast/OKX_GIS_day1.json` ([example][nws-gis]) | No | Public domain | Coastal WFOs, in beach season only ([NWS ILM][nws-ilm]) | Integrate later (P2): text varies by office |
| NWS NWPS rip probability | Hourly rip-current probability (%), total water level | NOMADS text/GRIB2 per WFO ([SCN 25-66][nwps-25], [SCN 26-91][nwps-26]) | No | Public domain | 9–11 pilot WFOs, "experimental", known high bias ([NWPS][nwps]) | Avoid for now (GRIB, experimental) |
| **NDBC buoys** (realtime2) | WVHT, DPD, APD, MWD, wind, WTMP every 10–60 min; `.spec` swell/wind-sea split | `https://www.ndbc.noaa.gov/data/realtime2/{ID}.txt` and `.spec` (text, 45 days) ([NDBC][ndbc-rt]) | No | Public domain; NDBC asks for "minimal" retrievals | US + partner buoys worldwide | **Integrate (P0)** via JSON route below; raw text needs the foundation option |
| NDBC via NOAA CoastWatch ERDDAP | Same standard met. fields as JSON | `https://coastwatch.pfeg.noaa.gov/erddap/tabledap/cwwcNDBCMet.json?station,time,wvht,dpd,apd,mwd,wtmp&station="46026"&time>=now-6hours` (*verify* name and lag) | No | Public domain | As NDBC | Integrate (JSON route for SV-SU2) |
| **CDIP buoys** (Scripps) | Directional waves, peak period/direction, SST; nearshore buoys near breaks | THREDDS/OPeNDAP and ERDDAP `erddap.cdip.ucsd.edu` (tabledap `.json`) ([CDIP ERDDAP][cdip-erddap]); also mirrored as NDBC 46xxx IDs | No | Public use; cite "Coastal Data Information Program, SIO" ([NCEI][cdip-ncei]); no explicit commercial clause found | ~70–90 buoys, mostly California, Hawaii, Pacific | Integrate through the NDBC IDs; note funding risk ([KQED/NorCal][cdip-cut]) |
| **NOAA CO-OPS** | Measured water temperature (6-min), also exact tide predictions | `api.tidesandcurrents.noaa.gov/api/prod/datagetter?product=water_temperature&station=ID&date=latest&units=metric&time_zone=gmt&format=json&application=…` (host already in `Forecast.endpoints.noaaTides`) | No | Public domain | US tide stations | **Integrate (P0)** |
| Open-Meteo marine (have) | SST, `ocean_current_velocity/direction`, UV (forecast API) | marine endpoint already used | Free non-commercial; key for commercial (PLAN 0.1) | As today | Global, modelled | Extend (current, UV in surf card) |
| **Ireland EPA Bathing Water API** | Active bathing restrictions (sewage, algae), locations, results | `https://data.epa.ie/bw/api/v1/alerts`, Swagger at `/bw/swagger/v1/swagger.json` ([data.gov.ie][ie-api]) | No | CC BY 4.0, commercial OK with attribution | Irish designated bathing waters | **Integrate (P1)** |
| **England EA Bathing Water Quality API** | Daily pollution-risk forecasts (171 of 424 sites), incidents, classifications, sample results | Linked-data API at `environment.data.gov.uk/bwq/` (JSON/CSV; exact risk-prediction path *verify*) ([EA API][ea-api]; [Defra blog][ea-blog]) | No | Open Government Licence ([api.gov.uk][ea-ogl]); commercial OK with attribution | England, season May–Sep | **Integrate (P1)** |
| Wales / Scotland / N. Ireland bathing waters | Similar profiles and forecasts | NRW `environment.data.gov.uk/wales/bathing-waters/`; SEPA site | — | OGL (Wales); others not checked | — | Later, same module |
| **NSW Beachwatch** | Daily pollution forecast per Sydney beach ("likely/possible/unlikely"), results | JSON/GeoJSON used by the community `beachwatch` Python wrapper ([PyPI][bw-pypi]); official bulletins on data.gov.au ([dataset][bw-data]) | No | CC BY 3.0 AU ([data.gov.au][bw-data]) | Sydney, Hunter, Illawarra, NSW | Integrate (P1) once the endpoint is confirmed |
| EEA Bathing Water Directive | Yearly classification (excellent…poor) for ~22,000 EU sites | Excel downloads and EEA Datahub ([EEA][eea]) | No | EEA reuse with attribution (terms not confirmed) | EU + | Bundle a yearly table (P2), no live data |
| Surfrider Blue Water Task Force | Volunteer bacteria samples | CSV/JSON download, some via MARACOOS ERDDAP ([data.gov][bwtf-dg]; [Surfrider][bwtf]) | No | Varies per record (CC0 to none) | US sites, sparse and weekly | Link only |
| Swim Guide (Swim Drink Fish) | Aggregated beach status | Open data *standard*, API terms not found ([Water Canada][swimguide]) | ? | Unknown | North America, some EU | Avoid until terms are known; link only |
| SAS Safer Seas & Rivers | Real-time sewage overflow alerts (water-company sensors) | Internal API only ([3 Sided Cube][sas-api]) | — | Not open | UK | **Link only**; underlying EA "storm overflow" open data may be usable later |
| UK CCO wave buoys | Live wave height/period/direction, SST, ~40 UK coastal buoys | `coastalmonitoring.org/observations/waves/latest?key=…` ([CCO API][cco]) | **Yes**, key tied to a Referer domain | OGL; not quality-controlled; attribution required | England/Wales | Integrate later (P2) if a key works from an app |
| Ireland Marine Institute / SmartBay | Irish wave buoys (M-buoys) | ERDDAP `erddap.marine.ie` (*verify*) ([data.gov.ie][smartbay]) | No | CC BY 4.0 (typical for data.gov.ie; *verify*) | Ireland | Integrate later (P2) with SV-SU2's ERDDAP parser |
| Copernicus Marine (CMEMS) | Currents, SST, waves, global grids | Toolbox (Python), OPeNDAP/ARCO; free account ([help][cmems-api]) | Account | Free incl. commercial with credit ([licence][cmems-lic]; [Mercator][mercator]) | Global | Avoid on device (no simple keyless REST); Open-Meteo already resells similar fields |
| SharkSmart WA / NSW SharkSmart | Tagged-shark detections, sightings, beach closures | App and web map only ([WA gov][sharkwa]) | — | Not published | WA, NSW | **Link only** |
| Windy Webcams API v3 | Webcam images near a point | `api.windy.com/webcams/api/v3/...` ([docs][windy-docs]) | Yes | Free tier image URLs expire after 10 min; terms "planned for update", commercial use unclear ([terms][windy-terms]) | Global, few true surf cams | Avoid; link to the break's own cam page instead |
| Stormglass | Aggregated marine forecasts | `api.stormglass.io` | Yes | Free tier 10 calls/day, **no commercial use**; paid from ~€19–129/month ([FAQ][sg-faq]) | Global | Avoid (duplicates Open-Meteo) |
| Surfline / MSW | Cams, forecasts | No public API; MSW closed 2023; unofficial use blocked ([Tidbyt][tidbyt]) | — | Not allowed | — | Avoid; link only |

## Complaint evidence: data gaps these services fill

- **Live buoys.** Surfers read buoys to see whether the forecast swell is
  actually arriving ([The Inertia][inertia-buoy]; [Stormsurf FAQ][stormsurf]).
  Apps show "no live wave height" where no buoy is near ([Snoflo][snoflo]).
  Paddlers report NOAA buoys going silent ([paddling.com][paddling]), and
  CDIP's ~90 buoys lost federal funding in 2025, with surf forecasts expected
  to get worse ([NorCal Public Media][cdip-cut]; [Inside Climate News][icn]).
  So the buoy card must handle stale or missing stations gracefully.
- **Forecasts that miss.** "Stuck trusting inaccurate forecasts or learning
  how to forecast themselves" ([Golden Gate Xpress][ggx]); surf.md §3. A
  buoy-against-forecast line answers "is it bigger than forecast?".
- **Water quality after rain.** In a 654-surfer San Diego study, illness
  rates rose after rain and fell to near baseline after about 3 days
  ([PMC5860265][pmc]; [CA Sea Grant][seagrant]). Health departments do not
  always post signs after rain ([Surfrider][sf-rain]). Rules differ by
  place: 72 h in California, 24 h (72 h near river mouths) in NSW
  ([Stab][stab]). Our SU8 runoff heuristic is generic. Official status
  (EPA IE, EA, Beachwatch) is better where it exists.
- **Sewage spills (UK).** Safer Seas reviewers value knowing about
  discharges before entering the sea ([App Store][sas-app]). SAS itself
  admits over-alerting and that each water company defines alerts
  differently ([SAS][sas-maps]). So link to it and do not copy its alerts.
- **Rip currents.** Only 44 % of US adults know rips are the main cause of
  beach drownings ([NWS social science][nws-ss]). Infrequent beachgoers, like
  our persona Jonas, have the biggest gap between the forecast and what they
  see ([NHESS][nhess]). Stories 77–78 in surf.md are still missing.
- **Water temperature and wetsuit.** Story 73 is met with model SST, which
  can be several degrees off near upwelling coasts. A measured CO-OPS or
  buoy WTMP is what surfers trust (*no direct complaint source found*).
- **Cams.** Cams are down for weeks, removed or paywalled (surf.md §4). We
  will not host cams (surf.md, "Not recommended"). A link to a free local
  cam is the honest answer.

Reddit threads were not read directly; complaints come from secondary
sources.

## Integration plan

Every item is an optional `source` in `Surf.sources` (fetched only for the
hub's lead activity and the open surf page; failure lands in
`forecast.missing[id]`), plus a `card` in `Surf.cards`. New per-break IDs go
in `editions/surf/breaks.lua`, which stays the one place for surf facts.
Tests go in `tests/test_weather_surf.lua` with recorded JSON fixtures under
`tests/fixtures/weather/surf/` (new folder), answered through
`F.network { answers = { [pattern] = fixture } }`. Every card names its
source in `detail`, and the settings page credits gain one line each.

| ID | Item | Pri | Size | Files | Extension point | Offline test | Attribution |
| --- | --- | --- | --- | --- | --- | --- | --- |
| SV-SU1 | **Beach hazard warnings**: Rip Current Statement, Beach Hazards Statement, High Surf Advisory/Warning, with the end time. A rip warning also adds "surf near a lifeguard" for beginners (stories 77–78) | P0 | S | new `editions/surf/hazards.lua`; `activities/surf.lua` | Reuse the `Weather.alertSource` pattern from `outdoors.lua` (keep `nwsCovers`); `parse` filters marine/beach `event`s into `forecast.beachHazards`; card `hazard`, tone `poor`, `rank` first, the hub shows it; it caps the hour's verdict while in force | Recorded `alerts/active?point=` GeoJSON with one Rip Current Statement and one unrelated alert → one card; empty `features` → no card; missing `features` → `missing["surf-hazards"]` | "Warnings: U.S. National Weather Service" |
| SV-SU2 | **Live buoy card**: "Buoy 46026, 23 min ago: 1.8 m at 14 s from WNW; 0.3 m bigger than forecast", and "swell arriving" when the period jumps | P0 | M | new `editions/surf/buoys.lua`; `breaks.lua` (`buoy = "46026"` per break; the nearest station when unset is a later step) | Source `surf-buoy` → CoastWatch ERDDAP `cwwcNDBCMet.json` (last 6 h). `parse` reads the `table.columnNames/rows` layout into `forecast.buoy`; card compares with `forecast.now` swell. Older than 3 h → "no recent reading", tone `info`. If ERDDAP lags, ask for a text source option and read `realtime2/{ID}.txt` | Recorded ERDDAP JSON (normal; all-NaN `wvht`; 5 h old); check face text, delta wording, stale handling | "Buoy data: NOAA National Data Buoy Center" (+ "CDIP, Scripps Institution of Oceanography" for CDIP stations) |
| SV-SU3 | **Measured water temperature** for the wetsuit row and card: the CO-OPS station or buoy WTMP when fresh, else model SST labelled "model" | P0 | S | `editions/surf/buoys.lua` (or `watertemp.lua`); `breaks.lua` (`coops = "9414290"`); `activities/surf.lua` `wetsuit()` row | Source `surf-water` → `Forecast.url("noaaTides", "product=water_temperature&date=latest&format=json…")`; `parse` sets `forecast.waterTemp = { value, epoch, station }`; the row prefers it | Recorded CO-OPS JSON (`data[1].v`) and an `error` answer → model SST still shown | "Water temperature: NOAA CO-OPS" |
| SV-SU4 | **Official water-quality status** replaces the runoff guess where one exists: Ireland EPA alerts, England EA pollution-risk forecast, NSW Beachwatch forecast. The SU8 rain rule stays as the fallback, worded "no official reading here" | P1 | M | new `editions/surf/waterquality.lua` with one small adapter per country; `breaks.lua` (`bathingWater = { ie = "IESWBWC090_0000_0300" }` style IDs) | Three optional sources whose `url` returns nil unless the break has that country's ID; `parse` normalises to `forecast.waterQuality = { status = "advice against bathing"/"risk"/"ok", cause, until, source }`; the existing `water` card reads it first | One recorded fixture per adapter (an EPA alert for the break; an EA "increased risk" prediction; a Beachwatch "likely" forecast) plus a no-match fixture | "Bathing water: EPA Ireland (CC BY 4.0)", "Contains Environment Agency data © Crown copyright (OGL v3)", "Beachwatch, NSW Government (CC BY 3.0 AU)" |
| SV-SU5 | **Comfort card**: UV and current strength in the surf card, not only the hub (stories 74–75); "strong current along the beach" from Open-Meteo `ocean_current_velocity` | P1 | S | `activities/surf.lua`, `waves.lua` (`Waves.COLUMNS` + current columns) | `Surf.fields.marine` gains `ocean_current_velocity`, `ocean_current_direction`; card `comfort`, `hub = false` | `F.marine()` with current 0.9 m/s → card says "strong"; UV 8 at noon → "sunscreen" | covered by Open-Meteo credit |
| SV-SU6 | **Links per break**: local cam, Safer Seas (UK), SharkSmart (WA/NSW), surf-zone forecast page, lifeguard info. No data copied | P1 | S | `breaks.lua` (`links = { cam = url, … }`); `activities/surf.lua` `act()` | Card `actions` (at most three) opening an external URL; nothing is fetched | `F.cards` shows the links for a break that has them and none otherwise | Name the site in the link text |
| SV-SU7 | **NWS Surf Zone Forecast** rip risk (low/moderate/high) for the day, even without a statement in force | P2 | M | `editions/surf/hazards.lua` | Source `surf-srf`: latest SRF product for the WFO (from `api.weather.gov/points` → `cwa`), parse the zone's "Rip Current Risk" line; card in `hazard` | Two recorded SRF texts from different offices (layouts differ) | "Surf zone forecast: NWS" |
| SV-SU8 | **More buoys outside the US**: UK CCO (needs a key; check the Referer rule works from an app), Irish Marine Institute ERDDAP, reusing SV-SU2's ERDDAP parser | P2 | M | `editions/surf/buoys.lua`; `Forecast.endpoints` gets `ccoKey` only if approved | Same `surf-buoy` source, chosen by the break's `buoy` prefix (`ndbc:`, `cco:`, `ie:`) | Recorded CCO and ERDDAP answers | "Contains data from the National Network of Regional Coastal Monitoring Programmes (OGL)"; "Marine Institute Ireland" |

Order: SV-SU1 and SV-SU3 first (keyless, public domain, small, reuse
code we already ship). SV-SU2 next (M because of station mapping and stale
data). Then SV-SU4 per country, starting with Ireland (JSON confirmed, CC BY).

### Open questions and licence honesty

- Public domain covers NWS, NDBC and CO-OPS data. It does not cover NOAA
  logos: do not use them.
- CDIP: no written terms found. The citation is requested and US federal
  funding is uncertain. Treat CDIP stations as "may disappear".
- EA: the API is OGL, but the pollution-risk path was not fetched. Check
  the JSON shape and season handling (no forecasts Oct–Apr) before building.
- Beachwatch: the only machine-readable endpoint found is the one used by a
  community wrapper. Confirm with NSW DCCEEW before shipping.
- Windy Webcams and Stormglass terms do not suit a paid app. SAS, SharkSmart
  and Swim Guide have no open API. Ask SAS about a data partnership if UK
  sewage alerts become a priority.
- All sources sit on top of the Open-Meteo licence decision (PLAN 0.1);
  none of them removes it.

[nws-api]: https://www.weather.gov/documentation/services-web-api
[nws-gis]: https://www.weather.gov/source/okx/beachforecast/OKX_GIS_day1.json
[nws-ilm]: https://www.weather.gov/ilm/surfmatricesinfo
[nwps]: https://polar.ncep.noaa.gov/nwps
[nwps-25]: https://www.weather.gov/media/notification/pdf_2025/scn25-66_nwps_1.4.20.pdf
[nwps-26]: https://www.weather.gov/media/notification/pdf_2026/scn26-91_nwps_1.5.0.pdf
[ndbc-rt]: https://www.ndbc.noaa.gov/faq/rt_data_access.shtml
[cdip-erddap]: https://erddap.cdip.ucsd.edu/erddap/tabledap/index.html
[cdip-ncei]: https://www.ncei.noaa.gov/archive/accession/SIO-CDIP-netCDF
[cdip-cut]: https://www.norcalpublicmedia.org/2025072598703/news-feed/tktktktk-2
[icn]: https://insideclimatenews.org/news/08122025/noaa-integrated-ocean-observing-system-funding/
[ie-api]: https://data.gov.ie/en_GB/dataset/beaches-api
[ea-api]: https://environment.data.gov.uk/bwq/
[ea-blog]: https://environmentagency.blog.gov.uk/2024/02/23/bathing-water-classifications-and-short-term-pollution/
[ea-ogl]: https://api.gov.uk/ea/bathing-water/
[bw-pypi]: https://pypi.org/project/beachwatch
[bw-data]: https://data.gov.au/data/dataset/nsw-beachwatch
[eea]: https://sdi.eea.europa.eu/catalogue/srv/api/records/5d9a4d94-511a-486d-afbb-4f01e5c73e23
[bwtf]: https://www.surfrider.org/news/2019-clean-water-report-released
[bwtf-dg]: https://catalog.data.gov/dataset/surfrider-station-731
[swimguide]: https://www.watercanada.net/swim-guide-releases-open-data-standard-for-recreational-water-quality/
[sas-api]: https://3sidedcube.com/blog/safer-seas-smarter-data-improving-the-ssrs-app-experience
[cco]: https://www.coastalmonitoring.org/ccoresources/api/
[smartbay]: https://data.gov.ie/en_GB/dataset/smartbay-ireland-cork-buoy-wave/resource/c63c114e-16b3-437a-89cb-3519fc5d1b92
[cmems-api]: https://help.marine.copernicus.eu/en/articles/4794731-which-programmatic-access-services-are-available
[cmems-lic]: https://marine.copernicus.eu/user-corner/service-commitments-and-licence
[mercator]: https://mercator-ocean.eu/en/?p=9340
[sharkwa]: https://www.wa.gov.au/service/maritime-services/search-and-rescue/view-shark-activity-sharksmart
[windy-docs]: https://api.windy.com/webcams/docs
[windy-terms]: https://api.windy.com/webcams/terms
[sg-faq]: https://stormglass.io/faq/
[tidbyt]: https://discuss.tidbyt.com/t/procedure-for-api-blocking-tidbyt-servers/5107
[inertia-buoy]: https://www.theinertia.com/surf/enhance-your-surf-sessions-learn-to-be-a-buoy-reading-badass/
[stormsurf]: https://www.stormsurf.com/buoy/mht/faqs.html
[snoflo]: https://snoflo.org/surfing/gulf-coast
[paddling]: https://forums.paddling.com/discussion/comment/2971016/
[ggx]: https://goldengatexpress.org/113424/opinion/opinion-surfline-sucks-now/
[pmc]: https://pmc.ncbi.nlm.nih.gov/articles/PMC5860265
[seagrant]: https://caseagrant.ucsd.edu/news/4-reasons-72-hour-beach-advisory
[sf-rain]: https://www.surfrider.org/news/surfing-after-a-rain-what-are-the-risks
[stab]: https://stabmag.com/features/should-you-surf-after-it-rains/
[sas-app]: https://apps.apple.com/gb/app/safer-seas-rivers-service/id1458786821
[sas-maps]: https://sas.org.uk/updates/the-problem-with-real-time-water-company-sewage-maps
[nws-ss]: https://vlab.noaa.gov/web/nws-social-science/-/rip-current
[nhess]: https://nhess.copernicus.org/preprints/nhess-2017-16/nhess-2017-16-manuscript-version4.pdf
