local ui = require "orca.UIKit"
local C = require "Octopus/views/components"
local People = require "Octopus/views/people"
local colors = C.colors
local Identity = {}

-- Sign in and sign up use the website's email/password accounts. Each route
-- builds only its own form; drafts belong to the app so switching keeps them.
function Identity.auth(self, mode)
    local signup = mode == "signup"
    self.authDraft = self.authDraft or {}
    local draft, busy, fields = self.authDraft, false, {}
    local function input(name, placeholder, value, props)
        props = props or {}
        props.Text, props.Height, props.FontSize = value or "", 52, 16
        fields[name] = C.input(name, placeholder, props)
        return fields[name]
    end
    local function remember()
        if fields.Email then draft.email = fields.Email.Text end
        if fields.DisplayName then draft.name = fields.DisplayName.Text end
        if fields.AccountName then draft.handle = fields.AccountName.Text end
    end
    local function field(label, node)
        return C.stack({ Spacing = 8 }, { C.text(label, { FontSize = 14, Foreground = colors.muted }), node })
    end
    local returnRoute = self.authReturn
    if returnRoute == nil or returnRoute:match("^/auth") then returnRoute = "/" end
    local function leave(route)
        if busy then return end
        remember()
        self:show(route)
    end
    self.status = C.text(self.authReason or "", { Name = "AuthStatus", WordWrap = true, FontSize = 13,
        Foreground = colors.muted })
    self.statusBar, self.authReason = nil, nil

    local form = C.stack({ Name = signup and "SignUpPage" or "SignInPage", Spacing = 16 })
    if signup then
        form:addChild(field(self.i18n:t("Account name"), input("AccountName", self.i18n:t("Your unique account name"), draft.handle)))
        form:addChild(C.text(self.i18n:t("1–36 letters, digits, dots, underscores or hyphens. This is your permanent account ID."), {
            FontSize = 12, Foreground = colors.muted, WordWrap = true }))
        form:addChild(field(self.i18n:t("Display name"), input("DisplayName", self.i18n:t("How people will see you"), draft.name)))
    end
    local email = input("Email", "you@example.com", draft.email)
    local password = input("Password", signup and self.i18n:t("At least 8 characters") or self.i18n:t("Your password"), nil, { Type = "Password" })
    form:addChild(field(self.i18n:t("Email address"), email))
    form:addChild(field(self.i18n:t("Password"), password))
    local confirm
    if signup then
        confirm = input("ConfirmPassword", self.i18n:t("Type it again"), nil, { Type = "Password" })
        form:addChild(field(self.i18n:t("Confirm password"), confirm))
    end
    local label = signup and self.i18n:t("Create account") or self.i18n:t("Sign in")
    local submit
    submit = C.primary(label, function()
        if busy then return end
        remember()
        busy = true
        submit:setTextContent(signup and self.i18n:t("Creating account…") or self.i18n:t("Signing in…"))
        self:notice("")
        local epoch = self.sessionEpoch
        self:task(function()
            local session, err
            if signup then session, err = self.client:signUp(fields.DisplayName.Text, email.Text, password.Text, confirm.Text, fields.AccountName.Text)
            else session, err = self.client:signIn(email.Text, password.Text) end
            busy = false
            if self.sessionEpoch ~= epoch then return end
            if not session then
                submit:setTextContent(label)
                self:notice(err and err.message or self.i18n:t("Please try again."), true)
                self:repaint()
                return
            end
            self.authDraft, self.authReturn = nil, nil
            self:resetSessionViews()
            -- A new account (no picture yet) gets a random character first.
            if People.assignRandom(self) then
                self.pictureReturn = returnRoute
                return self:show("/auth/picture")
            end
            self:show(returnRoute)
        end)
    end, { Name = signup and "SignUp" or "SignIn", Height = 52, FontSize = 16 })
    form:addChild(submit)
    form:addChild(self.status)
    if signup then
        fields.AccountName.Submit = function() fields.DisplayName:setFocus() end
        fields.DisplayName.Submit = function() email:setFocus() end
        email.Submit = function() password:setFocus() end
        password.Submit = function() confirm:setFocus() end
        confirm.Submit = function() submit:send("Node.LeftButtonUp") end
        form:addChild(C.button(self.i18n:t("Already have an account? Sign in"), function() leave("/auth") end, { Name = "GoToSignIn" }))
    else
        email.Submit = function() password:setFocus() end
        password.Submit = function() submit:send("Node.LeftButtonUp") end
        form:addChild(C.button(self.i18n:t("New here? Create an account"), function() leave("/auth/signup") end, { Name = "GoToSignUp" }))
    end
    form:addChild(C.button(self.i18n:t("Continue without signing in"), function() leave(returnRoute) end, {
        Name = "ContinueAsGuest", Foreground = colors.muted }))

    local brand = ui.Grid { Name = "AuthBrand", Columns = "40px 1fr", Height = 40, Spacing = 12, MarginBottom = 8 }
    local logo = ui.Grid { Name = "AuthLogo", Width = 40, Height = 40, Background = colors.accent }
    logo:addChild(C.logo(28))
    brand:addChild(logo)
    brand:addChild(C.text("OCTOPUS", { Name = "AuthWordmark", FontSize = 14, FontWeight = "Bold", LetterSpacing = 1.2,
        Height = 40, TextVerticalAlignment = "Center" }))
    local body = C.stack({ Name = "AuthPage", Padding = "24 28", Spacing = 20, Overflow = "hidden scroll",
        Background = colors.background }, {
        brand,
        C.stack({ Spacing = 12 }, {
            C.text(signup and self.i18n:t("join octopus") or self.i18n:t("sign in"), { Name = "AuthTitle", FontSize = 40, WordWrap = true }),
            C.text(signup and self.i18n:t("Write posts, follow people and join the conversation. Your account works on the website too.")
                or self.i18n:t("Welcome back. Your timeline is waiting."), { FontSize = 15, Foreground = colors.muted, WordWrap = true }),
        }), form,
    })
    if not self.remote then
        body:addChild(C.text(self.i18n:t("Offline demo · demo@example.invalid · password123"), {
            Name = "OfflineAuthHint", FontSize = 11, Foreground = colors.dim, WordWrap = true }))
    end
    return self:frame(body)
end

return Identity
