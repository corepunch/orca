local ui = require "orca.UIKit"
local C = require "Octopus/views/components"
local colors = C.colors
local Compose = {}

local KINDS = {
    { key = "text", icon = "file-text", label = "text" },
    { key = "photo", icon = "image", label = "photo" },
    { key = "quote", icon = "quote", label = "quote" },
    { key = "link", icon = "link", label = "link" },
}

-- Fields per post type: { draft key, input name, placeholder, label, extra props }.
local FIELDS = {
    text = {
        { "title", "PostTitle", "Give it a title", "title" },
        { "content", "PostContent", "What's on your mind? Markdown works on the website.", "post", { Multiline = true, Height = 180 } },
    },
    quote = {
        { "content", "QuoteText", "The words worth sharing", "quote", { Multiline = true, Height = 110 } },
        { "quoteSource", "QuoteSource", "Who said it? A person, book or site", "source (optional)" },
        { "photo", "QuotePhotoPreview", nil, "background photo (optional)" },
        { "userText", "QuoteThoughts", "What it means to you", "your thoughts (optional)", { Multiline = true, Height = 80 } },
    },
    link = {
        { "title", "LinkTitle", "What is it?", "title" },
        { "linkUrl", "LinkUrl", "https://", "address" },
        { "content", "LinkDescription", "Why it's worth a read", "description (optional)", { Multiline = true, Height = 96 } },
    },
    photo = {
        { "photo", "PhotoPreview", nil, "photo" },
        { "content", "PhotoCaption", "Say something about it", "caption (optional)", { Multiline = true, Height = 80 } },
    },
}

local function values(draft)
    local result = { postType = draft.postType, tags = draft.tags }
    for _, field in ipairs(FIELDS[draft.postType]) do result[field[1]] = draft[field[1]] end
    return result
end

local function unchanged(draft, submitted)
    local current = values(draft)
    for key, value in pairs(current) do if submitted[key] ~= value then return false end end
    for key, value in pairs(submitted) do if current[key] ~= value then return false end end
    return true
end

local function feedback(self, draft, message, isError)
    draft.notice, draft.error = message, isError
    if self.currentRoute == "/write" and self.composeView and self.composeView.draft == draft then
        self:notice(message, isError)
    end
end

local function photoField(self, view, field)
    local draft = view.draft
    local group = C.stack({ Spacing = 8 }, { C.text(field[4], { FontSize = 14, Foreground = colors.muted }) })
    local preview = ui.Grid { Name = field[2], Height = draft.photo and 200 or 0, Visible = draft.photo ~= nil,
        Background = colors.raised }
    local path = draft.photo and self.media:store(draft.photo.jpeg, "draft-" .. tostring(draft.photo))
    if path then preview:addChild(ui.ImageView { Src = path, Stretch = "Uniform", Foreground = "#ffffff", IgnoreHitTest = true }) end
    group:addChild(preview)
    group:addChild(C.outline(draft.photo and "Choose another photo" or "Choose a photo", function()
        if draft.publishing then return end
        view.capture()
        feedback(self, draft, "Opening photos…")
        local photo, err = self.media:pick()
        if not photo then
            local cancelled = type(err) == "table" and err.code == "cancelled"
            feedback(self, draft, cancelled and "" or (type(err) == "table" and err.message or tostring(err)), not cancelled)
            return
        end
        draft.photo = photo
        feedback(self, draft, "")
        self:show("/write")
    end, { Name = "ChoosePhoto", IgnoreHitTest = draft.publishing or false, Opacity = draft.publishing and 0.5 or 1 }))
    if draft.photo then
        group:addChild(C.button("Remove photo", function()
            if draft.publishing then return end
            view.capture()
            draft.photo = nil
            feedback(self, draft, "")
            self:show("/write")
        end, { Name = "RemovePhoto", IgnoreHitTest = draft.publishing or false, Opacity = draft.publishing and 0.5 or 1 }))
    end
    return group
end

local function guest(self)
    return C.page("write", C.stack({ Name = "WriteSignIn", Spacing = 16 }, {
        C.empty("sign in to write", "Guests can read everything. Writing, replying and following need an account."),
        C.primary("Sign in", function() self.authReturn = "/write"; self:show("/auth") end, { Name = "WriteSignInButton" }),
        C.outline("Create an account", function() self.authReturn = "/write"; self:show("/auth/signup") end, { Name = "WriteSignUp" }),
    }))
end

function Compose.publish(self, view)
    local draft = view.draft
    if draft.publishing then return end
    view.capture()
    local submitted, session = values(draft), self.sessionEpoch
    draft.publishing, view.publish.IgnoreHitTest, view.publish.Opacity = true, true, 0.5
    view.publish:setTextContent("Publishing…")
    feedback(self, draft, draft.photo and "Uploading photo…" or "")
    self:task(function()
        if self.sessionEpoch ~= session then draft.publishing = false; return end
        local post, err = self.client:createPost(submitted)
        draft.publishing = false
        if self.sessionEpoch ~= session then return end
        local current = self.currentRoute == "/write" and self.composeView
        local visible = current and current.draft == draft
        if visible then
            current.capture()
            current.publish.IgnoreHitTest, current.publish.Opacity = false, 1
            current.publish:setTextContent("Publish")
        end
        if not post then
            feedback(self, draft, err.message .. (err.code == "validation" and "" or " Your draft is safe."), true)
            self:repaint()
            return
        end
        local clear = unchanged(draft, submitted)
        if clear then
            local empty = { postType = draft.postType }
            self.composeDrafts[draft.postType] = empty
            if self.composeDraft == draft then self.composeDraft = empty end
        else feedback(self, draft, "Published. Your newer edits are kept.") end
        self:invalidate()
        if visible and clear then
            self.post, self.postReturn = post, "/"
            self:show("/post")
        else self:show(self.currentRoute) end
    end)
end

function Compose.page(self)
    if not self.client:getSession() then return self:tab("Write", guest(self)) end
    local draft = self.composeDraft
    local view = { draft = draft, inputs = {} }
    self.composeView = view
    self.status = C.text(draft.notice or "", { Name = "WriteStatus", FontSize = 13,
        Foreground = draft.error and colors.error or colors.dim, WordWrap = true })
    function view.capture()
        for key, input in pairs(view.inputs) do draft[key] = input.Text end
    end
    self.leaving = view.capture

    local kinds = ui.Grid { Name = "PostKinds", Columns = "1fr 1fr", Rows = "80px 80px", Spacing = 8, Height = 168 }
    for _, kind in ipairs(KINDS) do
        local selected = draft.postType == kind.key
        local ink = selected and colors.onAccent or colors.ink
        kinds:addChild(C.area(C.stack({ Spacing = 4, JustifyContent = "Center", IgnoreHitTest = true }, {
            C.icon(kind.icon, 28, ink),
            C.text(kind.label, { FontSize = 17, Foreground = ink, TextHorizontalAlignment = "Center" }),
        }), function()
            if draft.postType == kind.key then return end
            view.capture()
            self.composeDrafts[kind.key] = self.composeDrafts[kind.key] or { postType = kind.key }
            self.composeDraft = self.composeDrafts[kind.key]
            self:show("/write")
        end, { Name = "Kind" .. kind.key, Height = 80, Background = selected and colors.accent or colors.raised,
            class = "octopus-action" }))
    end

    local form = C.stack({ Name = "WriteForm", Spacing = 14, Overflow = "hidden scroll" }, { kinds })
    for _, field in ipairs(FIELDS[draft.postType]) do
        if field[1] == "photo" then form:addChild(photoField(self, view, field))
        else
            local key, props = field[1], {}
            for name, value in pairs(field[5] or {}) do props[name] = value end
            local input
            props.Text, props.KeyUp = draft[key] or "", function() draft[key] = input.Text end
            input = C.input(field[2], field[3], props)
            view.inputs[key] = input
            form:addChild(C.stack({ Spacing = 6 }, { C.text(field[4], { FontSize = 14, Foreground = colors.muted }), input }))
        end
    end
    local tags
    tags = C.input("PostTags", "design, reef, hello", { Text = draft.tags or "", KeyUp = function() draft.tags = tags.Text end })
    view.inputs.tags = tags
    form:addChild(C.stack({ Spacing = 6 }, { C.text("tags (optional)", { FontSize = 14, Foreground = colors.muted }), tags }))
    view.publish = C.primary(draft.publishing and "Publishing…" or "Publish", function() Compose.publish(self, view) end,
        { Name = "Publish", IgnoreHitTest = draft.publishing or false, Opacity = draft.publishing and 0.5 or 1 })
    form:addChild(view.publish)
    form:addChild(self.status)
    return self:tab("Write", C.page("new " .. (draft.postType == "text" and "post" or draft.postType), form))
end

return Compose
