-- This controller is copied into an isolated simulator bundle by capture.py.
-- Production Chat's package, startup, transport and user data are never changed.
local App = require "Chat/App"
local Chat = require "api.chat"
local Fixtures = require "Chat/promo/fixtures"

return setmetatable({}, {
    __call = function()
        local client = Chat.mock { fixtures = Fixtures.new, clock = function() return Fixtures.now end }
        assert(client:signIn("demo@example.invalid", "password123"))
        local self = App { client = client }
        self.pinned = { alex = true, weekend = true, coffee = true }
        local stage = os.getenv("CHAT_PROMO_STAGE") or "inbox"
        local activateRoute, started = self.activate_route, false
        self.activate_route = function(app, route)
            if not started then
                started = true
                route = stage:match("^contacts") and "/contacts" or stage == "inbox" and "/" or "/thread"
            end
            return activateRoute(app, route)
        end
        if stage == "direct" or stage == "group" or stage == "photos" or stage == "channel" then
            -- Direct thread startup skips the inbox, which normally prepares
            -- the authorized contact models used for group sender names.
            self.contacts, self.contactsById = assert(self.models.Contacts:select()), {}
            for _, contact in ipairs(self.contacts) do self.contactsById[contact.id] = contact end
            local id = ({ direct = "alex", group = "weekend", photos = "jules", channel = "coffee" })[stage]
            for _, room in ipairs(assert(self.models.Rooms:select())) do
                if room.id == id then self.room = room; break end
            end
            assert(self.room, "Missing promo conversation " .. id)
        end
        if stage == "photos" then
            local f = assert(io.open("Chat/promo/assets/lake.jpg", "rb"))
            local bytes = f:read("*a"); f:close()
            local attachment = { id = "promo-lake", kind = "photo", width = 1280, height = 960,
                object_key = "jules/promo-lake/photo.jpg", thumbnail_key = "jules/promo-lake/thumbnail.jpg" }
            local listMessages = client.listMessages
            client.listMessages = function(c, ...)
                local rows, err = listMessages(c, ...)
                if rows then for _, row in ipairs(rows) do
                    if row.id == "jules-3" then row.body, row.attachments = "", { attachment } end
                end end
                return rows, err
            end
            local listPage = client.listMessagePage
            if listPage then client.listMessagePage = function(c, ...)
                local page, err = listPage(c, ...)
                if page then for _, row in ipairs(page.messages or {}) do
                    if row.id == "jules-3" then row.body, row.attachments = "", { attachment } end
                end end
                return page, err
            end end
            client.getPhoto = function() return bytes end
        end
        if stage == "contacts-more" or stage == "contacts-all" then
            local afterPaint, panned = self.afterPaint, false
            self.afterPaint = function(app)
                afterPaint(app)
                if panned or not app.contactsState or not app.contactsState.contactsLoaded then return end
                panned = true
                if stage == "contacts-all" then
                    app.contactsPanorama.SelectedIndex = 1
                    app.screen:post("Window.Paint", require("orca.renderer").getSize())
                    return
                end
                -- Exercise the real public gesture path; leave native Panorama
                -- to choose its internal snap rather than assigning private offsets.
                local system = require "orca.system"
                local x, y, dx = 320, 330, -170
                system.dispatchMessage { target = app.screen, message = "LeftButtonDown", x = x, y = y }
                for step = 1, 4 do
                    system.dispatchMessage { target = app.screen, message = "LeftButtonDragged", x = x + dx * step / 4, y = y }
                end
                system.dispatchMessage { target = app.screen, message = "LeftButtonUp", x = x + dx, y = y }
                app.screen:post("Window.Paint", require("orca.renderer").getSize())
            end
        end
        return self
    end,
})
