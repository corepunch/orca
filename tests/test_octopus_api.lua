local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
filesystem.init("apps/octopus")

local Octopus = require "Octopus/api/octopus"
local json = require "Octopus/api/json"
local NOW = 1761400000
local function clock() return NOW end

-- Guests read the public timeline, posts, comments and profiles.
local client = Octopus.mock { clock = clock }
assert(client.mode == "mock" and not client:getSession(), "The mock starts signed out")
local posts = assert(client:listPosts { limit = 5 })
assert(#posts == 5 and posts[1].id == "p-reef" and posts[2].id == "p-eames", "Newest posts come first")
assert(posts[1].postType == "photo" and posts[1].imageId == "f-reef")
local more = assert(client:listPosts { limit = 5, cursor = posts[5].id })
assert(#more == 3 and more[1].id == "p-pivot", "Cursor pagination continues after the last post")
local stats = assert(client:stats { "p-reef", "p-small", "p-tide" })
assert(stats["p-reef"].likes == 3 and stats["p-reef"].comments == 2 and stats["p-reef"].reposts == 1)
assert(stats["p-tide"].likes == 0 and not stats["p-reef"].liked, "Guests have no likes of their own")
local tagged = assert(client:listPosts { tag = "#Reef" })
assert(#tagged == 2 and tagged[1].id == "p-reef" and tagged[2].id == "p-arms", "Tags match without # and case")
local found = assert(client:listPosts { search = "software" })
assert(#found == 1 and found[1].id == "p-small", "Search matches post titles")
local people = assert(client:searchPeople("maya"))
assert(#people == 1 and people[1].userId == "u-maya")
local comments = assert(client:listComments("p-hello"))
assert(#comments == 3 and comments[2].parentId == "c-welcome", "Comments keep reply threads in order")
local profile = assert(client:getProfile("u-sam"))
assert(profile.username == "Sam Rivera" and profile.posts == 2 and profile.followers == nil,
    "Follower counts need a signed-in reader")
local image = assert(client:getImage("f-reef"))
assert(Octopus.imageKind(image) == "png", "The fixture photo is a decodable PNG")

local _, err = client:homeTimeline()
assert(err.code == "unauthenticated", "Following needs a session")
_, err = client:createPost { title = "Hi", content = "There" }
assert(err.code == "unauthenticated")
_, err = client:toggleLike("p-reef", "post")
assert(err.code == "unauthenticated")

-- Sign in, follow, write and react.
_, err = client:signIn("demo@example.invalid", "wrong-password")
assert(err.status == 401 and not client:getSession(), "Wrong passwords must not sign in")
local session = assert(client:signIn(" demo@example.invalid ", "password123"))
assert(session.user.id == "u-ada" and session.user.name == "Ada Reef")
local timeline, following = client:homeTimeline()
assert(timeline and #following == 2, "Ada follows Maya and Sam")
for _, item in ipairs(timeline) do assert(item.authorId == "u-maya" or item.authorId == "u-sam") end
profile = assert(client:getProfile("u-ada"))
assert(profile.followers == 2 and profile.following == 2 and profile.posts == 1)

_, err = client:createPost { postType = "text", title = "", content = "Body" }
assert(err.code == "validation" and err.message == "Title is required.")
_, err = client:createPost { postType = "link", title = "A link", linkUrl = "javascript:alert(1)" }
assert(err.code == "validation", "Links must be http or https")
local quote = assert(client:createPost { postType = "quote", content = "Less, but better.", quoteSource = "Dieter Rams",
    tags = "#design, Craft design" })
assert(quote.authorId == "u-ada" and quote.tags[1] == "design" and quote.tags[2] == "craft" and #quote.tags == 2)
local text = assert(client:createPost { postType = "text", title = "Quotes \"and\" lines",
    content = "Line one\nLine \"two\"" })
assert((client:listPosts { limit = 1 })[1].id == text.id, "A new post tops the timeline")
local photo = assert(client:createPost { postType = "photo", content = "Shared from ORCA", photo = { jpeg = image } })
assert(photo.imageId and client:getImage(photo.imageId) == image, "Uploaded photos round-trip through storage")

local like = assert(client:toggleLike("p-reef", "post"))
stats = assert(client:stats { "p-reef" })
assert(stats["p-reef"].likes == 4 and stats["p-reef"].liked == like)
assert(client:toggleLike("p-reef", "post") == stats["p-reef"].liked, "A duplicate like returns the existing one")
assert(client:toggleLike("p-reef", "post", like) == false)
assert(assert(client:stats { "p-reef" })["p-reef"].likes == 3)
local repost = assert(client:toggleRepost("p-small"))
assert(assert(client:stats { "p-small" })["p-small"].reposted == repost)
assert(client:toggleRepost("p-small", repost) == false)

local reply = assert(client:addComment("p-reef", "  Lovely photo  ", ""))
assert(reply.body == "Lovely photo" and reply.authorName == "Ada Reef")
_, err = client:addComment("p-reef", "   ")
assert(err.code == "validation")
local commentLike = assert(client:toggleLike(reply.id, "comment"))
assert(assert(client:stats({ reply.id }, "comment"))[reply.id].liked == commentLike)

assert(client:followState("u-jules") == false)
local follow = assert(client:follow("u-jules"))
assert(client:followState("u-jules") == follow)
assert(client:follow("u-jules") == follow, "Following twice keeps one follow")
assert(client:unfollow("u-jules") == false and client:followState("u-jules") == false)
_, err = client:follow("u-ada")
assert(err.code == "validation", "People cannot follow themselves")

local updated = assert(client:updateProfile { username = "Ada R.", bio = "Hello from the tests" })
assert(updated.username == "Ada R." and updated.bio == "Hello from the tests")
_, err = client:deletePost(posts[1])
assert(err.code == "forbidden", "Only authors may delete their posts")
assert(client:deletePost(photo))
_, err = client:getPost(photo.id)
assert(err.status == 404 and not client:getImage(photo.imageId), "Deleting a post removes its photo")

assert(client:signOut() and not client:getSession())
_, err = client:homeTimeline()
assert(err.code == "unauthenticated")

-- Sign-up creates the account, signs in and creates the public profile.
local fresh = Octopus.mock { clock = clock }
_, err = fresh:signUp("New Person", "new@example.invalid", "short")
assert(err.code == "validation")
_, err = fresh:signUp("New Person", "new@example.invalid", "password123", "different")
assert(err.message == "Passwords do not match.")
_, err = fresh:signUp("New Person", "demo@example.invalid", "password123")
assert(err.status == 409, "Existing emails are rejected by the service")
session = assert(fresh:signUp("New Person", "new@example.invalid", "password123", "password123"))
local own = assert(fresh:getProfile(session.user.id))
assert(own.exists and own.username == "New Person" and own.posts == 0)

-- The HTTP transport serialises Appwrite queries, cookies and bodies.
local requests = {}
local function fakeFetch(url, options)
    requests[#requests + 1] = { url = url, options = options }
    if url:match("/account/sessions/email$") then
        return { status = 201, headers = { ["set-cookie"] = "a_session_proj1=abc123; path=/; secure" }, body = "{}" }
    elseif url:match("/account$") then
        return { status = 200, headers = {}, body = '{"$id":"user1","name":"Remote","email":"r@example.invalid"}' }
    elseif url:match("/profiles/documents/user1$") then
        return { status = 200, headers = {}, body = '{"$id":"user1","userId":"user1","username":"Remote"}' }
    elseif url:match("/posts/documents%?") then
        return { status = 200, headers = {}, body = '{"total":1,"documents":[{"$id":"p1","title":"T","authorId":"a",'
            .. '"authorName":"A","postType":"weird","tags":["x"],"$createdAt":"2026-10-25T10:00:00.000+00:00"}]}' }
    elseif url:match("/posts/documents$") then
        return { status = 201, headers = {}, body = options.body:gsub('^{', '{"$id":"p2",') }
    elseif url:match("/broken") then
        return { status = 500, headers = {}, body = "<html>oops</html>" }
    end
    return { status = 404, headers = {}, body = '{"message":"Not found","code":404,"type":"general_route_not_found"}' }
end
local remote = assert(Octopus.appwrite { endpoint = "https://example.invalid/v1", projectId = "proj1", fetch = fakeFetch })
local list = assert(remote:listPosts { tag = "news", limit = 3 })
assert(list[1].postType == "text", "Unknown post types display as text")
local url = requests[1].url
assert(url:match("^https://example%.invalid/v1/databases/octopus%-db/collections/posts/documents%?queries%[0%]="))
local decoded = {}
for value in url:gmatch("queries%[%d+%]=([^&]+)") do
    decoded[#decoded + 1] = json.decode((value:gsub("%%(%x%x)", function(h) return string.char(tonumber(h, 16)) end)))
end
assert(decoded[1].method == "equal" and decoded[1].attribute == "tags" and decoded[1].values[1] == "news")
assert(decoded[2].method == "orderDesc" and decoded[2].attribute == "$createdAt")
assert(decoded[3].method == "limit" and decoded[3].values[1] == 3)
assert(requests[1].options.headers["X-Appwrite-Project"] == "proj1" and requests[1].options.nocookies)
assert(not requests[1].options.headers.Cookie, "Guests send no session")
assert(remote:signIn("r@example.invalid", "password123"))
assert(requests[#requests].options.headers.Cookie == "a_session_proj1=abc123", "The session cookie is kept per client")
local created = assert(remote:createPost { postType = "text", title = 'Say "hi"', content = "a\nb" })
local body = json.decode(requests[#requests].options.body)
assert(body.documentId == "unique()" and body.data.title == 'Say "hi"' and body.data.content == "a\nb")
assert(body.data.tags == nil, "Empty tag lists are omitted")
assert(created.id == "p2" and requests[#requests].options.headers.Cookie == "a_session_proj1=abc123")
_, err = remote:_request("GET", "/broken")
assert(err.status == 500 and err.code == "http_error", "Non-JSON errors keep their status")
_, err = remote:getPost("missing")
assert(err.status == 404 and err.message == "This post was deleted or never existed.")
local ok = pcall(remote.transport, { method = "GET", path = "/../secret" })
assert(not ok, "Path traversal is rejected")
assert(not Octopus.appwrite { endpoint = "http://example.invalid/v1", projectId = "p" }, "HTTPS is required")

print("Octopus API: guest reads, sessions, posts of every type, likes, reposts, comments, follows, profiles, photos and HTTP queries passed")
