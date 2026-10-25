-- A yielding background refresh must never restore a logged-out account or
-- erase the replacement account's in-memory/private saved credentials.
package.path = "apps/chat/?.lua;" .. package.path
local Chat = require "api.chat"
local json = require "api.json"
local function session(token, user)
    return { access_token = token, refresh_token = "refresh-marker", expires_in = 900,
        user = { id = user or "old-user" } }
end
local function resume(co)
    local ok, value, err = coroutine.resume(co)
    assert(ok, value)
    return value, err
end

for _, status in ipairs { 200, 401, 403 } do
    local client = assert(Chat.client { mode = "appwrite", url = "https://mock.invalid",
        transport = function(request)
            if request.url:find("grant_type=refresh_token", 1, true) then
                coroutine.yield()
                return { status = status, body = status == 200 and session("late-jwt")
                    or { code = "expired", message = "Expired" } }
            end
            return { status = 204 }
        end,
    })
    assert(client:_setSession(session("old-jwt")))
    local pending = coroutine.create(function() return client:refreshSession() end)
    resume(pending)
    assert(client:signOut() and not client:getSession())
    local value, err = resume(pending)
    assert(value == nil and err.code == "session_changed" and not client:getSession(),
        "A late refresh must not restore the session after logout")
end

for _, status in ipairs { 200, 401, 403 } do
    local client = assert(Chat.client { mode = "appwrite", url = "https://mock.invalid",
        transport = function(request)
            if request.url:find("grant_type=refresh_token", 1, true) then
                coroutine.yield()
                return { status = status, body = status == 200 and session("late-jwt")
                    or { code = "expired", message = "Expired" } }
            end
            return { status = 200, body = session("new-jwt", "new-user") }
        end,
    })
    assert(client:_setSession(session("old-jwt")))
    local pending = coroutine.create(function() return client:refreshSession() end)
    resume(pending)
    assert(client:signIn("new@example.test", "password"))
    local value, err = resume(pending)
    assert(value == nil and err.code == "session_changed" and client:getSession().user.id == "new-user"
        and client:getSession().access_token == "new-jwt", "Late refresh success or failure must preserve the replacement session")
end

for _, failedPath in ipairs { "/account", "/account/jwts" } do
    local saved, clears, number, suspend = nil, 0, 0, false
    local store = {
        load = function() return saved end,
        save = function(value) saved = { Cookie = value.Cookie }; return true end,
        clear = function() saved = nil; clears = clears + 1; return true end,
    }
    local options = { endpoint = "https://fra.cloud.appwrite.io/v1", projectId = "project-one",
        functionId = "chat-api", sessionStore = store, encode = json.encode, decode = json.decode }
    options.transport = function(request)
        local path = request.url:sub(#options.endpoint + 1)
        if path == "/account/sessions/email" then
            number = number + 1
            return { status = 201, body = {}, headers = {
                ["set-cookie"] = "a_session_project-one=cookie-" .. number .. "; HttpOnly; Secure",
            } }
        end
        local cookie = assert(request.headers.Cookie)
        if suspend and cookie == "a_session_project-one=cookie-1" and path == failedPath then
            coroutine.yield()
            return { status = 401, body = { type = "user_unauthorized", message = "Old session expired" } }
        end
        if path == "/account" then return { status = 200, body = { ["$id"] = cookie, name = "User" } } end
        assert(path == "/account/jwts")
        return { status = 201, body = { jwt = "jwt-for-" .. cookie } }
    end
    local client = assert(Chat.appwrite(options))
    assert(client:signIn("old@example.test", "password"))
    suspend = true
    local pending = coroutine.create(function() return client:refreshSession() end)
    resume(pending)
    assert(client:signIn("new@example.test", "password"))
    local value, err = resume(pending)
    assert(value == nil and err.code == "session_changed")
    assert(clears == 0 and saved.Cookie == "a_session_project-one=cookie-2",
        "An old Account/JWT rejection must not delete the newly saved credential")
    assert(client:refreshSession())
    assert(client:getSession().user.id == saved.Cookie,
        "An old rejection must not clear the adapter's new private credential")
    local restarted = assert(Chat.appwrite(options))
    assert(restarted:restoreSession() and restarted:getSession().user.id == saved.Cookie,
        "The replacement credential must remain restorable after the old rejection")
end

print("PASS: Chat refresh races preserve logout and replacement sessions")
