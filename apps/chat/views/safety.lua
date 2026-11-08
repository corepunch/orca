local fs = require "orca.filesystem"
local I18n = require "Chat/i18n"
local C = require "Chat/views/components"
local function t(key) return I18n:t(key) end
local M = {}
function M.open(app)
    local target, session = app.safetyTarget, app.client and app.client:getSession()
    if not target or not session then return app:home() end
    local page = assert(fs.loadObject("Chat/Pages/Safety"), "Missing Chat/Pages/Safety")
    local status, block = page:findChild("SafetyStatus", true), page:findChild("BlockPerson", true)
    page:findChild("SafetyPerson", true):setTextContent(target.name or target.userId)
    local details, reasons = page:findChild("ReportDetails", true), page:findChild("ReportReasons", true)
    details.PlaceholderText = t("Optional details (up to 1000 characters)")
    local epoch, ready, blocked, busy, reported = app.sessionEpoch, false, false, false, false
    local function current() return app.sessionEpoch == epoch and app.currentRoute == "/safety" end
    local function notice(text, isError)
        status:setTextContent(t(text))
        status.Visible, status.StateClasses = text ~= "", isError and "chat-error" or "chat-muted"
    end
    local function label(text)
        block.TextResourceID = text
        block:setTextContent(t(text))
    end
    local function loadBlocks()
        local rows, err = app.client:listBlocks()
        if not current() then return end
        if not rows then notice(err.message, true); label("Retry safety settings"); return end
        for _, row in ipairs(rows) do if row.user_id == target.userId then blocked = true end end
        ready = true
        label(blocked and "unblock person" or "block person")
    end
    block.Click = function()
        if busy then return end
        busy = true
        if not ready then loadBlocks(); busy = false; return end
        local ok, err = app.client:setUserBlocked(target.userId, not blocked)
        busy = false
        if not current() then return end
        if not ok then notice(err.message, true); return end
        blocked = not blocked
        label(blocked and "unblock person" or "block person")
        notice(blocked and "Person blocked." or "Person unblocked.")
    end
    for _, entry in ipairs {
        { "ReportSpam", "spam" }, { "ReportHarassment", "harassment" }, { "ReportSexual", "sexual_content" },
        { "ReportViolence", "violence" }, { "ReportHate", "hate" }, { "ReportOther", "other" },
    } do
        page:findChild(entry[1], true).Click = function()
            if busy or reported then return end
            busy = true
            local result, err = app.client:reportContent(target.userId, entry[2], details.Text or "", target.messageId)
            busy = false
            if not current() then return end
            if not result then notice(err.message, true); return end
            reported, reasons.Visible = true, false
            notice("Report submitted. Moderators can review it and act on the account or content.")
        end
    end
    local screen = app:frame(C.page(t("safety"), page, { back = function() app:goBack(target.back or "/details") end }))
    app:load(loadBlocks)
    return screen
end
return M
