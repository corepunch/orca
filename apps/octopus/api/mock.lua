-- An in-memory Appwrite for offline use and tests. It serves the Account,
-- Databases and Storage paths the Octopus client calls, with the website's
-- collection permissions, unique indexes and query semantics.
local fixtures = require "Octopus/api/fixtures"
local M = {}

local COLLECTIONS = {
    posts = {}, comments = {}, profiles = { unique = { "userId" } },
    follows = { usersOnly = true, unique = { "followerId", "followingId" } },
    likes = { unique = { "userId", "targetType", "targetId" } },
    reposts = { unique = { "userId", "postId" } },
}

local function copy(value)
    if type(value) ~= "table" then return value end
    local result = {}
    for key, item in pairs(value) do result[key] = copy(item) end
    return result
end

local function reply(status, body, headers) return { status = status, body = body, headers = headers or {} } end
local function problem(status, kind, message) return reply(status, { code = status, type = kind, message = message }) end
local function unauthorized()
    return unauthorized()
end

local function contains(value, wanted)
    if type(value) == "table" then
        for _, item in ipairs(value) do if item == wanted then return true end end
        return false
    end
    return value == wanted
end

local function matches(doc, query)
    local value, values = doc[query.attribute], query.values or {}
    if query.method == "equal" then
        for _, wanted in ipairs(values) do if contains(value, wanted) then return true end end
        return false
    elseif query.method == "lessThan" then
        return value ~= nil and tostring(value) < tostring(values[1])
    elseif query.method == "notEqual" then
        return value ~= values[1]
    elseif query.method == "search" then
        local haystack = tostring(value or ""):lower()
        for word in tostring(values[1] or ""):lower():gmatch("%w+") do
            if haystack:find(word, 1, true) then return true end
        end
        return false
    end
    return true
end

-- Small PNG encoder (stored deflate blocks) for the fixture photo.
local function crc32(data)
    local crc = 0xffffffff
    for i = 1, #data do
        crc = crc ~ data:byte(i)
        for _ = 1, 8 do crc = (crc >> 1) ~ (0xedb88320 & -(crc & 1)) end
    end
    return crc ~ 0xffffffff
end

local function png(width, height, pixel)
    local rows = {}
    for y = 0, height - 1 do
        local row = { "\0" }
        for x = 0, width - 1 do row[#row + 1] = string.char(pixel(x, y)) end
        rows[#rows + 1] = table.concat(row)
    end
    local raw, blocks, a, b = table.concat(rows), {}, 1, 0
    for i = 1, #raw do a = (a + raw:byte(i)) % 65521; b = (b + a) % 65521 end
    for i = 1, #raw, 65535 do
        local block = raw:sub(i, i + 65534)
        blocks[#blocks + 1] = string.pack("<BI2I2", i + 65535 > #raw and 1 or 0, #block, ~#block & 0xffff) .. block
    end
    local function chunk(kind, data) return string.pack(">I4", #data) .. kind .. data .. string.pack(">I4", crc32(kind .. data)) end
    return "\137PNG\r\n\26\n" .. chunk("IHDR", string.pack(">I4I4BBBBB", width, height, 8, 2, 0, 0, 0))
        .. chunk("IDAT", "\120\1" .. table.concat(blocks) .. string.pack(">I4", (b << 16) | a)) .. chunk("IEND", "")
end

local function reefPhoto()
    local width, height = 156, 104
    return png(width, height, function(x, y)
        local depth = y / height
        local wave = math.sin(x / 9 + math.sin(y / 13) * 2) * 0.5 + 0.5
        local light = (y < height * 0.55 and wave > 0.82) and 40 or 0
        return math.floor(14 + 10 * depth) + light, math.floor(143 - 80 * depth) + light, math.floor(134 - 30 * depth) + light
    end)
end

function M.new(options)
    options = options or {}
    local clock = options.clock or os.time
    local projectId = options.projectId or "octopus-mock"
    local state = { users = {}, sessions = {}, collections = {}, files = {}, sequence = 0 }
    for name in pairs(COLLECTIONS) do state.collections[name] = {} end

    local function stamp(offset)
        state.sequence = state.sequence + 1
        return os.date("!%Y-%m-%dT%H:%M:%S", clock() - (offset or 0)) .. string.format(".%03d+00:00", state.sequence % 1000)
    end
    local function newId(prefix)
        state.sequence = state.sequence + 1
        return string.format("%s%07x", prefix or "m", state.sequence)
    end
    local function insert(name, data, id, owner, age)
        local doc = copy(data)
        doc["$id"], doc["$createdAt"], doc["$owner"] = id or newId(name:sub(1, 1)), stamp(age), owner
        doc["$updatedAt"] = doc["$createdAt"]
        table.insert(state.collections[name], doc)
        return doc
    end

    for _, user in ipairs(fixtures.users) do
        state.users[user.id] = { id = user.id, name = user.name, email = user.email, password = user.password }
        -- Demo portraits ship with the app; ORCA's io.open reads project files.
        local photo = user.avatar and io.open("Octopus/assets/avatars/" .. user.avatar .. ".jpg", "rb")
        local avatarId = photo and "f-avatar-" .. user.avatar
        if photo then
            state.files[avatarId] = { bytes = photo:read("*a"), owner = user.id }
            photo:close()
        end
        insert("profiles", { userId = user.id, username = user.name, bio = user.bio or "", avatarId = avatarId },
            user.id, user.id, 864000)
    end
    state.files["f-reef"] = { bytes = reefPhoto(), owner = "u-sam" }
    -- A fixed order keeps creation stamps, and so tie-breaks, reproducible.
    for _, name in ipairs { "posts", "comments", "likes", "reposts", "follows" } do
        local docs = fixtures.collections[name] or {}
        for _, data in ipairs(docs) do
            local values = copy(data)
            local id, owner, age = values.id, values.owner, values.age
            values.id, values.owner, values.age = nil, nil, nil
            insert(name, values, id, owner or values.authorId or values.userId or values.followerId, age)
        end
    end
    -- Fixture rows are listed newest first; keep storage in creation order.
    for _, docs in pairs(state.collections) do
        table.sort(docs, function(a, b) return a["$createdAt"] < b["$createdAt"] end)
    end

    local function currentUser(headers)
        local cookie, prefix = headers and headers.Cookie, "a_session_" .. projectId .. "="
        local token = cookie and cookie:sub(1, #prefix) == prefix and cookie:sub(#prefix + 1)
        return token and state.sessions[token] and state.users[state.sessions[token]]
    end

    local function public(user)
        return { ["$id"] = user.id, name = user.name, email = user.email }
    end

    local function account(method, path, body, user)
        if method == "POST" and path == "/account" then
            if type(body.email) ~= "string" or not body.email:match("^[^@]+@[^@]+$") then
                return problem(400, "general_argument_invalid", "Invalid `email` param: Value must be a valid email address")
            end
            if type(body.password) ~= "string" or #body.password < 8 then
                return problem(400, "general_argument_invalid", "Invalid `password` param: Password must be at least 8 characters")
            end
            for _, existing in pairs(state.users) do
                if existing.email:lower() == body.email:lower() then
                    return problem(409, "user_already_exists",
                        "A user with the same id, email, or phone already exists in this project.")
                end
            end
            local id = newId("u")
            state.users[id] = { id = id, name = body.name or "", email = body.email, password = body.password }
            return reply(201, public(state.users[id]))
        elseif method == "POST" and path == "/account/sessions/email" then
            for _, candidate in pairs(state.users) do
                if candidate.email:lower() == tostring(body.email):lower() and candidate.password == body.password then
                    local token = "mock" .. newId("s")
                    state.sessions[token] = candidate.id
                    return reply(201, { ["$id"] = token, userId = candidate.id, secret = "" },
                        { ["Set-Cookie"] = { "a_session_" .. projectId .. "=" .. token .. "; path=/; httponly" } })
                end
            end
            return problem(401, "user_invalid_credentials", "Invalid credentials. Please check the email and password.")
        elseif method == "GET" and path == "/account" then
            if not user then return problem(401, "general_unauthorized_scope", "User (role: guests) missing scope (account)") end
            return reply(200, public(user))
        elseif method == "DELETE" and path == "/account/sessions/current" then
            if not user then return problem(401, "general_unauthorized_scope", "User (role: guests) missing scope (account)") end
            for token, id in pairs(state.sessions) do if id == user.id then state.sessions[token] = nil end end
            return reply(204)
        end
    end

    local function list(name, docs, query)
        local filtered, order, limit, offset, cursor = {}, nil, 25, 0, nil
        for _, doc in ipairs(docs) do
            local keep = true
            for _, q in ipairs(query or {}) do if not matches(doc, q) then keep = false; break end end
            if keep then filtered[#filtered + 1] = doc end
        end
        for _, q in ipairs(query or {}) do
            if q.method == "orderDesc" or q.method == "orderAsc" then order = q
            elseif q.method == "limit" then limit = q.values[1]
            elseif q.method == "offset" then offset = q.values[1]
            elseif q.method == "cursorAfter" then cursor = q.values[1] end
        end
        if order then
            local attribute, descending = order.attribute, order.method == "orderDesc"
            table.sort(filtered, function(a, b)
                local x, y = tostring(a[attribute] or ""), tostring(b[attribute] or "")
                if x == y then return a["$id"] < b["$id"] end
                if descending then return x > y end
                return x < y
            end)
        end
        local start = offset + 1
        if cursor then
            start = nil
            for i, doc in ipairs(filtered) do if doc["$id"] == cursor then start = i + 1 end end
            if not start then
                return problem(400, "general_cursor_not_found", "Document with the requested ID '" .. cursor .. "' could not be found.")
            end
        end
        local page = {}
        for i = start, math.min(#filtered, start + limit - 1) do page[#page + 1] = copy(filtered[i]) end
        return reply(200, { total = #filtered, documents = page })
    end

    local function database(method, name, id, request, user)
        local config, docs = COLLECTIONS[name], state.collections[name]
        if not config then return problem(404, "collection_not_found", "Collection with the requested ID could not be found.") end
        if config.usersOnly and not user then
            return unauthorized()
        end
        local index, doc
        for i, candidate in ipairs(docs) do if candidate["$id"] == id then index, doc = i, candidate end end
        if method == "GET" and not id then return list(name, docs, request.query) end
        if method == "GET" then
            if not doc then return problem(404, "document_not_found", "Document with the requested ID could not be found.") end
            return reply(200, copy(doc))
        end
        if not user then return unauthorized() end
        local body = type(request.body) == "table" and request.body or {}
        if method == "POST" and not id then
            local data = body.data
            if type(data) ~= "table" then return problem(400, "document_invalid_structure", "Invalid document structure.") end
            local docId = body.documentId ~= "unique()" and body.documentId or nil
            for _, candidate in ipairs(docs) do
                local same = docId ~= nil and candidate["$id"] == docId
                if config.unique and not same then
                    same = true
                    for _, key in ipairs(config.unique) do if candidate[key] ~= data[key] then same = false end end
                end
                if same then return problem(409, "document_already_exists", "Document with the requested ID already exists.") end
            end
            return reply(201, copy(insert(name, data, docId, user.id)))
        end
        if not doc then return problem(404, "document_not_found", "Document with the requested ID could not be found.") end
        if doc["$owner"] ~= user.id then
            return unauthorized()
        end
        if method == "PATCH" then
            for key, value in pairs(type(body.data) == "table" and body.data or {}) do doc[key] = copy(value) end
            doc["$updatedAt"] = stamp()
            return reply(200, copy(doc))
        elseif method == "DELETE" then
            table.remove(docs, index)
            return reply(204)
        end
        return problem(405, "general_not_implemented", "Not supported by the mock.")
    end

    local function storage(method, rest, request, user)
        local fileId, action = rest:match("^/([^/?]+)/?([^?]*)")
        if method == "POST" and rest == "" then
            if not user then return unauthorized() end
            local body = type(request.body) == "string" and request.body or ""
            local bytes = body:match('name="file".-\r\n\r\n(.*)\r\n%-%-[^\r\n]*%-%-\r\n$')
            if not bytes or #bytes == 0 then return problem(400, "storage_file_empty", "Empty file passed to the endpoint.") end
            local id = newId("f")
            state.files[id] = { bytes = bytes, owner = user.id }
            return reply(201, { ["$id"] = id, sizeOriginal = #bytes })
        end
        local file = fileId and state.files[fileId]
        if not file then return problem(404, "storage_file_not_found", "The requested file could not be found.") end
        if method == "GET" and (action == "view" or action == "preview") then return reply(200, file.bytes) end
        if method == "DELETE" and action == "" then
            if not user or file.owner ~= user.id then
                return unauthorized()
            end
            state.files[fileId] = nil
            return reply(204)
        end
        return problem(405, "general_not_implemented", "Not supported by the mock.")
    end

    local transport = function(request)
        local user = currentUser(request.headers)
        local path = request.path or ""
        local response = account(request.method, path, type(request.body) == "table" and request.body or {}, user)
        if response then return response end
        local name, id = path:match("^/databases/[^/]+/collections/([^/]+)/documents/?([^/?]*)$")
        if name then return database(request.method, name, id ~= "" and id or nil, request, user) end
        local rest = path:match("^/storage/buckets/[^/]+/files(.*)$")
        if rest then return storage(request.method, rest, request, user) end
        return problem(404, "general_route_not_found", "The requested route was not found.")
    end
    return transport, state
end

return M
