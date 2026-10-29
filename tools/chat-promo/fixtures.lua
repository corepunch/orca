-- Fictional, deterministic, offline-only App Store screenshot content.
local F = {}
F.now = 1793180460 -- 2026-10-28 09:41 UTC; independent of the host time zone.

function F.new()
    local data = require("api.mock_fixtures").new(F.now)
    local function stamp(minutes) return os.date("!%Y-%m-%dT%H:%M:%SZ", F.now - minutes * 60) end
    local people = {
        { "me", "Sam Taylor", "you", 24 }, { "alex", "Alex Morgan", "alex", 2 },
        { "maya", "Maya Chen", "maya", 5 }, { "jules", "Jules Park", "jules", 16 },
        { "nina", "Nina Patel", "nina", 13 }, { "leo", "Leo Martin", "leo", 26 },
        { "ella", "Ella Brooks", "ella", 17 }, { "ben", "Ben Wilson", "ben", 18 },
        { "sofia", "Sofia Rossi", "sofia", 3 }, { "omar", "Omar Hassan", "omar", 8 },
    }
    local names = { "Amelia Reed", "Noah Clarke", "Iris Bell", "Ethan Wood", "Lena Fischer", "Hugo Silva",
        "Zoe Williams", "Oscar Hughes", "Ava Mitchell", "Theo Laurent", "Mila Jensen", "Oliver Grant",
        "Chloe Bennett", "Felix Weber", "Aria Shah", "Lucas Rivera", "Nora Evans", "Henry Scott",
        "Layla Ahmed", "Jack Murphy", "Eva Novak", "Adam Turner", "Sara Kim", "Max Cooper",
        "Isla Campbell", "Finn Walsh", "Ruby James", "Louis Dubois", "Amara Davis", "Daniel Lee",
        "Clara Moreau", "Arthur Wilson", "Grace Liu", "Rafael Costa", "Mina Ali", "Charlie Ross",
        "Freya Olsen", "David Brown", "Leila Khan", "George Harris", "Naomi Green", "Isaac Young",
        "Elena Garcia", "Sebastian Wolf", "Alice Walker", "Jacob King", "Rose Anderson", "Tom Parker",
        "Aisha Lewis", "Matteo Bianchi", "Emma Stein", "William Hall", "Daisy Moore", "Nathan Brooks" }
    local used = {}
    for _, person in ipairs(people) do used[person[4]] = true end
    local i = 0
    for portrait = 1, 64 do
        if not used[portrait] then
            i = i + 1
            local id = string.format("contact-%02d", portrait)
            people[#people + 1] = { id, names[i], names[i]:lower():gsub("[^a-z]", ""), portrait }
        end
    end
    data.users, data.profiles, data.avatars = {}, {}, {}
    data.profilePhotos, data.photoFiles, data.contacts = {}, {}, { me = {} }
    for _, p in ipairs(people) do
        data.users[#data.users + 1] = {
            id = p[1], email = p[1] == "me" and "demo@example.invalid" or p[1] .. "@example.invalid",
            password = "password123", email_confirmed_at = stamp(1440), user_metadata = { display_name = p[2] },
        }
        data.profiles[p[1]] = { id = p[1], display_name = p[2], username = p[1] == "me" and "samtaylor" or p[3],
            discoverable_email = false, discoverable_phone = false }
        local file = assert(io.open(string.format("Chat/promo/assets/profile-%02d.jpg", p[4]), "rb"))
        local id = "promo-avatar-" .. p[1]
        data.photoFiles[id] = file:read("*a"); file:close()
        data.profilePhotos[p[1]] = id
        if p[1] ~= "me" then data.contacts.me[p[1]] = true end
    end
    data.rooms, data.messages, data.members, data.roles = {}, {}, {}, {}
    local function room(id, name, kind, subtitle, unread, minutes, members, owner)
        data.rooms[#data.rooms + 1] = { id = id, name = name, kind = kind, subtitle = subtitle,
            unread = unread, created_at = stamp(1440 + minutes), updated_at = stamp(minutes),
            visibility = kind == "channel" and "public" or "private", owner_id = owner or "me" }
        data.members[id], data.messages[id] = { me = true }, {}
        for _, member in ipairs(members) do data.members[id][member] = true end
    end
    room("alex", "Alex Morgan", "direct", "Window seat secured. See you at 10!", 2, 0, { "alex" })
    room("weekend", "weekend crew", "group", "Maya: I'll bring the good snacks.", 5, 3, { "alex", "maya", "jules", "nina", "leo" })
    room("jules", "Jules Park", "direct", "Still thinking about this view.", 1, 7, { "jules" })
    room("coffee", "coffee club", "channel", "Saturday's little ritual, sorted.", 3, 12, { "nina", "omar" }, "nina")
    room("nina", "Nina Patel", "direct", "That book recommendation was perfect", 0, 18, { "nina" })
    room("supper", "supper club", "group", "Leo: Table for six. Thursday, 7 pm.", 0, 26, { "leo", "ella", "ben", "sofia", "omar" })
    room("ella", "Ella Brooks", "direct", "You: Sending you the photos tonight", 0, 35, { "ella" })
    room("leo", "Leo Martin", "direct", "A quick walk after work?", 1, 52, { "leo" })
    room("studio", "studio notes", "group", "Sofia: Love where this is going.", 0, 78, { "sofia", "ben" })
    local function messages(id, rows)
        for i, row in ipairs(rows) do
            data.messages[id][i] = { id = id .. "-" .. i, room_id = id, sender_id = row[1], body = row[2], created_at = stamp(row[3]) }
        end
    end
    messages("alex", {
        { "alex", "Morning! Coffee before the city wakes up?", 17 },
        { "me", "Absolutely. The little place on the corner?", 15 },
        { "alex", "Yes! They finally have the cinnamon buns again.", 14 },
        { "me", "Then I'm definitely coming.", 12 },
        { "me", "Ten minutes. Save me a seat?", 10 },
        { "alex", "Already on it. Oat flat white as usual?", 5 },
        { "me", "You know me too well.", 3 },
        { "alex", "Window seat secured. See you at 10!", 0 },
    })
    messages("weekend", {
        { "alex", "Forecast says sunshine all weekend.", 40 },
        { "maya", "Lake day? I've been waiting all week for this.", 36 },
        { "me", "Yes! Let's take the early train on Saturday.", 30 },
        { "jules", "I'll bring the camera. And a spare battery this time.", 25 },
        { "nina", "Found a walking trail that ends right by the water.", 20 },
        { "me", "Perfect. Meet at the station at 8:30?", 12 },
        { "alex", "Tickets booked. This is actually happening!", 8 },
        { "maya", "I'll bring the good snacks.", 3 },
    })
    messages("jules", {
        { "jules", "Finally went through the weekend photos.", 30 },
        { "me", "Please tell me you got the lake!", 28 },
        { "jules", "Still thinking about this view.", 7 },
        { "me", "Worth every step. Let's go back soon.", 4 },
        { "jules", "Same crew, same spot. Deal?", 2 },
    })
    messages("coffee", {
        { "nina", "THIS SATURDAY / 10 AM\nA slow morning at our favourite corner café.", 65 },
        { "nina", "New beans, warm pastries, and a table by the window. Bring a friend.", 45 },
        { "nina", "The plan: coffee first, a wander through the flower market after.", 25 },
        { "nina", "Saturday's little ritual, sorted.\nSee you at ten.", 12 },
    })
    for _, r in ipairs(data.rooms) do
        if #data.messages[r.id] == 0 then
            local sender = r.kind == "direct" and r.id or "leo"
            messages(r.id, { { sender, r.subtitle, 60 } })
        end
    end
    return data
end

return F
