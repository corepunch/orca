# Sailors' Weather — research

Sailors' Weather is the `sailing` edition of apps/Weather. It rates every hour for dinghy and keelboat sailing, windsurfing and kitesurfing.
The market is crowded at both ends. Free map apps (Windy) are strong at models and maps. Paid tools (PredictWind, iKitesurf) charge for routing, live stations and alerts. Neither end answers "should I go, when, and on what gear?" clearly, honestly and without ads.
To win, we should answer that question per spot and per rider, in knots, and show how confident the forecast is. Wind alerts should be fair and not hidden behind an expensive paywall, and the forecast should still work offline. We should not try to beat Windy at maps or PredictWind at routing.

Researched November 2026. Prices and ratings come from US App Store pages fetched on that date unless noted. They change often, so re-check them before quoting. Several comparison sources are blogs run by competing apps (WindUp, Wavve Boating); they are marked as such.

## Competitors

| App | Who it is for | Price (US, Nov 2026) | Rating | Standout | Main weakness |
| --- | --- | --- | --- | --- | --- |
| [Windy.com](https://apps.apple.com/pl/app/windy-com/id1161387262) | Everyone who reads weather maps | Free; Premium about $19–30/yr ([community](https://community.windy.com/topic/16441/why-is-premium-more-expensive-on-windy-com)) | 4.8 (about 12k on the PL store; far more worldwide) | 15+ models (ECMWF, GFS, ICON, AROME…), best animated map, webcams, a passage planner plugin | Free tier shows 3-hourly steps; 1-hour steps and 1 km models need Premium ([yacht.de](https://www.yacht.de/en/sailing-knowledge/meteorology/apps-for-sailing-the-best-weather-apps-for-the-2026-season/)). Not spot- or sport-oriented. Weak offline. |
| [Windy.app](https://apps.apple.com/us/app/windyapp/id997079492) (a different company) | Kite, windsurf, sail, fish | Free; Pro from $9.99/month to about $59.99/yr (many IAP rows) | 4.7 (about 73k); #1 in Fishing, boating & sailing | Sport profiles, 30k spots, spot chats, alerts that now include gusts and a chosen model, offline favourites, model-comparison dashboards | Many confusing price rows. Its map is weaker than Windy.com. |
| [Windfinder](https://apps.apple.com/us/app/windfinder-wind-weather-map/id336829635) | Kite and windsurf spot checks | Free with ads; Plus $1.99/mo or $12.99/yr; remove ads $4.99/yr | 4.8 (about 64k) | 160k spots, 21k live stations, tides at 20k places, "Superforecast" | Intrusive ads that are easy to tap by mistake. Confusing Free/Plus/Pro naming. No dark mode. Alerts cost extra. |
| Windguru (web + app) | Hard-core kiters and windsurfers | Free; PRO about €49/yr; popular model runs reportedly about 12 h late on the free tier ([WindUp, a competitor](https://www.windup.live/blog/best-windguru-alternative/)) | n/a (mainly a website) | A dense multi-model table that riders trust | Cluttered; the mobile app trails the website ([WindUp](https://www.windup.live/blog/windguru-vs-windy/)) |
| [iKitesurf / SailFlow](https://apps.apple.com/us/app/ikitesurf-weather-waves/id555643284) (WeatherFlow-Tempest) | US kiters and sailors | Free; Plus, Pro and Gold tiers from about $3.99 to $179.99 | 4.7 (about 2k) | 65k Tempest live stations, the Nearcast AI model, meteorologist-written daily forecasts, alerts | Alarms and the best live data are paid ([WindUp](https://www.windup.live/blog/best-wind-app-for-kitesurfing/)). Free users lost meters, and the free forecast stops at 2 days. Readings differ between apps. |
| [PredictWind](https://apps.apple.com/us/app/-/id477048487) | Cruisers, offshore and racing | Basic $29/yr; Standard $249/yr; Professional $499/yr | 4.8 (about 33k) | Proprietary PWG/PWE models plus ECMWF and others, weather routing, departure planner, satellite Offshore app, forecast validation | Expensive and "overkill" for coastal sailing ([yacht.de](https://www.yacht.de/en/sailing-knowledge/meteorology/apps-for-sailing-the-best-weather-apps-for-the-2026-season/)). Hard to cancel. Alerts trigger on a single hour. |
| [Savvy Navvy](https://apps.apple.com/GB/app/savvy-navvy/id1479182650) | Coastal cruisers and new skippers | About £45–89/yr ([pricing](https://www.savvy-navvy.com/pricing)) | about 4.6–4.7 (aggregated; uncertain) | Charts, tides and weather in one app; smart routing; a "comfort index" | Routing over-promises; it only plans as far ahead as the weather goes ([YBW](https://forums.ybw.com/threads/savvy-navvy.569070), [Yachting World](https://www.yachtingworld.com/gear-reviews/best-navigation-apps-for-sailors-savvy-navvy-135163)) |
| Navionics (Garmin Boating) | Chart users | Paid charts subscription; no free plan (uncertain on weather add-ons) | not checked | Charts, with weather overlays | Weather is secondary; not a wind-sports tool |
| [Meteoblue](https://www.yacht.de/en/sailing-knowledge/meteorology/apps-for-sailing-the-best-weather-apps-for-the-2026-season/) | Weather nerds | Free with ads; €6.99/yr ad-free | not checked | 40+ models; a "predictability" index | Confusing, aviation-oriented menus |
| WindUp ([blog](https://www.windup.live/blog/best-wind-app-for-kitesurfing/)) | Kiters | Free | not checked | A live-station alarm that rings through Do Not Disturb, with a gust ceiling and a direction filter | It is not a forecast. It writes most of the "best app" articles. |
| Windmaster, Hoolan, Surfr, Volo, WOO | Kite, wing and windsurf | Freemium | not checked | Session logging and jump tracking (WOO, Surfr), Watch apps | Mostly trackers. Forecasts, where offered, are thin. |
| National services (Met Office, DWD WarnWetter, DMI, SMHI, BOM, NOAA) | Locals | Mostly free | — | Official warnings and inshore and shipping forecasts. High-resolution local models (DMI HARMONIE 2.5 km, SMHI AROME). | Usually one country, sometimes one language. Not sport-specific. |
| Seabreeze (AU, [support](https://www.seabreeze.com.au/Support)) | Australian windsurfers and kiters | Freemium | — | Its own forecast tuned for watersports, with forums | Strength errors are reported often ([forum](https://www.seabreeze.com.au/forums/Kitesurfing/General/Forecast-looked-cooked-?page=1)) |

Notes:

- **Everyone uses the same public models.** Windguru, Windy and Windfinder mostly repackage GFS, ECMWF, ICON and AROME ([KiteReady interview](https://www.surf-magazin.de/en/windsurfing/scene-and-events/wind-and-weather-the-spot-is-calling-interview-about-the-kiteready-app/)). Presentation and interpretation are the real differences, which suits us: Open-Meteo serves the same models.
- **Paywalls usually block:** hourly or 1 km detail (Windy), alerts (Windfinder, iKitesurf), live stations (iKitesurf), fresh model runs (Windguru, reportedly) and routing (PredictWind).
- **The common workflow uses three apps:** Windy for the picture, Windguru or Windfinder for the spot numbers, and a live station or WindUp for "is it on right now?" ([newwavekiteboarding](https://newwavekiteboarding.com/blogs/kiteboarding/best-kitesurfing-forecast-apps-websites-how-to-pick-the-right-one), [YBW](https://forums.ybw.com/threads/wind-forecast-apps.616678/)). Doing all three in one app is the opening.
- **Uncertain:** I could not read Reddit directly; reddit.com blocked the fetcher. Reddit themes come from mirrors and search snippets and are thinner than the forum sources.

## What people complain about

1. **Gusts are under-forecast, and the gust number is not trusted.** Models average the wind, so gusts get smoothed out ([Windy.app blog](https://windy.app/blog/three-mistakes-of-weather-forecasts-for-yachting.html)). ECMWF itself says extreme gusts are under-represented, especially in convection. One Aegean sailor adds 40–60 % to Windy's gusts ([Windy community](https://community.windy.com/topic/35904/gusts-vs-wind)). An r/Kiteboarding thread reports an app showing 5–7 kn gusts when the real gusts were 10+ kn (mirror). The UK Met Office's automated site does the opposite and over-states gusts ([YBW](https://forums.ybw.com/threads/wind-forecast-apps.616678/), [YBW ECMWF vs GFS](https://forums.ybw.com/threads/ecmwf-vs-gfs-over-the-top-gust-predictions.548118/page-2)).
2. **Models disagree, and apps hide or bury it.** "Compare 4 or 5 models: if they agree it will probably happen" ([TrawlerForum](https://www.trawlerforum.com/threads/predict-wind-why-different-predictions-so-far-off.77822/post-1308137)). Riders like Windguru *because* it shows every model, but complain that it is cluttered. iKitesurf users are confused when readings differ between apps ([App Store](https://apps.apple.com/us/app/ikitesurf-weather-waves/id555643284)).
3. **Local effects: sea-breeze timing, thermals and terrain.** Grid models miss when a sea breeze starts or whether it starts at all. Seabreeze's own FAQ says forecasts can be several knots or an hour out ([Seabreeze](https://www.seabreeze.com.au/Support)). In one case the forecast was 20 kn and the station read 2 kn. Locals subtract 5–7 kn for some directions ([forum](https://www.seabreeze.com.au/forums/Kitesurfing/Newbies-Tips-Tricks/Melbourne-wind-for-learning?page=1), [SA thread](https://www.seabreeze.com.au/forums/Windsurfing/South-Australia/weather-forcasters?page=1)). There is no consensus on whether forecasts err high or low; it depends on the spot.
4. **Paywalls and pricing.** PredictWind is "complex and expensive" for coastal use and hard to cancel. Windy's 3-hour free steps are seen as restrictive. Windfinder's Plus and Pro naming is confusing. iKitesurf took meters away from long-time free users ([yacht.de](https://www.yacht.de/en/sailing-knowledge/meteorology/apps-for-sailing-the-best-weather-apps-for-the-2026-season/), [App Store reviews](https://apps.apple.com/us/app/-/id477048487), [justuseapp](https://justuseapp.com/en/app/336901296/windfinder-pro-wind-weather/reviews)).
5. **Ads.** Windfinder's ads are easy to tap by accident while reading the forecast ([App Store](https://apps.apple.com/us/app/windfinder-wind-weather-map/id336829635)). One reviewer says the website nags you to open the app, and then the app is paywalled ([WindUp, a competitor](https://www.windup.live/blog/best-windfinder-alternative/)).
6. **Alerts that are weak, paid or dumb.** Most apps only push "the forecast says X". They do not ring when a station actually reaches it ([WindUp](https://www.windup.live/blog/best-wind-app-for-kitesurfing/)). PredictWind alerts fire when a single hour meets the rule, not the whole window ([App Store](https://apps.apple.com/us/app/-/id477048487)). Alerts are paid in Windfinder and iKitesurf.
7. **Clutter and a learning curve.** Windguru's table is crowded, and Meteoblue's menus feel built for pilots. Beginners do not know what 18 kn gusting 27 means for *them*.
8. **Offline at sea.** Users screenshot Windy in several places to keep a forecast for remote trips. Losing the last downloaded forecast offline is called "quite disabling" ([Windy community](https://community.windy.com/topic/36210/request-for-offline-access-to-last-downloaded-weather-forecasts), [offline mode](https://community.windy.com/topic/5738/offline-mode)). Competitors now advertise "save forecast with the route" ([yacht.de NavigationOrca](https://www.yacht.de/en/equipment/electronics/navigation-orca-app-weather-update-offers-a-10-day-overview-offline-forecasts-and-a-new-tide-view/)).
9. **Forecasts change and you don't notice.** "Predictions shifted significantly within hours" ([Windfinder review](https://apps.apple.com/us/app/windfinder-wind-weather-map/id336829635)). Nobody clearly shows that a forecast was upgraded or downgraded since yesterday.
10. **Small things:** no dark mode (Windfinder), the map moving the forecast point when you zoom (Windy.com), app version regressions (WOO). I found no strong source on battery drain, so it is not counted as a theme.

## Persona

### Primary: Jonas, 34, weekend kiter and windsurfer

- **Context:** Lives an hour inland from Sankt Peter-Ording and Lake Constance and has a desk job. He owns 9 m and 12 m kites, a windsurf board with two sails, and recently a wing. He drives 60–90 minutes to a spot, so a wasted trip costs half a day.
- **Goals:** By Thursday, know whether Saturday or Sunday is on and at which spot. In the morning, decide when to leave and which kite to pack. Never go out in offshore wind or a squall.
- **Frustrations:** He checks three apps (Windy, Windguru, a station app) and sees three different numbers. Gusts are under-forecast. The sea breeze is late or never comes. Ads appear on the screen he reads in a hurry. Alerts are paywalled.
- **Devices:** iPhone, often wet or cold. Apple Watch on the water. A car head unit with CarPlay.
- **Usage moments:** Sunday evening, a week look-ahead. Thursday, the weekend decision. 7 am on the day, the decision and the leave time. The car park, "is it on now?". On the water between runs, the watch. Afterwards, "was the forecast right?".

### Secondary: Helen, 52, club dinghy and day keelboat sailor

- **Context:** Sails a Laser or RS Aero at a lake club and crews or skippers a 30-ft keelboat on the coast for day sails. She holds an RYA Day Skipper certificate.
- **Goals:** Choose a day with 8–16 kn, not too gusty, the wind steady in direction, and no thunder. Know when to reef. Time the return with the tide. Have a forecast she can still read in signal-free bays.
- **Frustrations:** PredictWind costs too much for day sailing. Apps show m/s or km/h by default. Inshore forecasts cover too wide an area. Wind shifts and thunderstorms in the afternoon catch her out.
- **Devices:** iPhone in a waterproof case, iPad at the chart table.
- **Usage moments:** The night before. At the club, before rigging. On board, "what does the afternoon bring?". Planning a day passage round a headland.

A third, light persona is **Leo, 19, a learner** at a kite school. He needs plain-language limits ("too strong for you"), safety and the reasons behind them.

## User stories

Tags: priority **[P0]** (needed to ship), **[P1]**, **[P2]**. Status against the current code: **[have]**, **[partial]**, **[missing]**.

### Units, reading and plain language

1. As Jonas, I want wind in knots everywhere so that I can read it the way every rider talks. [P0] [partial] (knots appear only in the "wind" metric note; everything else is m/s)
2. As Helen, I want to choose knots, m/s, km/h, mph or Beaufort so that I can match my club's habit. [P0] [missing]
3. As Helen, I want wave height in metres or feet and temperatures in °C or °F so that I can share forecasts abroad. [P1] [missing]
4. As Jonas, I want each hour rated good, fair or poor with a reason so that I can decide at a glance. [P0] [have]
5. As Leo, I want reasons in plain words ("gusty: sudden pulls") so that I understand the risk. [P0] [have]
6. As Jonas, I want the gust spread shown as "18 g 26" so that I can see the gust range in one number pair. [P1] [partial] (the note shows "wind gusts X")
7. As Helen, I want the Beaufort force and its sea description next to the wind so that it matches the shipping forecast. [P2] [missing]
8. As Jonas, I want the wind direction drawn as an arrow relative to the shore so that I can see onshore or offshore instantly. [P1] [partial] (text "onshore/offshore/cross-shore" only)
9. As Leo, I want a short glossary on tap (gust, cross-onshore, swell period) so that I learn while I check. [P2] [missing]
10. As Helen, I want dark and light themes, including a red night mode, so that I can read the app in sun and on night watch. [P2] [partial] (dark first)

### Spots and places

11. As Jonas, I want to search for any spot or town so that I am not limited to a built-in list. [P0] [missing]
12. As Jonas, I want to save favourite spots in my own order so that my weekend options are one swipe away. [P0] [missing]
13. As Helen, I want the app to use my location so that it opens on my nearest water. [P1] [missing]
14. As Jonas, I want to drop a pin on a map so that I can forecast an unnamed beach. [P1] [missing]
15. As Jonas, I want to set a spot's shore direction by turning an arrow so that onshore and offshore are right for my beach. [P0] [missing] (`facing` exists but is hard-coded per place)
16. As Jonas, I want to mark the safe wind directions for a spot so that a bad direction rules the hour out. [P0] [missing]
17. As Helen, I want lakes treated differently from the sea so that wave and tide noise disappears inland. [P0] [have] (`lake = true`)
18. As Jonas, I want a curated starter list of famous spots so that the app is useful before I add my own. [P1] [partial] (seven sailing places)
19. As Jonas, I want spot notes (launch, hazards, parking) that I write myself so that I remember local knowledge. [P2] [missing]
20. As Jonas, I want to compare my saved spots for one day so that I pick the best place to drive to. [P0] [missing]

### Wind and gear decisions

21. As Jonas, I want to enter my kite sizes and weight so that each hour says which kite to rig. [P0] [missing]
22. As Jonas, I want a windsurf sail size suggestion so that I pack the right rig. [P1] [missing]
23. As Helen, I want a reef suggestion for my boat (full main, one reef, two reefs) so that I can prepare before leaving. [P1] [missing]
24. As Jonas, I want my own minimum and maximum wind so that "good" means good for me, not for an average rider. [P0] [missing] (thresholds are fixed at 3/5/11/15 m/s)
25. As Leo, I want a beginner, intermediate or expert level so that strong wind is ruled out when I am learning. [P0] [missing]
26. As Helen, I want separate profiles for the dinghy and the keelboat so that each gets the right wind band. [P1] [missing]
27. As Jonas, I want gustiness weighed in the rating so that a steady 16 kn beats a gusty 18 kn. [P0] [have]
28. As Jonas, I want lulls below the riding minimum flagged so that I don't get stranded in the lulls. [P1] [missing]
29. As Jonas, I want wind-direction steadiness over the session shown so that I know whether a shift is coming. [P1] [missing]
30. As Helen, I want "too light, drifting" distinguished from "glassy, nice for beginners" so that light days are described fairly. [P2] [partial]
31. As Jonas, I want the rating to say what to do when the wind is too strong ("small kite, experts only") so that it reads as advice, not just a warning. [P1] [have]
32. As Jonas, I want wind at 80 m shown as a hint of what gusts could do in unstable air so that I can judge gust risk. [P2] [missing]
33. As Helen, I want the wind's apparent temperature (wind chill on the water) so that I dress right. [P1] [partial] (cold < 8 °C note only)
34. As Jonas, I want a wetsuit thickness suggestion from water and air temperature so that I pack the right one. [P1] [partial] (one cold rule)

### Forecast trust and models

35. As Jonas, I want to see several models side by side so that I can tell when they agree. [P0] [missing] (one `best_match` request)
36. As Jonas, I want a single confidence word (sure, likely, uncertain) per hour so that I don't have to read a model table. [P0] [missing]
37. As Helen, I want the spread of ensemble members shown as a band on the wind chart so that I can see the range of outcomes. [P1] [missing]
38. As Jonas, I want the highest-resolution model for my region used by default so that local effects are better captured. [P1] [partial] (best_match does some of this)
39. As Jonas, I want to pick my trusted model per spot so that the app follows my local experience. [P1] [missing]
40. As Jonas, I want to know when the forecast last updated and when the next model run is due so that I check at the right time. [P1] [missing]
41. As Jonas, I want to see how the forecast for Saturday has changed since yesterday so that I know if it is trending up or down. [P1] [missing]
42. As Jonas, I want a local correction ("this spot reads 15 % higher") so that the app learns my spot's bias. [P2] [missing]
43. As Helen, I want the app to show gusts as "could reach" with a safety margin when convection is forecast so that squalls don't surprise me. [P1] [missing] (`cape` is fetched but unused by sailing)
44. As Jonas, I want the app to say plainly that sea breezes are poorly forecast so that I check observations. [P2] [missing]
45. As Jonas, I want a sea-breeze likelihood hint (sunny, light gradient wind, warm land, cool sea) so that I can expect an afternoon thermal. [P2] [missing]

### Live conditions

46. As Jonas, I want the nearest live station reading so that I know if it is on right now. [P0] [missing]
47. As Jonas, I want forecast and observed wind on one chart so that I can see whether the forecast is running high or low today. [P1] [missing]
48. As Helen, I want nearby buoy wind and waves so that I can check the open water before leaving harbour. [P1] [missing]
49. As Jonas, I want links to spot webcams so that I can see the water. [P2] [missing]
50. As Jonas, I want the station's age shown ("12 min ago") so that I can trust or ignore it. [P1] [missing]

### Alerts and notifications

51. As Jonas, I want an alert when my spot is forecast good this weekend so that I can plan ahead. [P0] [missing]
52. As Jonas, I want alerts that require the whole window (for example 3 hours) to qualify so that a single good hour doesn't wake me. [P0] [missing]
53. As Jonas, I want an alert when the live station hits my range so that I leave at the right moment. [P1] [missing]
54. As Helen, I want a thunderstorm and gale warning for my saved spots so that I come in in time. [P0] [missing]
55. As Jonas, I want a "forecast upgraded or downgraded" notice for a day I planned so that I can change plans early. [P1] [missing]
56. As Jonas, I want quiet hours for alerts so that I'm not pinged at night. [P1] [missing]
57. As Jonas, I want alerts free, not behind a subscription, so that the core promise is not paywalled. [P0] [missing]

### Timing and planning

58. As Jonas, I want the best window per day shown on the week view so that I can choose my day. [P0] [have]
59. As Jonas, I want a 24-hour verdict band so that I see when the wind arrives and leaves. [P0] [have]
60. As Jonas, I want a "leave home at" time from my drive time so that I arrive when the wind does. [P2] [missing]
61. As Helen, I want sunrise and sunset on the timeline so that I plan to be in before dark. [P0] [have]
62. As Jonas, I want 15-minute detail for today so that I see the exact start of the breeze. [P2] [missing]
63. As Helen, I want a 10–14 day outlook marked "low confidence" so that I can pencil in trips. [P2] [missing] (7 days)
64. As Jonas, I want a "best spot this weekend" card on the hub so that one look answers my week. [P1] [missing]
65. As Helen, I want rain and visibility shown for sailing hours so that I can see whether fog or squalls will cut the day short. [P1] [partial] (fetched; not rated by sailing)

### Sea state, tides and currents

66. As Helen, I want tide times and heights so that I can plan launching and the return. [P0] [missing]
67. As Helen, I want a tidal-stream direction and strength hint so that I know wind-against-tide hours. [P1] [missing]
68. As Helen, I want "wind against tide: short steep chop" flagged so that I avoid the worst hours. [P1] [missing]
69. As Jonas, I want wind waves and swell told apart so that I know whether it's choppy or rolling. [P1] [partial] (`waveHeight` used; wind-wave fields not fetched)
70. As Jonas, I want the wave period and direction for wave riding so that I know if there are ramps or a shore break. [P1] [partial]
71. As Jonas, I want "low tide: shallow lagoon" warnings for spots that dry out so that I don't hit sandbanks. [P2] [missing]
72. As Helen, I want the water temperature so that I judge cold-shock risk if I capsize. [P0] [have]
73. As Helen, I want a comfort word for the sea (flat, choppy, rough) so that guests know what to expect. [P1] [missing]

### Safety

74. As Leo, I want offshore wind to cap the rating for kiting so that I am never encouraged to go out in it. [P0] [have]
75. As Helen, I want offshore wind treated as a caution for dinghies, not a ban, so that the rating fits my sport. [P1] [missing] (one rule for all sports)
76. As Jonas, I want lightning to rule the hour out so that I get off the water. [P0] [have]
77. As Jonas, I want an approaching-storm hint from CAPE and sudden pressure drops so that I see danger hours ahead. [P1] [partial] (`cape` and `pressureTrend` exist, not used)
78. As Helen, I want gale and small-craft warnings from the national service so that official advice is visible. [P2] [missing]
79. As Leo, I want a "go with a buddy, tell someone" checklist on poor or borderline days so that I stay safe. [P2] [missing]
80. As Helen, I want UV and sunburn time shown on bright days so that I protect myself on the water. [P2] [partial] (`uv` fetched)

### Offline and at sea

81. As Helen, I want the last forecast kept on the phone so that I can read it with no signal. [P0] [missing] (10-minute in-memory cache only)
82. As Helen, I want an explicit "save forecast for trip" for several spots so that a weekend away works offline. [P1] [missing]
83. As Helen, I want the age of the saved forecast shown clearly so that I know how stale it is. [P0] [missing]
84. As Helen, I want small downloads so that the app works on weak or expensive data. [P1] [have] (JSON point requests are small)
85. As Helen, I want low battery use so that the phone lasts a day on the water. [P1] [partial] (half-hourly poll; not measured)

### Watch, widgets and glanceability

86. As Jonas, I want a home-screen widget with my spot's verdict and wind so that I don't open the app. [P1] [missing] (live tiles noted as an ORCA gap)
87. As Jonas, I want an Apple Watch view of wind now and the next 6 hours so that I can check between runs. [P2] [missing]
88. As Jonas, I want a lock-screen complication with knots and direction so that I see it in one glance. [P2] [missing]
89. As Jonas, I want huge, high-contrast numbers on the "now" page so that I can read them in the sun with wet hands. [P1] [partial] (big Metro type)

### History and learning

90. As Jonas, I want to log a session (spot, kite, rating) in one tap so that I build my own record. [P2] [missing]
91. As Jonas, I want to compare logged sessions with the forecast so that I learn which model fits my spot. [P2] [missing]
92. As Helen, I want wind statistics by month for a spot so that I can plan a holiday. [P2] [missing]
93. As Jonas, I want my best past days found by conditions so that I know what "perfect" looks like here. [P2] [missing]

### Sharing and business

94. As Jonas, I want to share a clean forecast card with friends so that we agree on the day. [P1] [missing]
95. As Jonas, I want no ads so that I never tap a banner by mistake. [P0] [have]
96. As Jonas, I want one clear price (or a one-off purchase) so that I know what I get. [P1] [missing]
97. As Helen, I want the core forecast free forever so that I trust the app before paying. [P0] [have]
98. As Jonas, I want my spots and settings to sync to my iPad so that I plan on a bigger screen. [P2] [missing]
99. As Helen, I want a clear data attribution and model name so that I know where the forecast comes from. [P1] [missing]
100. As Helen, I want a calm, uncluttered layout with only sailing-relevant rows so that I don't learn a meteorologist's tool. [P0] [have]

## Feature gaps

Ordered by priority. Sizes: S (a day or two), M (about a week), L (several weeks or an engine gap).

### 1. Knots-first units and settings — S/M

- **What:** Knots by default, with m/s, km/h, mph and Beaufort available; metres or feet; °C or °F. Show gusts as "18 g 26". Shared by every edition.
- **Why:** Stories 1, 2, 3, 6, 7. Every competitor leads with knots for wind sports.
- **Data:** None new. Open-Meteo also accepts `wind_speed_unit=kn`, but converting in the app keeps the scoring rules in m/s.
- **Files:** `score.lua` (a fmt helper), `activities/sailing.lua` (note and metrics), `report.lua`, `sky.lua` (Beaufort), a new settings store, `Pages/Hub.xml` or a settings page, `tests/test_weather.lua`.

### 2. Rider profile: sport, level, wind range and gear — M

- **What:** Choose kite, windsurf, wing, dinghy or keelboat; beginner to expert; min and max wind; optional kite and sail sizes and weight. The rating's wind band comes from the profile instead of the fixed 3/5/11/15 m/s. The "now" card says "rig the 9 m".
- **Why:** Stories 21–26, 31, 75; complaint 7 (people don't know what the numbers mean for them). This is the main way to stand out from Windy and Windguru.
- **Data:** None. Kite-size rules of thumb are rider weight × factor ÷ wind; a source still needs choosing (uncertain).
- **Files:** `activities/sailing.lua` (thresholds become parameters; separate offshore rules for dinghies), `activities.lua`, the settings store, a new profile page or template, tests.

### 3. Saved spots, search, location and a shore-direction editor — M/L

- **What:** Search with the Open-Meteo Geocoding API, add a spot by pin or GPS, set the shore direction by turning an arrow, mark the safe direction sector, save and reorder favourites.
- **Why:** Stories 11–16, 18, 20. A fixed list of seven sailing places cannot carry a sailing app. Every competitor has tens of thousands of spots.
- **Data:** [Open-Meteo Geocoding](https://open-meteo.com/en/docs/geocoding-api) (free). Device location needs a native location API; check whether ORCA exposes one (uncertain). A curated spot list with shore directions could be seeded by hand for the top 200 spots.
- **Files:** `places.lua` (static seed plus user spots), `provider.lua` (dynamic place ids), `Pages/Hub.xml` (places section), `Templates/PlaceRow.xml`, a new spot editor page, persistence, tests. README already lists this as "not built".

### 4. Model agreement and a confidence word — M

- **What:** Request 3–5 models in one call with `models=` (for example ECMWF IFS, ICON, GFS, plus the local high-resolution model: AROME, ICON-D2, UKMO 2 km, KNMI, DMI, RRFS). Per hour, show agreement as "sure", "likely" or "uncertain", with a small model strip on the today view. Optionally pick a trusted model per spot.
- **Why:** Stories 35, 36, 38, 39; complaint 2, the most common forum advice. Windguru's table, made readable.
- **Data:** The Open-Meteo Forecast API accepts several models in one call; fields come back suffixed per model. Several models replace the old regional ones: HRRR and NAM are being retired (Nov 2026), replaced by RRFS. ICON global, EU and D2 also show as "discontinued 2026-11-30" in favour of native variants, so model names must be configurable.
- **Files:** `forecast.lua` (models parameter, parsing suffixed columns, a per-hour spread), `activities/sailing.lua` (confidence caps or notes), `report.lua`, `Pages/Activity.xml`, a new template for the model strip, tests with recorded multi-model JSON.

### 5. Wind alerts that require a whole window, free — L

- **What:** Per spot: "tell me when at least N daylight hours are good for my profile in the next X days", plus "storm or gale at my saved spots". Quiet hours, and a downgrade notice.
- **Why:** Stories 51–57; complaint 6. Paywalled alerts are a top grievance, and PredictWind's single-hour trigger is criticised by name.
- **Data:** The same forecast. The difficulty is background fetch and local notifications on iOS: ORCA's support for background tasks and notifications is unverified, and README says live tiles are "not available in ORCA yet". A server-side push alternative adds cost.
- **Files:** a new `alerts.lua` (rule evaluation, testable offline), `score.lua` or `report.lua` (the window logic already exists for "best window"), settings UI, plus native or platform work outside apps/Weather.

### 6. Offline forecast with a clear age — S/M

- **What:** Persist the last forecast per saved spot to disk. Show "updated 3 h ago" and warn when it is stale. Optional "save for trip" covering 7 days for several spots.
- **Why:** Stories 81–84; complaint 8. Cheap for us, because each forecast is a small JSON point response.
- **Data:** None.
- **Files:** `provider.lua` (cache to disk instead of the 10-minute memory cache), `forecast.lua` (keep the raw JSON for re-parsing), `report.lua` (the age line), `Templates/HubNow.xml`, tests with an injected clock.

### 7. Tides and wind against tide — M

- **What:** High and low tide times and heights per coastal spot; wind-against-tide hours flagged; shallow-at-low-tide warnings for spots marked as drying.
- **Why:** Stories 66–68, 71. Helen's top coastal need; Windfinder and Savvy Navvy have it. README lists tides as "not built".
- **Data:** The Open-Meteo Marine API has `sea_level_height_msl` (including tides) and `ocean_current_velocity` and `ocean_current_direction`. It is about 8 km resolution and explicitly "not suitable for coastal navigation", so label it as a guide or use better sources where available. NOAA CO-OPS tide predictions (US, free), UKHO Admiralty API (UK, a free discovery tier, terms uncertain) or WorldTides (paid, global). Find high and low water as local extremes of the hourly curve.
- **Files:** `forecast.lua` (the MARINE field list), `activities/sailing.lua` (rules plus a tide metric and chart), `places.lua` (a `drying` flag), `Pages/Activity.xml`, `report.lua`, tests.

### 8. Gust realism, squall risk and direction shifts — S

- **What:** Use the fields we already fetch. When `cape` is high or pressure drops sharply, say "gusts could be much stronger: squalls possible" and cap or reduce the rating. Flag direction shifts above about 45° within 3 hours. Flag lulls below the profile minimum (wind − (gust − wind) as a rough lull, a heuristic to label as such). Rate rain and visibility for sailing.
- **Why:** Stories 28, 29, 43, 65, 77; complaint 1. Gusts are the most-cited accuracy complaint.
- **Data:** Already fetched: `cape`, `pressureTrend`, `visibility`, `rain`, and wind direction per hour. Optionally add `wind_speed_80m`.
- **Files:** `activities/sailing.lua`, tests.

### 9. Live station and buoy observations — L

- **What:** The nearest station's latest wind, gust and direction with its age, overlaid on today's forecast chart.
- **Why:** Stories 46–50; the three-app workflow. "Is it on now?" is the car-park moment.
- **Data:** Open-Meteo has no observations. Free options: NOAA NDBC buoys and C-MAN stations (US and some global), METAR from aviationweather.gov (airports worldwide, often inland), national open data (DWD, KNMI, DMI, Met Éireann). Station networks such as Holfuy, Tempest and Windguru stations are commercial or need agreements (uncertain). A per-region adapter is needed, with a coverage risk.
- **Files:** a new `observations.lua`, `provider.lua`, `report.lua`, `Pages/Activity.xml` ("now" pivot), tests with recorded payloads.

### 10. Multi-spot "where should I go this weekend?" — M

- **What:** A hub section or card that ranks saved spots for today, tomorrow and the weekend by the profile's best window and confidence.
- **Why:** Stories 20, 64. It answers Jonas's Thursday question directly; no mainstream competitor does this cleanly.
- **Data:** One forecast per spot, which multiplies API calls by the number of spots; keep the cache.
- **Files:** `report.lua` (a ranking), `provider.lua` (loading several spots), `Pages/Hub.xml`, a new template, `Data/Hub.xml`, tests.

### 11. Ensemble range band — M

- **What:** Shade the 10–90 % range of ensemble members on the wind chart; use the spread for the confidence word beyond 48 h.
- **Why:** Stories 37, 63.
- **Data:** The [Open-Meteo Ensemble API](https://open-meteo.com/en/docs/ensemble-api) (ECMWF IFS 51 members, ICON-EU-EPS 40, ICON-D2-EPS 20 for 2 days, GFS 31, MOGREPS-UK). The pricing page lists the ensemble API only for free and Professional or Enterprise plans, not Standard.
- **Files:** `forecast.lua` (a second request), charts in `report.lua` and `Pages/Activity.xml`, tests.

### 12. Forecast trend since yesterday — M

- **What:** "Saturday: up 3 kn since yesterday" on the week view.
- **Why:** Stories 41, 55; complaint 9.
- **Data:** The Open-Meteo Previous Runs API (I believe it exists at /en/docs/previous-runs-api; not checked in this research), or store our own snapshots on the device.
- **Files:** `forecast.lua` or persistence, `report.lua`, the week template.

### 13. Widget and Watch — L (engine)

- **What:** A widget with the verdict, knots and direction; a Watch glance.
- **Why:** Stories 86–88.
- **Data:** None.
- **Files:** native work beyond the app; README and DESIGN mark live tiles and widgets as an ORCA gap.

### 14. Session log and spot bias — M (later)

- **What:** A one-tap log of a session; compare it with the forecast; learn a per-spot correction factor.
- **Why:** Stories 42, 90, 91; complaint 3 (local effects). This gives a long-term reason to stay.
- **Data:** On-device.
- **Files:** a new `log.lua`, a page, persistence.

### 15. Share card — S

- **What:** An image or text of today's band and best window.
- **Why:** Story 94.
- **Files:** `report.lua`, a share action (native share sheet support uncertain).

### Business prerequisite (not optional)

The free Open-Meteo API is for **non-commercial use** only ([pricing](https://open-meteo.com/en/pricing)). A paid or subscription App Store app almost certainly needs an API Standard or Professional plan and the `customer-` endpoint with an `apikey`. Ensembles need Professional. Plan for this before launch, and add the required attribution to the model providers and Open-Meteo. Files: `forecast.lua` (base URL and key), an about screen.

## What we must avoid

- **Ads, and ads near the forecast.** This is the clearest single complaint; a clean app is a selling point.
- **Paywalling safety or the core promise.** Keep alerts for storms and gales, offline, and the confidence word free. Charge for convenience (many spots, widgets, the Watch, ensembles, logs) if we charge at all.
- **Confusing tiers.** Windfinder Plus vs Pro and Windy.app's dozen price rows are bad examples. Use one plan, or one purchase.
- **Hiding uncertainty or pretending to be precise.** Never show one confident number when models disagree by 8 kn. Say "about" and show the range.
- **Trusting raw gusts.** Gusts are systematically smoothed. Add squall warnings and say so rather than presenting the gust column as gospel.
- **Implying navigation-grade tides or currents** from 8 km model data. Open-Meteo itself says "not suitable for coastal navigation." Label it as a guide.
- **Encouraging anyone out in offshore wind, thunder or the dark.** Keep the existing caps, and never let a rider profile override them.
- **Copying Windy's map or PredictWind's routing.** Those are deep, expensive wins for them. Our strength is the decision for a spot, a rider and a time.
- **A Windguru-style data wall.** Show only sailing-relevant rows, as the edition already does.
- **Single-hour alert triggers and nagging notifications.**
- **Hard-coded model names.** HRRR, NAM and some ICON variants are being retired in Nov 2026; make models configurable and fall back to `best_match`.
- **Shipping commercially on the free non-commercial API tier**, or without model attribution.
- **Only seven built-in spots at launch.** Search or a large curated seed is a launch requirement for this edition.
