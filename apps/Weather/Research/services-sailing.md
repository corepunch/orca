# Sailors' Weather — external services

Researched November 2026 for the `sailing` edition. The goal is "everything a sailor or kiter needs in one app", built from services that already exist. It builds on [sailing.md](sailing.md), whose competitors and complaints are not repeated here.

**Caveat on verification.** This session's network blocked every candidate host (NDBC, api.weather.gov, aviationweather.gov, api.pioupiou.fr, openwindmap.org, MeteoAlarm). Endpoints and licences come from web search over the providers' own pages plus a few secondary sources, which are marked. Each item below needs a live recording of a real answer before it is built; that recording is also the test fixture.

## Summary

- **The two gaps that matter are SA9 (is the wind on right now?) and SA10 (is there an official warning?).** Both have free, keyless sources that allow commercial use.
- **Warnings:**
  - US: [api.weather.gov alerts](https://www.weather.gov/documentation/services-web-API). Public domain, JSON. The marine events are Small Craft Advisory, Gale, Storm and Hurricane Force Wind Warning, and Special Marine Warning. The hiking edition already calls it (`editions/hiking/sources.lua`, `alerts`), so the pattern exists.
  - Europe: [MeteoAlarm](https://www.meteoalarm.org/en/live/page/terms-and-conditions) under terms equivalent to CC BY 4.0, with strict attribution rules.
- **Live wind:**
  - US coasts: NOAA NDBC buoys and C-MAN stations. Public domain, but published as plain text.
  - Worldwide airports: METAR from aviationweather.gov. Public domain, JSON, keyless.
  - Alps, France and Europe's lakes and coasts: OpenWindMap / Pioupiou, a crowd-sourced open network. Its Community Licence is free and allows commercial use with attribution.
  - Denmark: DMI. CC BY 4.0, but it needs a free key.
  - Netherlands and North Sea: KNMI. CC BY 4.0, keyed, and NetCDF/EDR.
- **The commercial station networks riders trust most are closed to us.** Holfuy issues keys only to station owners. Tempest/WeatherFlow charges for commercial use. Windguru has no published reuse terms. Windy Stations shares only "Open" stations, needs a key, and its commercial terms are unclear. These should be link-only.
- **Paid model APIs add nothing.** Windy Point Forecast costs €990/yr, excludes ECMWF and is reportedly 3-hourly. Open-Meteo already serves the same models, so avoid it.
- **One foundation blocker.** `Forecast.networkFetch` only decodes JSON (`response:json()`, forecast.lua:127). NDBC realtime2 is whitespace text, and the KNMI and NWS text products are not JSON either. We need a `format = "text"` option on a source, asked under "Requests to the foundation" in PLAN.md, before any NDBC work.

## Services

Verdicts: **integrate** (a `source` in the app), **link-only** (open the provider's page from a card or the place editor), **avoid**.

| Service | Data for sailors | Endpoint | Key? | Licence / commercial | Coverage | Format | Verdict |
| --- | --- | --- | --- | --- | --- | --- | --- |
| NWS alerts ([docs](https://www.weather.gov/documentation/services-web-API)) | Small Craft Advisory (US: about 20–33 kn sustained), Gale (34–47 kn), Storm and Hurricane Force Wind Warnings, Special Marine Warning ([NOAA](https://marinenavigation.noaa.gov/weather-warnings.html)) | `api.weather.gov/alerts/active?point=lat,lon`; or `?zone=PZZ530` for a marine zone | No; a User-Agent is required (403 without one) | US government work, public domain (a [secondary source](https://github.com/jacaudi/nws) says so; no official terms page found) | US land and coastal marine zones; Alaska's marine alerts are missing from CAP ([NWS](https://www.weather.gov/documentation/services-web-alerts)) | GeoJSON (`Accept: application/geo+json`) | **Integrate** (P0) |
| NWS `/points` + marine zones ([geolocation](https://www.weather.gov/media/documentation/docs/NWS_Geolocation.pdf)) | Coastal-waters text forecast (CWF, to day 5), zone ids | `/points/{lat},{lon}`, `/zones/…`, `/products/types/CWF` | No | Public domain | US coasts and Great Lakes | JSON-LD; the product body is text | Link-only for now (zone ids change, effective 16 Apr 2026) |
| NDBC realtime2 ([access](https://www.ndbc.noaa.gov/rt_data_access.shtml)) | Buoy and C-MAN wind, gusts, direction, waves, period, water temperature, pressure, every 10 min to 1 h | `ndbc.noaa.gov/data/realtime2/{ID}.txt`; all stations: `data/latest_obs/latest_obs.txt` (rebuilt every 5 min); list: `activestations.xml` | No | Public domain; "limit your retrievals to a minimal level" | US coasts, Great Lakes, some partner buoys worldwide | Whitespace text, `MM` = missing | **Integrate** (P1, after text fetch) |
| NDBC via ERDDAP ([cwwcNDBCMet](https://coastwatch.pfeg.noaa.gov/erddap/info/cwwcNDBCMet/index.html)) | The same buoys, queryable | `coastwatch.pfeg.noaa.gov/erddap/tabledap/cwwcNDBCMet.json?…` | No | Public domain | As NDBC | **JSON** | Fallback if the text option is refused: no foundation change (latency unverified) |
| aviationweather.gov Data API ([api](https://aviationweather.gov/data/api/)) | METAR: wind, gust, direction, visibility, every hour or less; TAF: 24–30 h forecast | `/api/data/metar?ids=EGHI&format=json`; `bbox=` or `/api/data/stationinfo` for the nearest; `/api/data/taf` | No | US government, public domain; reportedly 100 requests/min per IP, then 429 ([secondary](https://apis.io/rate-limits/aviationweather/aviationweather-rate-limits/)); old `/cgi-bin` and `/api/json` paths retired in Sept 2025 ([changes](https://testbed.aviationweather.gov/help/upcoming/)) | Every reporting airport worldwide | JSON / GeoJSON | **Integrate** (P0), labelled "airport, X km inland" |
| OpenWindMap / Pioupiou ([licence](http://developers.pioupiou.fr/data-licensing/), [live API](http://developers.pioupiou.fr/api/live/)) | Crowd-sourced wind, gust, direction, live, about every 4 min; volunteer-run since Pioupiou's maker went bankrupt ([forum](https://forum.openwindmap.org/topic/43/pioupiou-est-mort-pioupiou-is-dead)) | `api.pioupiou.fr/v1/live/all` (or `/v1/live/{id}`); `live-with-meta/all` for names and locations | No | Community Licence: free, commercial OK with attribution "© contributors of the Pioupiou wind network" and a link to openwindmap.org. Condition: other sensors shown *alongside* must themselves be open data. A third party labels it [ODbL](https://github.com/KBenjaminPajusco/WindSector) (unconfirmed). | France and the Alps (dense), parts of Europe and beyond; paragliding sites more than beaches | JSON | **Integrate** (P1); a single `/all` call is cacheable |
| MeteoAlarm feeds ([T&C](https://www.meteoalarm.org/en/live/page/terms-and-conditions)) | National warnings incl. wind, coastal events, thunderstorms, by region and level | `feeds.meteoalarm.org/api/v1/warnings/feeds-{country}` (reported keyless, [issue](https://github.com/leonardoneirotti/haven-daily/issues/18)); EDR/MQTT re-user APIs need approval ([portal](https://api-test.meteoalarm.org/)) | No (v1 feeds) | Terms equivalent to CC BY 4.0. Credit "EUMETNET – MeteoAlarm" (or the national service for one country), show the issue time, link to meteoalarm.org. A successor, "MeteoGate", is under discussion ([issue](https://github.com/seevee/cap_alerts/issues/56)). | 30+ European countries | JSON wrapping CAP; still lists expired warnings, so filter on `expires` | **Integrate** (P1) |
| DWD open data ([opendata.dwd.de](https://opendata.dwd.de/README.txt)) | CAP warnings incl. coastal and sea areas; Seewetterbericht (text); stations | `opendata.dwd.de/weather/alerts/…`, `weather/maritime/…` | No | GeoNutzV for warnings per a [third-party summary](https://raw.githubusercontent.com/maschinenlesbar-org/dwd-cli/HEAD/DATA_LICENSE.md); CC BY 4.0 for climate data (CDC). Altered warnings must not carry a plain DWD credit. | Germany, North and Baltic Sea | Zipped CAP XML, text | Avoid as a source (MeteoAlarm carries DWD's warnings as JSON); link the Seewetterbericht |
| DMI Open Data ([terms](https://opendatadocs.dmi.govcloud.dk/Terms_of_Use)) | 10-min observations incl. wind; HARMONIE forecasts | `dmigw.govcloud.dk/v2/metObs/collections/observation/items?…` | Free key per API | CC BY 4.0, commercial OK; raw, not quality-controlled; no availability guarantee | Denmark, Greenland, Faroes | GeoJSON | Integrate later (P2; needs key handling in `Forecast.endpoints`) |
| KNMI Data Platform ([10-min](https://dataplatform.knmi.nl/dataset/actuele10mindataknmistations-2)) | 10-min synoptic incl. North Sea platforms and wind poles; lidar on TenneT platforms | Open Data API (file) and the newer EDR collection | Free key | CC BY 4.0 (per dataset; some, e.g. the METAR archive, are closed) | Netherlands, North Sea | NetCDF (old), EDR JSON (new) | Integrate later via EDR (P2) |
| Copernicus Marine ([licence](https://marine.copernicus.eu/user-corner/service-commitments-and-licence)) | Currents, waves, sea temperature (analysis + forecast) | Copernicus Marine Toolbox / OPeNDAP | Free account | Free incl. commercial with "Generated using E.U. Copernicus Marine Service Information"; funded to 30 Jun 2028 | Global and regional seas | NetCDF / Zarr | Avoid (Open-Meteo marine already serves similar currents; not a phone-friendly API) |
| EMODnet Physics | Aggregated in-situ platforms (moorings, tide gauges) | ERDDAP / WMS | No | Mostly CC BY (per provider; not verified this time) | European seas | ERDDAP JSON | Explore later |
| Met Office shipping / inshore waters ([DataPoint FAQ](https://www.metoffice.gov.uk/services/data/datapoint/datapoint-retirement-faqs)) | Shipping forecast, inshore waters (4×/day, 24 h) | DataPoint retired 1 Dec 2025; "no like-for-like replacement"; data through the paid Data Provisioning Team | — | Paid / by agreement | UK | — | **Link-only** ([inshore waters](https://weather.metoffice.gov.uk/guides/coast-and-sea/inshore-waters-forecast)); MeteoAlarm carries UK warnings |
| Windy Stations ([API](https://stations.windy.com/api-reference), [AODL](https://account.windy.com/agreements/aggregator-open-data-licence)) | Personal weather stations | `stations.windy.com` (new API, Jan 2026) | Key | Only stations marked "Open", under Windy's Aggregator Open Data Licence; commercial use not stated; ask info@windy.com | Worldwide, uneven | JSON | Avoid until written terms are obtained |
| Windy Point Forecast ([pricing](https://api.windy.com/point-forecast/pricing)) | Model forecasts | `api.windy.com/api/point-forecast/v2` | Key | €990/yr, 10k req/day, no ECMWF; test key data is shuffled | Global | JSON | Avoid (duplicates Open-Meteo) |
| Holfuy ([API](https://api.holfuy.com)) | Very good spot stations at kite beaches and lakes | `api.holfuy.com/live/?s=…&pw=…` | Password; "only for station admins/owners" | No public reuse licence | Europe, Alps, lakes | JSON | **Link-only** (`holfuy.com/en/weather/{id}`) |
| Tempest / WeatherFlow ([policy](https://weatherflow.github.io/Tempest/api/remote-developer-policy.html)) | iKitesurf/SailFlow station network | `swd.weatherflow.com/swd/rest/…` | Token | Personal use only; commercial use needs a subscription or agreement (reportedly from about $100/month, [forum](https://community.weatherflow.com/t/tempest-api-for-all-public-tempest-stations/10155)) and may not be made public | US mainly | JSON | Avoid (link-only to the station page) |
| Windguru stations ([JSON API, 2015](https://stations.windguru.cz/json_api_stations.html)) | Station network | `stations.windguru.cz` | Station credentials | No published third-party terms | Europe and worldwide | JSON | Link-only |

## Complaint evidence (new; see sailing.md for the rest)

- **Forecast against reality on the day.** "Forecast wind this Sat morning is showing 20knots for Perth, but all live graphs are showing 5 knots" ([Seabreeze WA](https://smtp.seabreeze.com.au/forums/Windsurfing/Western-Australia/Forecast-vs-live-wind?page=1)). "Use the forecast as a guide but back it up with live wind observations" ([Seabreeze](https://gale.seabreeze.com.au/forums/Kitesurfing/General/Wind-predictionWTF)). See also [Poor forecasting](https://www.seabreeze.com.au/forums/Kitesurfing/South-East-Queensland/Poor-forecasting?page=1).
- **"Live" must say where it was measured.** One rider's "live" readings come from Gingin Airport, far from the coast; another warns that live readings "can also be unreliable … depending on where the wind meter is located" (same thread). The lesson: METAR needs its distance and an "inland" label. A dead offshore station went months without data ([Sunny Coast](https://storm.seabreeze.com.au/forums/Kitesurfing/South-East-Queensland/Sunny-Coast-Weather-Feed)), so we need station age and a fallback.
- **Users confuse forecast with measurement.** Windy's forum has to explain that the app's wind "is NOT a measurement. It is a forecast"; a boater saw 11 kn in the app against about 1 kn on a gauge ([Windy reliability](https://community.windy.com/topic/37441/reliability)). Riders value overlays of models against reported data and local Holfuy gauges ("much more accurate", [Seabreeze](https://seabreeze.com.au/forums/Wing-Foiling/Wind-Wings/Wanting-to-up-my-weather-game--please-suggest-resources?page=1)).
- **Warnings need a second app.** "I don't know a sailor that doesn't use this app. Yet we need to keep a second program to be aware of Small Craft Advisories" ([Windy community](https://community.windy.com/topic/8493/craft-advisories)). Apps that do show advisories advertise it ([Boating Weather](https://apps.apple.com/us/app/boating-weather/id314819528)), and PredictWind has added "Extreme Weather Alerts" ([BoatTest](https://boattest.com/article/predictwind-launches-world-first-extreme-weather-alerts-enhanced-safety-sea.md)).
- **Paywalls on live data** (iKitesurf, Windfinder) are covered in sailing.md §"What people complain about" items 4 and 6.

## Integration plan

Files stay in the edition: `editions/sailing/*.lua`, `activities/sailing.lua` (add to `Sailing.sources` and `Sailing.cards`) and `tests/test_weather_sailing.lua`. Each source is `optional = true`, so failure lands in `forecast.missing[id]` and the rating goes on.

- **Fixtures.** Record each fixture once with `curl -A "SailorsWeather/1.0 (ORCA)"` into `tests/fixtures/weather/sailing/<id>.json` (or `.txt`). A test feeds it to `source.parse` with a fixed `NOW`, as tests/test_weather_alerts.lua does with synthetic forecasts.
- **No live calls in tests.**
- **About page.** Attribution goes on the Settings "about" page (`Data/Settings.xml`) and as a small source line on each card.

### SV-SA1 Marine warnings, US: P0, S

- **What.** Covers SA10 for US spots.
- **Files.** `editions/sailing/warnings.lua` copies the shape of `editions/hiking/sources.lua` `alerts`.
- **Request.** `url = api.weather.gov/alerts/active?point=lat,lon` for spots inside the US bounding boxes (as `Models.REGIONAL`), with `headers = { Accept = "application/geo+json" }`.
- **Parse.** `parse` keeps `properties.event` in a marine set (Small Craft Advisory, Gale/Storm/Hurricane Force Wind Warning, Special Marine Warning, Hazardous Seas, also Gale Watch), and keeps `onset`/`ends`/`expires`, `headline` and `senderName`. It writes `forecast.sailing.warnings`.
- **Cards.** A warning card, the most urgent. Hours inside a Gale or Storm warning are capped, like the existing gale cap. A Small Craft Advisory caps beginner and dinghy profiles and is a caution for others.
- **Point vs zone.** A coastal point usually resolves to the land zone. If marine events are missing in the fixture, add a second call `?zone=` from `/points` `forecastZone`. Verify with a recording at Crissy Field.
- **Test.** Fixture `nws-alerts-crissy.json` with an active advisory and an expired one: the card shows, the expired one is dropped, and the cap applies.
- **Attribution.** "Warnings: US National Weather Service (weather.gov)".

### SV-SA2 Nearest airport METAR: P0, S–M

- **What.** "Is it on now?" for every coast and lake near an airport.
- **Files.** `editions/sailing/live.lua` with source `metar`.
- **Request.** `aviationweather.gov/api/data/metar?bbox=…&format=json&hours=2`, with a box of about ±0.5° around the spot. Pick the nearest station with `Rank.distance`.
- **Parse.** Read `wdir` (or `VRB`), `wspd` and `wgst` in knots, plus `obsTime`. Set `forecast.sailing.live = { kind = "airport", name, km, at, speed, gust, dir }`.
- **Card.** "Live: 14 g 21 kn WSW at Hyères airport, 6 km inland, 18 min ago", next to the forecast for this hour ("forecast 12 g 18: running 2 kn above"). This answers story 47 in one line.
- **Staleness.** Hide the card after 90 min.
- **Test.** Fixture `metar-bbox-tarifa.json`: nearest pick, knots, a VRB direction, a stale cutoff, and an empty answer.
- **Attribution.** "Observation: NOAA Aviation Weather Center (METAR)".

### SV-SA3 MeteoAlarm warnings, Europe: P1, M

- **What.** Covers SA10 for EU spots.
- **Files.** `editions/sailing/warnings.lua` (shared with SV-SA1).
- **Request.** `feeds.meteoalarm.org/api/v1/warnings/feeds-{country}`, with the country from a small bounding-box table or the place's `country` field.
- **Parse.**
  - Filter by `expires > now`.
  - Filter by `awareness_type` wind, coastal event or thunderstorm.
  - Filter by an area match. Use the CAP `geocode` (EMMA_ID) or polygon contains the spot; where there is no polygon, use the region name and mark it "regional".
  - Show the level colour (yellow, orange, red).
- **Card.** As SV-SA1. Orange or red wind caps the hours.
- **Test.** Fixture `meteoalarm-germany.json` (Bodensee) with an expired entry and an out-of-area entry.
- **Attribution** (mandatory). The national service's name (for example "Deutscher Wetterdienst via EUMETNET – MeteoAlarm"), the issue time, and a link to meteoalarm.org.
- **Risk.** The feed may move to MeteoGate. Keep the URL in `Forecast.endpoints`.

### SV-SA4 OpenWindMap live stations: P1, M

- **What.** Crowd-sourced beach and lake stations (Garda, Bodensee, French coast).
- **Files.** `editions/sailing/live.lua`, source `openwindmap`.
- **Request.** One `api.pioupiou.fr/v1/live-with-meta/all`. That is large, so cache it app-wide for 5 min, or use `/v1/live/{id}` once the place editor has stored the nearest station id. Prefer the second: smaller, and kinder to a volunteer service.
- **Parse.** `measurements.wind_speed_avg` and `wind_speed_max` in km/h, `wind_heading`, `date`, `status.state`. Drop stations that are offline or older than 30 min.
- **Pick.** When both exist, prefer a station within 10 km and at the shore over a METAR.
- **Licence condition.** Only open-data sources may appear *alongside* it. NOAA, DMI and KNMI qualify. A Holfuy *link* is not shown data, so it is fine; mixing in closed station data would not be.
- **Test.** Fixture `pioupiou-live-garda.json`: units, an offline station skipped, nearest pick.
- **Attribution.** "Wind station: © contributors of the Pioupiou / OpenWindMap network, openwindmap.org".

### SV-SA5 NDBC buoys and C-MAN: P1, M

- **What.** Helen's open-water check (story 48): wind, gusts, waves and period at the buoy.
- **Blocked on** a foundation request: a source flag `format = "text"` so `networkFetch` returns the body string.
- **Files.** `editions/sailing/buoys.lua`.
  - A curated `NEAR` table of station ids per US spot (Crissy: FTPC1, 46026; Islamorada: MLRF1), because `activestations.xml` is XML.
  - Request `ndbc.noaa.gov/data/realtime2/{ID}.txt`, and parse the first data row (`WDIR WSPD GST WVHT DPD … `, m/s, `MM` = missing).
- **Fallback with no foundation change.** ERDDAP `cwwcNDBCMet.json?…&station="46026"&time>=now-3hours`.
- **Card.** A "buoy" row on the live card.
- **Test.** Fixture `ndbc-46026.txt` with an `MM` gust and two header lines.
- **Attribution.** "Buoy: NOAA National Data Buoy Center".

### SV-SA6 Station links in the place editor: P1, S

- **What.** Data we may not ingest is still one tap away.
- **Files.** `editions/sailing/spots.lua` gets a `links = { { "Holfuy 1234", "holfuy.com/en/weather/1234" }, { "Met Office inshore waters", … } }` per spot. The place editor gets a free "station link" field. The activity page shows the links under the live card.
- **Seed.** Seed the curated spots with:
  - Holfuy or Windguru station pages;
  - the Met Office inshore waters page (UK spots);
  - the DWD Seewetterbericht (German coast);
  - the NWS coastal-waters forecast page (US).
- **Test.** The links render, and a place without links shows none.
- **Attribution.** None (links only).

### SV-SA7 Forecast-vs-observed bias per spot: P2, M

- **What.** Builds on SV-SA2, SV-SA4 and SV-SA5: the live reading and the matching forecast hour are saved per spot, using `storage.lua` with about 14 days of pairs.
- **Card.** "Here the forecast has run 3 kn low on westerlies this week". Shown only after at least 8 pairs, with "about".
- **Story.** Answers sailing.md story 47, and gap 14, "spot bias".
- **Files.** `editions/sailing/bias.lua`.
- **Test.** A synthetic series, no network.
- **Attribution.** The observation sources as above.

### SV-SA8 DMI and KNMI observations: P2, M

- **What.** Danish and Dutch coasts and the North Sea platforms, under CC BY 4.0.
- **Prerequisites.** Free keys in `Forecast.endpoints`, and the KNMI EDR JSON endpoint, not NetCDF.
- **Request.** DMI `metObs` `items?parameterId=wind_speed&bbox=…&limit=…`.
- **Test.** Recorded GeoJSON fixtures.
- **Attribution.** "Contains data from DMI (CC BY 4.0)" and "KNMI (CC BY 4.0)".

### Not planned

- The Windy Point Forecast API, Tempest, and Holfuy and Windguru data: closed or paid (link only).
- Copernicus Marine: duplicates Open-Meteo marine and needs NetCDF.
- Met Office feeds: paid since DataPoint closed.
- AIS: out of scope.

Every live card must say "observed" against "forecast", show the station's distance and age, and never override the safety caps (sailing.md, "What we must avoid").
