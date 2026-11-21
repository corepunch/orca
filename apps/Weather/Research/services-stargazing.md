# Stargazers' Weather: existing services to reuse

Research for the `stargazing` edition (November 2026). Goal: make the app
"everything needed to plan a night under the sky", by reusing services that
already exist instead of building new data. It builds on
[stargazing.md](stargazing.md) (competitors, complaints, the Open-Meteo
licensing note) and on [PLAN.md](../PLAN.md) ST1–ST14 (ST12–ST14 skipped,
ST10 partial).

**How this was checked.** Web search only. In this session, direct fetches of
celestrak.org, 7timer.info, n2yo.com and services.swpc.noaa.gov were blocked
by the network, so the endpoint shapes and terms below come from search
results, official pages quoted in them, and earlier knowledge. Entries
marked **(verify)** must be confirmed against the live page before
shipping. No terms were checked by a lawyer.

## Summary

- **Aurora is where the free data is best.** NOAA SWPC (public domain)
  offers much more than the Kp forecast we use: real-time solar wind (Bz,
  speed) from L1, the OVATION 30-minute aurora nowcast, observed Kp and
  alerts. GFZ Potsdam publishes Kp and the half-hourly Hp30 under CC BY 4.0.
  Forum and review complaints are mostly "Kp is too coarse and too late":
  real-time Bz and OVATION answer that directly.
- **Planets, meteor showers and eclipses need no service.** Compute planets
  locally (moon.lua already uses Schlyter's theory, which covers planets
  too); keep showers and eclipses as small static tables with credit.
  JPL Horizons serves only as the offline reference for tests.
- **ISS passes need orbital elements from CelesTrak and SGP4 in Lua.**
  The data is free, but CelesTrak polices download rates, and its terms for
  a commercial app are unclear. Ask before launch. N2YO needs a key embedded
  in the app (1,000 calls an hour shared by all users): avoid.
- **Light pollution is a licence trap.** The World Atlas (Falchi et al.) is
  CC BY-NC 4.0, so it cannot be used in a paid app. lightpollutionmap.info
  has no public API terms. NASA Black Marble (VIIRS VNP46A4) is open without
  restriction, so derive our own Bortle *estimate* offline from it.
- **Seeing services are paid or fragile.** Meteoblue (credits) and
  Astrospheric (personal use; commercial by email; North America only) are
  paid. 7Timer! is free but has no stated licence and a history of
  outages. Keep our own estimate (seeing.lua), and link out for a second
  opinion.

## Services

Verdicts: **integrate** (an optional `source`), **compute locally**,
**static table**, **link-only** (open the site), **avoid**.

| Service | Data | Endpoint | Key? | Licence / commercial | Coverage, format, reliability | Verdict |
| --- | --- | --- | --- | --- | --- | --- |
| SWPC Kp forecast (in use) | Planetary Kp, 3-hour steps, 3 days | `services.swpc.noaa.gov/products/noaa-planetary-k-index-forecast.json` | no | US government work, public domain; no implied NOAA endorsement; credit requested | Global; JSON rows; operational 24/7, preliminary values | integrated (aurora.lua) |
| SWPC real-time solar wind | Bz, Bt (nT); speed, density (DSCOVR/ACE at L1), 1-minute | `…/products/solar-wind/mag-2-hour.json`, `plasma-2-hour.json` (also 5-minute, 6-hour, 1-day, 7-day) | no | as above | Global (one spacecraft); JSON array with a header row; about 30–60 min warning before effects reach Earth; gaps during spacecraft outages | **integrate** |
| SWPC OVATION nowcast | Chance of visible aurora on a 1° grid, next 30 min, every 5 min | `…/json/ovation_aurora_latest.json` | no | as above | Global; `coordinates` = `[lon 0–359, lat −90…90, %]` with `Forecast Time` (verify the field names); about 900 KB per fetch; empirical, falls back to Kp when L1 data are bad | **integrate**, only on dark nights at high magnetic latitude |
| SWPC observed Kp, alerts, 3-day text | Observed and estimated Kp; watches, warnings, alerts (G scale); forecast discussion | `…/products/noaa-planetary-k-index.json`, `…/products/alerts.json`, `…/text/3-day-forecast.txt`, `…/json/planetary_k_index_1m.json` (verify) | no | as above | Global; JSON / text | **integrate** alerts.json (cheap); text link-only |
| GFZ Kp and Hp30/Hp60 | Kp (definitive, nowcast), Hp30 half-hourly index, forecasts | `kp.gfz.de/app/json/?start=…&end=…&index=Hp30` (verify); Hp30 forecast JSON on spaceweather.gfz.de | no | **CC BY 4.0**, commercial OK, credit "GFZ German Research Centre for Geosciences" | Global; JSON with licence in metadata; research institute, not 24/7 operations | **integrate** as P1 complement (finer than Kp) |
| NASA DONKI (CCMC) | CMEs, flares, storms with arrival estimates | `api.nasa.gov/DONKI/…` (key: DEMO_KEY 30/h, 50/day per IP); CCMC mirror `kauai.ccmc.gsfc.nasa.gov/DONKI/WS/get/…` (keyless, verify) | yes / no | US government, public | Global; JSON; event list, not a local forecast | **avoid** for now (SWPC alerts cover it); P2 "storm on the way" |
| NASA APOD | Picture of the day | `api.nasa.gov/planetary/apod` | yes | many images are **copyrighted** by their authors | — | **avoid** (rights per image) |
| CelesTrak GP (OMM) | Orbital elements for ISS, Tiangong, Hubble, bright satellites | `celestrak.org/NORAD/elements/gp.php?CATNR=25544&FORMAT=json` (also `GROUP=stations`, `visual`) | no | Free "to the space community"; **usage policy (May 2026): at most once an hour per data set, enforced with 403s and firewalling**; commercial redistribution not explicit (one third party says it is forbidden) — **ask CelesTrak** | Global; JSON/CSV/TLE (TLE ends with 6-digit catalog numbers: use OMM JSON); elements refresh about every 2 h | **compute locally** with SGP4; fetch at most daily per device and cache |
| Space-Track.org | Same elements, primary source | account API | yes (account) | US government user agreement limits redistribution | — | **avoid** in the client |
| N2YO | Visual passes, positions | `api.n2yo.com/rest/v1/satellite/visualpasses/{id}/{lat}/{lng}/{alt}/{days}/{minVis}&apiKey=` | yes | Free but 1,000 transactions/hour per key; commercial terms not stated | Global; JSON | **avoid** (one key shared by every install) |
| Heavens-Above | Passes, flares, sky charts | website only | — | no API, scraping not allowed | — | **link-only** |
| Open Notify / wheretheiss.at | ISS position now | `api.open-notify.org/iss-now.json` | no | free | position only; Open Notify's pass endpoint was retired | **avoid** |
| JPL Horizons | Precise ephemerides for any body | `ssd.jpl.nasa.gov/api/horizons.api` | no | NASA/JPL; no published rate limit or terms found | Global; JSON wrapping text | **reference only**: record expected values for tests |
| Local planet theory | Planet rise, set, height, brightness, conjunctions | Schlyter (as moon.lua) or Astronomy Engine (Don Cross; MIT, verify LICENSE; about 1′ accuracy, no Lua port) | — | our code / MIT | Offline, any date | **compute locally** |
| JPL SBDB / MPC | Comet and asteroid elements | `ssd-api.jpl.nasa.gov/sbdb.api`; MPC export format | no | NASA public / MPC credit | elements only; comet brightness forecasts are unreliable | **avoid** for now (P2: hand-picked bright comet) |
| IMO Meteor Shower Calendar / IAU MDC | Shower peaks, ZHR, radiants, activity windows | yearly PDF (`imo.net/files/meteor-shower/cal2026.pdf`); IAU MDC list | — | **no licence stated**; the dates and rates are facts, the calendar text is IMO's | Global; yearly | **static table** written by us, credited to IMO/IAU MDC; ask IMO |
| NASA eclipse tables (Espenak) | Solar and lunar eclipses, local circumstances | `eclipse.gsfc.nasa.gov` | — | "Permission is freely granted to reproduce this data" with the credit "Eclipse Predictions by Fred Espenak, NASA/GSFC Emeritus" | Global; −1999…+3000 | **static table** (next 10 years) |
| World Atlas of Artificial Night Sky Brightness (Falchi 2016) | Modelled zenith sky brightness, 30″ grid | GFZ data request form | — | **CC BY-NC 4.0**: not for a paid app | 2.9 GB GeoTIFF; 2015 VIIRS | **avoid** (unless permission is granted) |
| lightpollutionmap.info | VIIRS, World Atlas, SQM layers | website (no public API terms found) | — | unknown | — | **link-only** |
| NASA Black Marble VNP46A4 | Yearly VIIRS night lights, 15″ grid, 2012– | LAADS / Earthdata (account to download) | download only | "openly shared, without restriction" (EOSDIS); cite Wang et al. 2022 and the DOI | Global; HDF5 | **compute locally** (offline tool) → Bortle estimate |
| 7Timer! ASTRO | Cloud, seeing, transparency, lifted index, 3-hourly, 3 days | `7timer.info/bin/astro.php?lon=…&lat=…&ac=0&unit=metric&output=json&tzshift=0` (verify) | no | **no licence or terms found**; "free" per third parties | Global; GFS-based, coarse; repeated outages reported | **avoid** until terms are confirmed; link-only |
| Meteoblue Astronomy Seeing | Seeing (arcsec), jet stream, bad layers | website free 3 days; API by credits (about €2,400/year plans) | yes | paid | Global | **link-only** |
| Astrospheric | RDPS clouds, seeing, transparency, smoke | Data API for Pro members, personal use; commercial via email | yes | paid / by agreement | US and Canada | **link-only** |
| Clear Outside / Clear Sky Chart | Cloud layers grid; GEM-based charts | websites | — | no API | — | **link-only** |

## Complaint evidence (data gaps)

- **Kp is too coarse and too late for aurora.** AuroraWatch UK: Kp "only
  indicates geomagnetic activity within a 3 hour interval, which is too long
  to be useful for auroral alerts", and official Kp is not real-time
  ([AuroraWatch UK](https://wp.lancs.ac.uk/aurorawatchuk/2014/09/12/why-kp-is-a-poor-indicator-for-auroral-alerts/)).
  Sky & Telescope tells readers to watch the real-time Bz trace
  ([S&T](https://skyandtelescope.org/stargazing-and-observing/keep-track-of-the-aurora/)).
- **Alert apps miss substorms.** A substorm usually follows about an hour of
  southward Bz; the Glendale FAQ lists missed substorms as a common failing
  of alert apps (a competitor's claim)
  ([Glendale FAQ](https://aurora-alerts.uk/faqs.php),
  [advanced FAQ](https://aurora-alerts.uk/faqs-advanced.php)).
- **Stale or forecast Kp, late notifications, paywalled alerts.** App Store
  changelogs switch to measured Kp, fix "sustained Bz threshold monitoring"
  and "notification timing"; reviews call apps inaccurate when another app
  shows Kp 8, and one user uninstalled when an aurora alert opened a paywall
  ([Aurora Eos](https://apps.apple.com/us/app/id6748328478),
  [Aurora Forecast & Alerts](https://apps.apple.com/app/id1476831567),
  [AuroraCast](https://apps.apple.com/us/app/id1270592946),
  [Aurora Now](https://apps.apple.com/au/app/id6740248373)).
- **Seeing and cloud forecasts disagree; users cross-check.** AstroBin
  (July 2026): one finds Astrospheric best in the central US; others see
  Astrospheric and Clear Outside call cloud while radar shows a gap
  ([AstroBin thread](https://app.astrobin.com/forum/topic/243478/seeing/how-do-you-get-a-reliable-weather-forecast-before-a-session)).
  Meteoblue warns its seeing "may not reflect precisely reality" and
  ignores cloud cover
  ([meteoblue help](https://content.meteoblue.com/en/private-customers/website-help/outdoor-and-sports/astronomy-seeing)).
  Earlier Cloudy Nights threads on "clear" that is not clear are cited in
  [stargazing.md](stargazing.md#what-people-complain-about).
- **7Timer! is fragile.** IceInSpace threads report it down, frozen, or
  48 hours behind, several times over the years
  ([1](https://www.iceinspace.com.au/forum/archive/index.php/t-115934.html),
  [2](https://www.iceinspace.com.au/forum/archive/index.php/t-113753.html),
  [3](https://www.iceinspace.com.au/forum/archive/index.php/t-63340.html)).
- **ISS passes: timing is fine; visibility is the gap.** Reviews praise
  timing but note passes in daylight or in Earth's shadow, and ask for an
  alarm before a pass
  ([ISS Detector](https://apps.apple.com/us/app/iss-detector/id1198597805),
  [Sat Seeker](https://apps.apple.com/us/app/sat-seeker-iss-tracker/id528998089)).
  So we should list only *visible* passes on *clear* nights.
- **Light pollution without meaning.** The Light Pollution Map app has no
  legend linking colours to Bortle classes
  ([App Store](https://apps.apple.com/us/app/id1530464858)).
- Gap in this pass: Reddit r/astrophotography and Stargazers Lounge threads
  did not surface in search; no direct quotes from them are claimed here.

## Integration plan

Each item is an optional `source` (README, "sources") or local code; the
forecast stands when it fails. Each gets an offline test in
`tests/test_weather_stargazing.lua` with a recorded, trimmed response in the
test (or next to `tests/weather_fixture.lua`), never the live service.

| ID | Pri | Size | What | Files | Maps to | Test fixture | Attribution |
| --- | --- | --- | --- | --- | --- | --- | --- |
| SV-ST1 | P0 | S | **Solar wind now.** Bz, speed and density of the last 2 hours; "Bz south −8 nT for 40 min: the next hour is the one to watch". Only on dark, mostly clear nights where the oval can reach (aurora.lua already decides this). Rows older than 15 minutes count as missing. | `editions/stargazing/aurora.lua` (`Aurora.solarWind`), `activities/stargazing.lua` (`sources`, `auroraCard`, alerts) | two `sources`: `mag-2-hour.json`, `plasma-2-hour.json` → `forecast.solarWind = { bz, speed, density, minutesSouth, at }` | 2-hour mag/plasma rows: one quiet, one southward, one with nulls (spacecraft gap) | "Data: NOAA SWPC" |
| SV-ST2 | P0 | M | **OVATION 30-minute nowcast.** The chance of visible aurora at the grid cell nearest the place (lon 0–359), plus the cell on the poleward horizon (about 5° nearer the pole) for "low on the northern horizon". `url` returns nil unless it is dark at the place and the magnetic latitude is above about 45° (or Kp ≥ 5), because the file is about 900 KB. | `editions/stargazing/aurora.lua` (`Aurora.ovation`), `activities/stargazing.lua` (`auroraCard`, `note`) | `source` → `forecast.ovation = { chance, horizonChance, at }`; replaces the Kp oval estimate for the next 30 min | a cut-down file: header plus a few hundred cells around one test place; an old `Forecast Time` to test the stale path | "Aurora nowcast: NOAA SWPC OVATION" |
| SV-ST3 | P1 | S | **Storm alerts and observed activity.** SWPC `alerts.json` (G watches and warnings) as a calm line ("A G2 storm watch for tomorrow night"); GFZ Hp30 to say "activity is rising now" between Kp steps. | `editions/stargazing/aurora.lua`, `activities/stargazing.lua` | two optional `sources` → `forecast.spaceAlerts`, `forecast.hp30` | one alerts.json with a watch and a cancel; an Hp30 JSON slice | "NOAA SWPC"; "Hp30: GFZ Potsdam, CC BY 4.0" |
| SV-ST4 | P1 | M | **Planets tonight (part of ST12).** Mercury to Saturn: rise, set, highest point during the dark window, magnitude, "Jupiter and the moon close together". Computed locally with Schlyter's elements, like moon.lua. No network. | new `editions/stargazing/planets.lua`, `activities/stargazing.lua` (`cards`) | a card; `forecast.planets` built in `cards`, not a source | expected rise/set and heights for 3 places and dates recorded once from JPL Horizons; tolerance a few minutes | "Positions computed after P. Schlyter" (README) |
| SV-ST5 | P1 | S | **Meteor showers and eclipses (rest of ST12).** A static table of about 10 major showers (peak date, ZHR, radiant, active window) and the eclipses visible in the next 10 years; rated by moonlight from moon.lua and by tonight's cloud ("Perseids peak tonight, moon sets at 23:10, 60 an hour at best, clear after midnight"). Refresh the table once a year. | new `editions/stargazing/events.lua`, `activities/stargazing.lua` (`cards`, alerts) | a card; no source | fixed dates: a peak under a full moon, a peak on a clear dark night, a lunar eclipse night | "Shower data: IMO / IAU Meteor Data Center"; "Eclipse Predictions by Fred Espenak, NASA/GSFC Emeritus" |
| SV-ST6 | P1 | L | **Visible ISS and Tiangong passes.** OMM elements from CelesTrak by catalog number (25544, 48274), at most once a day per device, cached through `storage.lua`; SGP4 in Lua (port of the public reference code or satellite.js, MIT); keep only passes where the station is sunlit, the observer is in nautical darkness, and the peak is above 10°; brightness estimate; rated by cloud at that hour. Stop on any 403 and wait a day. **Blocker: written OK from CelesTrak for an app with many users; else a small caching proxy.** | new `editions/stargazing/satellites.lua`, `editions/stargazing/sgp4.lua`, `activities/stargazing.lua` | `source` with a cache-aware `url` (nil while the cached elements are under 24 h old) → `forecast.passes` | one recorded OMM JSON; positions checked against Vallado's SGP4 verification cases; passes checked against a recorded Heavens-Above list | "Orbital data: CelesTrak" |
| SV-ST7 | P2 | M/L | **Bortle estimate per place (ST14).** An offline tool turns Black Marble VNP46A4 radiance into a coarse sky-brightness grid (land only, about 0.1°, compressed), with a radiance-to-Bortle mapping calibrated on published SQM readings; the place page shows "Bortle about 4 (estimate): the Milky Way shows structure". The user may enter their own SQM reading in the place editor (`placeFields`), which wins. | new `editions/stargazing/darkness.lua` and a data file under `editions/stargazing/`; a tool under `tools/` | a card and a place field; no source | a tiny grid with three cells (city, suburb, dark site) | "Night lights: NASA Black Marble (VNP46A4), Wang et al. 2022" |
| SV-ST8 | P2 | S | **Second opinions.** Links from the relevant cards: SWPC aurora dashboard, Meteoblue seeing, Clear Outside, Heavens-Above (passes), lightpollutionmap.info. Opens the browser; no data copied. | `activities/stargazing.lua` (`cards`) | card links | none (or a check that each link has the place's coordinates) | the site's own page |

Order: SV-ST1 and SV-ST2 first (cheap, keyless, public domain, and they fix
the most-cited aurora complaint), then SV-ST4 and SV-ST5 (offline, no
licence risk), then SV-ST6 once CelesTrak answers, then SV-ST7.

## Attribution and licence actions

- Add a "Data sources" section to the stargazing about/settings text:
  Open-Meteo (CC BY 4.0, paid plan for commercial use), NOAA SWPC, GFZ
  Potsdam (CC BY 4.0), CelesTrak, NASA Black Marble, IMO / IAU MDC, Fred
  Espenak (NASA/GSFC Emeritus). Do not imply NOAA or NASA endorsement.
- Before launch: email CelesTrak about many-device use (or plan a proxy);
  ask IMO whether a derived shower table needs permission; confirm the
  Astronomy Engine licence if code is ported from it; do not use the World
  Atlas or 7Timer! data without written permission or published terms.
- Poll politely: OVATION and solar wind at most every 5–10 minutes and only
  while a page is open on a dark night; Kp and alerts every 30 minutes (the
  existing poll); orbital elements daily.

## Sources

- SWPC: [aurora 30-minute forecast](https://swpc.noaa.gov/products/aurora-30-minute-forecast),
  [JSON index](https://services.swpc.noaa.gov/json/),
  [solar-wind products](https://services.swpc.noaa.gov/products/solar-wind/),
  [data access](https://www.spaceweather.gov/content/data-access),
  [NODD licence](https://registry.opendata.aws/noaa-space-weather),
  [archived data policy](https://webarchive.library.unt.edu/web/20090114033218/http://www.swpc.noaa.gov/Data/about_data.html),
  [OVATION grid layout (Cartopy example)](https://scitools.org.uk/cartopy/docs/latest/gallery/scalar_data/aurora_forecast.html)
- GFZ: [Kp index](https://kp.gfz.de/en/), [Hpo](https://www.gfz.de/hpo-index),
  [Kp JSON format](https://kp.gfz.de/app/format/json.txt)
- NASA: [api.nasa.gov limits](https://api.data.gov/docs/developer-manual),
  [Horizons API](https://ssd-api.jpl.nasa.gov/doc/horizons.html),
  [SBDB API](https://ssd-api.jpl.nasa.gov/doc/sbdb.html),
  [eclipse copyright](https://eclipse.gsfc.nasa.gov/SEpubs/copyright.html),
  [Black Marble VNP46A4](https://www.earthdata.nasa.gov/data/catalog/laads-vnp46a4-2-0)
- Satellites: [CelesTrak usage policy](https://celestrak.org/usage-policy.php),
  [GP formats](https://celestrak.org/NORAD/documentation/gp-data-formats.php),
  [N2YO API](https://www.n2yo.com/api), [N2YO terms](https://www.n2yo.com/about/?a=terms)
- Light pollution: [World Atlas supplement (licence)](https://b2find.eudat.eu/dataset/df8cd562-df6a-51bf-ad64-7421cf8c8623),
  [lightpollutionmap.info help](https://www.lightpollutionmap.info/help.html)
- Meteors: [IMO calendar](https://www.imo.net/resources/calendar/),
  [2026 calendar](https://www.imo.net/the-2026-meteor-shower-calendar-is-here/)
- Paid seeing: [Astrospheric API](https://www.astrospheric.com/dynamiccontent/api_info.php),
  [meteoblue pricing](https://content.meteoblue.com/en/business-solutions/weather-apis/pricing)
- Planets: [Astronomy Engine](https://raw.githubusercontent.com/cosinekitty/astronomy/master/README.md)
