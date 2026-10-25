local ui = require "orca.UIKit"
local C = require "Chat/views/components"
local colors = C.colors
local Identity = {}

function Identity.auth(self)
    local busy = false
    local identifier = C.input("Identifier", "+1 415 555 0123 or you@example.com", { Text = self.loginIdentifier or "" })
    local code = C.input("VerificationCode", "6-digit verification code")
    local email = C.input("Email", "you@example.com")
    local password = C.input("Password", "Your password", { Type = "Password" })
    local displayName = C.input("DisplayName", "Your name, for a new account")
    self.status = C.text(self.remote and "Your phone or email. Your people."
        or "Offline mock · demo@example.invalid\nVerification code: 123456", {
        WordWrap = true, FontSize = 14, Foreground = colors.muted,
    })
    local function finish(session, err)
        if not session then self:notice(err and err.message or "Please try again.", true); return end
        if not self.client:getSession() then self:notice("Check your email to confirm your account."); return end
        self.loginIdentifier, self.loginCodeRequested = nil, nil
        self:resetSessionViews(); self:show()
    end
    local entry = C.stack({ Name = "IdentifierStep", Spacing = 12, Visible = not self.loginCodeRequested }, {
        C.eyebrow("PHONE NUMBER OR EMAIL"), identifier,
        C.text("Use an international number, including + and country code.", { FontSize = 13, Foreground = colors.dim, WordWrap = true }),
        C.button("Continue", function()
            if busy then return end
            busy = true; self:notice("Sending verification code…")
            local result, err = self.client:requestCode(identifier.Text)
            busy = false
            if not result then self:notice(err.message, true); return end
            self.loginIdentifier, self.loginCodeRequested = identifier.Text, true
            self:show("/auth")
        end, { Name = "RequestCode", Background = colors.accent, Foreground = colors.onAccent, Height = 52 }),
    })
    local verification = C.stack({ Name = "CodeStep", Spacing = 12, Visible = self.loginCodeRequested == true }, {
        C.eyebrow("CHECK YOUR MESSAGES"),
        C.text("Enter the code sent to " .. (self.loginIdentifier or "your phone or email"), { WordWrap = true, FontSize = 15 }), code,
        C.button("Verify & sign in", function()
            if busy then return end
            busy = true; self:notice("Verifying…")
            local session, err = self.client:verifyCode(self.loginIdentifier, code.Text)
            busy = false; finish(session, err)
        end,
            { Name = "VerifyCode", Background = colors.accent, Foreground = colors.onAccent, Height = 52 }),
        C.button("Resend code", function()
            if busy then return end
            busy = true; self:notice("Sending verification code…")
            local result, err = self.client:requestCode(self.loginIdentifier)
            busy = false
            self:notice(result and "A new code is on its way." or err.message, not result)
        end, { Name = "ResendCode", Background = colors.surface }),
        C.button("Use a different phone or email", function() self.loginCodeRequested = false; self:show("/auth") end,
            { Name = "ChangeIdentifier" }),
    })
    local passwordFields = C.stack({ Name = "PasswordLogin", Visible = false, Spacing = 12 }, {
        C.eyebrow("EMAIL ADDRESS"), email, C.eyebrow("PASSWORD"), password,
        C.button("Sign in", function()
            if busy then return end
            busy = true; self:notice("Signing in…")
            local session, err = self.client:signIn(email.Text, password.Text)
            busy = false; finish(session, err)
        end,
            { Name = "SignIn", Background = colors.accent, Foreground = colors.onAccent, Height = 52 }),
        displayName,
        C.button("Create an account", function()
            if busy then return end
            busy = true; self:notice("Creating your account…")
            local session, err = self.client:signUp(email.Text, password.Text, displayName.Text)
            busy = false; finish(session, err)
        end,
            { Name = "SignUp", Background = colors.surface }),
    })
    local body = C.stack({ Spacing = 20, Overflow = "hidden scroll" }, {
        C.text(self.loginCodeRequested and "A little code. A big hello." or "Your people. One message away.", {
            FontSize = 22, WordWrap = true }),
        self.status, entry, verification,
        C.button("Use email & password", function() passwordFields.Visible = not passwordFields.Visible end,
            { Name = "UsePassword" }), passwordFields,
    })
    local grid = ui.Grid { Background = colors.background, Foreground = colors.ink }
    grid:addChild(C.page(self.loginCodeRequested and "verify" or "sign in", body))
    return self:frame(grid)
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
    local code = C.input("LinkCode", "Verification code")
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
        C.avatar(profile.display_name or "You", 72, user.id), self.status,
        C.eyebrow("PUBLIC PROFILE"), name, username,
        C.text("People can find your exact username. Your phone and email are never shown in search results.", {
            FontSize = 13, WordWrap = true, Foreground = colors.dim }), phoneToggle, emailToggle,
        C.button("Save profile", function()
            local result, saveError = self.client:updateProfile { display_name = name.Text, username = username.Text,
                discoverable_phone = phoneDiscovery, discoverable_email = emailDiscovery }
            if self.sessionEpoch ~= epoch then return end
            notice(result and "Profile saved." or saveError.message, not result)
        end, { Name = "SaveProfile", Background = colors.accent, Foreground = colors.onAccent, Height = 50 }),
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
