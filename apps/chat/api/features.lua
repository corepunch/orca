return function(Client, helpers)
    local failure, escape, validId = helpers.failure, helpers.escape, helpers.validId
    local function identifier(value)
        if type(value) ~= "string" then return nil end
        value = value:match("^%s*(.-)%s*$")
        if value:sub(1, 1) == "+" then
            value = value:gsub("[%s%-%(%)]", "")
            if value:match("^%+[1-9]%d+$") and #value >= 9 and #value <= 16 then return "phone", value end
        elseif value:match("^[^%s@]+@[^%s@]+%.[^%s@]+$") then return "email", value:lower() end
    end
    function Client:_rpc(name, body)
        return self:_request("POST", "/rest/v1/rpc/" .. name, body, true)
    end
    function Client:requestCode(value)
        local field, normalized = identifier(value)
        if not field then return failure("validation", "Use a valid email or international phone number, starting with +.") end
        local data, err = self:_request("POST", "/auth/v1/otp", { [field] = normalized, create_user = true })
        if not data then return nil, err end
        return { identifier = normalized, delivery = field, expires_in = 300, retry_after = 60 }
    end
    function Client:verifyCode(value, code)
        local field, normalized = identifier(value)
        if not field or type(code) ~= "string" or not code:match("^%d%d%d%d%d%d$") then return failure("validation", "Enter the six-digit verification code.") end
        local data, err = self:_request("POST", "/auth/v1/verify", { [field] = normalized, token = code, type = field == "phone" and "sms" or "email" })
        if not data then return nil, err end
        return self:_setSession(data)
    end
    function Client:requestIdentityLink(value)
        local field, normalized = identifier(value)
        if not field then return failure("validation", "Use a valid email or international phone number.") end
        local data, err = self:_request("PUT", "/auth/v1/user", { [field] = normalized }, true)
        if not data then return nil, err end
        return { identifier = normalized, delivery = field, expires_in = 300, retry_after = 60 }
    end
    function Client:verifyIdentityLink(value, code)
        local field, normalized = identifier(value)
        if not field or type(code) ~= "string" or not code:match("^%d%d%d%d%d%d$") then return failure("validation", "Enter the six-digit verification code.") end
        if not self.session then return failure("unauthenticated", "Sign in to link an identifier.") end
        local id = self.session.user.id
        local data, err = self:_request("POST", "/auth/v1/verify", {
            [field] = normalized, token = code, type = field == "phone" and "phone_change" or "email_change",
        }, true)
        if not data then return nil, err end
        local user = data.user or data
        if user.id ~= id then return failure("identity_mismatch", "Verification did not confirm this account.") end
        if data.access_token then
            local session; session, err = self:_setSession(data)
            if not session then return nil, err end
        else self.session.user = user end
        local profile; profile, err = self:getProfile()
        if not profile then return nil, err end
        if type(profile) ~= "table" or profile.id ~= id then
            return failure("identity_mismatch", "Verification did not confirm this account's profile.")
        end
        local confirmedField, confirmed = identifier(profile[field])
        if confirmedField ~= field or confirmed ~= normalized then
            return failure("verification_pending", "Confirm the remaining verification code to finish linking this identifier.")
        end
        return profile
    end
    function Client:getProfile() return self:_rpc("get_chat_profile", {}) end
    function Client:updateProfile(values)
        if type(values) ~= "table" then return failure("validation", "Provide profile changes.") end
        local data, err = self:_rpc("update_chat_profile", { values = values })
        if data and self.session then self.session.user.user_metadata = { display_name = data.display_name } end
        return data, err
    end
    function Client:findPeople(query)
        if type(query) ~= "string" or not query:match("%S") or #query > 254 then return failure("validation", "Enter a username, email or phone number.") end
        local field, normalized = identifier(query)
        return self:_rpc("find_chat_people", { query = normalized or query:lower() })
    end
    function Client:findChannels(query)
        if type(query) ~= "string" or #query > 80 then return failure("validation", "Use a channel name of at most 80 characters.") end
        return self:_rpc("find_chat_channels", { query = query })
    end
    function Client:joinPublicChannel(roomId) return self:_rpc("join_public_chat_channel", { target_room = roomId }) end
    function Client:listMembers(roomId) return self:_rpc("list_chat_members", { target_room = roomId }) end
    function Client:setMemberRole(roomId, userId, role)
        return self:_rpc("set_chat_member_role", { target_room = roomId, target_user = userId, member_role = role })
    end
    function Client:removeMember(roomId, userId)
        return self:_rpc("remove_chat_member", { target_room = roomId, target_user = userId })
    end
    function Client:createInvite(roomId) return self:_rpc("create_chat_invite", { target_room = roomId }) end
    function Client:joinInvite(code) return self:_rpc("join_chat_invite", { invite_code = code }) end
    function Client:revokeInvite(roomId, code) return self:_rpc("revoke_chat_invite", { target_room = roomId, invite_code = code }) end
    function Client:discardPhoto(id)
        local data, err = self:_rpc("discard_chat_photo", { attachment_id = id })
        if not data then return nil, err end
        local keys = { data.object_key, data.thumbnail_key }
        local removed; removed, err = self:_request("DELETE", "/storage/v1/object/chat-photos", { prefixes = keys }, true)
        if not removed then return nil, err end
        return true
    end
    function Client:uploadPhoto(roomId, photo)
        if not validId(roomId) or type(photo) ~= "table" then return failure("validation", "Choose a conversation and a photo.") end
        local jpeg = require "api.jpeg"
        local width, height = jpeg.dimensions(photo.jpeg)
        if not width then return failure("validation", height) end
        local tw, th = jpeg.dimensions(photo.thumbnail)
        if not tw then return failure("validation", th) end
        if #photo.jpeg > 1000000 or #photo.thumbnail > 1000000 or math.max(width, height) > 1600 or math.max(tw, th) > 256 then
            return failure("validation", "Photo exceeds the image size or dimension limit.")
        end
        if (photo.width and photo.width ~= width) or (photo.height and photo.height ~= height) then return failure("validation", "Photo dimensions do not match the JPEG data.") end
        local attachment, err = self:_rpc("reserve_chat_photo", {
            target_room = roomId, photo_width = width, photo_height = height, photo_bytes = #photo.jpeg,
            thumbnail_width = tw, thumbnail_height = th, thumbnail_bytes = #photo.thumbnail,
        })
        if not attachment then return nil, err end
        for _, image in ipairs { { attachment.object_key, photo.jpeg }, { attachment.thumbnail_key, photo.thumbnail } } do
            local uploaded; uploaded, err = self:_request("POST", "/storage/v1/object/chat-photos/" .. image[1], image[2], true, false,
                { headers = { ["Content-Type"] = "image/jpeg", ["x-upsert"] = "false" } })
            if not uploaded then self:discardPhoto(attachment.id); return nil, err end
        end
        local ready; ready, err = self:_request("POST", "/functions/v1/finalize-chat-photo", { attachment_id = attachment.id }, true)
        if not ready then self:discardPhoto(attachment.id); return nil, err end
        return ready
    end
    function Client:getPhoto(id, thumbnail)
        if not validId(id) then return failure("validation", "Choose a photo.") end
        local rows, err = self:_request("GET", "/rest/v1/chat_attachments?select=*&id=eq." .. escape(id), nil, true)
        if not rows then return nil, err end
        local attachment = rows[1]
        if not attachment then return failure("not_found", "Photo is unavailable.") end
        local key = thumbnail and attachment.thumbnail_key or attachment.object_key
        return self:_request("GET", "/storage/v1/object/authenticated/chat-photos/" .. key, nil, true, false, { responseType = "binary" })
    end
    -- Public profile photos, shared with Octopus and keyed by Auth user ID.
    -- Returns userId -> photo ID, or false for people without one. Answers are
    -- cached per client; lookups need no session.
    function Client:getAvatars(userIds)
        if type(userIds) ~= "table" then return failure("validation", "Choose people.") end
        local wanted, seen = {}, {}
        for _, id in ipairs(userIds) do
            if not validId(id) then return failure("validation", "Choose valid people.") end
            if self.avatarIds[id] == nil and not seen[id] then seen[id] = true; wanted[#wanted + 1] = id end
        end
        for first = 1, #wanted, 100 do
            local chunk = { table.unpack(wanted, first, math.min(#wanted, first + 99)) }
            local rows, err = self:_request("GET", "/rest/v1/profile_photos?select=user_id,avatar_id&user_id=in.("
                .. escape(table.concat(chunk, ",")) .. ")")
            if not rows then return nil, err end
            if type(rows) ~= "table" then return failure("invalid_response", "The chat service returned invalid profile photos.") end
            for _, id in ipairs(chunk) do self.avatarIds[id] = false end
            for _, row in ipairs(rows) do
                if type(row) == "table" and seen[row.user_id] and validId(row.avatar_id) then
                    self.avatarIds[row.user_id] = row.avatar_id
                end
            end
        end
        local avatars = {}
        for _, id in ipairs(userIds) do avatars[id] = self.avatarIds[id] end
        return avatars
    end
    function Client:getAvatar(avatarId)
        if not validId(avatarId) then return failure("validation", "Choose a profile photo.") end
        local bytes, err = self:_request("GET", "/storage/v1/object/public/profile-photos/" .. escape(avatarId), nil, false, false,
            { responseType = "binary" })
        if not bytes then return nil, err end
        if type(bytes) ~= "string" or not (bytes:sub(1, 3) == "\255\216\255" or bytes:sub(1, 8) == "\137PNG\r\n\26\n") then
            return failure("invalid_image", "This profile photo can't be shown.")
        end
        return bytes
    end
end
