-- orca.notifications for the alerts (N2): the module where this build can
-- deliver local notifications, else nil. Tests pass a stand-in (`false`
-- for none).
local Notify = {}

function Notify.module(notifier)
    if notifier ~= nil then return notifier or nil end
    local ok, module = pcall(require, "orca.notifications")
    if not (ok and type(module) == "table" and module.scheduleLocal and module.supported) then return nil end
    local supported, yes = pcall(module.supported)
    return supported and yes and module or nil
end

-- Asks once, when the user turns notifications on; the system remembers.
function Notify.authorize(notifier)
    if notifier and notifier.requestLocalAuthorization then pcall(notifier.requestLocalAuthorization) end
end

return Notify
