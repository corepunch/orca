local notifications = require "orca.notifications"
assert(type(notifications.supported()) == "boolean")
assert(notifications.poll() == nil, "An empty native queue must return nil")
if not notifications.supported() then
    local ok, message = notifications.requestAuthorization()
    assert(ok == nil and type(message) == "string" and #message > 0)
    assert(notifications.poll() == nil, "Unsupported requests must not enqueue a fake token")
end
print("Notification module contract passed")
