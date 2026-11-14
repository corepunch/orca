local Fixtures = require "api.mock_fixtures"
local Jpeg = require "api.jpeg"

return function(state, env)
    local copy, response, fail = Fixtures.copy, env.response, env.failure
    local now, find, issue, auth = env.clock, env.find, env.issue, env.authenticate
    local unique, timestamp = env.uniqueId, env.timestamp
    local P = {}
    state.blocks, state.reports, state.rulesAccepted = {}, {}, {}
    function P.blocked(a, b)
        return state.blocks[a] and state.blocks[a][b] or state.blocks[b] and state.blocks[b][a] or false
    end
    state.profiles, state.roles = state.profiles or {}, state.roles or {}
    state.challenges, state.lookups, state.invites = {}, {}, {}
    state.attachments, state.objects = {}, {}
    state.sequence.attachment, state.sequence.invite = 0, 0
    for _, user in ipairs(state.users) do
        state.profiles[user.id] = state.profiles[user.id] or { id = user.id,
            display_name = (user.user_metadata or {}).display_name or "Member",
            discoverable_phone = false, discoverable_email = false }
    end
    for _, room in ipairs(state.rooms) do
        room.kind, room.visibility = room.kind or "group", room.visibility or "private"
        local members = state.members[room.id] or {}
        state.roles[room.id] = state.roles[room.id] or {}
        local owner = room.owner_id
        if not owner and members.me then owner = "me" end
        if not owner then for id in pairs(members) do owner = id; break end end
        room.owner_id = owner
        for id in pairs(members) do
            state.roles[room.id][id] = state.roles[room.id][id] or (id == owner and "owner" or (room.kind == "channel" and "subscriber" or "member"))
        end
    end
    local function member(roomId, id) return state.members[roomId] and state.members[roomId][id] == true end
    local function role(roomId, id) return member(roomId, id) and (state.roles[roomId] or {})[id] end
    function P.canPost(roomId, id)
        local r, value = find(state.rooms, roomId), role(roomId, id)
        if r and r.kind == "direct" then
            for peer in pairs(state.members[roomId] or {}) do if peer ~= id and P.blocked(id, peer) then return false end end
        end
        return r and (value == "owner" or value == "admin" or (r.kind ~= "channel" and value == "member")) or false
    end
    function P.canSee(roomId, actor, target)
        local r = find(state.rooms, roomId)
        if not r or not member(roomId, actor) or not member(roomId, target) then return false end
        local a, b = role(roomId, actor), role(roomId, target)
        return r.kind ~= "channel" or actor == target or a == "owner" or a == "admin" or b == "owner" or b == "admin"
    end
    function P.contactVisible(actor, target)
        if actor == target then return false end
        if P.blocked(actor, target) then return false end
        if state.contacts and state.contacts[actor] and state.contacts[actor][target] then return true end
        for roomId in pairs(state.members) do if P.canSee(roomId, actor, target) then return true end end
        return false
    end
    function P.profile(id) return copy(state.profiles[id]) end
    function P.ensureUser(user)
        state.profiles[user.id] = state.profiles[user.id] or { id = user.id,
            display_name = (user.user_metadata or {}).display_name or "Member", discoverable_phone = false, discoverable_email = false }
    end
    function P.roomRow(room, id)
        local row = copy(room)
        row.role, row.can_post = role(room.id, id), P.canPost(room.id, id)
        -- Direct chats name the other person, as the Appwrite function does.
        if room.kind == "direct" then
            for userId in pairs(state.members[room.id] or {}) do if userId ~= id then row.peer_id = userId end end
        end
        return row
    end
    function P.messageRow(message)
        local row = copy(message); row.attachments = {}
        for _, attachment in pairs(state.attachments) do
            if attachment.message_id == message.id and attachment.status == "attached" then row.attachments[#row.attachments + 1] = copy(attachment) end
        end
        table.sort(row.attachments, function(a, b)
            if a.position and b.position then return a.position < b.position end
            return a.id < b.id
        end)
        local room = find(state.rooms, message.room_id)
        if room and room.kind == "channel" then row.sender_name = room.name end
        return row
    end
    local function ownProfile(user)
        local row = P.profile(user.id)
        row.notification_previews = row.notification_previews ~= false
        if user.email_confirmed_at then row.email = user.email end
        -- The typed contact number wins; an older verified login phone seeds it.
        local phone = user.contact_phone
        if phone == nil and user.phone_confirmed_at then phone = user.phone end
        row.phone = phone ~= "" and phone or nil
        return row
    end
    local function identity(body)
        if type(body.email) == "string" and body.email:match("^[^%s@]+@[^%s@]+%.[^%s@]+$") then return "email", body.email:lower() end
        if type(body.phone) == "string" and body.phone:match("^%+[1-9]%d+$") and #body.phone >= 9 and #body.phone <= 16 then return "phone", body.phone end
    end
    local function identityOwner(field, value)
        for _, user in ipairs(state.users) do if user[field] == value then return user end end
    end
    local function challenge(field, value, owner)
        local key = (owner and owner.id or "login") .. ":" .. field .. ":" .. value
        local old = state.challenges[key]
        if old and old.sent_at + 60 > now() then return fail(429, "over_request_rate_limit", "Wait 60 seconds before requesting another code.") end
        if owner then
            local other = identityOwner(field, value)
            if other and other.id ~= owner.id then return fail(422, "identity_exists", "This identifier already belongs to another account.") end
        end
        state.challenges[key] = { field = field, value = value, owner_id = owner and owner.id,
            sent_at = now(), expires_at = now() + 300, attempts = 0 }
        return response(200, {})
    end
    local function verify(body, headers)
        local field, value = identity(body)
        if not field then return fail(400, "validation", "Invalid identifier.") end
        local linking = body.type == "email_change" or body.type == "phone_change"
        if body.type ~= (field == "phone" and (linking and "phone_change" or "sms") or (linking and "email_change" or "email")) then
            return fail(400, "validation", "Invalid verification type.")
        end
        local actor, err
        if linking then actor, err = auth(headers); if not actor then return err end end
        local key = (actor and actor.id or "login") .. ":" .. field .. ":" .. value
        local code = state.challenges[key]
        if not code or code.expires_at <= now() or code.attempts >= 5 then return fail(403, "otp_expired", "The code is invalid or expired. Request a new code.") end
        code.attempts = code.attempts + 1
        if body.token ~= (state.otp_code or Fixtures.otp_code) then return fail(403, "otp_invalid", "The verification code is incorrect.") end
        local user = identityOwner(field, value)
        if actor and user and user.id ~= actor.id then return fail(422, "identity_exists", "This identifier already belongs to another account.") end
        if actor then user = actor end
        if not user then
            user = { id = unique("user", state.users), user_metadata = { display_name = "Member" } }
            state.users[#state.users + 1] = user
            state.profiles[user.id] = { id = user.id, display_name = "Member", discoverable_phone = false, discoverable_email = false }
        end
        user[field], user[field .. "_confirmed_at"] = value, timestamp()
        state.challenges[key] = nil
        if actor then return response(200, { user = { id = user.id, email = user.email, phone = user.phone, user_metadata = copy(user.user_metadata) } }) end
        return issue(user)
    end
    function P.postMessage(user, roomId, text, ids, status)
        if not P.canPost(roomId, user.id) then return fail(403, "42501", "You cannot publish in this conversation.") end
        ids, text = ids or {}, text or ""
        if type(text) ~= "string" or #text > 4000 or type(ids) ~= "table" or #ids > 9 or (not text:match("%S") and #ids == 0) then
            return fail(400, "validation", "Write a message or attach up to nine photos.")
        end
        local seen = {}
        for _, id in ipairs(ids) do
            local a = state.attachments[id]
            if seen[id] or not a or a.room_id ~= roomId or a.uploader_id ~= user.id or a.status ~= "ready" or a.created_epoch + 3600 <= now() then
                return fail(400, "attachment_unavailable", "Photo is unavailable or unfinished.")
            end
            seen[id] = true
        end
        local all = {}; for _, list in pairs(state.messages) do for _, message in ipairs(list) do all[#all + 1] = message end end
        local message = { id = unique("message", all), room_id = roomId, sender_id = user.id, body = text, created_at = timestamp() }
        for position, id in ipairs(ids) do local a = state.attachments[id]; a.status, a.message_id, a.position = "attached", message.id, position end
        state.messages[roomId][#state.messages[roomId] + 1] = message
        find(state.rooms, roomId).subtitle = text ~= "" and text or "Photo"
        return response(status or 200, { P.messageRow(message) })
    end
    -- Same policy as the Appwrite function: authors edit their own text; authors,
    -- or staff outside direct chats, delete for everyone along with the photos.
    function P.changeMessage(name, user, body)
        local message, list, index
        for _, messages in pairs(state.messages) do
            for i, item in ipairs(messages) do if item.id == body.target_message then message, list, index = item, messages, i end end
        end
        if not message then return fail(404, "not_found", "The message was not found.") end
        local room, value = find(state.rooms, message.room_id), role(message.room_id, user.id)
        if not value then return fail(403, "42501", "Conversation unavailable.") end
        local latest = index == #list
        if name == "edit_chat_message" then
            local text = body.message_body
            if message.sender_id ~= user.id or not P.canPost(room.id, user.id) then return fail(403, "42501", "You can edit only your own messages.") end
            if type(text) ~= "string" or #text > 4000 then return fail(400, "validation", "Write a message of at most 4000 text bytes.") end
            if not text:match("%S") and #P.messageRow(message).attachments == 0 then return fail(400, "validation", "A message without photos needs text.") end
            if text ~= message.body then message.body, message.edited_at = text, timestamp() end
            if latest then room.subtitle = text ~= "" and text or "Photo" end
            return response(200, { P.messageRow(message) })
        end
        if message.sender_id ~= user.id and not (room.kind ~= "direct" and (value == "owner" or value == "admin")) then
            return fail(403, "42501", "You cannot delete this message.")
        end
        table.remove(list, index)
        for id, a in pairs(state.attachments) do
            if a.message_id == message.id then state.objects[a.object_key], state.objects[a.thumbnail_key], state.attachments[id] = nil, nil, nil end
        end
        if latest then
            local last = list[#list]
            room.subtitle = last and (last.body ~= "" and last.body or "Photo") or "Start the conversation"
        end
        return response(200, true)
    end
    local function addMember(room, user)
        state.members[room.id][user.id] = true
        state.roles[room.id][user.id] = state.roles[room.id][user.id] or (room.kind == "channel" and "subscriber" or "member")
    end
    local function createRoom(user, body)
        local kind, visibility = body.room_kind or "group", body.room_visibility or "private"
        local ids, name = body.member_ids, body.room_name
        if (kind ~= "direct" and kind ~= "group" and kind ~= "channel") or (visibility ~= "private" and visibility ~= "public")
            or type(ids) ~= "table" or #ids > 50 or type(name) ~= "string" or not name:match("%S") or #name > 80 then
            return fail(400, "validation", "Invalid conversation name, type or members.")
        end
        if kind == "direct" and (#ids ~= 1 or ids[1] == user.id or visibility ~= "private") then return fail(400, "validation", "Choose exactly one other person for a private direct chat.") end
        for _, id in ipairs(ids) do if P.blocked(user.id, id) then return fail(403, "user_blocked", "This person cannot be contacted.") end end
        local pair
        if kind == "direct" then
            pair = user.id < ids[1] and (user.id .. ":" .. ids[1]) or (ids[1] .. ":" .. user.id)
            for _, room in ipairs(state.rooms) do
                if room.direct_pair == pair then
                    if not member(room.id, user.id) then return fail(403, "42501", "Conversation unavailable.") end
                    return response(200, { P.roomRow(room, user.id) })
                end
            end
        end
        for _, id in ipairs(ids) do if not find(state.users, id) then return fail(400, "23503", "A member account does not exist.") end end
        local room = { id = unique("room", state.rooms), name = name:match("^%s*(.-)%s*$"), created_at = timestamp(),
            kind = kind, visibility = visibility, owner_id = user.id, direct_pair = pair, unread = 0, subtitle = "Start the conversation" }
        state.rooms[#state.rooms + 1] = room
        state.members[room.id], state.roles[room.id], state.messages[room.id] = { [user.id] = true }, { [user.id] = "owner" }, {}
        for _, id in ipairs(ids) do addMember(room, find(state.users, id)) end
        return response(200, { P.roomRow(room, user.id) })
    end
    local function rpc(name, body, user)
        if name == "list_chat_blocks" then
            local rows = {}
            for id in pairs(state.blocks[user.id] or {}) do
                local profile = state.profiles[id] or {}
                rows[#rows + 1] = { user_id = id, display_name = profile.display_name, username = profile.username }
            end
            return response(200, rows)
        elseif name == "set_chat_user_block" then
            if body.target_user == user.id or not find(state.users, body.target_user) or type(body.blocked) ~= "boolean" then
                return fail(400, "validation", "Choose another person to block or unblock.")
            end
            state.blocks[user.id] = state.blocks[user.id] or {}
            state.blocks[user.id][body.target_user] = body.blocked or nil
            return response(200, true)
        elseif name == "report_chat_content" then
            local reasons = { spam=1,harassment=1,sexual_content=1,violence=1,hate=1,other=1 }
            if not find(state.users, body.target_user) or body.target_user == user.id or not reasons[body.reason] then
                return fail(400, "validation", "Choose a report reason and keep details under 1000 characters.")
            end
            local authorized, reportedMessage
            for roomId, members in pairs(state.members) do if members[user.id] then
                if members[body.target_user] then authorized = true end
                for _, message in ipairs(state.messages[roomId] or {}) do
                    if message.id == body.target_message and message.sender_id == body.target_user then reportedMessage = message end
                end
            end end
            if not authorized or body.target_message and not reportedMessage then return fail(403, "forbidden", "You cannot report this content.") end
            local id = "mock-report-" .. (#state.reports + 1)
            state.reports[#state.reports + 1] = { id = id, reporter_id = user.id, target_user = body.target_user,
                reason = body.reason, details = body.details, target_message = body.target_message, message = copy(reportedMessage), status = "open" }
            return response(200, { id = id, submitted = true })
        elseif name == "get_chat_rules_state" then
            return response(200, { version = "2026-11-08", accepted_version = state.rulesAccepted[user.id] })
        elseif name == "accept_chat_rules" then
            if body.version ~= "2026-11-08" then return fail(400, "validation", "Read the current community rules first.") end
            state.rulesAccepted[user.id] = body.version
            return response(200, { accepted = true, version = body.version })
        elseif name == "delete_chat_account" then
            if body.confirmation ~= "DELETE" then return fail(400, "validation", "Confirm account deletion first.") end
            for _, room in ipairs(state.rooms) do
                local members, roles = state.members[room.id] or {}, state.roles[room.id] or {}
                members[user.id], roles[user.id] = nil, nil
                if room.owner_id == user.id then
                    local successors = {}
                    for id in pairs(members) do successors[#successors + 1] = id end
                    table.sort(successors, function(a, b)
                        if (roles[a] == "admin") ~= (roles[b] == "admin") then return roles[a] == "admin" end
                        return a < b
                    end)
                    room.owner_id = successors[1]
                    if room.owner_id then roles[room.owner_id] = "owner" end
                end
                local messages = state.messages[room.id] or {}
                for i = #messages, 1, -1 do if messages[i].sender_id == user.id then table.remove(messages, i) end end
            end
            for i = #state.rooms, 1, -1 do
                local room = state.rooms[i]
                if not next(state.members[room.id] or {}) then
                    state.messages[room.id], state.members[room.id], state.roles[room.id] = nil, nil, nil
                    table.remove(state.rooms, i)
                end
            end
            for id, photo in pairs(state.attachments) do
                if photo.uploader_id == user.id then
                    state.objects[photo.object_key], state.objects[photo.thumbnail_key] = nil, nil
                    state.attachments[id] = nil
                end
            end
            for id, invite in pairs(state.invites) do if invite.created_by == user.id then state.invites[id] = nil end end
            for key, challenge in pairs(state.challenges) do
                if challenge.owner_id == user.id or challenge.value == user.email or challenge.value == user.phone then state.challenges[key] = nil end
            end
            for id, session in pairs(state.sessions) do if session.user_id == user.id then
                state.accessTokens[session.access_token], state.refreshTokens[session.refresh_token], state.sessions[id] = nil, nil, nil
            end end
            for i = #state.users, 1, -1 do if state.users[i].id == user.id then table.remove(state.users, i) end end
            local avatar = state.profilePhotos[user.id]
            if avatar then state.photoFiles[avatar] = nil end
            state.profilePhotos[user.id], state.profiles[user.id] = nil, nil
            state.blocks[user.id], state.rulesAccepted[user.id] = nil, nil
            for _, blocked in pairs(state.blocks) do blocked[user.id] = nil end
            for _, contacts in pairs(state.contacts or {}) do contacts[user.id] = nil end
            return response(200, { accepted = true, deleted = true })
        elseif name == "list_chat_messages" then
            if not member(body.target_room, user.id) then return fail(403, "forbidden", "Join this conversation first.") end
            local limit = tonumber(body.limit) or 30
            limit = math.max(1, math.min(100, math.floor(limit)))
            local rows = copy(state.messages[body.target_room] or {})
            table.sort(rows, function(a, b)
                if a.created_at ~= b.created_at then return a.created_at > b.created_at end
                return a.id > b.id
            end)
            local start = 1
            if body.before then
                local found
                for i, row in ipairs(rows) do if row.id == body.before then start, found = i + 1, true; break end end
                if not found then return fail(409, "stale_cursor", "History changed. Reload the latest messages.") end
            end
            local last = math.min(#rows, start + limit - 1)
            local messages = {}
            for i = last, start, -1 do messages[#messages + 1] = P.messageRow(rows[i]) end
            local more = last < #rows
            return response(200, { messages = messages, has_more = more, next_cursor = more and messages[1].id or nil })
        elseif name == "get_chat_profile" then return response(200, ownProfile(user))
        elseif name == "update_chat_profile" then
            local values, profile = body.values, state.profiles[user.id]
            if type(values) ~= "table" then return fail(400, "validation", "Provide profile fields.") end
            local changed = copy(profile)
            for key, value in pairs(values) do
                if key ~= "display_name" and key ~= "username" and key ~= "discoverable_phone" and key ~= "discoverable_email"
                    and key ~= "notification_previews" and key ~= "phone" then
                    return fail(400, "validation", "That profile field cannot be edited.")
                end
                if key == "display_name" then
                    if type(value) ~= "string" or not value:match("%S") or #value > 80 then return fail(400, "validation", "Display name must be 1–80 characters.") end
                    changed[key] = value:match("^%s*(.-)%s*$")
                elseif key == "username" then
                    if type(value) ~= "string" then return fail(400, "validation", "Username must be text.") end
                    value = value:lower():match("^%s*(.-)%s*$")
                    if value ~= "" and (not value:match("^[a-z][a-z0-9_]+$") or #value < 3 or #value > 32) then return fail(400, "validation", "Username must be 3–32 lowercase letters, numbers or underscores.") end
                    for id, person in pairs(state.profiles) do if id ~= user.id and value ~= "" and person.username == value then return fail(409, "23505", "Username is unavailable.") end end
                    changed[key] = value ~= "" and value or nil
                elseif key == "phone" then
                    if type(value) ~= "string" or value ~= "" and (not value:match("^%+[1-9]%d+$") or #value < 9 or #value > 16) then
                        return fail(400, "validation", "Enter the number with + and your country code.")
                    end
                    user.contact_phone = value
                    if value ~= user.phone then changed.discoverable_phone = false end
                elseif key == "notification_previews" then
                    if type(value) ~= "boolean" then return fail(400, "validation", "Message previews must be on or off.") end
                    changed[key] = value
                else
                    local field = key == "discoverable_phone" and "phone" or "email"
                    local verified = user[field .. "_confirmed_at"] and (field == "email" or ownProfile(user).phone == user.phone)
                    if type(value) ~= "boolean" or (value and not verified) then return fail(400, "validation", "Verify this identifier before enabling discovery.") end
                    changed[key] = value
                end
            end
            state.profiles[user.id] = changed
            user.user_metadata = { display_name = changed.display_name }
            return response(200, ownProfile(user))
        elseif name == "find_chat_people" then
            local limit = state.lookups[user.id]
            if not limit or limit.start + 60 <= now() then limit = { start = now(), hits = 0 }; state.lookups[user.id] = limit end
            limit.hits = limit.hits + 1
            if limit.hits > 20 then return fail(429, "rate_limit", "Too many lookups. Wait a minute.") end
            local query = type(body.query) == "string" and body.query:lower():match("^%s*(.-)%s*$") or ""
            local rows = {}
            if #query < 3 or #query > 254 then return response(200, rows) end
            for _, account in ipairs(state.users) do
                local p = state.profiles[account.id]
                if account.id ~= user.id and not P.blocked(user.id, account.id) and (p.username == query:gsub("^@", "") or
                    (p.discoverable_email and account.email_confirmed_at and account.email == query) or
                    (p.discoverable_phone and account.phone_confirmed_at and account.phone == query)) then
                    rows[#rows + 1] = { id = p.id, display_name = p.display_name, username = p.username }
                end
            end
            return response(200, rows)
        elseif name == "create_chat_room" then return createRoom(user, body)
        elseif name == "list_chat_members" then
            if not member(body.target_room, user.id) then return fail(403, "42501", "Conversation unavailable.") end
            local rows = {}
            for id in pairs(state.members[body.target_room]) do
                if P.canSee(body.target_room, user.id, id) then
                    local p = state.profiles[id]; rows[#rows + 1] = { user_id = id, display_name = p.display_name, username = p.username, role = role(body.target_room, id) }
                end
            end
            table.sort(rows, function(a, b) return a.role == b.role and a.display_name < b.display_name or a.role < b.role end)
            return response(200, rows)
        elseif name == "leave_chat_room" then
            local r, value = find(state.rooms, body.target_room), role(body.target_room, user.id)
            if not r or not value or r.kind == "direct" or value == "owner" then
                return fail(403, "42501", "Only non-owner group and channel members can leave.")
            end
            state.members[r.id][user.id], state.roles[r.id][user.id] = nil, nil
            return response(200, true)
        elseif name == "update_chat_room" then
            for field in pairs(body) do
                if field ~= "target_room" and field ~= "room_name" then return fail(400, "validation", "Only the conversation name can be changed.") end
            end
            local name = body.room_name
            local length = type(name) == "string" and select(2, name:gsub("[^\128-\191]", "")) or 0
            if type(name) ~= "string" or not name:match("%S") or length > 80 then return fail(400, "validation", "Use a conversation name of 1–80 characters.") end
            local r, value = find(state.rooms, body.target_room), role(body.target_room, user.id)
            if not r or r.kind == "direct" or value ~= "owner" then return fail(403, "42501", "Only the owner can rename a group or channel.") end
            r.name = name:match("^%s*(.-)%s*$")
            return response(200, P.roomRow(r, user.id))
        elseif name == "set_chat_member_role" or name == "remove_chat_member" then
            local r = find(state.rooms, body.target_room)
            local actor, target = role(body.target_room, user.id), role(body.target_room, body.target_user)
            if not r or r.kind == "direct" or not target or target == "owner" then return fail(403, "42501", "You cannot change this membership.") end
            if name == "set_chat_member_role" then
                local value = body.member_role
                if actor ~= "owner" then return fail(403, "42501", "Only the owner can change roles.") end
                if value ~= "admin" and value ~= (r.kind == "channel" and "subscriber" or "member") then return fail(400, "validation", "Invalid membership role.") end
                state.roles[r.id][body.target_user] = value
            else
                if actor ~= "owner" and (actor ~= "admin" or target == "admin") then return fail(403, "42501", "You cannot remove this member.") end
                state.members[r.id][body.target_user], state.roles[r.id][body.target_user] = nil, nil
            end
            return response(200, true)
        elseif name == "create_chat_invite" or name == "revoke_chat_invite" then
            local r, value = find(state.rooms, body.target_room), role(body.target_room, user.id)
            if not r or r.kind == "direct" or (value ~= "owner" and value ~= "admin") then return fail(403, "42501", "Only admins can manage invitations.") end
            if name == "revoke_chat_invite" then
                local invite = state.invites[body.invite_code]
                if not invite or invite.room_id ~= r.id then return response(200, false) end
                invite.revoked = true; return response(200, true)
            end
            state.sequence.invite = state.sequence.invite + 1
            local invite = { code = "mock-invite-" .. state.sequence.invite, room_id = r.id, expires_at = os.date("!%Y-%m-%dT%H:%M:%SZ", now() + 86400),
                expires_epoch = now() + 86400, max_uses = 10, uses = 0, revoked = false, created_by = user.id }
            state.invites[invite.code] = invite
            return response(200, invite)
        elseif name == "join_chat_invite" then
            local invite = state.invites[body.invite_code]
            if not invite or invite.revoked or invite.expires_epoch <= now() or invite.uses >= invite.max_uses then return fail(400, "invite_unavailable", "Invitation is unavailable or expired.") end
            local room = find(state.rooms, invite.room_id)
            if invite.created_by and P.blocked(user.id, invite.created_by) then return fail(403, "user_blocked", "This invitation is unavailable.") end
            if not member(room.id, user.id) then addMember(room, user); invite.uses = invite.uses + 1 end
            return response(200, P.roomRow(room, user.id))
        elseif name == "find_chat_channels" then
            local query = type(body.query) == "string" and body.query:lower():match("^%s*(.-)%s*$") or ""
            local rows = {}
            if #query >= 2 and #query <= 80 then
                for _, room in ipairs(state.rooms) do if room.kind == "channel" and room.visibility == "public" and room.name:lower():find(query, 1, true) then rows[#rows + 1] = P.roomRow(room, user.id) end end
            end
            table.sort(rows, function(a, b) return a.name < b.name end)
            return response(200, rows)
        elseif name == "join_public_chat_channel" then
            local room = find(state.rooms, body.target_room)
            if not room or room.kind ~= "channel" or room.visibility ~= "public" then return fail(403, "42501", "Channel unavailable.") end
            if not member(room.id, user.id) then addMember(room, user) end
            return response(200, P.roomRow(room, user.id))
        elseif name == "post_chat_message" then return P.postMessage(user, body.target_room, body.message_body, body.attachment_ids)
        elseif name == "edit_chat_message" or name == "delete_chat_message" then return P.changeMessage(name, user, body)
        end
    end

    local function photoAccess(a, user, download)
        return a and member(a.room_id, user.id) and (a.status == "attached" or (a.uploader_id == user.id and (a.status == "ready" or (not download and a.status == "pending"))))
    end
    local function photoRpc(name, body, user)
        if name == "reserve_chat_photo" then
            if not P.canPost(body.target_room, user.id) then return fail(403, "42501", "You cannot upload here.") end
            for _, key in ipairs { "photo_width", "photo_height", "photo_bytes", "thumbnail_width", "thumbnail_height", "thumbnail_bytes" } do
                local value = body[key]
                local maximum = key:find("bytes") and 1000000 or (key:find("thumbnail") and 256 or 1600)
                if type(value) ~= "number" or value % 1 ~= 0 or value < 1 or value > maximum then return fail(400, "validation", "Photo exceeds its size or dimension limit.") end
            end
            local count = 0
            for _, a in pairs(state.attachments) do if a.uploader_id == user.id and (a.status == "pending" or a.status == "ready") and a.created_epoch + 3600 > now() then count = count + 1 end end
            if count >= 20 then return fail(429, "rate_limit", "Too many unfinished uploads.") end
            state.sequence.attachment = state.sequence.attachment + 1
            local id = "mock-photo-" .. state.sequence.attachment
            local a = { id = id, room_id = body.target_room, uploader_id = user.id, kind = "photo", mime_type = "image/jpeg", status = "pending",
                width = body.photo_width, height = body.photo_height, byte_count = body.photo_bytes, thumbnail_width = body.thumbnail_width,
                thumbnail_height = body.thumbnail_height, thumbnail_byte_count = body.thumbnail_bytes, created_at = timestamp(), created_epoch = now(),
                object_key = body.target_room .. "/" .. id .. "/photo.jpg", thumbnail_key = body.target_room .. "/" .. id .. "/thumbnail.jpg" }
            state.attachments[id] = a; return response(200, a)
        elseif name == "discard_chat_photo" then
            local a = state.attachments[body.attachment_id]
            if not a or a.uploader_id ~= user.id or a.status == "attached" then return fail(403, "42501", "Photo cannot be discarded.") end
            a.status = "discarded"; return response(200, a)
        end
    end
    -- Magic and provider links: the mock keeps the link it "sent" so tests can open it.
    local function signInLink(kind, email)
        local owner = identityOwner("email", email)
        state.sequence.link = (state.sequence.link or 0) + 1
        local secret = ("mock-link-secret-%d"):format(state.sequence.link)
        local userId = owner and owner.id or unique("user", state.users)
        state.linkTokens = state.linkTokens or {}
        state.linkTokens[secret] = { userId = userId, email = email, expires_at = now() + 3600 }
        state.lastSignInLink = ("appwrite-callback-mock://%s?userId=%s&secret=%s"):format(kind, userId, secret)
    end
    local function linkSession(body)
        local token = type(body.secret) == "string" and state.linkTokens and state.linkTokens[body.secret]
        if not token or token.userId ~= body.userId or token.expires_at <= now() then
            return fail(401, "user_invalid_token", "This sign-in link is invalid or expired. Request a new one.")
        end
        state.linkTokens[body.secret] = nil
        local user = identityOwner("email", token.email)
        if not user then
            user = { id = token.userId, email = token.email, email_confirmed_at = timestamp(),
                user_metadata = { display_name = "Member" } }
            state.users[#state.users + 1] = user
            state.profiles[user.id] = { id = user.id, display_name = "Member", discoverable_phone = false, discoverable_email = false }
        end
        return issue(user)
    end
    function P.handle(method, path, params, body, headers)
        if path == "/auth/v1/magiclink" and method == "POST" then
            local field, value
            if type(body) == "table" then field, value = identity(body) end
            if field ~= "email" then return fail(400, "validation", "Enter a valid email address.") end
            signInLink("magic", value)
            return response(200, {})
        elseif path == "/auth/v1/authorize" and method == "POST" then
            local provider = type(body) == "table" and body.provider
            if provider ~= "apple" and provider ~= "google" then return fail(400, "validation", "Choose Apple or Google.") end
            signInLink("oauth", provider .. "@example.invalid")
            return response(200, { url = "https://mock.invalid/oauth/" .. provider, scheme = "appwrite-callback-mock" })
        elseif path == "/auth/v1/link-session" and method == "POST" then
            return linkSession(type(body) == "table" and body or {})
        elseif path == "/auth/v1/otp" and method == "POST" then
            local field, value
            if type(body) == "table" then field, value = identity(body) end
            if not field then return fail(400, "validation", "Invalid identifier.") end
            return challenge(field, value)
        elseif path == "/auth/v1/verify" and method == "POST" then
            if type(body) ~= "table" then return fail(400, "validation", "Provide a verification code.") end
            return verify(body, headers)
        elseif path == "/auth/v1/user" then
            local user, err = auth(headers); if not user then return err end
            if method == "PUT" then
                local field, value
                if type(body) == "table" then field, value = identity(body) end
                if not field then return fail(400, "validation", "Invalid identifier.") end
                return challenge(field, value, user)
            end
            if method == "GET" then return response(200, { id = user.id, phone = user.phone, email = user.email, user_metadata = copy(user.user_metadata) }) end
        end
        local name = path:match("^/rest/v1/rpc/(.+)$")
        local known = name and ({ list_chat_blocks=1,set_chat_user_block=1,report_chat_content=1,get_chat_rules_state=1,accept_chat_rules=1,
            delete_chat_account=1,list_chat_messages=1,get_chat_profile=1,update_chat_profile=1,find_chat_people=1,create_chat_room=1,list_chat_members=1,
            set_chat_member_role=1,remove_chat_member=1,leave_chat_room=1,update_chat_room=1,create_chat_invite=1,join_chat_invite=1,revoke_chat_invite=1,
            find_chat_channels=1,join_public_chat_channel=1,post_chat_message=1,edit_chat_message=1,delete_chat_message=1,
            reserve_chat_photo=1,discard_chat_photo=1 })[name]
        local storageKey = path:match("^/storage/v1/object/chat%-photos/(.+)$")
        local downloadKey = path:match("^/storage/v1/object/authenticated/chat%-photos/(.+)$")
        if not known and not storageKey and not downloadKey and path ~= "/storage/v1/object/chat-photos"
            and path ~= "/functions/v1/finalize-chat-photo" and path ~= "/rest/v1/chat_attachments" then return nil end
        local user, err = auth(headers); if not user then return err end
        if known then
            if method ~= "POST" or type(body) ~= "table" then return fail(400, "bad_request", "Use a JSON POST request.") end
            return rpc(name, body, user) or photoRpc(name, body, user)
        elseif path == "/rest/v1/chat_attachments" and method == "GET" then
            local id = params.id and params.id:match("^eq%.(.+)$")
            local rows = {}
            for _, a in pairs(state.attachments) do if (not id or a.id == id) and photoAccess(a, user, false) then rows[#rows + 1] = copy(a) end end
            return response(200, rows)
        elseif storageKey and method == "POST" then
            local a
            for _, item in pairs(state.attachments) do if item.object_key == storageKey or item.thumbnail_key == storageKey then a = item; break end end
            if not a or a.uploader_id ~= user.id or a.status ~= "pending" or a.created_epoch + 3600 <= now() or not P.canPost(a.room_id, user.id) then return fail(403, "42501", "Upload is unavailable.") end
            if state.objects[storageKey] then return fail(409, "object_exists", "This upload is immutable.") end
            local w, h = Jpeg.dimensions(body)
            local thumb = storageKey == a.thumbnail_key
            if not w or headers["Content-Type"] ~= "image/jpeg" or #body > 1000000
                or #body ~= (thumb and a.thumbnail_byte_count or a.byte_count)
                or w ~= (thumb and a.thumbnail_width or a.width) or h ~= (thumb and a.thumbnail_height or a.height) then
                return fail(400, "invalid_photo", "JPEG data does not match the reserved photo.")
            end
            state.objects[storageKey] = body; return response(200, { Key = storageKey })
        elseif path == "/functions/v1/finalize-chat-photo" and method == "POST" then
            local a = type(body) == "table" and state.attachments[body.attachment_id]
            if not a or a.uploader_id ~= user.id or a.status ~= "pending" or a.created_epoch + 3600 <= now() or not P.canPost(a.room_id, user.id) then return fail(403, "42501", "Photo is unavailable.") end
            if not state.objects[a.object_key] or not state.objects[a.thumbnail_key] then return fail(400, "unfinished_upload", "Both JPEG files must be uploaded.") end
            a.status = "ready"; return response(200, a)
        elseif downloadKey and method == "GET" then
            local a
            for _, item in pairs(state.attachments) do if item.object_key == downloadKey or item.thumbnail_key == downloadKey then a = item; break end end
            if not photoAccess(a, user, true) or not state.objects[downloadKey] then return fail(403, "42501", "Photo is unavailable.") end
            return response(200, state.objects[downloadKey])
        elseif path == "/storage/v1/object/chat-photos" and method == "DELETE" then
            if type(body) ~= "table" or type(body.prefixes) ~= "table" then return fail(400, "validation", "Provide object keys.") end
            for _, key in ipairs(body.prefixes) do
                local found
                for _, a in pairs(state.attachments) do if a.uploader_id == user.id and a.status == "discarded" and (a.object_key == key or a.thumbnail_key == key) then found = true end end
                if not found then return fail(403, "42501", "Object cannot be deleted.") end
            end
            for _, key in ipairs(body.prefixes) do state.objects[key] = nil end
            return response(200, {})
        end
        return fail(405, "method_not_allowed", "Unsupported method.")
    end
    return P
end
