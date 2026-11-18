-- The application shell every weather profile app shares. A profile app's
-- App.lua is one line: `return require("WeatherKit/App")(require "Fishing/profile")`.
-- The shell owns one store, the provider and the single route; the page,
-- scope and styles come from the kit, the verdicts from the profile.
local ui = require "orca.UIKit"
local Application = require "orca.core.application"
local data = require "orca.data"
local Routes = require "orca.data.routes"
local Provider = require "WeatherKit/provider"

return function(profile)
    return Application:extend {
        layout = function(self, ctx) return ctx.slots.inner end,

        -- options.fetch and options.clock replace the network and the time (tests).
        __init = function(self, options)
            Application.__init(self)
            options = options or {}
            self.profile = profile
            self.provider = options.provider or Provider.new(profile, options)
            self.store = data.Store.new { schema = data.schema("WeatherKit/Data/Weather.Schema.xml"), scope = profile.id,
                computed = self.provider.computed }
            self.routes = Routes.load("WeatherKit/Data/Routes.xml")
            for _, route in ipairs(self.routes.list) do
                self:match(route.path, route.path, function(app) return app:open(route.path) end)
            end
        end,

        open = function(self, path)
            if self.scope then self.scope:dispose() end
            local page, scope = self.routes:open(path, self, { provider = self.provider, store = self.store, host = self.host })
            self.scope = scope
            page:findChild("WeatherPanorama", true).Title = profile.title
            local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = PLATFORM == "ios" and "CanResize" or "NoResize",
                ClearColor = "#000000", StyleSheet = "WeatherKit/Styles/weather.css" }
            screen:addChild(page)
            scope:activate()
            return screen
        end,
    }
end
