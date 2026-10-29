package.path = "apps/chat/?.lua;" .. package.path
local savedCore, savedPolling = package.loaded["orca.core"], package.loaded["Chat/polling"]
package.loaded["Chat/polling"] = require "polling"
local owner
package.loaded["orca.core"] = { Timer = function(props)
    owner = props
    owner.nextId, owner.delays, owner.canceled = 0, {}, {}
    function owner:setTimer(delay)
        self.nextId = self.nextId + 1
        self.delays[self.nextId] = delay
        return self.nextId
    end
    function owner:cancelTimer(id) self.canceled[id] = true end
    return owner
end }
local Sync = require "sync"
local function fixture()
    local session = { user = { id = "me" } }
    local screen = {}
    local app = { screen = screen, navigationEpoch = 1, sessionEpoch = 1, composer = { Text = "draft" } }
    app.client = { getSession = function() return session end }
    function app:notice() error("Quiet polling must not replace foreground status") end
    function app:resetSessionViews() self.sessionEpoch = self.sessionEpoch + 1 end
    function app:show(route) self.redirect = route end
    function app:logout() session = nil end
    return app
end
local function fire(id)
    id = id or owner.nextId
    if owner.Tick then owner.Tick(owner, { timerId = id }) end
end
local app = fixture()
local sync = Sync.new(app)
-- The inbox polls through its Rooms query (tests/test_chat_inbox.lua); the
-- timer bridge serves the open conversation only.
sync:activate()
assert(owner.nextId == 0, "Without an open thread nothing is scheduled")

-- Thread polling preserves scroll/drafts and uses its own three-second cadence.
app.room = { id = "thread" }
app.messageList = { getRoot = function() return app.screen end }
function app:renderMessages(options)
    assert(options.current() and options.preserveScroll and options.quiet)
    self.threadReads = (self.threadReads or 0) + 1
    if self.threadHook then self.threadHook() end
    return true
end
sync:activate()
assert(owner.delays[owner.nextId] == 3000)
app.threadHook = function() assert(owner.canceled[1], "Native repeating timer must stop before HTTP work") end
fire()
app.threadHook = nil
assert(app.threadReads == 1 and app.composer.Text == "draft")
fire(1)
assert(app.threadReads == 1, "Queued ticks from canceled timers must do nothing")
sync:setForeground(false)
fire()
assert(app.threadReads == 1)
sync:setForeground(true)
assert(owner.delays[owner.nextId] == 1, "Resume must schedule an immediate refresh")
fire()
assert(app.threadReads == 2)
app.sending = true
fire()
assert(app.threadReads == 2, "Polling must not compete with an active send")
app.sending = false
function app:renderMessages() self:logout(); return nil, { status = 401 } end
fire()
assert(app.redirect == "/", "Revocation must remove authenticated views")
package.loaded["orca.core"], package.loaded["Chat/polling"] = savedCore, savedPolling
package.loaded.sync = nil
print("PASS: Chat timer bridge, foreground lifecycle and thread polling")
