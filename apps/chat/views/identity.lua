local ui = require "orca.UIKit"
local C = require "Chat/views/components"
local colors = C.colors
local Identity = {}

-- Each authentication route constructs only its own form. Drafts belong to the
-- app, so switching methods never leaves hidden, focusable inputs in the tree.
function Identity.auth(self, mode)
    mode = mode or "phone"
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
        if fields.Identifier then draft.phone = fields.Identifier.Text end
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
    self.status = C.text("", { Name = "AuthStatus", WordWrap = true, FontSize = 13, Foreground = colors.muted })
    local function finish(session, err)
        if not session then self:notice(err and err.message or "Please try again.", true); return end
        if not self.client:getSession() then self:notice("Check your email to confirm your account."); return end
        self:resetSessionViews()
        self:show()
    end
    local function action(label, name, pending, operation)
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
        end, { Name = name, Background = colors.accent, Foreground = colors.onAccent, Height = 52, FontSize = 16 })
        return button
    end
    local function field(label, node)
        return C.stack({ Spacing = 8 }, { C.text(label, { FontSize = 14, Foreground = colors.muted }), node })
    end
    local function requestCode(identifier, returnRoute)
        local result, err = self.client:requestCode(identifier)
        if not result then self:notice(err.message, true); return end
        self.loginIdentifier, self.loginReturnRoute = identifier, returnRoute
        self:show("/auth/code")
    end
    local title, description, form
    if mode == "email" or mode == "signup" then
        local signup = mode == "signup"
        title = signup and "create account" or "sign in"
        description = signup and "A name, an email, and you're in." or "Welcome back. Pick up the conversation."
        local email = input("Email", "you@example.com", draft.email, { Type = "Email" })
        local password = input("Password", signup and "At least 8 characters" or "Your password", nil, { Type = "Password" })
        form = C.stack({ Name = signup and "SignUpPage" or "EmailPage", Spacing = 16 })
        if signup then
            form:addChild(field("Your name", input("DisplayName", "How people will know you", draft.name)))
        end
        form:addChild(field("Email address", email))
        form:addChild(field("Password", password))
        form:addChild(action(signup and "Create account" or "Sign in", signup and "SignUp" or "SignIn",
            signup and "Creating account…" or "Signing in…", function()
                local session, err
                if signup then session, err = self.client:signUp(email.Text, password.Text, fields.DisplayName.Text)
                else session, err = self.client:signIn(email.Text, password.Text) end
                finish(session, err)
            end))
        form:addChild(self.status)
        email.Submit = function() password:setFocus() end
        password.Submit = function() form:findChild(signup and "SignUp" or "SignIn"):send("Node.LeftButtonUp") end
        if signup then
            fields.DisplayName.Submit = function() email:setFocus() end
            form:addChild(link("Already have an account? Sign in", "BackToSignIn", "/auth/email"))
        else
            form:addChild(action("Email me a sign-in code", "SendEmailCode", "Sending code…", function()
                requestCode(email.Text, "/auth/email")
            end))
            -- The password form keeps one visually primary action.
            local sendCode = form:findChild("SendEmailCode")
            sendCode.Background, sendCode.Foreground = "#00000000", colors.accentText
            sendCode.TextHorizontalAlignment = "Left"
            form:addChild(link("Create an account", "CreateAccount", "/auth/signup"))
        end
        form:addChild(link("Use phone number", "UsePhone", "/auth"))
    elseif mode == "code" then
        title, description = "check your messages", "Enter the code sent to " .. (self.loginIdentifier or "your phone") .. "."
        local code = input("VerificationCode", "6-digit code", nil, { Type = "Number" })
        form = C.stack({ Name = "CodeStep", Spacing = 16 }, {
            field("Verification code", code),
            action("Verify & sign in", "VerifyCode", "Verifying…", function()
                local session, err = self.client:verifyCode(self.loginIdentifier, code.Text)
                finish(session, err)
            end), self.status,
            action("Resend code", "ResendCode", "Sending code…", function()
                local result, err = self.client:requestCode(self.loginIdentifier)
                self:notice(result and "A new code is on its way." or err.message, not result)
            end),
            link("Change phone number or email", "ChangeIdentifier", self.loginReturnRoute or "/auth"),
        })
        local resend = form:findChild("ResendCode")
        resend.Background, resend.Foreground = "#00000000", colors.accentText
        resend.TextHorizontalAlignment = "Left"
        code.Submit = function() form:findChild("VerifyCode"):send("Node.LeftButtonUp") end
    else
        title, description = "your phone", "Your people are one message away. Sign in or get started with your number."
        local phone = input("Identifier", "+49 151 1234 5678", draft.phone, { Type = "Phone" })
        form = C.stack({ Name = "PhonePage", Spacing = 16 }, {
            field("Phone number", phone),
            C.text("Include + and your country code. We'll send you a verification code.", {
                FontSize = 13, Foreground = colors.muted, WordWrap = true }),
            action("Continue", "RequestCode", "Sending code…", function()
                if not phone.Text:match("^%s*%+") then
                    self:notice("Enter your phone number with + and country code.", true)
                    phone:setFocus()
                    return
                end
                requestCode(phone.Text, "/auth")
            end), self.status,
            link("Use email & password", "UsePassword", "/auth/email"),
            link("Create an account", "CreateAccount", "/auth/signup"),
        })
        phone.Submit = function() form:findChild("RequestCode"):send("Node.LeftButtonUp") end
    end
    local brand = ui.Grid { Name = "AuthBrand", Columns = "36px 1fr", Height = 36, Spacing = 12, MarginBottom = 12 }
    local logo = ui.Grid { Width = 36, Height = 36, Background = colors.accent }
    logo:addChild(C.icon("message-circle", 22, colors.onAccent))
    logo.Name = "AuthLogo"
    brand:addChild(logo)
    brand:addChild(C.text("ORCA / CHAT", { Name = "AuthWordmark", FontSize = 14, FontWeight = "Bold", LetterSpacing = 1.2,
        Height = 36, TextVerticalAlignment = "Center" }))
    local body = C.stack({ Name = "AuthPage", Padding = "24 28", Spacing = 20,
        Overflow = "hidden scroll", Background = colors.background }, {
        brand,
        C.stack({ Spacing = 12 }, {
            C.text(title, { Name = "AuthTitle", FontSize = 38, WordWrap = true }),
            C.text(description, { FontSize = 15, Foreground = colors.muted, WordWrap = true }),
        }), form,
    })
    if not self.remote then
        body:addChild(C.text("Offline demo · +4915110000001 · code 123456\nEmail: demo@example.invalid · password123", {
            Name = "OfflineAuthHint", FontSize = 11, Foreground = colors.dim, WordWrap = true }))
    end
    return self:frame(body)
end

function Identity.profile(self)
    if not self.client or not self.client:getSession() then return self:home() end
    if self.shell and self.shell:has("Profile") then self.status = self.profileStatus; return self.shell:show("Profile") end
    local navigation, generation = self.navigationEpoch, self.sessionEpoch
    local profile, err = self.client:getProfile()
    if self.navigationEpoch ~= navigation or self.sessionEpoch ~= generation then return self.screen end
    local user = self.client:getSession().user
    profile = profile or { display_name = (user.user_metadata or {}).display_name or "You" }
    local name = C.input("ProfileName", "Your name", { Text = profile.display_name or "You" })
    local username = C.input("Username", "Choose a public username", { Text = profile.username or "" })
    local identifier = C.input("LinkIdentifier", "Phone number or email to verify")
    local code = C.input("LinkCode", "Verification code", { Type = "Number" })
    local verified = C.text("", { Name = "VerifiedIdentities", WordWrap = true, FontSize = 14, Foreground = colors.muted })
    local function describe(p)
        local phone, email = p.phone or user.phone, p.email or user.email
        verified:setTextContent("Verified phone: " .. ((phone and phone ~= "") and phone or "Not added")
            .. "\nVerified email: " .. ((email and email ~= "") and email or "Not added"))
    end
    describe(profile)
    self.status = C.text(err and err.message or "Your login details stay private. Choose how people find you.", {
        Name = "ProfileStatus", WordWrap = true, FontSize = 14, Foreground = err and colors.error or colors.muted,
    })
    self.profileStatus = self.status
    local epoch, status = self.sessionEpoch, self.status
    local function notice(message, isError)
        status:setTextContent(message or "")
        status.Foreground = isError and colors.error or colors.muted
    end
    local phoneDiscovery, emailDiscovery = profile.discoverable_phone == true, profile.discoverable_email == true
    local phoneToggle, emailToggle
    phoneToggle = C.button("Find me by phone: " .. (phoneDiscovery and "On" or "Off"), function()
        phoneDiscovery = not phoneDiscovery; phoneToggle:setTextContent("Find me by phone: " .. (phoneDiscovery and "On" or "Off"))
    end, { Name = "DiscoverablePhone", Background = colors.surface })
    emailToggle = C.button("Find me by email: " .. (emailDiscovery and "On" or "Off"), function()
        emailDiscovery = not emailDiscovery; emailToggle:setTextContent("Find me by email: " .. (emailDiscovery and "On" or "Off"))
    end, { Name = "DiscoverableEmail", Background = colors.surface })
    local verification = C.stack({ Name = "LinkVerification", Visible = false, Spacing = 10 }, {
        code,
        C.button("Verify & link", function()
            local result, linkError = self.client:verifyIdentityLink(self.pendingIdentity, code.Text)
            if self.sessionEpoch ~= epoch then return end
            if not result then notice(linkError.message, true); return end
            local updated = self.client:getProfile()
            if self.sessionEpoch ~= epoch then return end
            if updated then describe(updated) end
            identifier.Text, code.Text = "", ""
            notice("Verified and linked to this account.")
        end, { Name = "VerifyLink", Background = colors.surface }),
    })
    local body = C.stack({ Spacing = 14, Overflow = "hidden scroll" }, {
        C.avatar(profile.display_name or "You", 72, user.id, self, user.id), self.status,
        C.eyebrow("PUBLIC PROFILE"), name, username,
        C.text("People can find your exact username. Your phone and email are never shown in search results.", {
            FontSize = 13, WordWrap = true, Foreground = colors.dim }), phoneToggle, emailToggle,
        C.button("Save profile", function()
            local result, saveError = self.client:updateProfile { display_name = name.Text, username = username.Text,
                discoverable_phone = phoneDiscovery, discoverable_email = emailDiscovery }
            if self.sessionEpoch ~= epoch then return end
            notice(result and "Profile saved." or saveError.message, not result)
        end, { Name = "SaveProfile", Background = colors.accent, Foreground = colors.onAccent }),
        C.eyebrow("VERIFIED LOGIN DETAILS"), verified, identifier,
        C.button("Send verification code", function()
            local result, linkError = self.client:requestIdentityLink(identifier.Text)
            if self.sessionEpoch ~= epoch then return end
            if not result then notice(linkError.message, true); return end
            self.pendingIdentity, verification.Visible = identifier.Text, true
            notice(self.remote and "Enter the code sent to your new login detail." or "Offline verification code: 123456")
        end, { Name = "RequestLink", Background = colors.surface }), verification,
        C.button("Sign out", function() self:signOut() end, { Name = "SignOut", Foreground = colors.error }),
    })
    return self:frame(C.page("profile", body), "Profile")
end

return Identity
