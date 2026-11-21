# Weather — external services to reuse

Scope: the full app (`activities/outdoors.lua` and the everyday lenses run,
cycle, dog walk, picnic and laundry), plus the shared services every edition
would reuse: warnings outside the US, radar, pollen, heat and lightning. It
builds on [weather.md](weather.md); edition-specific sources (tides, buoys,
avalanche, astronomy) are in the edition files. Research date: November 2026.
Method: web search only. In this session the agent could not fetch
meteoalarm.org, rainviewer.com or eccc-msc.github.io (the hosts were blocked),
so licence wording comes from search snippets of the official pages. Read each
licence in full before release; this is not legal advice.

Summary:
- **Warnings are the biggest everyday gap, and they can be filled for free.**
  Today only US places get warnings (NWS, W10). Three keyless JSON services
  are licensed CC BY 4.0 or similar and add Canada (ECCC GeoMet OGC API),
  Norway (MET MetAlerts) and about 38 European countries (MeteoAlarm). They
  fit the existing `sources` contract, which decodes JSON only: `forecast.lua`
  uses `response:json()`, so CAP XML feeds would need an XML reader. Germany
  can also come straight from DWD through Bright Sky.
- **The UK and Australia are the exceptions.** The Met Office warnings API
  needs a key and a request form, and BoM's free feeds are non-commercial.
  In those countries show "see the official warnings" with a link, never "no
  warnings".
- **There is no free global radar.** RainViewer's free API now serves only
  the past 2 hours, with zoom capped at 7, no nowcast, and terms that say
  "personal, educational and small-scale community use"; Flowx lost its radar
  in January 2026. Regional open radar exists: NOAA/IEM for the US, ECCC for
  Canada, DWD for Germany, and EUMETNET OPERA (CC BY 4.0 since the end of
  2025) for Europe. Radar needs a map view (engine work), so W12 stays P2.
  The cheaper win is a **radar-backed "rain nearby" line** for Germany using
  Bright Sky `/radar` (JSON, 5 min, 1 km), with no map needed.
- **Pollen outside Europe is paid only.** Google's Pollen API costs about
  $0.01 per call in 65+ countries, and Ambee is also paid. Keep pollen
  Europe-only and say so.
- **Heat, UV and lightning risk come cheaply from what we already have.**
  For heat, NWS HeatRisk is free, US-only and experimental. UV is already in
  Open-Meteo, and `lightning_potential` is too, from ICON-D2 in Central
  Europe. Blitzortung forbids commercial use and warning use: avoid it.

## Services

Licence column: CC BY = attribution required; PD = US public domain. "Comm."
says whether a paid or commercial app may use it on the stated terms.

| Service | Data | Endpoint (JSON unless noted) | Key | Licence / comm. | Coverage | Verdict |
| --- | --- | --- | --- | --- | --- | --- |
| **NWS alerts** (done) | CAP warnings | `api.weather.gov/alerts/active?point=lat,lon` (GeoJSON) | no, User-Agent | PD / yes | US | keep |
| **ECCC GeoMet OGC API** | all public alerts (warnings, watches, advisories, statements), EN/FR | `api.weather.gc.ca/collections/weather-alerts/items?f=json&bbox=…` (GeoJSON; queryables `alert_name_en`, `alert_text_en`, …) | no | free and anonymous; credit "Environment and Climate Change Canada"; MSC usage policy / yes | Canada and its waters | **integrate** |
| ECCC Datamart CAP | CAP XML files | `dd.weather.gc.ca/alerts/cap/…` | no | same | Canada | skip (XML; the OGC API is easier) |
| **MeteoAlarm** | warnings from about 38 European national services | Atom/CAP per country: `feeds.meteoalarm.org/feeds/meteoalarm-legacy-atom-<country>`; JSON: `feeds.meteoalarm.org/api/v1/warnings/feeds-<country>` (used by Home Assistant's meteoalertapi, *not documented*) | no | "terms equivalent to CC BY 4.0" plus extra rules: credit "EUMETNET – MeteoAlarm" (or the national service for one country), show the issue time, link meteoalarm.org, show the delay disclaimer, no material changes / probably yes, *confirm in writing* | Europe | **integrate** (after the licence check) |
| **MET Norway MetAlerts 2.0** | Norwegian warnings | `api.met.no/weatherapi/metalerts/2.0/current.json?lat=…&lon=…` (GeoJSON in beta; also CAP and RSS) | no, but an identifying User-Agent | CC BY 4.0 (from MET's catalogue) / yes | Norway | integrate (cheap; its point query is better than MeteoAlarm's regions) |
| **DWD warnings via Bright Sky** | DWD CAP warnings, radar | `api.brightsky.dev/alerts?lat=…&lon=…`; `/radar?lat=…&lon=…` (5-min RV composite, 1 km) | no | Bright Sky code MIT; DWD data under GeoNutzV / CC BY 4.0 with source credit / yes, *but a hobby-run instance* | Germany | integrate radar line; warnings optional (MeteoAlarm covers DE) |
| DWD direct | CAP warnings, RV/RADOLAN radar | `opendata.dwd.de/weather/alerts/cap/…` (zip/XML), GeoServer WMS `dwd:RV-Produkt` | no | GeoNutzV / CC BY 4.0 / yes | Germany | fallback if Bright Sky goes away; self-host Bright Sky |
| Met Office NSWWS API | UK warnings (Atom) | `metoffice.github.io/nswws-public-api` (docs) | yes, request form | NSWWS Open Licence; must say "Met Office Weather Warnings"; endpoints "transient" / probably | UK | **link-only now**, apply for a key (P1) |
| Met Office RSS | UK warnings per region (XML) | `weather.metoffice.gov.uk/guides/rss` | no | website terms; may stop at any time | UK | avoid (XML, no guarantee) |
| BoM warnings | CAP-AU, RSS | `bom.gov.au/rss/`, Registered User FTP | RSS no; FTP paid | RSS "personal and non-commercial use only"; paid Data Licence Agreement / **no** for free | Australia | link-only |
| JMA bosai JSON | Japanese warnings | `www.jma.go.jp/bosai/warning/data/warning/<area>.json` (*undocumented*) | no | Public Data Licence 1.0 (CC BY 4.0 compatible) / yes; area-code mapping needed | Japan | P2, link-only first |
| WMO Alert Hub / SWIC / IFRC hub | CAP from about 150 countries | `alert-hub.org`, `severeweather.wmo.int/feeds.html` (RSS/CAP XML) | no | free; each authority's own terms apply | global | link-only (XML, per-source licences) |
| WeatherKit `weatherAlerts`, `forecastNextHour` | alerts in many countries, radar nowcast | REST with a signed JWT | yes (Apple Developer) | 500k calls/month in the membership, Apple attribution / yes | many | consider later: one source for UK/AU/JP, needs a token server or native code |
| Pirate Weather | forecast, minutely, US alerts | `api.pirateweather.net` | yes, free key | about 10k calls/day free (third-party listing) / check | global, alerts US | avoid (adds nothing over NWS + Open-Meteo) |
| **RainViewer** | past radar tiles (2 h, 10 min) | `api.rainviewer.com/public/weather-maps.json` + PNG tiles | no | "personal, educational, small-scale community use", attribution / **no** for a paid app | global | avoid |
| **NOAA MRMS / NWS map services** | US radar mosaic, 5 min | `mapservices.weather.noaa.gov/eventdriven/rest/services/radar/radar_base_reflectivity/MapServer` (WMS, ArcGIS) | no | federal, "public" access, no licence field / yes (PD) | US, AK, HI, PR, Guam | integrate when a map exists |
| IEM tile cache | NEXRAD n0q TMS tiles | `mesonet.agron.iastate.edu/cache/tile.py/1.0.0/nexrad-n0q-900913/{z}/{x}/{y}.png` | no | "public domain… any lawful purpose", attribution appreciated; university service, no SLA / yes | US | integrate with the map (simplest tiles) |
| ECCC radar WMS | `RADAR_1KM_RRAI` / `RRSN`, 10 min, 3 h history | `geo.weather.gc.ca/geomet?service=WMS&layers=RADAR_1KM_RRAI` | no | MSC open data terms / yes | Canada and northern US | integrate with the map |
| EUMETNET OPERA | European radar composite | Open Radar Data API (European Weather Cloud, MeteoGate) | registration? | composites CC BY 4.0 from the 2025b release; "Données EUMETNET OPERA" / yes | Europe | P2: raw HDF5 needs server-side tiling |
| Paid radar (Tomorrow.io, Xweather, MapTiler Weather) | global tiles, nowcast | vendor tile URLs | yes | paid / yes | global | the only global option; decide with the business model |
| **NWS HeatRisk** | 0–4 heat-health risk, 7 days | `mapservices.weather.noaa.gov/experimental/rest/services/NWS_HeatRisk/ImageServer/identify?geometry=lon,lat…` | no | PD; experimental, "no guarantee" / yes | CONUS | integrate (optional, P2) |
| Open-Meteo (have) | `uv_index`, `uv_index_clear_sky`, `lightning_potential` (ICON-D2), `sunshine_duration`, `freezing_level_height` | existing forecast call (`fields`) | no / commercial key | CC BY 4.0; API non-commercial unless paid (decision 0.1) | global; lightning DE/Central Europe | reuse |
| Google Pollen API | pollen index, 5 days, 1 km | `pollen.googleapis.com/v1/forecast:lookup` | yes, billing | $0.01 per call (0–100k/month), Maps terms / yes | 65+ countries | avoid now; paid option for US pollen |
| Ambee | pollen, fire, AQ | `api.ambeedata.com` | yes | paid | global | avoid |
| Blitzortung | live lightning | none public | — | "commercial use prohibited"; not for storm warning | global | **avoid** |
| Daylight | sunrise, sunset, twilight | already computed (`sky.lua`) and Open-Meteo daily | — | — | global | nothing to add |
| Road conditions (511, state DOT, Autobahn API) | closures, ice | many APIs, one per region | mostly no | varies | regional | out of scope; weather wording only ("icy roads likely" from temp and precip) |

## What people complain about (data gaps)

1. **Warnings missing or from the wrong country.** A Weather & Radar review
   says that after an update it showed "alerts just for my country"; on the
   Windy forum, "weather warnings still don't work" for the UK, and a French
   alert was shown in the UK. Alert-only apps (Warningfy, Weather Alert
   Europe) exist because general apps leave this gap.
   ([Weather & Radar App Store][wr], [Windy UK][windy-uk], [Windy wrong alert][windy-wrong], [Warningfy][warningfy])
2. **Radar disappearing or being paywalled.** Flowx lost RainViewer radar in
   January 2026. A user asked "what is the point of an app that is designed
   to show the map with no radar data?", and others asked for refunds or
   moved to Windy or Ventusky. Windy's forum warned that its "rain radar may
   stop forever". The Apple Community has a thread titled "When is Apple
   going to fix the Weather app radar".
   ([Flowx: RainViewer gone][flowx-gone], [Flowx thread][flowx-thread], [Windy radar][windy-radar], [Apple radar][apple-radar])
3. **Next-hour rain and totals are wrong or contradict each other.** People
   describe hourly bars and the daily sentence as "completely misaligned",
   a forecast of 43" of snow when about 3" fell, and iOS 18 dropping the
   hourly amounts. Apple limits next-hour precipitation to some countries.
   ([Apple: inconsistent precipitation][apple-incons], [MacObserver][macobs], [Apple: hourly amounts][apple-hourly], [MacRumors][mr-bad], [Gizmodo AU outage][giz])
4. **Pollen is US-centric or vague.** Reviews say "only supports the
   contiguous US and the UK", "the closest station is 508 miles away" (from
   Canada), and "just a color code". A 2024 study found public pollen sites
   inaccurate in five US cities.
   ([My Pollen Forecast][mypollen], [Pollen Wise reviews][pollenwise], [Healio study][healio])
5. Earlier evidence (accuracy, ads, Dark Sky) is in [weather.md](weather.md#what-people-complain-about).

Product lesson: never present a missing source as "all clear". Name the
issuer and the issue time, and say where coverage ends. This matches PLAN.md
principle "silent failure" and the current `outdoors.lua` choice to "say
nothing rather than 'no warnings'".

## Integration plan

Shared modules go in a new `apps/Weather/services/` folder, because every
edition reuses warnings (SA10 gales, HI7, GA7). Each module exports one
`sources` entry (the README contract: `id`, `url(spot, Forecast)` returning
nil outside its coverage, `optional = true`, `parse(json, forecast)`,
`headers`) and writes the shared `forecast.warnings` shape
`{ event, severity, sender, ends, issued, link }`. `outdoors.warningCards`
already turns that shape into cards. Add the hosts to `Forecast.endpoints`.
Each module gets a recorded response under `tests/fixtures/weather/` and a
case in `tests/test_weather_alerts.lua` (or a new
`tests/test_weather_services.lua`, added to `APP_LUA_TESTS`). Tests stay
offline: parse the fixture into a `weather_fixture.lua` forecast and assert
the cards, the expiry filter, coverage (`url` returns nil outside) and that
an answer missing its list fails the source instead of becoming "no
warnings".

| ID | Item | Pri | Size | Files | Maps to | Attribution |
| --- | --- | --- | --- | --- | --- | --- |
| SV-W1 | **Warning registry**: move NWS out of `outdoors.lua` into `services/warnings.lua` (a list of providers, each with a bbox or country test and its own source), plus `warnings.coverage(spot)`, which returns `"covered"`, `"link"` (with a URL) or `"none"`. The card says "official warnings: see Met Office" in link countries. | P0 | S–M | `services/warnings.lua`, `services/nws.lua`, `activities/outdoors.lua` (`sources = Warnings.sources()`), editions reuse | `sources`, `cards` (`warningCards`), `Alerts` severe kind later | per provider, shown in the card's `detail` |
| SV-W2 | **Canada (ECCC)**: one GeoJSON items request with a small bbox around the spot, then a point-in-polygon test in Lua. Map `alert_name_en` to `event` and the alert type (warning/watch/advisory) to `severity`. Use `alert_name_fr` when the locale is French. | P0 | S | `services/eccc.lua`, fixture `eccc-alerts.json` | `sources` | "Environment and Climate Change Canada" |
| SV-W3 | **MeteoAlarm (Europe)**: first email meteoalarm@geosphere.at to confirm in writing that a paid app may use it and that showing the warning type, level and times counts as "no material change". Then one country feed per spot (country from geocoding `country_code`). Match the spot to EMMA/NUTS regions with a bundled lookup, or use the CAP polygon when present. Map the awareness level (yellow/orange/red) to `severity`, and keep the issue time and a link to meteoalarm.org. | P1 | M–L | `services/meteoalarm.lua`, `services/data/emma-regions.lua`, fixtures for DE and FR | `sources` (optional; `url` nil outside Europe) | "EUMETNET – MeteoAlarm" or the national service; issue time; link; delay disclaimer on the settings page |
| SV-W4 | **Norway (MetAlerts)**: query by point, which beats MeteoAlarm's regions there; then let MeteoAlarm skip NO. | P1 | S | `services/metno.lua`, fixture | `sources` | "MET Norway", CC BY 4.0 |
| SV-W5 | **Radar-backed "rain nearby" line (Germany)**: ask Bright Sky `/radar` for the point with a small `distance`, and read the latest frames: "rain 8 km west, moving closer" or "radar: dry within 10 km". It goes in the outdoors next-hour card beside the `minutely_15` model line, and only where radar exists. Nowcasting stays the model's job: never claim radar minutes. | P1 | M | `services/brightsky.lua`, `activities/outdoors.lua` (next-hour card), fixture `brightsky-radar.json` | `sources` (optional), `cards` | "Radar: Deutscher Wetterdienst via Bright Sky" |
| SV-W6 | **HeatRisk (US)** and heat wording: ImageServer `identify` at the point, a 0–4 level per day for 7 days, read by outdoors, run and dog walk ("too hot for paws", "run before 8:00"). Elsewhere use the existing apparent-temperature rules. Experimental, so optional and silent when it fails. | P2 | S–M | `services/heatrisk.lua`, `activities/run.lua`, `activities/dogwalk.lua` | `sources`, `cards`, rate caps | "NWS HeatRisk (experimental)" |
| SV-W7 | **Lightning risk without Blitzortung**: add `lightning_potential` to `fields` (Central Europe only, ICON-D2) and combine it with `cape` and codes 95–99 for a "thunder possible 15:00–18:00" line in outdoors, picnic and cycle. Shared with the hiking storm clock (HI3). | P2 | S | `activities/outdoors.lua` `fields`, `score.lua` helper | `fields`, `cards` | Open-Meteo / DWD ICON (already credited) |
| SV-W8 | **Radar map (W12)**, once a map view exists: a tile source per region (IEM n0q for the US, ECCC WMS, DWD WMS, later OPERA) behind one `services/radar.lua`. Elsewhere, no radar or a paid provider. | P2 | L | `services/radar.lua`, a new page, engine map/tile view | a page, not `sources` | per tile layer, on the map |

Not planned: a paid global pollen API (revisit if US users ask); Met Office
NSWWS until the key arrives (then it is an S task in the SV-W1 registry);
BoM, JMA and WMO hubs as link-only entries in `warnings.coverage`.

Request volume: one extra small request per place per refresh, and only for
the lead activity and the open page (the existing optional-source rule).
Warnings change slowly. A 10-minute cache is fine; a WebSub push from
MeteoAlarm would need a server.

## Sources

- Warnings: [ECCC alerts readme][eccc-readme], [GeoMet OGC API weather-alerts][eccc-api], [queryables][eccc-q], [MSC open data licence][msc-od]; [MeteoAlarm terms][ma-terms], [Redistribution Hub][ma-hub], [feed example][ma-feed], [GitHub note on unclear terms][ma-gh]; [MET MetAlerts 2.0][metno], [MetAlerts GeoJSON][metno-gj], [MET catalogue][metno-cat]; [DWD copyright][dwd-copy], [DWD README][dwd-readme], [Bright Sky][brightsky], [Bright Sky radar][bs-radar]; [Met Office NSWWS API][mo-api], [NSWWS Open Licence][mo-lic], [Met Office RSS][mo-rss]; [BoM RSS][bom-rss], [BoM data feeds][bom-feeds]; [JMA copyright][jma]; [WMO SWIC feeds][swic], [WMO Alert Hub][wmo-hub].
- Radar: [RainViewer API][rv-api], [RainViewer transition FAQ][rv-faq], [LibreWXR on free-tier limits][librewxr]; [NWS radar map service][nws-radar], [IEM OGC/tiles][iem-ogc], [IEM disclaimer][iem-disc]; [ECCC radar on GeoMet][eccc-radar]; [DWD RADOLAN licence (wradlib)][wradlib]; [OPERA open radar data][opera], [OPERA 2025b slides][opera-slides].
- Health and other: [NWS HeatRisk data][heatrisk], [HeatRisk ImageServer][heatrisk-is]; [Google Pollen billing][gpollen], [Pollen coverage][gpollen-ov]; [Blitzortung compendium][blitz], [Blitzortung contact][blitz-c]; [Open-Meteo DWD API][om-dwd]; [Pirate Weather pricing (third party)][pirate].

[wr]: https://apps.apple.com/ro/app/weather-radar/id545993260
[windy-uk]: https://community.windy.com/topic/29126/no-weather-warnings-for-uk
[windy-wrong]: https://community.windy.com/topic/27994/strange-severe-weather-warning-notification
[warningfy]: https://apps.apple.com/us/app/-/id1589764767
[flowx-gone]: https://forum.flowx.io/t/rainviewer-is-officially-gone/2138
[flowx-thread]: https://forum.flowx.io/t/rainviewer-will-discontinue-jan-2026/1993
[windy-radar]: https://community.windy.com/topic/40878/windy-s-rain-radar-may-stop-forever/
[apple-radar]: https://discussions.apple.com/thread/255614513
[apple-incons]: https://discussions.apple.com/thread/256201417
[macobs]: https://www.macobserver.com/?p=440573
[apple-hourly]: https://discussions.apple.com/thread/255935833
[mr-bad]: https://forums.macrumors.com/threads/why-is-apples-weather-app-so-bad-seriously.2430858/page-3
[giz]: https://gizmodo.com.au/?p=1872038
[mypollen]: https://apps.apple.com/us/app/id1244428929
[pollenwise]: https://justuseapp.com/en/app/1474856970/pollen-wise/reviews
[healio]: https://cm.healio.com/news/allergy-asthma/20240510/public-websites-show-low-accuracy-in-predicting-pollen-counts-in-five-us-cities
[eccc-readme]: https://eccc-msc.github.io/open-data/msc-data/alerts/readme_alerts_en
[eccc-api]: https://api.weather.gc.ca/collections/weather-alerts
[eccc-q]: https://api.weather.gc.ca/collections/weather-alerts/queryables?f=html
[msc-od]: https://eccc-msc.github.io/open-data/readme_en
[ma-terms]: https://www.meteoalarm.org/en/live/page/terms-and-conditions
[ma-hub]: https://meteoalarm.org/en/page/redistribution-hub
[ma-feed]: https://feeds.meteoalarm.org/feeds/meteoalarm-legacy-atom-andorra
[ma-gh]: https://github.com/vincenzofiore/rootology/issues/136
[metno]: https://api.met.no/weatherapi/metalerts/2.0/
[metno-gj]: https://docs.api.met.no/doc/metalerts/geojson
[metno-cat]: https://api.met.no/catalog.xml
[dwd-copy]: https://beobachtungen.dwd.de/dwd_cas/copyright_en
[dwd-readme]: https://opendata.dwd.de/README.txt
[brightsky]: https://github.com/jdemaeyer/brightsky
[bs-radar]: https://github.com/jdemaeyer/brightsky/issues/144
[mo-api]: https://metoffice.github.io/nswws-public-api
[mo-lic]: https://www.metoffice.gov.uk/api/assets/file/NSWWSOpenLicencepdf
[mo-rss]: https://weather.metoffice.gov.uk/guides/rss
[bom-rss]: https://www.bom.gov.au/rss/
[bom-feeds]: https://www.bom.gov.au/catalogue/data-feeds.shtml
[jma]: https://www.jma.go.jp/jma/en/copyright.html
[swic]: https://severeweather.wmo.int/feeds.html
[wmo-hub]: https://wmo.int/node/25777
[rv-api]: https://rainviewer.com/api.html
[rv-faq]: https://rainviewer.com/api/transition-faq.html
[librewxr]: https://github.com/JoshuaKimsey/LibreWXR
[nws-radar]: https://mapservices.weather.noaa.gov/eventdriven/rest/services/radar/radar_base_reflectivity/MapServer
[iem-ogc]: https://mesonet.agron.iastate.edu/ogc/
[iem-disc]: https://mesonet.agron.iastate.edu/disclaimer.php
[eccc-radar]: https://eccc-msc.github.io/open-data/msc-data/obs_radar/readme_radar_geomet_en
[wradlib]: https://docs.wradlib.org/projects/radolan/en/latest/data/ATTRIBUTION.html
[opera]: https://eumetnet.github.io/openradardata-documentation/1-ORD-API-overview/
[opera-slides]: https://gebrada.upc.es/resources/docs/inline/2.1-vonlerber.pdf
[heatrisk]: https://www.wpc.ncep.noaa.gov/heatrisk/data.html
[heatrisk-is]: https://mapservices.weather.noaa.gov/experimental/rest/services/NWS_HeatRisk/ImageServer
[gpollen]: https://developers.google.com/maps/documentation/pollen/usage-and-billing
[gpollen-ov]: https://developers.google.com/maps/documentation/pollen/overview
[blitz]: https://www.blitzortung.org/en/compendium.php
[blitz-c]: https://www.blitzortung.org/contact.php
[om-dwd]: https://open-meteo.com/en/docs/dwd-api
[pirate]: https://apispine.com/pirate-weather/pricing
