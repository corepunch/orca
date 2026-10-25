local ui = require "orca.UIKit"
local C = require "Octopus/views/components"
local colors = C.colors
local Compose = {}

local KINDS = {
    { key = "text", icon = "file-text", label = "text" },
    { key = "quote", icon = "quote", label = "quote" },
    { key = "link", icon = "link", label = "link" },
    { key = "photo", icon = "image", label = "photo" },
}

-- Fields per post type: { draft key, input name, placeholder, label, extra props }.
local FIELDS = {
    text = {
        { "title", "PostTitle", "Give it a title", "title" },
        { "content", "PostContent", "What's on your mind? Markdown works on the website.", "post", { Multiline = true, Height = 180 } },
    },
    quote = {
        { "content", "QuoteText", "The words worth sharing", "quote", { Multiline = true, Height = 110 } },
        { "quoteSource", "QuoteSource", "Who said it? A person, book or site", "source" },
        { "userText", "QuoteThoughts", "Optional: what it means to you", "your thoughts", { Multiline = true, Height = 80 } },
    },
    link = {
        { "title", "LinkTitle", "What is it?", "title" },
        { "linkUrl", "LinkUrl", "https://", "address" },
        { "content", "LinkDescription", "Why it's worth a read", "description", { Multiline = true, Height = 96 } },
    },
    photo = {
        { "content", "PhotoCaption", "Say something about it", "caption", { Multiline = true, Height = 80 } },
    },
}

local function guest(self)
    return C.page("write", C.stack({ Name = "WriteSignIn", Spacing = 16 }, {
        C.empty("sign in to write", "Guests can read everything. Writing, replying and following need an account."),
        C.primary("Sign in", function() self.authReturn = "/write"; self:show("/auth") end, { Name = "WriteSignInButton" }),
        C.outline("Create an account", function() self.authReturn = "/write"; self:show("/auth/signup") end, { Name = "WriteSignUp" }),
    }))
end

function Compose.publish(self, view)
    if view.busy then return end
    view.capture()
    view.busy = true
    view.publish:setTextContent("Publishing…")
    self:notice(view.draft.photo and "Uploading photo…" or "")
    local draft, session = view.draft, self.sessionEpoch
    self:task(function()
        local post, err = self.client:createPost(draft)
        view.busy = false
        if self.sessionEpoch ~= session then return end
        if self.composeView == view then view.publish:setTextContent("Publish") end
        if not post then self:notice(err.message .. (err.code == "validation" and "" or " Your draft is safe."), true); return end
        self.composeDraft = { postType = draft.postType }
        self:invalidate()
        self.post, self.postReturn = post, "/"
        self:show("/post")
    end)
end

function Compose.page(self)
    if not self.client:getSession() then return self:tab("Write", guest(self)) end
    local draft = self.composeDraft
    local view = { draft = draft, inputs = {} }
    self.composeView = view
    self.status = C.text("", { Name = "WriteStatus", FontSize = 13, Foreground = colors.dim, WordWrap = true })
    function view.capture()
        for key, input in pairs(view.inputs) do draft[key] = input.Text end
    end
    self.leaving = view.capture

    local kinds = ui.Grid { Name = "PostKinds", Columns = "1fr 1fr 1fr 1fr", Spacing = 8, Height = 64 }
    for _, kind in ipairs(KINDS) do
        local selected = draft.postType == kind.key
        local ink = selected and colors.onAccent or colors.ink
        kinds:addChild(C.area(C.stack({ Spacing = 4, JustifyContent = "Center", IgnoreHitTest = true }, {
            C.icon(kind.icon, 20, ink),
            C.text(kind.label, { FontSize = 13, Foreground = ink, TextHorizontalAlignment = "Center" }),
        }), function()
            if draft.postType == kind.key then return end
            view.capture()
            draft.postType = kind.key
            self:show("/write")
        end, { Name = "Kind" .. kind.key, Height = 64, Background = selected and colors.accent or colors.raised,
            class = "octopus-action" }))
    end

    local form = C.stack({ Name = "WriteForm", Spacing = 14, Overflow = "hidden scroll" }, { kinds })
    for _, field in ipairs(FIELDS[draft.postType]) do
        local key, props = field[1], {}
        for name, value in pairs(field[5] or {}) do props[name] = value end
        local input
        props.Text, props.KeyUp = draft[key] or "", function() draft[key] = input.Text end
        input = C.input(field[2], field[3], props)
        view.inputs[key] = input
        form:addChild(C.stack({ Spacing = 6 }, { C.text(field[4], { FontSize = 14, Foreground = colors.muted }), input }))
    end
    if draft.postType == "photo" then
        local preview = ui.Grid { Name = "PhotoPreview", Height = draft.photo and 200 or 0, Visible = draft.photo ~= nil,
            Background = colors.raised }
        local path = draft.photo and self.media:store(draft.photo.jpeg, "draft-" .. tostring(draft.photo))
        if path then preview:addChild(ui.ImageView { Src = path, Stretch = "Uniform", IgnoreHitTest = true }) end
        form:addChild(preview)
        form:addChild(C.outline(draft.photo and "Choose another photo" or "Choose a photo", function()
            view.capture()
            self:notice("Opening photos…")
            local photo, err = self.media:pick()
            if not photo then
                local cancelled = type(err) == "table" and err.code == "cancelled"
                self:notice(cancelled and "" or (type(err) == "table" and err.message or tostring(err)), not cancelled)
                return
            end
            draft.photo = photo
            self:show("/write")
        end, { Name = "ChoosePhoto" }))
    end
    local tags
    tags = C.input("PostTags", "design, reef, hello", { Text = draft.tags or "", KeyUp = function() draft.tags = tags.Text end })
    view.inputs.tags = tags
    form:addChild(C.stack({ Spacing = 6 }, { C.text("tags", { FontSize = 14, Foreground = colors.muted }), tags }))
    view.publish = C.primary("Publish", function() Compose.publish(self, view) end, { Name = "Publish" })
    form:addChild(view.publish)
    form:addChild(self.status)
    return self:tab("Write", C.page("new " .. (draft.postType == "text" and "post" or draft.postType), form))
end

return Compose
