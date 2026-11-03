-- Browser and link sign-in: Apple and Google pages in a private browser session,
-- and emailed magic links the system opens the app with. Native events carry
-- only a URL; the client validates it and Appwrite checks its secret.
local I18n = require "Chat/i18n"
local SignIn = {}
SignIn.__index = SignIn

local function t(key, params) return I18n:t(key, params) end

function SignIn.new(app, options)
    options = options or {}
    local native = options.native
    if not native then
        local ok, module = pcall(require, "orca.signin")
        if not ok then return nil end
        native = module
    end
    local self = setmetatable({ app = app, native = native, supported = native.supported() == true }, SignIn)
    if options.manual then return self end
    -- One quiet, repeating poll: links can arrive at launch or while the app runs.
    local core = require "orca.core"
    self.owner = core.Timer { Name = "ChatSignIn" }
    self.owner.Tick = function() self:poll() end
    return self
end

function SignIn:activate()
    if self.owner and not self.timer then self.timer = self.owner:setTimer(250) end
end

function SignIn:suspend()
    if self.owner and self.timer then self.owner:cancelTimer(self.timer) end
    self.timer = nil
end

-- Opens Apple's or Google's page; the result arrives through poll().
function SignIn:start(provider)
    if not self.supported then return nil, { message = t("This device can't open the sign-in page.") } end
    local page, err = self.app.client:authorizeProvider(provider)
    if not page then return nil, err end
    local ok, message = self.native.open(page.url, page.scheme)
    if not ok then return nil, { message = message or t("The sign-in page could not be opened.") } end
    self.pending = provider
    return true
end

function SignIn:complete(url)
    local app = self.app
    if app.client:getSession() then app:notice(t("You're already signed in.")); return end
    if self.busy then return end
    self.busy = true
    local session, err = app.client:completeSignInLink(url)
    self.busy = false
    if not session then app:notice(t(err and err.message or "Please try again."), true); return end
    app.authDraft = nil
    app:resetSessionViews()
    app:show()
end

function SignIn:poll()
    -- A bounded drain keeps an event burst from monopolizing the UI.
    for _ = 1, 8 do
        local event = self.native.poll()
        if not event then return end
        if event.type == "callback" and self.pending then
            self.pending = nil
            self:complete(event.url)
        elseif event.type == "url" and type(event.url) == "string" then
            self:complete(event.url)
        elseif event.type == "error" then
            self.pending = nil
            if event.code ~= "cancelled" then self.app:notice(t(event.message or "Please try again."), true) end
        end
    end
end

return SignIn
