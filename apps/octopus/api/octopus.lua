-- The Octopus client speaks Appwrite's Account, Databases and Storage REST APIs,
-- the same services and collections as the Octopus website. A transport sends
-- { method, path, query, body, headers, binary } and returns { status, body, headers };
-- network and in-process mock transports share this client unchanged.
local Octopus = {}
local Client = {}
Client.__index = Client

local POST_TYPES = { text = true, photo = true, quote = true, link = true }
local PAGE = 100

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

local function escape(value)
    return (tostring(value):gsub("([^%w%-_%.~])", function(c) return string.format("%%%02X", c:byte()) end))
end

-- Appwrite query objects. The HTTP transport serialises them as JSON queries[].
local Q = {}
function Q.equal(attribute, values)
    return { method = "equal", attribute = attribute, values = type(values) == "table" and values or { values } }
end
function Q.notEqual(attribute, value) return { method = "notEqual", attribute = attribute, values = { value } } end
function Q.search(attribute, value) return { method = "search", attribute = attribute, values = { value } } end
function Q.orderDesc(attribute) return { method = "orderDesc", attribute = attribute } end
function Q.orderAsc(attribute) return { method = "orderAsc", attribute = attribute } end
function Q.limit(count) return { method = "limit", values = { count } } end
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
    return {
        id = doc["$id"], createdAt = doc["$createdAt"], title = doc.title or "", content = doc.content or "",
        authorId = doc.authorId, authorName = doc.authorName or "Someone", tags = tags,
        postType = POST_TYPES[doc.postType] and doc.postType or "text", imageId = doc.imageId,
        linkUrl = doc.linkUrl, quoteSource = doc.quoteSource, userText = doc.userText,
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
        binary = options.binary,
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

function Client:_list(name, query)
    local data, err = self:_request("GET", self:collection(name), { query = query })
    if not data then return nil, err end
    if type(data) ~= "table" or type(data.documents) ~= "table" then
        return failure("invalid_response", "Octopus returned an invalid list.")
    end
    return data
end

-- Walk every page of a filtered collection, up to `pages` requests.
function Client:_listAll(name, filters, pages)
    local documents, cursor = {}, nil
    for _ = 1, pages or 10 do
        local query = {}
        for i, filter in ipairs(filters) do query[i] = filter end
        query[#query + 1] = Q.limit(PAGE)
        if cursor then query[#query + 1] = Q.cursorAfter(cursor) end
        local data, err = self:_list(name, query)
        if not data then return nil, err end
        for _, doc in ipairs(data.documents) do documents[#documents + 1] = doc end
        if #data.documents < PAGE then break end
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
        or email:match("^[^@]+"), email = account.email } }
    self:ensureProfile()
    return self:getSession()
end

function Client:signUp(name, email, password, confirm)
    name, email = trim(name), trim(email)
    if not text(name) or #name > 128 then return failure("validation", "Enter a display name of up to 128 characters.") end
    if not email:match("^[^%s@]+@[^%s@]+%.[^%s@]+$") then return failure("validation", "Enter a valid email address.") end
    if type(password) ~= "string" or #password < 8 then
        return failure("validation", "Password must be at least 8 characters.")
    end
    if confirm ~= nil and confirm ~= password then return failure("validation", "Passwords do not match.") end
    local created, err = self:_request("POST", "/account",
        { body = { userId = "unique()", email = email, password = password, name = name } })
    if not created then return nil, err end
    return self:signIn(email, password)
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
    if options.tag then query[#query + 1] = Q.equal("tags", (tostring(options.tag):gsub("^#+", ""):lower())) end
    if options.search then
        if not text(options.search) then return {} end
        query[#query + 1] = Q.search("title", trim(options.search))
    end
    if options.exclude then query[#query + 1] = Q.notEqual("$id", options.exclude) end
    query[#query + 1] = Q.orderDesc("$createdAt")
    query[#query + 1] = Q.limit(limit)
    if options.cursor then query[#query + 1] = Q.cursorAfter(options.cursor) end
    local data, err = self:_list("posts", query)
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
    local docs, err = self:_listAll("follows", { Q.equal("followerId", userId) }, 1)
    if not docs then return nil, err end
    local ids = {}
    for _, doc in ipairs(docs) do ids[#ids + 1] = doc.followingId end
    return ids
end

-- Public profiles for user IDs, keyed by user ID. Their photos are remembered
-- for `avatars`.
function Client:_profiles(userIds)
    local found, wanted, seen = {}, {}, {}
    for _, id in ipairs(userIds) do if not seen[id] then seen[id] = true; wanted[#wanted + 1] = id end end
    for first = 1, #wanted, 100 do
        local chunk = { table.unpack(wanted, first, math.min(#wanted, first + 99)) }
        local docs, err = self:_listAll("profiles", { Q.equal("userId", chunk) }, 1)
        if not docs then return nil, err end
        for _, doc in ipairs(docs) do found[doc.userId] = profile(doc) end
    end
    for _, id in ipairs(wanted) do self.avatarIds[id] = found[id] and found[id].avatarId or false end
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

-- A Twitter-style timeline: posts by `authorIds` and reposts by `reposterIds`,
-- newest first. Each item is { post, at, repostedBy = { id, name } }. Paging
-- uses the time of the last item; returns items, next cursor (nil when done).
function Client:timeline(authorIds, reposterIds, options)
    options = options or {}
    local limit = tonumber(options.limit) or 20
    local function page(name, field, ids)
        if #ids == 0 then return { documents = {} } end
        local query = { Q.equal(field, ids), Q.orderDesc("$createdAt"), Q.limit(limit) }
        if options.before then query[#query + 1] = Q.lessThan("$createdAt", options.before) end
        return self:_list(name, query)
    end
    local posts, err = page("posts", "authorId", authorIds)
    if not posts then return nil, err end
    local reposts
    reposts, err = page("reposts", "userId", reposterIds)
    if not reposts then return nil, err end
    local items, originals, reposters = {}, {}, {}
    for _, doc in ipairs(posts.documents) do items[#items + 1] = { post = post(doc), at = doc["$createdAt"] } end
    if #reposts.documents > 0 then
        local ids, users = {}, {}
        for _, doc in ipairs(reposts.documents) do ids[#ids + 1], users[#users + 1] = doc.postId, doc.userId end
        local found
        found, err = self:_list("posts", { Q.equal("$id", ids), Q.limit(100) })
        if not found then return nil, err end
        for _, doc in ipairs(found.documents) do originals[doc["$id"]] = post(doc) end
        reposters, err = self:names(users)
        if not reposters then return nil, err end
    end
    for _, doc in ipairs(reposts.documents) do
        -- A repost of a deleted post has nothing to show.
        if originals[doc.postId] then
            items[#items + 1] = { post = originals[doc.postId], at = doc["$createdAt"],
                repostedBy = { id = doc.userId, name = reposters[doc.userId] } }
        end
    end
    table.sort(items, function(a, b) return a.at > b.at end)
    local result, seen = {}, {}
    for _, item in ipairs(items) do
        if #result == limit then break end
        if not seen[item.post.id] then seen[item.post.id] = true; result[#result + 1] = item end
    end
    local more = #posts.documents == limit or #reposts.documents == limit
    return result, more and #result > 0 and result[#result].at or nil
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
function Client:stats(ids, targetType)
    targetType = targetType or "post"
    local result, me = {}, self:currentUserId()
    for _, id in ipairs(ids) do result[id] = { likes = 0, comments = 0, reposts = 0 } end
    if #ids == 0 then return result end
    local likes, err = self:_listAll("likes", { Q.equal("targetType", targetType), Q.equal("targetId", ids) })
    if not likes then return nil, err end
    for _, doc in ipairs(likes) do
        local entry = result[doc.targetId]
        if entry then
            entry.likes = entry.likes + 1
            if doc.userId == me then entry.liked = doc["$id"] end
        end
    end
    if targetType ~= "post" then return result end
    local comments
    comments, err = self:_listAll("comments", { Q.equal("postId", ids) })
    if not comments then return nil, err end
    for _, doc in ipairs(comments) do
        if result[doc.postId] then result[doc.postId].comments = result[doc.postId].comments + 1 end
    end
    local reposts
    reposts, err = self:_listAll("reposts", { Q.equal("postId", ids) })
    if not reposts then return nil, err end
    for _, doc in ipairs(reposts) do
        local entry = result[doc.postId]
        if entry then
            entry.reposts = entry.reposts + 1
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
    self.avatarIds[userId] = result.avatarId or false
    local posts = self:_list("posts", { Q.equal("authorId", userId), Q.limit(1) })
    result.posts = posts and posts.total
    if self.session then
        local followers = self:_list("follows", { Q.equal("followingId", userId), Q.limit(1) })
        local following = self:_list("follows", { Q.equal("followerId", userId), Q.limit(1) })
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
    local doc
    doc, err = self:_request("PATCH", self:collection("profiles") .. "/" .. escape(session.user.id),
        { body = { data = { username = username, bio = bio } } })
    if not doc then return nil, err end
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

-- ORCA decodes JPEG and PNG; other uploads from the website are converted by
-- Appwrite's preview endpoint.
function Client:getImage(fileId)
    if not validId(fileId) then return failure("validation", "This post has no photo.") end
    local base = "/storage/buckets/" .. self.bucketId .. "/files/" .. escape(fileId)
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
        or not validId(options.bucketId or "post-images") then
        return failure("configuration", "Set the public Appwrite project, database and bucket IDs.")
    end
    return setmetatable({ mode = options.mode or "custom", transport = options.transport,
        projectId = options.projectId, databaseId = options.databaseId or "octopus-db",
        bucketId = options.bucketId or "post-images", clock = options.clock or os.time, avatarIds = {} }, Client)
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
        databaseId = options.databaseId, bucketId = options.bucketId, clock = options.clock }
end

-- The offline mock serves the same REST paths from in-memory fixtures.
function Octopus.mock(options)
    options = options or {}
    local transport = require("Octopus/api/mock").new(options)
    return assert(Octopus.client { mode = "mock", transport = transport, projectId = "octopus-mock",
        clock = options.clock })
end

return Octopus
