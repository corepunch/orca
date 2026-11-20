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

local App

-- The full app lets you choose and order what "good for" lists (W7):
-- outdoors' settings gain the order and one switch per other activity. An
-- edition has only its own, so it keeps them all.
local function offerFollowing()
    local outdoors = Activities.byId.outdoors
    if Activities.edition or not outdoors or outdoors.following then return end
    outdoors.following = true
    outdoors.settings = outdoors.settings or {}
    local list = outdoors.settings
    list[#list + 1] = { key = "order", title = "good for order", kind = "choice", default = "listed",
        options = { { "listed", "as listed" }, { "best", "best first" } } }
    for _, activity in ipairs(Activities.list) do
        if activity.id ~= "outdoors" then
            list[#list + 1] = { key = "follow_" .. activity.id, title = "good for " .. activity.title, kind = "toggle",
                default = activity.followed ~= false }
        end
    end
end

App = Application:extend {
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
        offerFollowing()
        Settings.init { storage = self.storage, edition = Activities.edition, region = region }
        self.provider = Provider.new(self.places,
            { fetch = options.fetch, clock = options.clock, storage = self.storage })
        -- The hub's everyday answers and the day page, which the shared
        -- provider does not know.
        self.provider.filters.answerRows = function(record) return record.values.Kind == "answer" end
        self.provider.operations.day = function(params)
            local forecast, err, place = self.provider.forecastFor(params.PlaceId)
            if not forecast then return nil, err end
            return self.provider.drew(Report.day(place, forecast, params.Date, Activities.at(place)), forecast)
        end
        self.provider.drawn = function(forecast) self:restyle(forecast) end
        local handlers = self.provider.handlers
        handlers.openActivity = function(scope, params) return self:openActivity(params.item, scope) end
        handlers.openDay = function(_, params) return self:openDay(params.item) end
        handlers.choosePlace = function(_, params) return self:choosePlace(params.item) end
        handlers.openSettings = function() return self:activate_route("/settings") end
        handlers.changeSetting = function(scope, params) return self:changeSetting(scope, params.item, 1) end
        handlers.lowerSetting = function(scope, params) return self:changeSetting(scope, params.item, -1) end
        handlers.goBack = function() return self:goBack() end
        handlers.cardAction = function(scope, params) return self:cardAction(scope, params.item, params.sender) end
        handlers.openActivityPage = function()
            local page = self.activity and self.activity.page
            if not page then return end
            self.pageReturn = "/activity"
            return self:activate_route(page.path)
        end
        self.store = data.Store.new { schema = data.schema("Weather/Data/Weather.Schema.xml"), scope = "weather",
            computed = self.provider.computed }
        self.routes = Routes.load("Weather/Data/Routes.xml")
        for _, route in ipairs(self.routes.list) do
            self:match(route.path, route.path, function(app) return app:open(route.path) end)
        end
        -- An activity's own page (`activity.page`): its routes, opened with its provider.
        for _, activity in ipairs(Activities.list) do
            if activity.page then
                local routes = Routes.load(activity.page.routes)
                for _, route in ipairs(routes.list) do
                    self:match(route.path, route.path, function(app) return app:openActivityPage(activity, routes, route.path) end)
                end
            end
        end
    end,

    -- An edition may bring its own sheet (activity.stylesheet, a resource
    -- path, or a function returning one) that @imports Styles/weather.css
    -- and adds to it.
    styleSheet = function(self)
        local lead = Activities.edition and Activities.byId[Activities.edition]
        local sheet = lead and lead.stylesheet
        if type(sheet) == "function" then sheet = sheet() end
        return sheet or "Weather/Styles/weather.css"
    end,

    screenFor = function(self, page)
        local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = PLATFORM == "ios" and "CanResize" or "NoResize",
            ClearColor = "#000000", StyleSheet = self:styleSheet() }
        screen:addChild(page)
        self:dress(screen, self:pageClasses())
        return screen
    end,

    -- The edition's own page classes now (`lead.pageClasses(forecast)`,
    -- such as red night mode's "night"), as a string; `forecast` is the one
    -- just drawn, if any.
    pageClasses = function(self, forecast)
        local lead = Activities.edition and Activities.byId[Activities.edition]
        return lead and lead.pageClasses and lead.pageClasses(forecast) or ""
    end,

    -- Puts a screen's page in `extra` classes (the ones it wore before
    -- taken off) and the sheet of the moment. A new sheet styles only nodes
    -- made after it, so changing the classes with it restyles the rest.
    dress = function(self, screen, extra)
        self.worn = self.worn or setmetatable({}, { __mode = "k" })
        local page, old = screen:getFirstChild(), self.worn[screen] or ""
        if not page or old == extra then return end
        local drop, list = {}, {}
        for name in old:gmatch("%S+") do drop[name] = true end
        for name in (page.StateClasses or ""):gmatch("%S+") do if not drop[name] then list[#list + 1] = name end end
        for name in extra:gmatch("%S+") do list[#list + 1] = name end
        screen.StyleSheet = self:styleSheet()
        page.StateClasses = table.concat(list, " ")
        self.worn[screen] = extra
    end,

    -- A page was drawn: the hub and the screen in front follow the edition's
    -- page classes (red mode turns on at dusk, or when the setting changes).
    restyle = function(self, forecast)
        local extra = self:pageClasses(forecast)
        if self.hubScreen then self:dress(self.hubScreen, extra) end
        if self.screen and self.screen ~= self.hubScreen then self:dress(self.screen, extra) end
    end,

    open = function(self, path)
        local options = { provider = self.provider, store = self.store, host = self.host }
        if self.activityScope then self.activityScope:dispose(); self.activityScope = nil end
        if path == "/" then
            if not self.hubScreen then
                self.hubPage, self.hubScope = self.routes:open("/", self, options)
                -- An edition's hub wears its activity's accent.
                if Activities.edition then self.hubPage.StateClasses = Activities.edition end
                local lead = Activities.lead()
                if lead then self.hubPage:findChild("HubBandTitle", true).Text = lead.title .. ", next 24 hours" end
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
            local link, own = page:findChild("OpenActivityPage", true), self.activity.page
            link.Visible = own ~= nil
            if own then link.Text = own.link end
        end
        scope:activate()
        return self:screenFor(page)
    end,

    -- An activity's own page, over the provider it makes from the app's:
    -- the place's forecast, every place, the place shown, the clock and Back.
    openActivityPage = function(self, activity, routes, path)
        if self.activityScope then self.activityScope:dispose(); self.activityScope = nil end
        if self.hubScope then self.hubScope:deactivate() end
        local provider = activity.page.provider { forecastFor = self.provider.forecastFor, places = self.places,
            place = function() return self.place end, clock = self.provider.clock,
            back = function() return self:activate_route(self.pageReturn or "/activity") end }
        local page, scope = routes:open(path, self, { provider = provider, store = self.store, host = self.host })
        self.activityScope = scope
        page.StateClasses = activity.id
        scope:activate()
        return self:screenFor(page)
    end,

    -- A card's link: its activity acts (`activity.act(id, context)`) at the
    -- place shown. A returned path opens that route (settings, the
    -- activity's own page); otherwise the cards redraw from the forecast
    -- the app already has.
    cardAction = function(self, scope, record, sender)
        local values = record and record.values
        local activity = values and Activities.byId[values.Activity]
        local index = sender and tostring(sender.Name or ""):match("(%d)$") or "1"
        local id = values and values["Action" .. index]
        local onHub = scope == self.hubScope
        local place = onHub and self:selectedPlace() or self.place
        if not (activity and activity.act and id and id ~= "" and place) then return end
        local forecast = self.provider.forecastFor(place.id, nil, not onHub and activity or nil)
        local path = activity.act(id, { place = place, forecast = forecast, clock = self.provider.clock(),
            where = onHub and "hub" or "activity", card = values.Id })
        if type(path) == "string" then
            self.activity, self.place = activity, place
            if onHub then self.backPath, self.pageReturn = nil, "/" else self.pageReturn = "/activity" end
            return self:activate_route(path)
        end
        self.provider.redrawing = true
        scope:member("Report"):refresh { force = true }
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

    -- The "today" charts follow the activity: a title, its unit and a note
    -- ("2 days back"), columns or a line, and an optional fixed range
    -- (percentages run 0 to 100).
    configureCharts = function(self, page, charts)
        for index, name in ipairs { "A", "B", "C" } do
            local spec, chart = charts[index], page:findChild("Chart" .. name, true)
            chart.Visible = spec ~= nil
            if spec then
                local columns, line = chart:findChild("Chart" .. name .. "Columns"), chart:findChild("Chart" .. name .. "Line")
                columns.Visible, line.Visible = spec.kind ~= "line", spec.kind == "line"
                local shown = spec.kind == "line" and line or columns
                local unit = spec.unit and Units.label(spec.unit) or ""
                shown.Title = (unit ~= "" and spec.title .. " " .. unit or spec.title)
                    .. (spec.note and " · " .. spec.note or "")
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
