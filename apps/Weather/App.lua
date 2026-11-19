-- The weather app: a hub per place and a page per activity (DESIGN.md).
-- The hub's page and scope live as long as the app, so its place, panorama
-- position and scroll survive a visit to an activity; an activity page and
-- its scope are made when it opens and disposed when it closes. Data rules
-- live in the provider and activities; this file only navigates.
local ui = require "orca.UIKit"
local Application = require "orca.core.application"
local data = require "orca.data"
local Routes = require "orca.data.routes"
local Provider = require "Weather/provider"
local Activities = require "Weather/activities"
local Places = require "Weather/places"

local App = Application:extend {
    layout = function(self, ctx) return ctx.slots.inner end,

    -- options.places, options.fetch and options.clock replace the places,
    -- the network and the time (tests).
    __init = function(self, options)
        Application.__init(self)
        options = options or {}
        self.places = options.places or Places
        self.placesById = {}
        for _, place in ipairs(self.places) do self.placesById[place.id] = place end
        self.provider = Provider.new(self.places, options)
        self.provider.handlers.openActivity = function(_, params) return self:openActivity(params.item) end
        self.provider.handlers.choosePlace = function(_, params) return self:choosePlace(params.item) end
        self.provider.handlers.goBack = function() return self:goBack() end
        self.store = data.Store.new { schema = data.schema("Weather/Data/Weather.Schema.xml"), scope = "weather",
            computed = self.provider.computed }
        self.routes = Routes.load("Weather/Data/Routes.xml")
        for _, route in ipairs(self.routes.list) do
            self:match(route.path, route.path, function(app) return app:open(route.path) end)
        end
    end,

    screenFor = function(self, page)
        local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = PLATFORM == "ios" and "CanResize" or "NoResize",
            ClearColor = "#000000", StyleSheet = "Weather/Styles/weather.css" }
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
            end
            self.hubScope:activate()
            return self.hubScreen
        end
        if self.hubScope then self.hubScope:deactivate() end
        local page, scope = self.routes:open(path, self, options)
        self.activityScope = scope
        page.StateClasses = self.activity.id
        page:findChild("ActivityPivot", true).Title = self.activity.title .. " · " .. self.place.title
        self:configureCharts(page, self.activity.charts or {})
        scope:activate()
        return self:screenFor(page)
    end,

    -- The "today" charts follow the activity: a title, columns or a line,
    -- and an optional fixed range (percentages run 0 to 100).
    configureCharts = function(self, page, charts)
        for index, name in ipairs { "A", "B" } do
            local spec, chart = charts[index], page:findChild("Chart" .. name, true)
            chart.Visible = spec ~= nil
            if spec then
                local columns, line = chart:findChild("Chart" .. name .. "Columns"), chart:findChild("Chart" .. name .. "Line")
                columns.Visible, line.Visible = spec.kind ~= "line", spec.kind == "line"
                local shown = spec.kind == "line" and line or columns
                shown.Title = spec.title
                chart.Minimum, chart.Maximum = spec.min or 0 / 0, spec.max or 0 / 0
            end
        end
    end,

    selectedPlace = function(self)
        local selected = self.hubScope and self.hubScope:member("PlaceList").selected
        return selected and self.placesById[selected.key]
    end,

    -- A "good for" row: open its activity at the selected place.
    openActivity = function(self, record)
        local activity = record and Activities.byId[record.values.Activity]
        local place = self:selectedPlace()
        if not (activity and place) then return end
        self.activity, self.place = activity, place
        return self:activate_route("/activity")
    end,

    -- A place row: show its hub from the first section.
    choosePlace = function(self, record)
        if not record then return end
        self.hubScope:member("PlaceList"):select(record)
        self.hubPage:findChild("HubPanorama", true).SelectedIndex = 0
    end,

    goBack = function(self) return self:activate_route("/") end,
}

return App
