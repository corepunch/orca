-- Safety and account pages: private reports, blocked people, the community
-- rules, the privacy policy and account deletion. Each page is XML
-- (Pages/Report.xml, Blocked.xml, Document.xml, DeleteAccount.xml); this
-- module wires its commands to the client.
local filesystem = require "orca.filesystem"
local C = require "Octopus/views/components"
local colors = C.colors
local Safety = {}

Safety.privacyUrl = "https://corepunch.github.io/orca/chat/privacy/"

local REASONS = {
    ReasonSpam = "spam", ReasonHarassment = "harassment", ReasonSexualContent = "sexual_content",
    ReasonViolence = "violence", ReasonHate = "hate", ReasonOther = "other",
}

local function load(self, name)
    local page = assert(filesystem.loadObject("Octopus/Pages/" .. name), "Missing Octopus/Pages/" .. name)
    self.i18n:apply(page)
    return page
end

-- Chat hosts these pages too; its frame takes no content.
local function pushed(self, title, page, back)
    self.statusBar = nil
    local screen = self:frame()
    screen:addChild(C.page(self.i18n:t(title), page, { back = back }))
    self.i18n:apply(screen)
    return screen
end

-- Open the report page for { userId, name, postId, commentId }.
function Safety.open(self, target)
    if not self:requireSession("Sign in to report.") then return end
    self.reportTarget, self.reportReturn = target, self.currentRoute
    self:show("/report", "Forward")
end

-- Block or unblock a person, then drop what was loaded with or without them.
function Safety.setBlocked(self, userId, name, blocked, done)
    if not self:requireSession("Sign in to block people.") then return end
    self:notice(blocked and "Blocking…" or "Unblocking…")
    self:task(function()
        local ok, err = self.client:setUserBlocked(userId, blocked)
        if not ok then self:notice(err.message, true); return end
        self:invalidate()
        if done then return done() end
        self:notice(self.i18n:t(blocked and "%{name} is blocked." or "%{name} is unblocked.", { name = name or "This person" }))
    end)
end

function Safety.report(self)
    local target = self.reportTarget
    if not target or not self.client:getSession() then return self:home() end
    local page = load(self, "Report")
    local function back() self:goBack(self.reportReturn or "/") end
    local subject = target.commentId and "Report a comment by %{name}" or target.postId and "Report a post by %{name}"
        or "Report %{name}"
    page:findChild("ReportSubject", true):setTextContent(self.i18n:t(subject, { name = target.name or "this person" }))
    self.status = page:findChild("ReportStatus", true)
    local details, busy, sent = page:findChild("ReportDetails", true), false, false
    local block, done = page:findChild("BlockReported", true), page:findChild("ReportDone", true)
    for name, reason in pairs(REASONS) do
        page:findChild(name, true).Click = function()
            if busy or sent then return end
            busy = true
            self:notice("Sending report…")
            self:task(function()
                local result, err = self.client:reportContent { userId = target.userId, reason = reason,
                    details = details.Text or "", postId = target.postId, commentId = target.commentId }
                busy = false
                if not result then self:notice(err.message, true); return end
                sent = true
                for button in pairs(REASONS) do page:findChild(button, true).Visible = false end
                details.Visible = false
                block.Visible, done.Visible = not self.client:isBlocked(target.userId), true
                self:notice("Thank you. Your report was sent privately and will be reviewed within 24 hours.")
            end)
        end
    end
    block.Click = function()
        Safety.setBlocked(self, target.userId, target.name, true, function()
            self.post = nil
            self:goBack("/")
        end)
    end
    done.Click = back
    return pushed(self, "report", page, back)
end

function Safety.blocked(self)
    if not self.client:getSession() then return self:home() end
    local page = load(self, "Blocked")
    local list = page:findChild("BlockedPeople", true)
    self.status = page:findChild("BlockedStatus", true)
    local function fill()
        local people, err = self.client:listBlocks()
        if not people then self:notice(err.message, true); return end
        C.clear(list)
        for _, person in ipairs(people) do
            local row = require("orca.UIKit").Grid { Name = "Blocked" .. person.userId, Columns = "1fr 112px", Spacing = 12, Height = 44 }
            row:addChild(C.text(person.name, { FontSize = 17, TextVerticalAlignment = "Center", Height = 44 }))
            row:addChild(C.outline(self.i18n:t("unblock"), function()
                Safety.setBlocked(self, person.userId, person.name, false, fill)
            end, { Name = "Unblock", Height = 44 }))
            list:addChild(row)
        end
        self:notice(#people == 0 and "You have not blocked anyone." or "")
        self:repaint()
    end
    self:notice("Loading…")
    self:task(fill)
    return pushed(self, "blocked people", page, function() self:goBack("/me") end)
end

local function document(self, title, asset, link)
    local page = load(self, "Document")
    page:findChild("DocumentText", true).Source = assert(filesystem.readTextFile("Octopus/assets/" .. asset),
        "Missing Octopus/assets/" .. asset)
    local url = page:findChild("DocumentLink", true)
    url:setTextContent(link or "")
    url.Visible = link ~= nil
    self.status = nil
    local route = self.documentReturn or "/"
    return pushed(self, title, page, function() self:goBack(route) end)
end

function Safety.rules(self) return document(self, "community rules", "community-rules.md") end
function Safety.privacy(self) return document(self, "privacy policy", "privacy.md", Safety.privacyUrl) end

-- Rules and the privacy policy are readable before signing in.
function Safety.openDocument(self, route)
    self.documentReturn = self.currentRoute
    self:show(route, "Forward")
end

function Safety.deleteAccount(self)
    if not self.client:getSession() then return self:home() end
    local page = load(self, "DeleteAccount")
    self.status = page:findChild("DeleteAccountStatus", true)
    local confirm, busy, asked = page:findChild("ConfirmDeleteAccount", true), false, false
    local function back() if not busy then self:goBack("/me") end end
    page:findChild("KeepAccount", true).Click = back
    confirm.Click = function()
        if busy then return end
        if not asked then
            asked = true
            confirm.TextResourceID = "Tap again to delete permanently"
            confirm.Text = self.i18n:t(confirm.TextResourceID)
            self:notice("This cannot be undone.", true)
            return
        end
        busy = true
        self:notice("Deleting your account…")
        self:task(function()
            local ok, err = self.client:deleteAccount("DELETE")
            busy = false
            if not ok then self:notice(err.message, true); return end
            self:resetSessionViews()
            self:show("/")
            self:notice("Your account was deleted.")
        end)
    end
    return pushed(self, "delete account", page, back)
end

return Safety
