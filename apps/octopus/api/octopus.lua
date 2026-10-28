-- The Octopus client speaks Appwrite's Account, Databases and Storage REST APIs,
-- the same services and collections as the Octopus website. A transport sends
-- { method, path, query, body, headers, binary } and returns { status, body, headers };
-- network and in-process mock transports share this client unchanged.
local Octopus = {}
local Client = {}
Client.__index = Client

local POST_TYPES = { text = true, photo = true, quote = true, link = true }
local PAGE = 100
local MAX_IMAGE_EDGE = 65535

local function imageDimensions(width, height)
    for _, value in ipairs { width, height } do
        if type(value) ~= "number" or value < 1 or value > MAX_IMAGE_EDGE or value % 1 ~= 0 then return nil end
    end
    if width == nil or height == nil then return nil end
    return width, height
end

local function failure(code, message, status)
    return nil, { code = code, message = message, status = status }
end

local function text(value)
    return type(value) == "string" and value:match("%S") ~= nil
end

local function trim(value)
    return type(value) == "string" and value:match("^%s*(.-)%s*$") or ""
end

local function validId(value)
    return type(value) == "string" and #value <= 36 and value:match("^[%w][%w%._%-]*$") ~= nil
end

local function canonicalHandle(value)
    value = trim(value):lower()
    if #value >= 1 and #value <= 36 and value:match("^[a-z0-9][a-z0-9._-]*$") then return value end
end

local function handleValidation()
    return failure("validation", "Choose an account name of 1–36 letters, digits, dots, underscores or hyphens, starting with a letter or digit.")
end

local function escape(value)
    return (tostring(value):gsub("([^%w%-_%.~])", function(c) return string.format("%%%02X", c:byte()) end))
end

-- Appwrite query objects. The HTTP transport serialises them as JSON queries[].
local Q = {}
function Q.equal(attribute, values)
    return { method = "equal", attribute = attribute, values = type(values) == "table" and values or { values } }
end
function Q.notEqual(attribute, value) return { method = "notEqual", attribute = attribute, values = { value } } end
function Q.contains(attribute, value) return { method = "contains", attribute = attribute, values = { value } } end
function Q.search(attribute, value) return { method = "search", attribute = attribute, values = { value } } end
function Q.orderDesc(attribute) return { method = "orderDesc", attribute = attribute } end
function Q.orderAsc(attribute) return { method = "orderAsc", attribute = attribute } end
function Q.limit(count) return { method = "limit", values = { count } } end
function Q.select(fields) return { method = "select", values = fields } end
function Q.cursorAfter(id) return { method = "cursorAfter", values = { id } } end
function Q.lessThan(attribute, value) return { method = "lessThan", attribute = attribute, values = { value } } end
Octopus.Query = Q

-- Tags follow the website: comma or space separated, lowercase, without '#'.
function Octopus.parseTags(value)
    local tags, seen = {}, {}
    for word in tostring(value or ""):gmatch("[^,%s]+") do
        local tag = word:gsub("^#+", ""):lower():sub(1, 64)
        if tag ~= "" and not seen[tag] and #tags < 10 then tags[#tags + 1], seen[tag] = tag, true end
    end
    return tags
end

local function post(doc)
    local tags = {}
    for _, tag in ipairs(type(doc.tags) == "table" and doc.tags or {}) do tags[#tags + 1] = tag end
    local avatarId
    if doc.sourceCreatedAt then avatarId = validId(doc.avatarId) and doc.avatarId or false end
    local width, height = imageDimensions(doc.imageWidth, doc.imageHeight)
    return {
        id = doc["$id"], createdAt = doc.sourceCreatedAt or doc["$createdAt"], title = doc.title or "", content = doc.content or "",
        authorId = doc.authorId, authorName = doc.authorName or "Someone", tags = tags,
        postType = POST_TYPES[doc.postType] and doc.postType or "text", imageId = validId(doc.imageId) and doc.imageId or nil,
        imageWidth = width, imageHeight = height,
        avatarId = avatarId,
        linkUrl = doc.linkUrl, quoteSource = doc.quoteSource, userText = doc.userText,
        summary = doc.sourceCreatedAt ~= nil,
        stats = doc.statsUpdatedAt and { likes = doc.likeCount, comments = doc.commentCount,
            reposts = doc.repostCount, pending = true } or nil,
    }
end

local function comment(doc)
    return { id = doc["$id"], createdAt = doc["$createdAt"], postId = doc.postId, authorId = doc.authorId,
        authorName = doc.authorName or "Someone", body = doc.body or "", parentId = doc.parentId or "" }
end

local function profile(doc, userId)
    return { id = doc and doc["$id"] or userId, userId = doc and doc.userId or userId,
        username = doc and doc.username or "Unknown", bio = doc and doc.bio or "", exists = doc ~= nil,
        avatarId = doc and validId(doc.avatarId) and doc.avatarId or nil }
end

-- Appwrite returns its session as a cookie named after the project.
local function sessionCookie(headers, projectId)
    local name, cookie, legacy = "a_session_" .. projectId:lower(), nil, nil
    local function inspect(value)
        if type(value) == "table" then for _, line in ipairs(value) do inspect(line) end
        elseif type(value) == "string" then
            for line in value:gmatch("[^\r\n]+") do
                local key, content = line:match("^%s*([^=; ]+)=([^;\r\n]+)")
                if content and not content:find("[%z\1-\32\127]") then
                    if key:lower() == name then cookie = key .. "=" .. content
                    elseif key:lower() == name .. "_legacy" then legacy = key .. "=" .. content end
                end
            end
        end
    end
    for key, value in pairs(headers or {}) do
        if type(key) == "string" and key:lower() == "set-cookie" then inspect(value) end
    end
    return cookie or legacy
end

function Client:collection(name)
    return "/databases/" .. self.databaseId .. "/collections/" .. name .. "/documents"
end

function Client:_request(method, path, options)
    options = options or {}
    local headers = {}
    for key, value in pairs(options.headers or {}) do headers[key] = value end
    if self.session then headers.Cookie = self.session.cookie end
    local ok, response = pcall(self.transport, {
        method = method, path = path, query = options.query, body = options.body, headers = headers,
        binary = options.binary, params = options.params,
    })
    if not ok or type(response) ~= "table" or type(response.status) ~= "number" then
        return failure("network", "Could not reach Octopus. Check your connection and try again.")
    end
    if response.status < 200 or response.status >= 300 then
        local data = type(response.body) == "table" and response.body or {}
        if response.status == 401 and self.session and path:match("^/account") then self.session = nil end
        return failure(data.type or data.code or "http_error",
            data.message or "Octopus could not complete the request.", response.status)
    end
    if options.raw then return response end
    if response.body == nil then return true end
    return response.body
end

function Client:_list(name, query, includeTotal)
    local data, err = self:_request("GET", self:collection(name), { query = query,
        params = { total = includeTotal == true } })
    if not data then return nil, err end
    if type(data) ~= "table" or type(data.documents) ~= "table" then
        return failure("invalid_response", "Octopus returned an invalid list.")
    end
    return data
end

-- Walk filtered pages without silently truncating totals on popular posts.
function Client:_listAll(name, filters, pages)
    local documents, cursor = {}, nil
    local page = 0
    while true do
        page = page + 1
        local query = {}
        for i, filter in ipairs(filters) do query[i] = filter end
        query[#query + 1] = Q.limit(PAGE)
        if cursor then query[#query + 1] = Q.cursorAfter(cursor) end
        local data, err = self:_list(name, query)
        if not data then return nil, err end
        for _, doc in ipairs(data.documents) do documents[#documents + 1] = doc end
        if #data.documents < PAGE then break end
        if pages and page >= pages then break end
        cursor = data.documents[#data.documents]["$id"]
    end
    return documents
end

function Client:_create(name, data)
    return self:_request("POST", self:collection(name), { body = { documentId = "unique()", data = data } })
end

function Client:_delete(name, id)
    return self:_request("DELETE", self:collection(name) .. "/" .. escape(id))
end

function Client:_requireSession()
    if not self.session then return failure("unauthenticated", "Sign in to continue.") end
    return self.session
end

-- At most three independent requests per group; injected unit clients remain
-- synchronous. Each scheduled job always settles, including thrown failures.
function Client:_parallel(jobs)
    local results, remaining = {}, #jobs
    local function run(index, job)
        local ok, value, err = pcall(job)
        if ok then results[index] = { value, err }
        else results[index] = { nil, { code = "network", message = "Could not load Octopus data. Try refresh." } } end
        remaining = remaining - 1
    end
    for index, job in ipairs(jobs) do
        if self.schedule and coroutine.isyieldable() then self.schedule(function() run(index, job) end)
        else run(index, job) end
    end
    while remaining > 0 do coroutine.yield() end
    return results
end

-- Account ------------------------------------------------------------------

function Client:getSession()
    if not self.session then return nil end
    local user = self.session.user
    return { user = { id = user.id, name = user.name, email = user.email } }
end

function Client:currentUserId()
    return self.session and self.session.user.id
end

function Client:signIn(email, password)
    email = trim(email)
    if not text(email) or not text(password) then return failure("validation", "Enter your email and password.") end
    self.session = nil
    local response, err = self:_request("POST", "/account/sessions/email",
        { body = { email = email, password = password }, raw = true })
    if not response then return nil, err end
    local cookie = sessionCookie(response.headers, self.projectId)
    if not cookie then return failure("invalid_response", "Octopus did not return a session.") end
    local account
    account, err = self:_request("GET", "/account", { headers = { Cookie = cookie } })
    if not account then return nil, err end
    if type(account) ~= "table" or not validId(account["$id"]) then
        return failure("invalid_response", "Octopus returned an invalid account.")
    end
    self.session = { cookie = cookie, user = { id = account["$id"], name = account.name ~= "" and account.name
        or email:match("^[^@]+"), email = account.email, accountName = account.name } }
    local public = self:ensureProfile()
    if public and public.exists and text(public.username) then self.session.user.name = public.username end
    return self:getSession()
end

function Client:signUp(name, email, password, confirm, handle)
    name, email = trim(name), trim(email)
    if not text(name) or #name > 128 then return failure("validation", "Enter a display name of up to 128 characters.") end
    if not email:match("^[^%s@]+@[^%s@]+%.[^%s@]+$") then return failure("validation", "Enter a valid email address.") end
    if type(password) ~= "string" or #password < 8 then
        return failure("validation", "Password must be at least 8 characters.")
    end
    if confirm ~= nil and confirm ~= password then return failure("validation", "Passwords do not match.") end
    handle = canonicalHandle(handle)
    if not handle then return handleValidation() end
    local created, err
    created, err = self:_request("POST", "/account",
        { body = { userId = handle, email = email, password = password, name = name } })
    if not created then return nil, err end
    local session
    session, err = self:signIn(email, password)
    if session then
        local claimed
        claimed, err = self:claimHandle(handle)
        if claimed then return session end
    end
    -- Account creation and protected handle claims live in separate services.
    -- An existing or concurrent claim may win; retain the new account
    -- for email/password recovery instead of claiming signup was atomic.
    self:signOut()
    return nil, { code = err and err.status == 409 and "handle_taken" or "signup_incomplete",
        status = err and err.status, accountCreated = true,
        message = err and err.status == 409
            and "Your account was created, but this account name was claimed before setup finished. Sign in with your email and use shared account settings or support to choose another name."
            or "Your account was created, but account-name setup could not finish. Sign in with your email and retry account-name setup." }
end

-- Protected profile operations use the existing authenticated shared backend.
-- JWTs remain inside the session and never enter public DTOs or disk snapshots.
function Client:_profileRpc(name, values)
    local session, err = self:_requireSession()
    if not session then return nil, err end
    if not session.jwt or self.clock() >= (session.jwtExpires or 0) then
        local token
        token, err = self:_request("POST", "/account/jwts", { body = {} })
        if not token then return nil, err end
        if type(token) ~= "table" or not text(token.jwt) then
            return failure("invalid_response", "Octopus did not return an account token.")
        end
        session.jwt, session.jwtExpires = token.jwt, self.clock() + 840
    end
    local json = require "Octopus/api/json"
    local envelope = { method = "POST", path = "/rest/v1/rpc/" .. name, body = values or {} }
    local execution
    execution, err = self:_request("POST", "/functions/" .. escape(self.functionId) .. "/executions", {
        headers = { ["X-Appwrite-JWT"] = session.jwt }, body = { body = json.encode(envelope), async = false },
    })
    if not execution then return nil, err end
    if type(execution) ~= "table" or execution.status ~= "completed" or type(execution.responseStatusCode) ~= "number"
        or type(execution.responseBody) ~= "string" then
        return failure("invalid_response", "The shared account service did not complete the request.")
    end
    local ok, result = pcall(json.decode, execution.responseBody)
    if not ok or type(result) ~= "table" or type(result.status) ~= "number" or result.status % 1 ~= 0
        or result.status < 100 or result.status > 599 or result.encoding ~= nil
        or type(result.body) ~= "table" then
        return failure("invalid_response", "The shared account service returned an invalid response.")
    end
    if result.status < 200 or result.status >= 300 then
        if result.status == 401 then session.jwt, session.jwtExpires = nil, nil end
        return failure(result.body.code or result.body.type or "http_error",
            result.body.message or "The shared account service could not complete the request.", result.status)
    end
    if execution.responseStatusCode < 200 or execution.responseStatusCode >= 300 then
        return failure("invalid_response", "The shared account service failed.")
    end
    if result.body.id ~= session.user.id then
        return failure("invalid_response", "The shared account service returned a different account.")
    end
    if result.body.username ~= nil and canonicalHandle(result.body.username) ~= result.body.username then
        return failure("invalid_response", "The shared account service returned an invalid account name.")
    end
    local handle = type(result.body.username) == "string" and result.body.username or nil
    session.user.handle = handle
    return { id = session.user.id, userId = session.user.id, username = result.body.display_name or session.user.name,
        handle = handle, avatarId = result.body.avatar_id }
end

function Client:getAccountProfile()
    return self:_profileRpc("get_chat_profile")
end

-- Finish an explicit readable-ID signup. General alias changes also pass through
-- the shared backend, which reserves native Account IDs and enforces uniqueness.
function Client:claimHandle(handle)
    local session, err = self:_requireSession()
    if not session then return nil, err end
    handle = canonicalHandle(handle)
    if not handle then return handleValidation() end
    return self:_profileRpc("update_chat_profile", { values = { username = handle } })
end

function Client:signOut()
    if not self.session then return true end
    local result, err = self:_request("DELETE", "/account/sessions/current")
    -- Forget the session locally even when the server cannot be reached.
    self.session = nil
    return result, err
end

-- The website creates each author's profile document with the user's ID.
function Client:ensureProfile()
    local session = self.session
    if not session then return failure("unauthenticated", "Sign in to continue.") end
    local doc, err = self:_request("GET", self:collection("profiles") .. "/" .. escape(session.user.id))
    if doc then return profile(doc) end
    if err.status ~= 404 then return nil, err end
    doc, err = self:_request("POST", self:collection("profiles"), { body = { documentId = session.user.id,
        data = { userId = session.user.id, username = session.user.name, bio = "" } } })
    if not doc then return nil, err end
    return profile(doc)
end

-- Posts --------------------------------------------------------------------

-- Enable only after the materialized collection has been provisioned/backfilled.
function Client:_postList(query)
    if not self.feedCollectionId then return self:_list("posts", query) end
    local selected = {}
    for i, q in ipairs(query) do
        if q.attribute == "$createdAt" then
            selected[i] = { method = q.method, attribute = "sourceCreatedAt", values = q.values }
        else selected[i] = q end
    end
    local data, err = self:_list(self.feedCollectionId, selected)
    if not data then return nil, err end
    for _, doc in ipairs(data.documents) do
        self.avatarIds[doc.authorId] = validId(doc.avatarId) and doc.avatarId or false
    end
    return data
end

function Client:readFeedCache(name)
    if not self.cache or self.session or name ~= "discover" then return nil end
    local saved = self.cache:readJSON("discover", 86400)
    if type(saved) ~= "table" or type(saved.items) ~= "table" or #saved.items > 20
        or (saved.cursor ~= nil and not validId(saved.cursor)) then return nil end
    for _, item in ipairs(saved.items) do
        if type(item) ~= "table" or type(item.post) ~= "table" or not validId(item.post.id)
            or not validId(item.post.authorId) or type(item.post.tags) ~= "table"
            or type(item.post.createdAt) ~= "string" or type(item.post.content) ~= "string"
            or type(item.post.title) ~= "string" or type(item.post.authorName) ~= "string"
            or not POST_TYPES[item.post.postType] then return nil end
        local p = item.post
        if #p.tags > 10 or (item.at ~= nil and type(item.at) ~= "string") then return nil end
        for _, tag in ipairs(p.tags) do if type(tag) ~= "string" or #tag > 64 then return nil end end
        if p.imageId ~= nil and not validId(p.imageId) then return nil end
        if (p.imageWidth ~= nil or p.imageHeight ~= nil) and not imageDimensions(p.imageWidth, p.imageHeight) then return nil end
        if p.avatarId ~= nil and p.avatarId ~= false and not validId(p.avatarId) then return nil end
        for _, field in ipairs { "linkUrl", "quoteSource", "userText" } do
            if p[field] ~= nil and type(p[field]) ~= "string" then return nil end
        end
        if p.stats ~= nil then
            if type(p.stats) ~= "table" then return nil end
            for _, field in ipairs { "likes", "reposts", "comments" } do
                local n = p.stats[field]
                if type(n) ~= "number" or n < 0 or n % 1 ~= 0 or n == math.huge then return nil end
            end
        end
        if item.repostedBy ~= nil and (type(item.repostedBy) ~= "table" or not validId(item.repostedBy.id)
            or type(item.repostedBy.name) ~= "string") then return nil end
    end
    for _, item in ipairs(saved.items) do
        if item.post.avatarId ~= nil then self.avatarIds[item.post.authorId] = item.post.avatarId end
    end
    return saved.items, saved.cursor
end

function Client:writeFeedCache(name, items, cursor)
    if self.cache and not self.session and name == "discover" and #items <= 20 then
        self.cache:writeJSON("discover", { items = items, cursor = cursor })
    end
end

-- options: limit, cursor (load posts after this ID), authorIds, tag, search.
function Client:listPosts(options)
    options = options or {}
    local limit = tonumber(options.limit) or 20
    if limit % 1 ~= 0 or limit < 1 or limit > 100 then return failure("validation", "Choose between 1 and 100 posts.") end
    local query = {}
    if options.authorIds then
        if #options.authorIds == 0 then return {} end
        query[#query + 1] = Q.equal("authorId", options.authorIds)
    end
    if options.tag then query[#query + 1] = Q.contains("tags", (tostring(options.tag):gsub("^#+", ""):lower())) end
    if options.search then
        if not text(options.search) then return {} end
        query[#query + 1] = Q.search("title", trim(options.search))
    end
    if options.exclude then query[#query + 1] = Q.notEqual("$id", options.exclude) end
    query[#query + 1] = Q.orderDesc("$createdAt")
    query[#query + 1] = Q.orderDesc("$id")
    query[#query + 1] = Q.limit(limit)
    if options.cursor then query[#query + 1] = Q.cursorAfter(options.cursor) end
    local data, err = self:_postList(query)
    if not data then return nil, err end
    local posts = {}
    for _, doc in ipairs(data.documents) do posts[#posts + 1] = post(doc) end
    return posts
end

function Client:getPost(id)
    if not validId(id) then return failure("validation", "Choose a post.") end
    local doc, err = self:_request("GET", self:collection("posts") .. "/" .. escape(id))
    if not doc then
        if err.status == 404 then err.message = "This post was deleted or never existed." end
        return nil, err
    end
    return post(doc)
end

function Client:listFollowingIds(userId)
    userId = userId or self:currentUserId()
    if not validId(userId) then return failure("unauthenticated", "Sign in to see who you follow.") end
    local docs, err = self:_listAll("follows", { Q.equal("followerId", userId), Q.select { "$id", "followingId" } })
    if not docs then return nil, err end
    local ids = {}
    for _, doc in ipairs(docs) do ids[#ids + 1] = doc.followingId end
    return ids
end

-- Public profiles for user IDs, keyed by user ID. Their photos are remembered
-- for `avatars`.
function Client:_profiles(userIds)
    local found, wanted, seen = {}, {}, {}
    for _, id in ipairs(userIds) do
        if self.profiles[id] and self.clock() - self.profiles[id].at < 300 then found[id] = self.profiles[id].value
        elseif not seen[id] then seen[id] = true; wanted[#wanted + 1] = id end
    end
    for first = 1, #wanted, 100 do
        local chunk = { table.unpack(wanted, first, math.min(#wanted, first + 99)) }
        local docs, err = self:_listAll("profiles", { Q.equal("userId", chunk),
            Q.select { "$id", "userId", "username", "avatarId" } }, 1)
        if not docs then return nil, err end
        for _, doc in ipairs(docs) do found[doc.userId] = profile(doc) end
    end
    for _, id in ipairs(wanted) do
        self.avatarIds[id] = found[id] and found[id].avatarId or false
        self.profiles[id] = { value = found[id], at = self.clock() }
    end
    return found
end

-- Display names for user IDs, from their public profiles.
function Client:names(userIds)
    local found, err = self:_profiles(userIds)
    if not found then return nil, err end
    local names = {}
    for _, id in ipairs(userIds) do names[id] = found[id] and found[id].username or "Someone" end
    return names
end

-- Profile photo file IDs for user IDs, or false for people without one. The
-- website, this app and Chat share profiles.avatarId, keyed by Appwrite user.
function Client:avatars(userIds)
    local wanted = {}
    for _, id in ipairs(userIds) do if self.avatarIds[id] == nil then wanted[#wanted + 1] = id end end
    if #wanted > 0 then
        local ok, err = self:_profiles(wanted)
        if not ok then return nil, err end
    end
    local avatars = {}
    for _, id in ipairs(userIds) do avatars[id] = self.avatarIds[id] end
    return avatars
end

-- Merge independently ordered post and repost streams. Cursors record only
-- consumed events, so a full page never discards the other stream's tail.
-- Each request reads one batch per source at most; deleted reposts may produce
-- an empty page with a progress cursor rather than downloading its full history.
function Client:timeline(authorIds, reposterIds, options)
    options = options or {}
    local limit = tonumber(options.limit) or 20
    if limit % 1 ~= 0 or limit < 1 or limit > 100 then return failure("validation", "Choose between 1 and 100 posts.") end
    local previous, boundary = options.before, nil
    if type(previous) == "string" then boundary, previous = previous, nil
    elseif previous ~= nil and (type(previous) ~= "table" or previous.version ~= 1) then
        return failure("validation", "Refresh this timeline to continue.")
    end
    if previous then boundary = previous.before end
    if boundary ~= nil and not text(boundary) then return failure("validation", "Refresh this timeline to continue.") end
    local streams = {
        { name = "posts", field = "authorId", ids = authorIds },
        { name = "reposts", field = "userId", ids = reposterIds },
    }
    for _, stream in ipairs(streams) do
        local saved = previous and previous[stream.name]
        if saved == nil then saved = {} end
        if type(saved) ~= "table" or (saved.after ~= nil and not validId(saved.after))
            or (saved.done ~= nil and type(saved.done) ~= "boolean") then
            return failure("validation", "Refresh this timeline to continue.")
        end
        stream.after, stream.exhausted = saved.after, saved.done == true or #stream.ids == 0
        stream.docs, stream.index = {}, 1
    end
    local originals, reposters = {}, {}
    local function fetch(stream)
        if stream.exhausted or stream.index <= #stream.docs then return true end
        local query = { Q.equal(stream.field, stream.ids), Q.orderDesc("$createdAt"), Q.orderDesc("$id"), Q.limit(limit) }
        if boundary then query[#query + 1] = Q.lessThan("$createdAt", boundary) end
        if stream.after then query[#query + 1] = Q.cursorAfter(stream.after) end
        local data, err
        if stream.name == "posts" then data, err = self:_postList(query)
        else
            query[#query + 1] = Q.select { "$id", "$createdAt", "postId", "userId" }
            data, err = self:_list("reposts", query)
        end
        if not data then return nil, err end
        stream.docs, stream.index, stream.exhausted = data.documents, 1, #data.documents < limit
        if stream.name == "reposts" and #data.documents > 0 then
            local ids, users, wanted, seen = {}, {}, {}, {}
            for _, doc in ipairs(data.documents) do
                if originals[doc.postId] == nil and not wanted[doc.postId] then
                    ids[#ids + 1], wanted[doc.postId] = doc.postId, true
                end
                if not reposters[doc.userId] and not seen[doc.userId] then
                    users[#users + 1], seen[doc.userId] = doc.userId, true
                end
            end
            if #ids > 0 then
                local found
                found, err = self:_postList { Q.equal("$id", ids), Q.limit(100) }
                if not found then return nil, err end
                for _, id in ipairs(ids) do originals[id] = false end
                for _, doc in ipairs(found.documents) do originals[doc["$id"]] = post(doc) end
            end
            if #users > 0 then
                local names
                names, err = self:names(users)
                if not names then return nil, err end
                for id, name in pairs(names) do reposters[id] = name end
            end
        end
        return true
    end
    local initial = self:_parallel {
        function() return fetch(streams[1]) end,
        function() return fetch(streams[2]) end,
    }
    for _, result in ipairs(initial) do if not result[1] then return nil, result[2] end end
    local result, seen = {}, {}
    while #result < limit do
        local chosen, doc, at, waiting
        for _, stream in ipairs(streams) do
            local nextDoc = stream.docs[stream.index]
            if not nextDoc and not stream.exhausted then waiting = true end
            if nextDoc then
                local nextAt = nextDoc.sourceCreatedAt or nextDoc["$createdAt"]
                if not doc or nextAt > at or (nextAt == at and nextDoc["$id"] > doc["$id"]) then
                    chosen, doc, at = stream, nextDoc, nextAt
                end
            end
        end
        -- An unread batch could precede the other stream's buffered tail.
        if waiting or not chosen then break end
        chosen.after, chosen.index = doc["$id"], chosen.index + 1
        -- Deleted originals still consume their repost event and cannot stall paging.
        local value = chosen.name == "posts" and post(doc) or originals[doc.postId]
        if value and not seen[value.id] then
            seen[value.id] = true
            result[#result + 1] = { post = value, at = at,
                repostedBy = chosen.name == "reposts" and { id = doc.userId, name = reposters[doc.userId] } or nil }
        end
    end
    local cursor, more = { version = 1, before = boundary }, false
    for _, stream in ipairs(streams) do
        local done = stream.exhausted and stream.index > #stream.docs
        cursor[stream.name] = { after = stream.after, done = done }
        if not done then more = true end
    end
    return result, more and cursor or nil
end

-- Your posts plus posts and reposts from the people you follow.
function Client:homeTimeline(options)
    local me = self:currentUserId()
    local ids, err = self:listFollowingIds()
    if not ids then return nil, err end
    local authors = { me }
    for _, id in ipairs(ids) do authors[#authors + 1] = id end
    return self:timeline(authors, ids, options)
end

-- A profile's posts with the posts they reposted.
function Client:userTimeline(userId, options)
    if not validId(userId) then return failure("validation", "Choose a person.") end
    return self:timeline({ userId }, { userId }, options)
end

-- Connect: who liked, reposted or replied to your posts, and who followed you.
-- Items are { kind, userId, name, at, post, body }, newest first.
function Client:activity(options)
    local session, err = self:_requireSession()
    if not session then return nil, err end
    local me, limit = session.user.id, (options and options.limit) or 40
    local mine
    mine, err = self:_list("posts", { Q.equal("authorId", me), Q.orderDesc("$createdAt"), Q.limit(100) })
    if not mine then return nil, err end
    local posts, ids = {}, {}
    for _, doc in ipairs(mine.documents) do posts[doc["$id"]] = post(doc); ids[#ids + 1] = doc["$id"] end
    local items = {}
    local function add(name, query, build)
        local data, listError = self:_list(name, query)
        if not data then return listError end
        for _, doc in ipairs(data.documents) do
            local item = build(doc)
            if item and item.userId ~= me then item.at = doc["$createdAt"]; items[#items + 1] = item end
        end
    end
    local recent = { Q.orderDesc("$createdAt"), Q.limit(limit) }
    local function query(...)
        local result = { ... }
        for _, q in ipairs(recent) do result[#result + 1] = q end
        return result
    end
    err = add("follows", query(Q.equal("followingId", me)), function(doc)
        return { kind = "follow", userId = doc.followerId }
    end)
    if not err and #ids > 0 then
        err = add("likes", query(Q.equal("targetType", "post"), Q.equal("targetId", ids)), function(doc)
            return { kind = "like", userId = doc.userId, post = posts[doc.targetId] }
        end) or add("reposts", query(Q.equal("postId", ids)), function(doc)
            return { kind = "repost", userId = doc.userId, post = posts[doc.postId] }
        end) or add("comments", query(Q.equal("postId", ids)), function(doc)
            return { kind = "comment", userId = doc.authorId, name = doc.authorName, post = posts[doc.postId], body = doc.body }
        end)
    end
    if err then return nil, err end
    local users = {}
    for _, item in ipairs(items) do if not item.name then users[#users + 1] = item.userId end end
    local names = {}
    if #users > 0 then
        names, err = self:names(users)
        if not names then return nil, err end
    end
    for _, item in ipairs(items) do item.name = item.name or names[item.userId] end
    table.sort(items, function(a, b) return a.at > b.at end)
    for i = #items, limit + 1, -1 do items[i] = nil end
    return items
end

-- Validates like the website's composer, uploads a photo when present, then publishes.
function Client:createPost(values)
    local session, err = self:_requireSession()
    if not session then return nil, err end
    values = values or {}
    local kind = values.postType or "text"
    if not POST_TYPES[kind] then return failure("validation", "Choose text, photo, quote or link.") end
    local data = { title = "", content = "", tags = Octopus.parseTags(values.tags), authorId = session.user.id,
        authorName = session.user.name, postType = kind }
    if kind == "text" then
        data.title, data.content = trim(values.title), trim(values.content)
        if data.title == "" then return failure("validation", "Title is required.") end
        if data.content == "" then return failure("validation", "Write something before publishing.") end
    elseif kind == "photo" then
        if not values.photo then return failure("validation", "Choose a photo to share.") end
        data.content = trim(values.content)
    elseif kind == "quote" then
        data.content, data.quoteSource = trim(values.content), trim(values.quoteSource)
        if data.content == "" then return failure("validation", "Quote text is required.") end
        local thoughts = trim(values.userText)
        if thoughts ~= "" then data.userText = thoughts end
    else
        data.title, data.content, data.linkUrl = trim(values.title), trim(values.content), trim(values.linkUrl)
        if data.title == "" then return failure("validation", "Title is required.") end
        if data.linkUrl == "" then return failure("validation", "URL is required.") end
        if not data.linkUrl:match("^https?://[^%s/]+") then
            return failure("validation", "URL must start with http:// or https://")
        end
    end
    if #data.title > 256 then return failure("validation", "Keep the title under 256 characters.") end
    if #data.content > 65535 then return failure("validation", "This post is too long.") end
    if #data.tags == 0 then data.tags = nil end
    if (kind == "photo" or kind == "quote") and values.photo then
        local file
        file, err = self:uploadImage(values.photo)
        if not file then return nil, err end
        data.imageId = file
        -- Derive layout metadata from the uploaded bytes, not caller-supplied fields.
        local bytes = type(values.photo) == "table" and values.photo.jpeg or values.photo
        data.imageWidth, data.imageHeight = Octopus.imageDimensions(bytes)
    end
    local doc
    doc, err = self:_create("posts", data)
    if not doc then return nil, err end
    return post(doc)
end

function Client:deletePost(item)
    if not item or item.authorId ~= self:currentUserId() then
        return failure("forbidden", "You can only delete your own posts.")
    end
    local ok, err = self:_delete("posts", item.id)
    if not ok then return nil, err end
    -- The post is gone; a leftover image is harmless, so cleanup is best effort.
    if item.imageId then self:_request("DELETE", "/storage/buckets/" .. self.bucketId .. "/files/" .. escape(item.imageId)) end
    return true
end

-- Likes, reposts and comment counts for a set of posts (or comments), with the
-- signed-in reader's own like/repost document IDs for undoing them.
function Client:stats(ids, targetType, posts)
    targetType = targetType or "post"
    local result, me, missing, snapshots = {}, self:currentUserId(), {}, {}
    for _, item in ipairs(posts or {}) do snapshots[item.id] = item.stats end
    for _, id in ipairs(ids) do
        local counts = targetType == "post" and snapshots[id]
        result[id] = { likes = counts and counts.likes or 0, comments = counts and counts.comments or 0,
            reposts = counts and counts.reposts or 0 }
        if not counts then missing[#missing + 1] = id end
    end
    if #ids == 0 then return result end
    -- Materialized counts need no reaction download. Only retrieve this reader's
    -- undo IDs; guests need no additional requests at all.
    if #missing == 0 then
        if not me then return result end
        local lists = self:_parallel {
            function() return self:_listAll("likes", { Q.equal("targetType", targetType), Q.equal("targetId", ids),
                Q.equal("userId", me), Q.select { "$id", "targetId" } }) end,
            function() return self:_listAll("reposts", { Q.equal("postId", ids), Q.equal("userId", me),
                Q.select { "$id", "postId" } }) end,
        }
        local likes, err = table.unpack(lists[1], 1, 2)
        if not likes then return nil, err end
        for _, doc in ipairs(likes) do if result[doc.targetId] then result[doc.targetId].liked = doc["$id"] end end
        local reposts
        reposts, err = table.unpack(lists[2], 1, 2)
        if not reposts then return nil, err end
        for _, doc in ipairs(reposts) do if result[doc.postId] then result[doc.postId].reposted = doc["$id"] end end
        return result
    end
    -- Legacy collections remain supported until the feed migration is enabled.
    local lists = self:_parallel {
        function() return self:_listAll("likes", { Q.equal("targetType", targetType), Q.equal("targetId", ids),
            Q.select { "$id", "targetId", "userId" } }) end,
        function()
            if targetType ~= "post" then return {} end
            return self:_listAll("comments", { Q.equal("postId", missing), Q.select { "$id", "postId" } })
        end,
        function()
            if targetType ~= "post" then return {} end
            return self:_listAll("reposts", { Q.equal("postId", ids), Q.select { "$id", "postId", "userId" } })
        end,
    }
    local likes, err = table.unpack(lists[1], 1, 2)
    if not likes then return nil, err end
    for _, doc in ipairs(likes) do
        local entry = result[doc.targetId]
        if entry then
            if not snapshots[doc.targetId] then entry.likes = entry.likes + 1 end
            if doc.userId == me then entry.liked = doc["$id"] end
        end
    end
    if targetType ~= "post" then return result end
    local comments
    comments, err = table.unpack(lists[2], 1, 2)
    if not comments then return nil, err end
    for _, doc in ipairs(comments) do
        if result[doc.postId] then result[doc.postId].comments = result[doc.postId].comments + 1 end
    end
    local reposts
    reposts, err = table.unpack(lists[3], 1, 2)
    if not reposts then return nil, err end
    for _, doc in ipairs(reposts) do
        local entry = result[doc.postId]
        if entry then
            if not snapshots[doc.postId] then entry.reposts = entry.reposts + 1 end
            if doc.userId == me then entry.reposted = doc["$id"] end
        end
    end
    return result
end

-- Returns the new like document ID, or false once the like is removed.
function Client:toggleLike(targetId, targetType, likeId)
    local session, err = self:_requireSession()
    if not session then return nil, err end
    if not validId(targetId) then return failure("validation", "Choose something to like.") end
    if likeId then
        local ok
        ok, err = self:_delete("likes", likeId)
        if not ok and err.status ~= 404 then return nil, err end
        return false
    end
    local doc
    doc, err = self:_create("likes", { targetId = targetId, targetType = targetType or "post", userId = session.user.id })
    if doc then return doc["$id"] end
    if err.status ~= 409 then return nil, err end
    local existing = self:_list("likes", { Q.equal("targetId", targetId), Q.equal("targetType", targetType or "post"),
        Q.equal("userId", session.user.id), Q.limit(1) })
    return existing and existing.documents[1] and existing.documents[1]["$id"] or false
end

function Client:toggleRepost(postId, repostId)
    local session, err = self:_requireSession()
    if not session then return nil, err end
    if not validId(postId) then return failure("validation", "Choose a post to repost.") end
    if repostId then
        local ok
        ok, err = self:_delete("reposts", repostId)
        if not ok and err.status ~= 404 then return nil, err end
        return false
    end
    local doc
    doc, err = self:_create("reposts", { postId = postId, userId = session.user.id })
    if doc then return doc["$id"] end
    if err.status ~= 409 then return nil, err end
    local existing = self:_list("reposts", { Q.equal("postId", postId), Q.equal("userId", session.user.id), Q.limit(1) })
    return existing and existing.documents[1] and existing.documents[1]["$id"] or false
end

-- Comments -----------------------------------------------------------------

function Client:listComments(postId)
    if not validId(postId) then return failure("validation", "Choose a post.") end
    local data, err = self:_list("comments", { Q.equal("postId", postId), Q.orderAsc("$createdAt"), Q.limit(100) })
    if not data then return nil, err end
    local comments = {}
    for _, doc in ipairs(data.documents) do comments[#comments + 1] = comment(doc) end
    return comments
end

function Client:addComment(postId, body, parentId)
    local session, err = self:_requireSession()
    if not session then return nil, err end
    body = trim(body)
    if not validId(postId) then return failure("validation", "Choose a post.") end
    if body == "" or #body > 4096 then return failure("validation", "Write a comment of up to 4096 characters.") end
    if parentId and parentId ~= "" and not validId(parentId) then return failure("validation", "Choose a comment to reply to.") end
    local doc
    doc, err = self:_create("comments", { postId = postId, authorId = session.user.id, authorName = session.user.name,
        body = body, parentId = parentId or "" })
    if not doc then return nil, err end
    return comment(doc)
end

-- People -------------------------------------------------------------------

-- Follower counts need a signed-in reader: the follows collection is readable by users only.
function Client:getProfile(userId)
    if not validId(userId) then return failure("validation", "Choose a person.") end
    local doc, err = self:_request("GET", self:collection("profiles") .. "/" .. escape(userId))
    if not doc and err.status ~= 404 then return nil, err end
    local result = profile(doc, userId)
    if userId == self:currentUserId() then
        local trusted = self:getAccountProfile()
        result.handle = trusted and trusted.handle or nil
    end
    self.avatarIds[userId] = result.avatarId or false
    local posts = self:_list("posts", { Q.equal("authorId", userId), Q.limit(1), Q.select { "$id" } }, true)
    result.posts = posts and posts.total
    if self.session then
        local followers = self:_list("follows", { Q.equal("followingId", userId), Q.limit(1), Q.select { "$id" } }, true)
        local following = self:_list("follows", { Q.equal("followerId", userId), Q.limit(1), Q.select { "$id" } }, true)
        result.followers, result.following = followers and followers.total, following and following.total
    end
    return result
end

function Client:updateProfile(values)
    local session, err = self:_requireSession()
    if not session then return nil, err end
    local username, bio = trim(values.username), trim(values.bio)
    if username == "" or #username > 128 then return failure("validation", "Enter a name of up to 128 characters.") end
    if #bio > 1024 then return failure("validation", "Keep your bio under 1024 characters.") end
    local ok
    ok, err = self:ensureProfile()
    if not ok then return nil, err end
    ok, err = self:_request("PATCH", "/account/name", { body = { name = username } })
    if not ok then return nil, err end
    local doc
    doc, err = self:_request("PATCH", self:collection("profiles") .. "/" .. escape(session.user.id),
        { body = { data = { username = username, bio = bio } } })
    if not doc then
        -- Account and Databases cannot share a transaction. Keep a failed Save
        -- from silently changing the name used by subsequent posts.
        local restored = self:_request("PATCH", "/account/name", {
            body = { name = session.user.accountName or session.user.name },
        })
        if not restored then
            session.user.accountName = username
            return failure("profile_sync", "Your account display name changed, but the profile could not be saved. Retry Save to finish syncing.", err.status)
        end
        return nil, err
    end
    session.user.name, session.user.accountName = username, username
    self.profiles[session.user.id] = nil
    return profile(doc)
end

-- Upload a photo and make it the reader's profile photo.
function Client:setAvatar(photo)
    local session, err = self:_requireSession()
    if not session then return nil, err end
    local ok, file, doc
    ok, err = self:ensureProfile()
    if not ok then return nil, err end
    file, err = self:uploadImage(photo)
    if not file then return nil, err end
    doc, err = self:_request("PATCH", self:collection("profiles") .. "/" .. escape(session.user.id),
        { body = { data = { avatarId = file } } })
    if not doc then return nil, err end
    self.avatarIds[session.user.id] = file
    self.profiles[session.user.id] = nil
    return profile(doc)
end

function Client:searchPeople(query)
    if not text(query) then return {} end
    local data, err = self:_list("profiles", { Q.search("username", trim(query)), Q.limit(10) })
    if not data then return nil, err end
    local people = {}
    for _, doc in ipairs(data.documents) do
        people[#people + 1] = profile(doc)
        self.avatarIds[doc.userId] = people[#people].avatarId or false
    end
    return people
end

-- Returns the follow document ID, or false.
function Client:followState(userId)
    local me = self:currentUserId()
    if not me or me == userId then return false end
    local data, err = self:_list("follows", { Q.equal("followerId", me), Q.equal("followingId", userId), Q.limit(1) })
    if not data then return nil, err end
    return data.documents[1] and data.documents[1]["$id"] or false
end

function Client:follow(userId)
    local session, err = self:_requireSession()
    if not session then return nil, err end
    if not validId(userId) or userId == session.user.id then return failure("validation", "Choose someone else to follow.") end
    local doc
    doc, err = self:_create("follows", { followerId = session.user.id, followingId = userId })
    if doc then return doc["$id"] end
    if err.status == 409 then return self:followState(userId) end
    return nil, err
end

function Client:unfollow(userId, followId)
    local session, err = self:_requireSession()
    if not session then return nil, err end
    followId = followId or self:followState(userId)
    if not followId then return false end
    local ok
    ok, err = self:_delete("follows", followId)
    if not ok and err.status ~= 404 then return nil, err end
    return false
end

-- Images -------------------------------------------------------------------

local function imageKind(bytes)
    if type(bytes) ~= "string" then return nil end
    if bytes:sub(1, 3) == "\255\216\255" then return "jpg" end
    if bytes:sub(1, 8) == "\137PNG\r\n\26\n" then return "png" end
end
Octopus.imageKind = imageKind

-- Read only bounded image headers. Native photo preparation already rotates its
-- JPEG; raw JPEG uploads can retain EXIF orientation, so reserve displayed size.
function Octopus.imageDimensions(bytes)
    local kind = imageKind(bytes)
    if not kind then return nil end
    local function integer(at, count, little, last)
        if at < 1 or at + count - 1 > (last or #bytes) then return nil end
        local n = 0
        for i = 0, count - 1 do
            n = n * 256 + bytes:byte(at + (little and count - i - 1 or i))
        end
        return n
    end
    if kind == "png" then
        if #bytes < 33 or integer(9, 4) ~= 13 or bytes:sub(13, 16) ~= "IHDR" then return nil end
        return imageDimensions(integer(17, 4), integer(21, 4))
    end
    local function orientation(first, last)
        if bytes:sub(first, first + 5) ~= "Exif\0\0" then return nil end
        local base = first + 6
        local order = bytes:sub(base, base + 1)
        if order ~= "II" and order ~= "MM" then return nil end
        local little = order == "II"
        if integer(base + 2, 2, little, last) ~= 42 then return nil end
        local offset = integer(base + 4, 4, little, last)
        if not offset or offset < 8 then return nil end
        local directory = base + offset
        local count = integer(directory, 2, little, last)
        if not count or directory + 2 + count * 12 - 1 > last then return nil end
        for i = 0, count - 1 do
            local entry = directory + 2 + i * 12
            if integer(entry, 2, little, last) == 274
                and integer(entry + 2, 2, little, last) == 3
                and integer(entry + 4, 4, little, last) == 1 then
                return integer(entry + 8, 2, little, last)
            end
        end
    end
    local at, width, height, rotate = 3
    while at <= #bytes do
        if bytes:byte(at) ~= 255 then return nil end
        while bytes:byte(at) == 255 do at = at + 1 end
        local marker = bytes:byte(at)
        at = at + 1
        if not marker or marker == 0 or marker == 217 then break end
        if marker == 218 then break end -- Entropy-coded scan is not header data.
        if marker ~= 1 and not (marker >= 208 and marker <= 216) then
            local length = integer(at, 2)
            if not length or length < 2 or at + length - 1 > #bytes then return nil end
            if marker == 225 then rotate = orientation(at + 2, at + length - 1) or rotate end
            if marker >= 192 and marker <= 207 and marker ~= 196 and marker ~= 200 and marker ~= 204 then
                if length < 8 then return nil end
                height, width = integer(at + 3, 2), integer(at + 5, 2)
            end
            at = at + length
        end
    end
    if rotate and rotate >= 5 and rotate <= 8 then width, height = height, width end
    return imageDimensions(width, height)
end

-- ORCA decodes JPEG and PNG; other uploads from the website are converted by
-- Appwrite's preview endpoint.
function Client:getImage(fileId, variant, onPreview)
    if type(variant) == "function" then onPreview, variant = variant, nil end
    if not validId(fileId) then return failure("validation", "This post has no photo.") end
    if variant and variant ~= "avatar" and variant ~= "photo" then
        return failure("validation", "Choose an image size.")
    end
    local base = "/storage/buckets/" .. self.bucketId .. "/files/" .. escape(fileId)
    -- A complete small JPEG works with stb_image while the selected full-size
    -- image is pending. Transformations are optional on the Appwrite project.
    if type(onPreview) == "function" then
        local preview = self:_request("GET", base .. "/preview?width=96&output=jpg&quality=35", { binary = true })
        if preview and imageKind(preview) then
            -- Explicit false cancels remaining work; a consumer error does not.
            local ok, proceed = pcall(onPreview, preview)
            if ok and proceed == false then return failure("cancelled", "This photo is no longer needed.") end
        end
    end
    if variant then
        local size = variant == "avatar" and "width=192&height=192" or "width=780"
        local bytes = self:_request("GET", base .. "/preview?" .. size .. "&output=jpg&quality=80", { binary = true })
        if bytes and imageKind(bytes) then return bytes end
    end
    local bytes, err = self:_request("GET", base .. "/view", { binary = true })
    if bytes and imageKind(bytes) then return bytes end
    if not bytes and err.status == 404 then return nil, err end
    bytes, err = self:_request("GET", base .. "/preview?width=780&output=jpg&quality=80", { binary = true })
    if not bytes then return nil, err end
    if not imageKind(bytes) then return failure("invalid_image", "This photo format is not supported.") end
    return bytes
end

function Client:uploadImage(photo)
    local session, err = self:_requireSession()
    if not session then return nil, err end
    local bytes = type(photo) == "table" and photo.jpeg or photo
    local kind = imageKind(bytes)
    if not kind then return failure("invalid_image", "Choose a JPEG or PNG photo.") end
    if #bytes > 10485760 then return failure("invalid_image", "Photos must be smaller than 10 MB.") end
    self.uploads = (self.uploads or 0) + 1
    local boundary = "----OctopusOrca" .. os.time() .. self.uploads
    local body = table.concat {
        "--", boundary, "\r\nContent-Disposition: form-data; name=\"fileId\"\r\n\r\nunique()\r\n",
        "--", boundary, "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"photo.", kind,
        "\"\r\nContent-Type: image/", kind == "jpg" and "jpeg" or "png", "\r\n\r\n", bytes, "\r\n",
        "--", boundary, "--\r\n",
    }
    local file
    file, err = self:_request("POST", "/storage/buckets/" .. self.bucketId .. "/files", { body = body,
        headers = { ["Content-Type"] = "multipart/form-data; boundary=" .. boundary } })
    if not file then return nil, err end
    if type(file) ~= "table" or not validId(file["$id"]) then
        return failure("invalid_response", "Octopus did not return the uploaded photo.")
    end
    return file["$id"]
end

-- Construction -------------------------------------------------------------

function Octopus.client(options)
    options = options or {}
    if type(options.transport) ~= "function" then return failure("configuration", "Provide a transport function.") end
    if not validId(options.projectId) or not validId(options.databaseId or "octopus-db")
        or (options.functionId and not validId(options.functionId))
        or (options.feedCollectionId and not validId(options.feedCollectionId))
        or not validId(options.bucketId or "post-images") then
        return failure("configuration", "Set the public Appwrite project, database and bucket IDs.")
    end
    return setmetatable({ mode = options.mode or "custom", transport = options.transport,
        projectId = options.projectId, databaseId = options.databaseId or "octopus-db",
        bucketId = options.bucketId or "post-images", clock = options.clock or os.time, avatarIds = {}, profiles = {},
        feedCollectionId = options.feedCollectionId, cache = options.cache, schedule = options.schedule,
        functionId = options.functionId or "orca-chat-api" }, Client)
end

function Octopus.appwrite(options)
    options = options or {}
    local endpoint = options.endpoint
    if type(endpoint) ~= "string" or not (endpoint:match("^https://[%w%.%-]+/v1/?$")
            or endpoint:match("^http://localhost:%d+/v1/?$") or endpoint:match("^http://127%.0%.0%.1:%d+/v1/?$")) then
        return failure("configuration", "Set an Appwrite HTTPS endpoint ending in /v1.")
    end
    local transport = require("Octopus/api/appwrite").new {
        endpoint = endpoint, projectId = options.projectId, fetch = options.fetch,
    }
    return Octopus.client { mode = "appwrite", transport = transport, projectId = options.projectId,
        databaseId = options.databaseId, bucketId = options.bucketId, clock = options.clock,
        feedCollectionId = options.feedCollectionId, cache = options.cache, schedule = options.schedule, functionId = options.functionId }
end

-- The offline mock serves the same REST paths from in-memory fixtures.
function Octopus.mock(options)
    options = options or {}
    local transport = require("Octopus/api/mock").new(options)
    return assert(Octopus.client { mode = "mock", transport = transport, projectId = "octopus-mock",
        clock = options.clock })
end

return Octopus
