local I18n = require "Chat/i18n"
local C = require "Chat/views/components"
local filesystem = require "orca.filesystem"
local data = require "orca.data"
local PublicPost = require "Chat/public_post"
local Social = {}
local function t(key, params) return I18n:t(key, params) end

local function refreshMe(self, page, name)
    local epoch, shell = self.sessionEpoch, self.shell
    self:load(function()
        local profile, err = self.client:getProfile()
        if self.sessionEpoch ~= epoch or self.shell ~= shell or self.meContent ~= page then return end
        if profile then
            page:findChild("MeName", true):setTextContent(profile.display_name or name)
            page:findChild("MeHandle", true):setTextContent((profile.username or "") ~= "" and "@" .. profile.username or "")
            self.meStatus.Visible = false
        elseif err then
            self.meStatus:setTextContent(t(err.message)); self.meStatus.Visible = true
        end
    end)
end

function Social.me(self)
    local session = self.client and self.client:getSession()
    if not session then return self:home() end
    if self.shell and self.shell:has("Me") then
        self.status = self.meStatus
        refreshMe(self, self.meContent, (session.user.user_metadata or {}).display_name or t("You"))
        return self.shell:show("Me")
    end
    local page = assert(filesystem.loadObject("Chat/Pages/Me"), "Missing Chat/Pages/Me")
    local name = (session.user.user_metadata or {}).display_name or session.user.name or t("You")
    local avatar = C.avatar(name, 96, session.user.id, self, session.user.id)
    page:findChild("MeAvatarHost", true):addChild(avatar)
    page:findChild("MeName", true):setTextContent(name)
    page:findChild("MeHandle", true):setTextContent("")
    page:findChild("MyPublicProfile", true).Click = function() self:show("/posts/me", "Forward") end
    page:findChild("MyPosts", true).Click = function() self:show("/posts/me", "Forward") end
    page:findChild("MeSettings", true).Click = function() self:show("/profile", "Forward") end
    page:findChild("MeSignOut", true).Click = function() self:signOut() end
    self.meContent = page
    self.meStatus = page:findChild("MeStatus", true)
    self.status = self.meStatus
    local screen = self:frame(C.page(t("me"), page), "Me")
    refreshMe(self, page, name)
    return screen
end

function Social.messagePerson(self, id, name)
    local session = self.client and self.client:getSession()
    if not session then
        self.publicAuthResume = function(app) app:messagePerson(id, name) end
        return self:show("/auth")
    end
    if id == session.user.id then return self:show("/me") end
    local epoch, navigation = self.sessionEpoch, self.navigationEpoch
    self:load(function()
        local room, err = self.models.Rooms:create { name = name or t("Member"), member_ids = { id }, kind = "direct", visibility = "private" }
        if self.sessionEpoch ~= epoch or self.navigationEpoch ~= navigation then return end
        local active = self.client:getSession()
        if not active or active.user.id ~= session.user.id then
            self:resetSessionViews(); self:show("/"); return
        end
        if not room then self:notice(err and err.message or t("Could not open this conversation. Try again."), true); return end
        self:openRoom(room)
    end)
end

function Social.sharePost(self, post)
    local message = PublicPost.message(post)
    if not message then self:notice(t("This post cannot be shared."), true); return end
    self.pendingPostShare = { text = message, title = post.title or t("Public post"), back = self.currentRoute or "/posts" }
    self:show("/share-post", "Forward")
end

function Social.sharePage(self)
    local pending = self.pendingPostShare
    if not self.client or not self.client:getSession() then
        if pending then
            self.publicAuthResume = function(app)
                app.pendingPostShare = pending
                app:show("/share-post")
            end
        end
        return self:auth()
    end
    if not pending then return self:show("/posts") end
    self:dataScope()
    local shared = self.sessionData
    local scope = data.scope("Chat/Data/Share.xml", { provider = shared.provider, store = shared.store, host = shared.host, valid = shared.valid })
    self.routeScope = scope
    local page = assert(filesystem.loadObject("Chat/Pages/Share"), "Missing Chat/Pages/Share")
    scope:attach(page)
    page:findChild("SharedPostTitle", true):setTextContent(pending.title)
    self.status = nil
    self.visibility:watch(page, scope)
    local screen = self:frame(C.page(t("share in chat"), page, { back = function() self:goBack(pending.back) end }))
    scope:activate()
    return screen
end

function Social.selectShareRoom(self, room)
    local pending = self.pendingPostShare
    if not pending or not self.client:getSession() or room.can_post == false then return false end
    local old = self.drafts[room.id] or ""
    self.drafts[room.id] = old ~= "" and old .. "\n\n" .. pending.text or pending.text
    self.pendingPostShare = nil
    self:openRoom(room)
    return true
end

return Social
