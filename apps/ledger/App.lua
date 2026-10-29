-- The ledger app: one store, the provider and the route registry. Every
-- screen is a route (Data/Routes.xml) with a scope and an XML page; this
-- file only gives pages a screen.
local ui = require "orca.UIKit"
local Application = require "orca.core.application"
local data = require "orca.data"
local Routes = require "orca.data.routes"
local LedgerProvider = require "Ledger/providers/ledger"

local App = Application:extend {
    layout = function(self, ctx) return ctx.slots.inner end,

    __init = function(self, options)
        Application.__init(self)
        options = options or {}
        self.provider = options.provider or LedgerProvider.new()
        self.store = data.Store.new { schema = data.schema("Ledger/Data/Ledger.Schema.xml"), scope = "ledger",
            computed = self.provider.computed }
        self.routes = Routes.load("Ledger/Data/Routes.xml")
        for _, route in ipairs(self.routes.list) do
            self:match(route.path, route.path, function(app) return app:open(route.path) end)
        end
    end,

    open = function(self, path)
        if self.scope then self.scope:dispose() end
        local page, scope = self.routes:open(path, self, { provider = self.provider, store = self.store, host = self.host })
        self.scope = scope
        local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = PLATFORM == "ios" and "CanResize" or "NoResize",
            ClearColor = "#ffffff", StyleSheet = "Ledger/Styles/app.css" }
        screen:addChild(page)
        scope:activate()
        return screen
    end,
}

return App
