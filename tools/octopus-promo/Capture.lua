-- This controller is copied into an isolated simulator bundle by capture.py.
-- Production Octopus's package, startup, backend and user data are never changed.
local Octopus = require "Octopus/api/octopus"
local Fixtures = require "Octopus/promo/fixtures"

return setmetatable({}, {
    __call = function()
        local stage = os.getenv("OCTOPUS_PROMO_STAGE") or "home"
        local clock = function() return Fixtures.now end
        local transport, state = require("Octopus/api/mock").new { fixtures = Fixtures, clock = clock }
        for id, file in pairs(Fixtures.files) do
            local f = assert(io.open(file.path, "rb"))
            state.files[id] = { bytes = f:read("*a"), owner = file.owner }
            f:close()
        end
        local client = assert(Octopus.client { mode = "mock", transport = transport,
            projectId = "octopus-mock", clock = clock })
        assert(client:signIn("demo@example.invalid", "password123"))
        local App = require "Octopus/App"
        local self = App { client = client }
        local feeds = { home = "home", connect = "connect", discover = "discover", light = "home" }
        if feeds[stage] then self.feed = feeds[stage] end
        local route = ({ post = "/post", album = "/post", profile = "/user", write = "/write", me = "/me" })[stage] or "/"
        if stage == "post" or stage == "album" then
            self.post = assert(client:getPost(stage == "post" and "p-lake" or "p-bookclub"))
        end
        if stage == "profile" then self.profileId, self.profileName = "u-maya", "Maya Chen" end
        -- The mock's own status line is a developer notice, not app content.
        local notice = self.notice
        self.notice = function(app, text, isError)
            if text == "Offline mock · posts stay in this session" then text = "" end
            return notice(app, text, isError)
        end
        local activateRoute, started = self.activate_route, false
        self.activate_route = function(app, requested)
            if not started then started = true; requested = route end
            return activateRoute(app, requested)
        end
        return self
    end,
})
