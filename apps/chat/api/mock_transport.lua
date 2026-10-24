local Fixtures = require "api.mock_fixtures"
local Mock = {}
local copy = Fixtures.copy

local function response(status, body)
    return { status = status, body = copy(body) }
end

local function failure(status, code, message)
    return response(status, { code = code, message = message })
end

local function text(value)
    return type(value) == "string" and value:match("%S") ~= nil
end

local function decode(value)
    return (value:gsub("%+", " "):gsub("%%(%x%x)", function(hex)
        return string.char(tonumber(hex, 16))
    end))
end

local function parseUrl(url)
    if type(url) ~= "string" then return nil end
    local relative = url:gsub("^https?://[^/]+", "")
    local path, query = relative:match("^([^?#]+)%??([^#]*)$")
    if not path or path:sub(1, 1) ~= "/" then return nil end
    local params = {}
    for part in query:gmatch("[^&]+") do
        local key, value = part:match("^([^=]+)=?(.*)$")
        key, value = decode(key), decode(value)
        if params[key] ~= nil then return nil end
        params[key] = value
    end
    return path, params
end

local function find(rows, id)
    for _, row in ipairs(rows) do if row.id == id then return row end end
end

local function publicUser(user)
    return { id = user.id, email = user.email, phone = user.phone, email_confirmed_at = user.email_confirmed_at,
        phone_confirmed_at = user.phone_confirmed_at, user_metadata = copy(user.user_metadata or {}) }
end

local function newest(rows)
    table.sort(rows, function(a, b)
        if a.created_at ~= b.created_at then return a.created_at > b.created_at end
        return a.id > b.id
    end)
    return rows
end

function Mock.new(options)
    options = options or {}
    local clock = options.clock or os.time
    assert(type(clock) == "function", "Mock clock must be a function")
    local fixtures = options.fixtures
    if type(fixtures) == "function" then fixtures = fixtures(clock()) end
    assert(fixtures == nil or type(fixtures) == "table", "Mock fixtures must be a table or factory")
    local state = copy(fixtures or Fixtures.new(clock()))
    state.users, state.rooms = state.users or {}, state.rooms or {}
    state.members, state.messages = state.members or {}, state.messages or {}
    for _, room in ipairs(state.rooms) do state.messages[room.id] = state.messages[room.id] or {} end
    state.sessions, state.accessTokens, state.refreshTokens = {}, {}, {}
    state.sequence = { user = 0, room = 0, message = 0, token = 0 }

    local function timestamp()
        return os.date("!%Y-%m-%dT%H:%M:%SZ", clock())
    end

    local function issue(user, session)
        state.sequence.token = state.sequence.token + 1
        local tokenId = tostring(state.sequence.token)
        session = session or { id = "session-" .. tokenId, user_id = user.id }
        if session.access_token then state.accessTokens[session.access_token] = nil end
        if session.refresh_token then state.refreshTokens[session.refresh_token] = nil end
        session.access_token, session.refresh_token = "mock-access-" .. tokenId, "mock-refresh-" .. tokenId
        session.expires_at = clock() + 3600
        state.sessions[session.id] = session
        state.accessTokens[session.access_token] = session
        state.refreshTokens[session.refresh_token] = session
        return response(200, { access_token = session.access_token, refresh_token = session.refresh_token,
            expires_at = session.expires_at, expires_in = 3600, token_type = "bearer", user = publicUser(user) })
    end

    local function authenticate(headers)
        local authorization = headers.Authorization or headers.authorization
        local token = type(authorization) == "string" and authorization:match("^[Bb]earer%s+(.+)$")
        local session = token and state.accessTokens[token]
        if not session then return nil, failure(401, "invalid_jwt", "Sign in to continue.") end
        if session.expires_at <= clock() then return nil, failure(401, "token_expired", "Your session has expired.") end
        local user = find(state.users, session.user_id)
        if not user then return nil, failure(401, "invalid_jwt", "This account no longer exists.") end
        return user, session
    end

    local function isMember(roomId, userId)
        return state.members[roomId] and state.members[roomId][userId] == true
    end

    local function uniqueId(kind, rows)
        local id
        repeat
            state.sequence[kind] = state.sequence[kind] + 1
            id = string.format("mock-%s-%012d", kind, state.sequence[kind])
        until not find(rows, id)
        return id
    end

    local product = require("api.mock_features")(state, {
        clock = clock, response = response, failure = failure, find = find, issue = issue,
        authenticate = authenticate, uniqueId = uniqueId, timestamp = timestamp,
    })

    local function authRequest(path, params, body, headers)
        if path == "/auth/v1/token" and params.grant_type == "password" then
            if not text(body.email) or not text(body.password) then
                return failure(400, "validation", "Enter your email and password.")
            end
            for _, user in ipairs(state.users) do
                if type(user.email) == "string" and user.email:lower() == body.email:lower() and user.password == body.password then return issue(user) end
            end
            return failure(400, "invalid_credentials", "Invalid email or password.")
        elseif path == "/auth/v1/token" and params.grant_type == "refresh_token" then
            local session = type(body.refresh_token) == "string" and state.refreshTokens[body.refresh_token]
            local user = session and find(state.users, session.user_id)
            if not user then return failure(400, "refresh_token_not_found", "Sign in again.") end
            return issue(user, session)
        elseif path == "/auth/v1/token" then
            return failure(400, "unsupported_grant_type", "Unsupported authentication grant.")
        elseif path == "/auth/v1/signup" then
            if not text(body.email) or not body.email:match("^[^%s@]+@[^%s@]+%.[^%s@]+$")
                or type(body.password) ~= "string" or #body.password < 8 then
                return failure(400, "validation", "Enter an email and a password of at least 8 characters.")
            end
            for _, user in ipairs(state.users) do
                if type(user.email) == "string" and user.email:lower() == body.email:lower() then return failure(422, "user_exists", "This email is already registered.") end
            end
            local name = type(body.data) == "table" and body.data.display_name or ""
            if type(name) ~= "string" then return failure(400, "validation", "Display name must be text.") end
            local user = { id = uniqueId("user", state.users), email = body.email, password = body.password,
                email_confirmed_at = timestamp(), user_metadata = { display_name = name } }
            state.users[#state.users + 1] = user
            product.ensureUser(user)
            return issue(user)
        elseif path == "/auth/v1/logout" then
            local user, session = authenticate(headers)
            if not user then return session end
            state.accessTokens[session.access_token], state.refreshTokens[session.refresh_token] = nil, nil
            state.sessions[session.id] = nil
            return response(204)
        end
    end

    local function transport(request)
        if type(request) ~= "table" then return failure(400, "bad_request", "A request table is required.") end
        local path, params = parseUrl(request.url)
        if not path then return failure(400, "bad_request", "Invalid request URL or query.") end
        local method = type(request.method) == "string" and request.method:upper() or "GET"
        local body = request.body or {}
        local headers = type(request.headers) == "table" and request.headers or {}
        local handled = product.handle(method, path, params, body, headers)
        if handled then return handled end
        if type(body) ~= "table" then return failure(400, "bad_request", "Request body must be a decoded JSON object.") end
        if path == "/auth/v1/token" or path == "/auth/v1/signup" or path == "/auth/v1/logout" then
            if method ~= "POST" then return failure(405, "method_not_allowed", "Use POST for this authentication endpoint.") end
            return authRequest(path, params, body, headers)
        end
        if path ~= "/rest/v1/chat_profiles" and path ~= "/rest/v1/chat_rooms"
            and path ~= "/rest/v1/chat_messages" then
            return failure(404, "not_found", "Unknown mock endpoint.")
        end
        local user, session = authenticate(headers)
        if not user then return session end

        if path == "/rest/v1/chat_profiles" and method == "GET" then
            local rows = {}
            if params.id and not params.id:match("^neq%..+") and not params.id:match("^eq%..+") then
                return failure(400, "bad_request", "Unsupported profile ID filter.")
            end
            for _, account in ipairs(state.users) do
                local excluded = params.id and params.id:match("^neq%.(.+)$")
                local included = params.id and params.id:match("^eq%.(.+)$")
                if product.contactVisible(user.id, account.id) and account.id ~= excluded and (not included or account.id == included) then
                    local p = product.profile(account.id)
                    rows[#rows + 1] = { id = account.id, display_name = p.display_name, username = p.username }
                end
            end
            table.sort(rows, function(a, b)
                if a.display_name == b.display_name then return a.id < b.id end
                return a.display_name < b.display_name
            end)
            return response(200, rows)
        elseif path == "/rest/v1/chat_rooms" and method == "GET" then
            local rows = {}
            for _, room in ipairs(state.rooms) do
                if isMember(room.id, user.id) then rows[#rows + 1] = product.roomRow(room, user.id) end
            end
            return response(200, newest(rows))
        elseif path == "/rest/v1/chat_messages" and method == "GET" then
            local roomId = params.room_id and params.room_id:match("^eq%.([%w%-]+)$")
            if params.room_id and not roomId then return failure(400, "bad_request", "Unsupported conversation filter.") end
            local limit = 100
            if params.limit then limit = tonumber(params.limit) end
            if not limit or limit % 1 ~= 0 or limit < 1 or limit > 100 then
                return failure(400, "validation", "Message limit must be between 1 and 100.")
            end
            local rows = {}
            for id, messages in pairs(state.messages) do
                if (not roomId or roomId == id) and isMember(id, user.id) then
                    for _, message in ipairs(messages) do rows[#rows + 1] = product.messageRow(message) end
                end
            end
            newest(rows)
            for i = #rows, limit + 1, -1 do rows[i] = nil end
            return response(200, rows)
        elseif path == "/rest/v1/chat_messages" and method == "POST" then
            return failure(403, "42501", "Use the atomic post_chat_message operation to publish messages.")
        end
        return failure(405, "method_not_allowed", "Unsupported method for this mock endpoint.")
    end

    return transport, state
end

return Mock
