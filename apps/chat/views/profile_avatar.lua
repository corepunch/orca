local C = require "Chat/views/components"
local filesystem = require "orca.filesystem"
local data = require "orca.data"
local Avatars = require "orca.avatars"
local Avatar = {}

function Avatar.close(app)
    local picker = app.avatarPicker
    if not picker then return end
    app.avatarPicker = nil
    picker.scope:dispose()
    picker.root:removeFromParent()
end

function Avatar.controls(app, tile, status)
    local session = app.client:getSession()
    local userId, epoch = session.user.id, app.sessionEpoch
    local busy = false
    local function sameAccount()
        local active = app.client:getSession()
        return app.sessionEpoch == epoch and active and active.user.id == userId
    end
    local function notice(node, message, failed)
        node:setTextContent(message or "")
        node.Visible = message ~= nil and message ~= ""
        node.StateClasses = failed and "chat-error" or "chat-muted"
    end
    local function upload(photo, target, current, complete)
        if busy or not sameAccount() or not current() then return end
        busy = true
        notice(target, "Saving picture…")
        app:load(function()
            if not sameAccount() or not current() then
                busy = false
                if sameAccount() and target == status then notice(status, "") end
                return
            end
            local ok, result, err = pcall(app.client.setAvatar, app.client, photo)
            busy = false
            if not sameAccount() then return end
            if ok and result then
                app.avatars:invalidate(userId)
                local path = app.media:storePreview(photo.jpeg or photo.bytes, "avatar-" .. result.avatar_id)
                if path then
                    app.avatarPaths[userId] = path
                    C.photo(tile, path)
                else app:avatarPhoto(tile, userId) end
            end
            if ok and result then notice(status, "Picture updated.") end
            if not current() and target ~= status then return end
            if not ok or not result then
                notice(target, type(err) == "table" and err.message or "Could not save your picture. Choose again to retry.", true)
            else
                if complete and current() then complete() end
            end
        end)
    end
    local function choose()
        if busy or not sameAccount() then return end
        Avatar.close(app)
        local root = assert(filesystem.loadObject("Chat/Pages/AvatarPicker"), "Missing avatar picker")
        local screen, navigation = app.screen, app.navigationEpoch
        local picker = { root = root }
        local function current()
            return app.avatarPicker == picker and app.screen == screen and app.navigationEpoch == navigation
        end
        local scope = data.scope("Chat/Data/AvatarChoices.xml", { valid = sameAccount,
            provider = {
                operations = { choices = function() return Avatars.entries end },
                handlers = {
                    CloseAvatarPicker = function() if current() then Avatar.close(app) end end,
                    ChooseAvatar = function(_, params)
                        if busy or not current() then return end
                        local record = params.item
                        if not record then return end
                        local photo, err = Avatars.load(record.values.Id)
                        local target = root:findChild("AvatarChoiceStatus", true)
                        if not photo then
                            notice(target, err and err.message or "Could not load this character. Choose again to retry.", true)
                            return
                        end
                        upload(photo, target, current, function() Avatar.close(app) end)
                    end,
                },
            },
        })
        picker.scope, app.avatarPicker = scope, picker
        scope:attach(root)
        screen:addChild(root)
        scope:activate()
    end
    local function pick()
        if busy or not sameAccount() then return end
        local screen, navigation = app.screen, app.navigationEpoch
        local function current() return app.screen == screen and app.navigationEpoch == navigation end
        local ok, photo, err = pcall(app.media.pick, app.media)
        if not sameAccount() or not current() then return end
        if not ok or not photo then
            if not ok or not err or err.code ~= "cancelled" then
                notice(status, type(err) == "table" and err.message or "Could not open this photo.", true)
            end
            return
        end
        upload(photo, status, current)
    end
    return C.stack({ Spacing = 0 }, {
        C.command("ChooseAvatar", "user-round", "choose a character", choose),
        C.command("UploadAvatar", "camera", "upload photo", pick),
    })
end

return Avatar
