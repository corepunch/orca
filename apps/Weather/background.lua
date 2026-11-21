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
    local notifier = require("Weather/notify").module(options.notifier)
    -- Alerts need notifications on; the widget (N3) needs a build with an
    -- App Group, which writeWidgetSnapshot reports.
    local widget = notifier and notifier.writeWidgetSnapshot
    if not (Settings.get(Alerts.GROUP, "on") or widget) then return false end
    local now = options.clock and options.clock()
    local home = MyPlaces.mine()[1]
    local plan, forecast = Alerts.refresh { home = home, storage = store, notifier = notifier, fetch = options.fetch,
        now = now }
    if forecast and widget then
        local written = require("Weather/widget").write(forecast, home, { now = now, notifier = notifier })
        if written and notifier.scheduleRefresh then pcall(notifier.scheduleRefresh, Alerts.REFRESH) end
    end
    return plan ~= nil
end
