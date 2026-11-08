local filesystem = require "orca.filesystem"
local I18n = require "Chat/i18n"
local C = require "Chat/views/components"
local function t(key) return I18n:t(key) end

local M = {}
function M.open(app)
    if not app.client or not app.client:getSession() and not app.client.accountDeletion then
        app.accountDeletionPending = nil
        return app:home()
    end
    local page = assert(filesystem.loadObject("Chat/Pages/DeleteAccount"), "Missing Chat/Pages/DeleteAccount")
    local confirm = assert(page:findChild("ConfirmDeleteAccount", true))
    local cancel = assert(page:findChild("CancelDeleteAccount", true))
    local status = assert(page:findChild("DeleteAccountStatus", true))
    local busy = app.accountDeletionBusy == true
    local screen
    local function label(key)
        confirm.TextResourceID = key
        confirm:setTextContent(t(key))
    end
    local function back()
        if busy or app.client.accountDeletion then return end
        app.accountDeletionPending, app.accountDeletionError = nil, nil
        app:goBack("/profile")
    end
    if app.accountDeletionError then
        status.Visible = true
        status:setTextContent(t(app.accountDeletionError))
        label("Retry account deletion")
    end
    if app.client.accountDeletion then
        cancel.Visible = false
        status.Visible = true
        status:setTextContent(t("Account deletion has started. Retry to remove the saved login from this device."))
        label("Retry device cleanup")
    end
    if busy then
        confirm.Visible, cancel.Visible, status.Visible = false, false, true
        status:setTextContent(t("Deleting account…"))
    end
    cancel.Click = back
    confirm.Click = function()
        if busy then return end
        busy, app.accountDeletionBusy, app.accountDeletionPending = true, true, true
        app.accountDeletionError = nil
        label("Deleting account…")
        cancel.Visible, status.Visible = false, false
        -- Stop polling and discard cached private content before the server
        -- revokes the account. No stale callback can repopulate the old views.
        app:resetSessionViews()
        if app.push then app.push:suspend() end
        if app.signin then app.signin:suspend() end
        app:load(function()
            local result, err = app.client:deleteAccount("DELETE")
            busy, app.accountDeletionBusy = false, false
            if result then
                app.accountDeletionPending, app.accountDeletionError, app.restorePending, app.logoutPending = nil, nil, false, false
                app:resetSessionViews()
                app:show("/")
                app:notice(result.deleted and t("Your account has been deleted.")
                    or t("Account deletion has started. You have been signed out of Chat and Octopus."))
                return
            end
            if not app.client:getSession() and not app.client.accountDeletion then
                app.accountDeletionPending = nil
                app:show("/")
                app:notice(err.message, true)
                return
            end
            status:setTextContent(t(err.message))
            app.accountDeletionError = err.message
            status.Visible = true
            cancel.Visible = app.client.accountDeletion == nil
            label(app.client.accountDeletion and "Retry device cleanup" or "Retry account deletion")
            -- A system callback may have attempted navigation while this
            -- request yielded; reconstruct the pending page with its result.
            if app.screen ~= screen then app:show("/delete-account") end
        end)
    end
    screen = app:frame(C.page(t("delete account"), page, { back = back }))
    return screen
end
return M
