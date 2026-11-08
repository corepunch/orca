local I18n = require "Chat/i18n"
local function t(key, params) return I18n:t(key, params) end
local ui = require "orca.UIKit"
local C = require "Chat/views/components"
local ProfileAvatar = require "Chat/views/profile_avatar"
local colors = C.colors
local Identity = {}

-- Each authentication route constructs only its own form. Drafts belong to the
-- app, so switching methods never leaves hidden, focusable inputs in the tree.
function Identity.auth(self, mode)
    mode = mode or "email-code"
    self.authDraft = self.authDraft or {}
    local draft, busy = self.authDraft, false
    local fields = {}
    local function input(name, placeholder, value, props)
        props = props or {}
        props.Text, props.Height, props.FontSize = value or "", 52, 16
        local field = C.input(name, placeholder, props)
        fields[name] = field
        return field
    end
    local function remember()
        if fields.Email then draft.email = fields.Email.Text end
        if fields.DisplayName then draft.name = fields.DisplayName.Text end
    end
    local function go(route)
        if busy then return end
        remember()
        self:show(route)
    end
    local function link(label, name, route)
        local button = C.button(label, function() go(route) end, { Name = name, FontSize = 15 })
        button.TextHorizontalAlignment = "Left"
        return button
    end
    self.status = C.text("", { Name = "AuthStatus", Visible = false, WordWrap = true, FontSize = 13, StateClasses = "chat-muted" })
    local function finish(session, err)
        if not session then self:notice(err and err.message or t("Please try again."), true); return end
        if not self.client:getSession() then self:notice(t("Check your email to confirm your account.")); return end
        self:resetSessionViews()
        self:show()
    end
    -- `secondary` draws the action as a left-aligned link beside a primary one.
    local function action(label, name, pending, operation, secondary)
        local button
        button = C.button(label, function()
            if busy then return end
            remember()
            busy = true
            button:setTextContent(pending)
            self:notice("")
            operation()
            busy = false
            button:setTextContent(label)
        end, secondary and { Name = name, Height = 52, FontSize = 16, TextHorizontalAlignment = "Left" }
            or { Name = name, class = "chat-primary", Height = 52, FontSize = 16 })
        return button
    end
    local function field(label, node)
        return C.stack({ Spacing = 8 }, { C.text(label, { FontSize = 14, class = "chat-muted" }), node })
    end
    local function requestCode(identifier, returnRoute)
        local result, err = self.client:requestCode(identifier)
        if not result then self:notice(err.message, true); return end
        self.loginIdentifier, self.loginReturnRoute = identifier, returnRoute
        self:show("/auth/code")
    end
    -- Every form keeps one primary action and one way out, after Microsoft's
    -- "Other sign-in options"; the options page lists the alternatives.
    local function others() return link(t("Other sign-in options"), "OtherOptions", "/auth/options") end
    -- Provider pages open in a private browser session; the app finishes the
    -- sign-in when the page returns. Shown only where the platform supports it.
    local function providers()
        local signin = self.signin
        if not signin or not signin.supported then return nil end
        local function provider(name, label, id)
            return C.button(label, function()
                if busy then return end
                remember()
                self:notice("")
                local ok, err = signin:start(id)
                if not ok then self:notice(err and err.message or t("Please try again."), true) end
            end, { Name = name, Height = 52, FontSize = 16, Background = "#00000000" })
        end
        return C.stack({ Name = "ProviderSignIn", Spacing = 12 }, {
            provider("ContinueApple", t("Continue with Apple"), "apple"),
            provider("ContinueGoogle", t("Continue with Google"), "google"),
        })
    end
    local title, description, form
    if mode == "options" then
        title, description = t("sign-in options"), t("Choose another way to continue.")
        form = C.stack({ Name = "OptionsPage", Spacing = 0 }, {
            C.command("UseEmailCode", "mail", t("Email me a sign-in code"), function() go("/auth") end),
            C.command("UseMagicLink", "link", t("Email me a sign-in link"), function() go("/auth/magic") end),
            C.command("UsePassword", "key-round", t("Email and password"), function() go("/auth/email") end),
            C.command("CreateAccount", "user-round-plus", t("Create an account"), function() go("/auth/signup") end),
        })
    elseif mode == "magic" then
        title, description = t("sign in"), t("We'll email you a link. Open it on this device to sign in, or to create your account.")
        local email = input("Email", "you@example.com", draft.email, { Type = "Email" })
        form = C.stack({ Name = "MagicLinkPage", Spacing = 16 }, {
            field(t("Email address"), email),
            action(t("Email me a link"), "SendMagicLink", t("Sending link…"), function()
                local result, err = self.client:requestMagicLink(email.Text)
                if not result then self:notice(err.message, true); return end
                self.loginIdentifier = result.identifier
                self:show("/auth/magic-sent")
            end), self.status, others(),
        })
        email.Submit = function() form:findChild("SendMagicLink"):send("Node.LeftButtonUp") end
    elseif mode == "magic-sent" then
        title, description = t("check your email"), t("We sent a sign-in link to %{identifier}. Open it on this device to continue.",
            { identifier = self.loginIdentifier or t("your email") })
        form = C.stack({ Name = "MagicLinkSent", Spacing = 16 }, {
            action(t("Resend link"), "ResendMagicLink", t("Sending link…"), function()
                local result, err = self.client:requestMagicLink(self.loginIdentifier)
                self:notice(result and t("A new link is on its way.") or err.message, not result)
            end, true), self.status,
            link(t("Change email"), "ChangeIdentifier", "/auth/magic"),
            others(),
        })
    elseif mode == "email" or mode == "signup" then
        local signup = mode == "signup"
        title = signup and t("create account") or t("sign in")
        description = signup and t("A name, an email, and you're in.") or t("Welcome back. Pick up the conversation.")
        local email = input("Email", "you@example.com", draft.email, { Type = "Email" })
        local password = input("Password", signup and t("At least 8 characters") or t("Your password"), nil, { Type = "Password" })
        form = C.stack({ Name = signup and "SignUpPage" or "EmailPage", Spacing = 16 })
        if signup then
            form:addChild(field(t("Your name"), input("DisplayName", t("How people will know you"), draft.name)))
        end
        form:addChild(field(t("Email address"), email))
        form:addChild(field(t("Password"), password))
        form:addChild(action(signup and t("Create account") or t("Sign in"), signup and "SignUp" or "SignIn",
            signup and t("Creating account…") or t("Signing in…"), function()
                local session, err
                if signup then session, err = self.client:signUp(email.Text, password.Text, fields.DisplayName.Text)
                else session, err = self.client:signIn(email.Text, password.Text) end
                finish(session, err)
            end))
        form:addChild(self.status)
        email.Submit = function() password:setFocus() end
        password.Submit = function() form:findChild(signup and "SignUp" or "SignIn"):send("Node.LeftButtonUp") end
        if signup then fields.DisplayName.Submit = function() email:setFocus() end end
        form:addChild(others())
    elseif mode == "code" then
        title, description = t("check your email"), t("Enter the code sent to %{identifier}.", { identifier = self.loginIdentifier or t("your email") })
        local code = input("VerificationCode", t("6-digit code"), nil, { Type = "Number" })
        form = C.stack({ Name = "CodeStep", Spacing = 16 }, {
            field(t("Verification code"), code),
            action(t("Verify & sign in"), "VerifyCode", t("Verifying…"), function()
                local session, err = self.client:verifyCode(self.loginIdentifier, code.Text)
                finish(session, err)
            end), self.status,
            action(t("Resend code"), "ResendCode", t("Sending code…"), function()
                local result, err = self.client:requestCode(self.loginIdentifier)
                self:notice(result and t("A new code is on its way.") or err.message, not result)
            end, true),
            link(t("Change email"), "ChangeIdentifier", self.loginReturnRoute or "/auth"),
        })
        code.Submit = function() form:findChild("VerifyCode"):send("Node.LeftButtonUp") end
    else
        -- Email codes are the default way in: no password to remember and no SMS cost.
        title, description = t("sign in"), t("We'll email you a code. New here? The same code creates your account.")
        local email = input("Email", "you@example.com", draft.email, { Type = "Email" })
        form = C.stack({ Name = "EmailCodePage", Spacing = 16 }, {
            field(t("Email address"), email),
            action(t("Email me a code"), "RequestCode", t("Sending code…"), function()
                requestCode(email.Text, "/auth")
            end), self.status,
        })
        local buttons = providers()
        if buttons then form:addChild(buttons) end
        form:addChild(others())
        email.Submit = function() form:findChild("RequestCode"):send("Node.LeftButtonUp") end
    end
    local brand = ui.Grid { Name = "AuthBrand", Columns = "36px 1fr", Height = 36, Spacing = 12, MarginBottom = 12 }
    local logo = C.appIcon(36)
    logo.Name = "AuthLogo"
    brand:addChild(logo)
    brand:addChild(C.text("ORCA / CHAT", { Name = "AuthWordmark", FontSize = 14, FontWeight = "Bold", LetterSpacing = 1.2,
        Height = 36, TextVerticalAlignment = "Center" }))
    local body = C.stack({ Name = "AuthPage", Padding = "24 28", Spacing = 20,
        Overflow = "hidden scroll", class = "chat-background" }, {
        brand,
        C.stack({ Spacing = 12 }, {
            C.text(title, { Name = "AuthTitle", FontSize = 38, WordWrap = true }),
            C.text(description, { FontSize = 15, class = "chat-muted", WordWrap = true }),
        }), form,
    })
    body:addChild(C.button(t("Privacy policy"), function() remember(); self:show("/privacy") end,
        { Name = "AuthPrivacyPolicy", FontSize = 14 }))
    body:addChild(C.button(t("Community rules"), function() remember(); self:show("/rules") end,
        { Name = "AuthCommunityRules", FontSize = 14 }))
    if not self.remote then
        body:addChild(C.text(t("Offline demo · code 123456\nEmail: demo@example.invalid · password123"), {
            Name = "OfflineAuthHint", FontSize = 11, class = "chat-dim", WordWrap = true }))
    end
    return self:frame(body)
end

-- Settings follow Telegram for Windows Phone's settings list (its open
-- SettingsView.xaml): your photo and name, then one column of rows, each an
-- icon with the setting's name and its current value under it. Rows open
-- their editor in place; check boxes save as they change.
local function profileBody(self, profile)
    local user = self.client:getSession().user
    profile = profile or { display_name = (user.user_metadata or {}).display_name or t("You") }
    local name = C.input("ProfileName", t("Your name"), { Text = profile.display_name or t("You") })
    local username = C.input("Username", t("Choose a public username"), { Text = profile.username or "" })
    local identifier = C.input("LinkIdentifier", t("New email address"), { Type = "Email" })
    -- The number is a private contact detail, saved as typed without an SMS check.
    local phone = C.input("PhoneNumber", "+49 151 1234 5678", { Type = "Phone", Text = profile.phone or "" })
    local code = C.input("LinkCode", t("Verification code"), { Type = "Number" })
    local function statusText(id)
        return C.text("", { Name = id, Visible = false, WordWrap = true, FontSize = 14, StateClasses = "chat-muted" })
    end
    -- Photo results show under the photo commands, everything else under the rows.
    self.status = statusText("ProfileStatus")
    self.profileStatus = self.status
    local epoch, settingsStatus = self.sessionEpoch, statusText("SettingsStatus")
    local function notice(message, isError)
        settingsStatus:setTextContent(t(message or ""))
        settingsStatus.StateClasses = isError and "chat-error" or "chat-muted"
        settingsStatus.Visible = (message or "") ~= ""
    end
    local photo = C.avatar(profile.display_name or t("You"), 96, user.id, self, user.id)
    local title = C.text("", { Name = "ProfileTitle", FontSize = 28, TextOverflow = "Ellipsis" })
    local handle = C.text("", { Name = "ProfileHandle", FontSize = 15, class = "chat-muted", TextOverflow = "Ellipsis" })
    local link = C.stack({ Name = "LinkForm", Visible = false, Spacing = 10 })
    -- One editor is open at a time, directly under the row that opened it.
    local editors = {}
    local function open(editor)
        for _, other in ipairs(editors) do other.Visible = other == editor and not editor.Visible end
        notice("")
    end
    local nameRow = C.setting("NameSetting", "pencil", t("Name"), function() open(editors[1]) end)
    local usernameRow = C.setting("UsernameSetting", "at-sign", t("Username"), function() open(editors[2]) end)
    local phoneRow = C.setting("PhoneSetting", "smartphone", t("Mobile"), function() open(editors[3]) end)
    local emailRow = C.setting("EmailSetting", "mail", t("Email"), function() open(link) end)
    -- Telegram's header line: the phone and @username people know you by.
    local function describe(p)
        local phone, email = p.phone, p.email or user.email
        phone, email = (phone or "") ~= "" and phone or nil, (email or "") ~= "" and email or nil
        local at = (p.username or "") ~= "" and "@" .. p.username or nil
        title:setTextContent(p.display_name or t("You"))
        local parts = {}
        for _, part in ipairs { phone or email, at } do parts[#parts + 1] = part end
        handle:setTextContent(table.concat(parts, " · "))
        nameRow:findChild("NameSettingValue", true):setTextContent(p.display_name or t("You"))
        usernameRow:findChild("UsernameSettingValue", true):setTextContent(at or t("Add"))
        phoneRow:findChild("PhoneSettingValue", true):setTextContent(phone or t("Add"))
        emailRow:findChild("EmailSettingValue", true):setTextContent(email or t("Add"))
    end
    describe(profile)
    -- Saves one change; a refused change reports why and is undone.
    local function save(values, undo)
        local result, saveError = self.client:updateProfile(values)
        if self.sessionEpoch ~= epoch then return end
        if not result then
            if undo then undo() end
            notice(saveError.message, true)
            return
        end
        describe(result)
        if values.phone then phone.Text = result.phone or "" end
        return result
    end
    -- `hint` puts a caption under the field; a saved change closes the editor.
    local function editor(id, field, values, hint)
        local bar = ui.Grid { Name = id .. "Bar", Columns = "1fr auto", Spacing = 6 }
        local box = C.stack({ Name = id, Spacing = 6, MarginBottom = 8, Visible = false }, { bar })
        bar:addChild(field)
        bar:addChild(C.button(t("save"), function()
            if save(values()) then box.Visible = false end
        end, { Name = id .. "Save", Width = 88, class = "chat-primary" }))
        if hint then box:addChild(C.text(hint, { FontSize = 13, class = "chat-muted", WordWrap = true })) end
        return box
    end
    editors[1] = editor("NameEditor", name, function() return { display_name = name.Text } end)
    editors[2] = editor("UsernameEditor", username, function() return { username = username.Text } end)
    editors[3] = editor("PhoneEditor", phone, function() return { phone = phone.Text } end,
        t("Include + and your country code. Only you can see your number."))
    editors[4] = link
    local function toggle(id, label, key, checked)
        local box, restoring
        box = C.checkbox(id, label, checked, function(value)
            if not box or restoring then return end
            save({ [key] = value }, function()
                restoring = true
                box.IsChecked = not value
                restoring = false
            end)
        end, true)
        return box
    end
    -- Phone discovery needs a verified number; only accounts that already have it keep the switch.
    local phoneToggle = profile.discoverable_phone == true
        and toggle("DiscoverablePhone", t("Find me by phone"), "discoverable_phone", true) or nil
    local emailToggle = toggle("DiscoverableEmail", t("Find me by email"), "discoverable_email", profile.discoverable_email == true)
    local previewToggle = toggle("NotificationPreviews", t("Message previews"), "notification_previews",
        profile.notification_previews ~= false)
    local verification
    verification = C.stack({ Name = "LinkVerification", Visible = false, Spacing = 10 }, {
        code,
        C.button(t("Verify & link"), function()
            local result, linkError = self.client:verifyIdentityLink(self.pendingIdentity, code.Text)
            if self.sessionEpoch ~= epoch then return end
            if not result then notice(linkError.message, true); return end
            local updated = self.client:getProfile()
            if self.sessionEpoch ~= epoch then return end
            if updated then describe(updated) end
            identifier.Text, code.Text = "", ""
            link.Visible, verification.Visible = false, false
            notice(t("Verified and linked to this account."))
        end, { Name = "VerifyLink", Background = colors.surface }),
    })
    for _, child in ipairs {
        identifier,
        C.button(t("Send verification code"), function()
            local result, linkError = self.client:requestIdentityLink(identifier.Text)
            if self.sessionEpoch ~= epoch then return end
            if not result then notice(linkError.message, true); return end
            self.pendingIdentity, verification.Visible = identifier.Text, true
            notice(self.remote and t("Enter the code sent to your new login detail.") or t("Offline verification code: 123456"))
        end, { Name = "RequestLink", Background = colors.surface }), verification,
    } do link:addChild(child) end
    local header = ui.Grid { Name = "ProfileHeader", Columns = "96px 1fr", Spacing = 16, AlignItems = "Center" }
    header:addChild(photo)
    header:addChild(C.stack({ Spacing = 2 }, { title, handle }))
    local function rows(children) return C.stack({ Spacing = 0 }, children) end
    return C.stack({ Spacing = 14, Overflow = "hidden scroll" }, {
        header,
        ProfileAvatar.controls(self, photo, self.status),
        self.status,
        C.eyebrow(t("account")),
        rows { nameRow, editors[1], usernameRow, editors[2], phoneRow, editors[3], emailRow },
        link,
        settingsStatus,
        C.eyebrow(t("privacy")),
        rows(phoneToggle and { phoneToggle, emailToggle } or { emailToggle }),
        C.button(t("blocked people"), function() self:show("/blocked", "Forward") end,
            { Name = "BlockedPeople", FontSize = 18, TextHorizontalAlignment = "Left" }),
        C.eyebrow(t("notifications")),
        rows { previewToggle },
        C.eyebrow(t("about")),
        -- Twemoji's CC-BY license requires visible credit; see THIRD_PARTY_NOTICES.md.
        C.text(t("Emoji graphics by Twemoji, licensed under CC-BY 4.0. Noto fonts by Google, SIL Open Font License. Font rendering by FreeType (FreeType License). PNG decoding by libpng (libpng License) and JPEG decoding by libjpeg (IJG License). LZ4 compression by Yann Collet, BSD 2-Clause License. Lua by Lua.org, PUC-Rio, MIT License. Networking by curl (Daniel Stenberg, curl license), XML parsing by libxml2 (MIT License) and TLS by Mbed TLS (Apache License 2.0)."),
            { Name = "Credits", FontSize = 14, class = "chat-muted", WordWrap = true }),
        C.button(t("Privacy policy"), function() self:show("/privacy", "Forward") end,
            { Name = "PrivacyPolicy", FontSize = 18 }),
        C.button(t("Community rules"), function() self:show("/rules", "Forward") end,
            { Name = "CommunityRules", FontSize = 18 }),
        C.command("SignOut", "log-out", t("sign out"), function() self:signOut() end, "chat-error"),
        C.button(t("delete account"), function() self:show("/delete-account", "Forward") end,
            { Name = "DeleteAccount", FontSize = 18, class = "chat-error" }),
    })
end

function Identity.profile(self)
    if not self.client or not self.client:getSession() then return self:home() end
    if self.shell and self.shell:has("Profile") then self.status = self.profileStatus; return self.shell:show("Profile") end
    return self:loadingPage(t("settings"), "Profile", function() return self.client:getProfile() end,
        function(profile) return profileBody(self, profile) end)
end

return Identity
