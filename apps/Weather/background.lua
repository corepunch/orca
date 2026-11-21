-- The background refresh (N2), best effort. iOS runs this now and then
-- (BGAppRefreshTask, about every few hours at the system's choice) in a
-- Lua state of its own, without screens or the engine's objects: only the
-- shared modules, orca.network and orca.notifications. It fetches the home
-- place's forecast, plans the alerts again and reschedules them. Returns
-- false when there was nothing to do or it failed; the next refresh tries
-- again. `options` (tests only) may pass fetch, notifier and clock.
return function(options)
    options = options or {}
    local Storage = require "Weather/storage"
    local store = Storage.default()
    if not store then return false end
    Storage.use(store)
    local Activities = require "Weather/activities"
    local Units = require "Weather/units"
    local Settings = require "Weather/settings"
    Settings.init { storage = store, edition = Activities.edition, region = Units.region(), device = Units.device() }
    local MyPlaces = require "Weather/myplaces"
    MyPlaces.init { storage = store, curated = require "Weather/places" }
    local Alerts = require "Weather/alerts"
    if not Settings.get(Alerts.GROUP, "on") then return false end
    local notifier = require("Weather/notify").module(options.notifier)
    local plan = Alerts.refresh { home = MyPlaces.mine()[1], storage = store, notifier = notifier, fetch = options.fetch,
        now = options.clock and options.clock() }
    return plan ~= nil
end
