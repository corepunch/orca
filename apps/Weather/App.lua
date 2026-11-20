-- The weather app: a hub per place and a page per activity (DESIGN.md).
-- The hub's page and scope live as long as the app, so its place, panorama
-- position and scroll survive a visit to an activity; an activity page and
-- its scope are made when it opens and disposed when it closes, and so are
-- settings. Data rules live in the provider and activities; this file only
-- navigates and titles.
local ui = require "orca.UIKit"
local Application = require "orca.core.application"
local data = require "orca.data"
local Routes = require "orca.data.routes"
local Provider = require "Weather/provider"
local Activities = require "Weather/activities"
local Places = require "Weather/places"
local Storage = require "Weather/storage"
local Settings = require "Weather/settings"
local Units = require "Weather/units"
local Report = require "Weather/report"
local Sky = require "Weather/sky"

local App = Application:extend {
    layout = function(self, ctx) return ctx.slots.inner end,

    -- options.places, options.fetch and options.clock replace the places,
    -- the network and the time; options.storage (a Storage store) or
    -- options.storageDir the folder for files, and options.region the
    -- region the default units come from (tests).
    __init = function(self, options)
        Application.__init(self)
        options = options or {}
        self.places = options.places or Places
        self.placesById = {}
        for _, place in ipairs(self.places) do self.placesById[place.id] = place end
        self.storage = options.storage or (options.storageDir and Storage.new { dir = options.storageDir })
            or Storage.default()
        Storage.use(self.storage)
        local region = options.region
        if region == nil then region = Units.region() end
        Settings.init { storage = self.storage, edition = Activities.edition, region = region }
        self.provider = Provider.new(self.places,
            { fetch = options.fetch, clock = options.clock, storage = self.storage })
        -- The hub's everyday answers and the day page, which the shared
        -- provider does not know.
        self.provider.filters.answerRows = function(record) return record.values.Kind == "answer" end
        self.provider.operations.day = function(params)
            local forecast, err, place = self.provider.forecastFor(params.PlaceId)
            if not forecast then return nil, err end
            return Report.day(place, forecast, params.Date, Activities.at(place))
        end
        local handlers = self.provider.handlers
        handlers.openActivity = function(scope, params) return self:openActivity(params.item, scope) end
        handlers.openDay = function(_, params) return self:openDay(params.item) end
        handlers.choosePlace = function(_, params) return self:choosePlace(params.item) end
        handlers.openSettings = function() return self:activate_route("/settings") end
        handlers.changeSetting = function(scope, params) return self:changeSetting(scope, params.item, 1) end
        handlers.lowerSetting = function(scope, params) return self:changeSetting(scope, params.item, -1) end
        handlers.goBack = function() return self:goBack() end
        self.store = data.Store.new { schema = data.schema("Weather/Data/Weather.Schema.xml"), scope = "weather",
            computed = self.provider.computed }
        self.routes = Routes.load("Weather/Data/Routes.xml")
        for _, route in ipairs(self.routes.list) do
            self:match(route.path, route.path, function(app) return app:open(route.path) end)
        end
    end,

    -- An edition may bring its own sheet (activity.stylesheet, a resource
    -- path) that @imports Styles/weather.css and adds to it.
    styleSheet = function(self)
        local lead = Activities.edition and Activities.byId[Activities.edition]
        return lead and lead.stylesheet or "Weather/Styles/weather.css"
    end,

    screenFor = function(self, page)
        local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = PLATFORM == "ios" and "CanResize" or "NoResize",
            ClearColor = "#000000", StyleSheet = self:styleSheet() }
        screen:addChild(page)
        return screen
    end,

    open = function(self, path)
        local options = { provider = self.provider, store = self.store, host = self.host }
        if self.activityScope then self.activityScope:dispose(); self.activityScope = nil end
        if path == "/" then
            if not self.hubScreen then
                self.hubPage, self.hubScope = self.routes:open("/", self, options)
                -- An edition's hub wears its activity's accent.
                if Activities.edition then self.hubPage.StateClasses = Activities.edition end
                self.hubScreen = self:screenFor(self.hubPage)
                self.hubRevision = Settings.revision()
            end
            self:titleCharts(self.hubPage)
            self.hubScope:activate()
            -- Changed units redraw the hub from the forecast it already has.
            if self.hubRevision ~= Settings.revision() then
                self.hubRevision = Settings.revision()
                self.provider.redrawing = true
                self.hubScope:member("Report"):refresh { force = true }
            end
            return self.hubScreen
        end
        if self.hubScope then self.hubScope:deactivate() end
        local page, scope = self.routes:open(path, self, options)
        self.activityScope = scope
        if path == "/settings" then
            if Activities.edition then page.StateClasses = Activities.edition end
            page:findChild("SettingsPivot", true).Title = Activities.appName:lower()
            local edition = Activities.byId[Activities.edition]
            if edition then page:findChild("ActivitySettingsSection", true).Header = edition.title end
        elseif path == "/day" then
            if Activities.edition then page.StateClasses = Activities.edition end
            page:findChild("DayPivot", true).Title = Sky.weekday(self.date) .. " · " .. self.place.title
            self:titleCharts(page)
        else
            page.StateClasses = self.activity.id
            page:findChild("ActivityPivot", true).Title = self:activityTitle(self.activity, self.place)
            self:configureCharts(page, self.activity.charts or {})
        end
        scope:activate()
        return self:screenFor(page)
    end,

    -- "fishing · Lake Tahoe"; an edition's own activity says the app's name
    -- instead ("surfers' weather · Bondi") when the line has room for it.
    activityTitle = function(self, activity, place)
        local title = activity.title .. " · " .. place.title
        if activity.id ~= Activities.edition then return title end
        local named = Activities.appName:lower() .. " · " .. place.title
        return utf8.len(named) <= 34 and named or title
    end,

    -- A page's hourly charts (the hub's, a day's), titled in the user's units.
    titleCharts = function(self, page)
        local function title(name, text) local node = page:findChild(name, true); if node then node.Title = text end end
        title("TempColumns", "temperature " .. Units.label("temp"))
        title("WindLine", "wind " .. Units.label("wind"))
        title("RainColumns", "chance of rain %")
    end,

    -- A setting row: step it and redraw the settings rows.
    changeSetting = function(self, scope, record, direction)
        local group, key = Settings.split(record and record.values.Setting)
        if not (group and Settings.step(group, key, direction)) then return end
        scope:member("Settings"):refresh { force = true }
    end,

    -- The "today" charts follow the activity: a title and its unit, columns
    -- or a line, and an optional fixed range (percentages run 0 to 100).
    configureCharts = function(self, page, charts)
        for index, name in ipairs { "A", "B" } do
            local spec, chart = charts[index], page:findChild("Chart" .. name, true)
            chart.Visible = spec ~= nil
            if spec then
                local columns, line = chart:findChild("Chart" .. name .. "Columns"), chart:findChild("Chart" .. name .. "Line")
                columns.Visible, line.Visible = spec.kind ~= "line", spec.kind == "line"
                local shown = spec.kind == "line" and line or columns
                local unit = spec.unit and Units.label(spec.unit) or ""
                shown.Title = unit ~= "" and spec.title .. " " .. unit or spec.title
                chart.Minimum = spec.min and Units.value(spec.unit, spec.min) or 0 / 0
                chart.Maximum = spec.max and Units.value(spec.unit, spec.max) or 0 / 0
            end
        end
    end,

    selectedPlace = function(self)
        local selected = self.hubScope and self.hubScope:member("PlaceList").selected
        return selected and self.placesById[selected.key]
    end,

    -- A "good for" row: open its activity at the selected place. From a
    -- day page, Back returns to that day.
    openActivity = function(self, record, scope)
        local activity = record and Activities.byId[record.values.Activity]
        local place = self:selectedPlace()
        if not (activity and place) then return end
        self.activity, self.place = activity, place
        self.backPath = scope and scope ~= self.hubScope and "/day" or nil
        return self:activate_route("/activity")
    end,

    -- A day of the hub's week: open its hours.
    openDay = function(self, record)
        local date = record and record.values.Id:match(":day:(.+)$")
        local place = self:selectedPlace()
        if not (date and place) then return end
        self.date, self.place, self.backPath = date, place, nil
        return self:activate_route("/day")
    end,

    -- A place row: show its hub from the first section.
    choosePlace = function(self, record)
        if not record then return end
        self.hubScope:member("PlaceList"):select(record)
        self.hubPage:findChild("HubPanorama", true).SelectedIndex = 0
    end,

    goBack = function(self)
        local path = self.backPath or "/"
        self.backPath = nil
        return self:activate_route(path)
    end,
}

return App
