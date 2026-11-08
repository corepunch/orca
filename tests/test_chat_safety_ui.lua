local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/chat")
require("orca.core.startup").load_plugins()
local App, Chat = require "Chat/App", require "api.chat"
local savedAsync = orca.async
orca.async = function(fn, ...) return fn(...) end
local function drain()
    while true do
        local msg = orca.system.peekMessage()
        if not msg then return end
        if not msg:is("Window.Paint") then orca.system.dispatchMessage(nil, msg) end
    end
end
local client = Chat.demo()
local app = App { client = client, language = "en" }
app.show = function(self, route) self.screen = self:dispatch(route or "/").view; drain() end
local function click(name)
    assert(app.screen:findChild(name, true), "Missing " .. name):send("Node.LeftButtonUp")
    drain()
end
app:show("/")
local room = assert(app.scope.store:get("ChatRoom", "alex"))
app.room = room.raw
app:show("/thread")
local message = assert(app.threadScope.store:get("ChatMessage", "a1"))
assert(message.values.CanReport, "Readers can report incoming messages")
app:openMessageActions(message)
click("ReportMessage")
assert(app.screen:findChild("SafetyPage", true) and app.safetyTarget.messageId == "a1",
    "The message action opens the XML safety page with authorized evidence")
app.screen:UpdateLayout(390, 844)
click("BlockPerson")
assert(#assert(client:listBlocks()) == 1, "Block is saved through the shared client API")
app:show("/blocked")
assert(app.routeScope:member("Blocks"):records()[1].values.UserId == "alex",
    "Settings keeps a persisted unblock path when discovery hides the person")
local blocked = assert(app.screen:findChild("Blockedalex", true))
blocked:send("Node.LeftButtonUp")
drain()
assert(app.screen:findChild("SafetyPage", true) and app.safetyTarget.userId == "alex"
    and app.safetyTarget.back == "/blocked", "The bound blocked-user row opens its unblock action")
app:openSafety("alex", "a1", "Alex Morgan", "/blocked")
app.screen:findChild("ReportDetails", true).Text = "Review this message"
click("ReportHarassment")
assert(not app.screen:findChild("ReportReasons", true).Visible
    and app.screen:findChild("SafetyStatus", true):getTextContent():find("Report submitted", 1, true),
    "The page confirms submitted reports and prevents duplicate reason submissions")
click("BlockPerson")
assert(#assert(client:listBlocks()) == 0, "The same control unblocks after server success")
app:show("/details")
local member = app.routeScope.store:get("ChatMember", "alex:alex")
assert(member and member.values.CanReport, "Conversation members expose safety actions for other people")
assert(app.screen:findChild("Memberalex", true)):findChild("MemberSafety", true):send("Node.LeftButtonUp")
drain()
assert(app.screen:findChild("SafetyPage", true) and not app.safetyTarget.messageId,
    "A person can be reported or blocked independently of one message")
app:show("/rules")
assert(app.screen:findChild("CommunityRulesText", true).Source:find("abuse", 1, true), "Rules are readable from the bundled document")
click("AcceptCommunityRules")
assert(assert(client:getRulesState()).accepted_version == "2026-11-08"
    and not app.screen:findChild("AcceptCommunityRules", true).Visible, "Rules acceptance is explicit and persisted")
orca.async = savedAsync
print("PASS: Chat XML safety actions, reporting, block/unblock and rules acceptance")
