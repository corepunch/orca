local Fixtures = {}
Fixtures.otp_code = "123456" -- Offline test code; never used by the network client.

function Fixtures.copy(value)
    if type(value) ~= "table" then return value end
    local result = {}
    for key, item in pairs(value) do result[key] = Fixtures.copy(item) end
    return result
end

function Fixtures.new(now)
    now = now or os.time()
    local function timestamp(seconds)
        return os.date("!%Y-%m-%dT%H:%M:%SZ", now - seconds)
    end
    local function user(id, email, name)
        return { id = id, email = email, password = "password123",
            email_confirmed_at = timestamp(86400),
            phone = id == "me" and "+4915110000001" or nil,
            phone_confirmed_at = id == "me" and timestamp(86400) or nil,
            user_metadata = { display_name = name } }
    end
    local function message(id, room, sender, body, seconds)
        return { id = id, room_id = room, sender_id = sender, body = body,
            created_at = timestamp(seconds) }
    end
    return {
        otp_code = Fixtures.otp_code,
        profiles = {
            me = { id = "me", display_name = "You", username = "you", discoverable_phone = false, discoverable_email = false },
            alex = { id = "alex", display_name = "Alex Morgan", username = "alex", discoverable_phone = false, discoverable_email = true },
            maya = { id = "maya", display_name = "Maya Chen", username = "maya", discoverable_phone = false, discoverable_email = false },
            jules = { id = "jules", display_name = "Jules Park", username = "jules", discoverable_phone = false, discoverable_email = false },
        },
        users = {
            user("me", "demo@example.invalid", "You"),
            user("alex", "alex@example.invalid", "Alex Morgan"),
            user("maya", "maya@example.invalid", "Maya Chen"),
            user("jules", "jules@example.invalid", "Jules Park"),
        },
        rooms = {
            { id = "alex", name = "Alex Morgan", kind = "direct", visibility = "private", owner_id = "me", direct_pair = "alex:me", created_at = timestamp(3600),
                initials = "AM", color = "#d9a56f", subtitle = "See you at the little coffee place?", unread = 2 },
            { id = "weekend", name = "Weekend people", kind = "group", visibility = "private", owner_id = "me", created_at = timestamp(7200),
                initials = "WP", color = "#a7a3d3", subtitle = "Maya: That sounds like a plan", unread = 0 },
            { id = "jules", name = "Jules Park", kind = "direct", visibility = "private", owner_id = "me", direct_pair = "jules:me", created_at = timestamp(10800),
                initials = "JP", color = "#82aaa0", subtitle = "The photos turned out so good", unread = 0 },
        },
        members = {
            alex = { me = true, alex = true },
            weekend = { me = true, alex = true, maya = true },
            jules = { me = true, jules = true },
        },
        messages = {
            alex = {
                message("a1", "alex", "alex", "Hey! How's your morning going?", 240),
                message("a2", "alex", "me", "Pretty good. Just finished a long walk by the river.", 180),
                message("a3", "alex", "alex", "That sounds lovely. I'm ready for a coffee break.", 120),
                message("a4", "alex", "alex", "See you at the little coffee place?", 60),
            },
            weekend = { message("w1", "weekend", "maya", "That sounds like a plan", 3600) },
            jules = { message("j1", "jules", "jules", "The photos turned out so good", 7200) },
        },
    }
end

return Fixtures
