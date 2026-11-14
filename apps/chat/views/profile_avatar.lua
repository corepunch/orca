local I18n = require "Chat/i18n"
local function t(key, params) return I18n:t(key, params) end
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

-- A saved photo replaces cached copies at once; returns its preview path.
function Avatar.saved(app, userId, photo, result)
    app.avatars:invalidate(userId)
    if app.postsFeature then app.postsFeature:avatarChanged(userId) end
    local path = app.media:storePreview(photo.jpeg or photo.bytes, "avatar-" .. result.avatar_id)
    if path then app.avatarPaths[userId] = path end
    return path
end

-- Every account starts with a picture, as on Discord: an account without one
-- gets a random character. Returns true when it had none (a new account).
function Avatar.assignRandom(app)
    local session = app.client:getSession()
    if not session then return false end
    local userId = session.user.id
    local ok, avatars = pcall(app.client.getAvatars, app.client, { userId })
    if not ok or type(avatars) ~= "table" or avatars[userId] ~= false then return false end
    local photo = Avatars.load(Avatars.random(app.randomRoll).id)
    if not photo then return true end
    local saved, result = pcall(app.client.setAvatar, app.client, photo)
    local active = app.client:getSession()
    if saved and result and active and active.user.id == userId then Avatar.saved(app, userId, photo, result) end
    return true
end

-- `options.camera` adds taking a photo where the device has a camera.
function Avatar.controls(app, tile, status, options)
    local session = app.client:getSession()
    local userId, epoch = session.user.id, app.sessionEpoch
    local busy = false
    local function sameAccount()
        local active = app.client:getSession()
        return app.sessionEpoch == epoch and active and active.user.id == userId
    end
    local function notice(node, message, failed)
        node:setTextContent(t(message or ""))
        node.Visible = message ~= nil and message ~= ""
        node.StateClasses = failed and "chat-error" or "chat-muted"
    end
    local function upload(photo, target, current, complete)
        if busy or not sameAccount() or not current() then return end
        busy = true
        notice(target, t("Saving picture…"))
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
                local path = Avatar.saved(app, userId, photo, result)
                if path then C.photo(tile, path) else app:avatarPhoto(tile, userId) end
            end
            if ok and result then notice(status, t("Picture updated.")) end
            if not current() and target ~= status then return end
            if not ok or not result then
                notice(target, type(err) == "table" and err.message or t("Could not save your picture. Choose again to retry."), true)
            else
                if complete and current() then complete() end
            end
        end)
    end
    local function choose()
        if busy or not sameAccount() then return end
        Avatar.close(app)
        local root = assert(filesystem.loadObject("Chat/Pages/AvatarPicker"), "Missing avatar picker")
        I18n:apply(root)
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
                            notice(target, err and err.message or t("Could not load this character. Choose again to retry."), true)
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
    local function pick(source)
        if busy or not sameAccount() then return end
        local screen, navigation = app.screen, app.navigationEpoch
        local function current() return app.screen == screen and app.navigationEpoch == navigation end
        local ok, photo, err = pcall(app.media[source or "pick"], app.media)
        if not sameAccount() or not current() then return end
        if not ok or not photo then
            if not ok or not err or err.code ~= "cancelled" then
                notice(status, type(err) == "table" and err.message or t("Could not open this photo."), true)
            end
            return
        end
        upload(photo, status, current)
    end
    local commands = C.stack({ Spacing = 0 }, {
        C.command("ChooseAvatar", "user-round", t("choose a character"), choose),
        C.command("UploadAvatar", "image", t("upload photo"), function() pick("pick") end),
    })
    local media = app.media
    if (not options or options.camera ~= false) and media.canTake and media:canTake() then
        commands:addChild(C.command("TakeAvatarPhoto", "camera", t("take photo"), function() pick("take") end))
    end
    return commands
end

-- The step after creating an account: the character chosen at random, with
-- the same ways to change it as settings. Continue carries on to where
-- sign-in was heading.
function Avatar.welcome(app)
    local session = app.client and app.client:getSession()
    if not session then return app:home() end
    local user = session.user
    local name = user.user_metadata and user.user_metadata.display_name or t("You")
    local tile = C.avatar(name, 160, user.id, app, user.id)
    tile.Name = "WelcomeAvatar"
    local status = C.text("", { Name = "WelcomePictureStatus", Visible = false, WordWrap = true, FontSize = 14,
        StateClasses = "chat-muted" })
    local continue = C.button(t("Continue"), function()
        local resume = app.pictureResume
        app.pictureResume = nil
        Avatar.close(app)
        if resume then resume(app) else app:show() end
    end, { Name = "WelcomePictureContinue", class = "chat-primary", Height = 52, FontSize = 16 })
    local body = C.stack({ Name = "WelcomePicture", Spacing = 16, Overflow = "hidden scroll" }, {
        C.text(t("We picked a sea creature for you. Keep it, choose another, or add a photo of yourself. You can change it later in settings."),
            { FontSize = 15, class = "chat-muted", WordWrap = true }),
        tile, status,
        Avatar.controls(app, tile, status),
        continue,
    })
    return app:frame(C.page(t("your picture"), body))
end

return Avatar
