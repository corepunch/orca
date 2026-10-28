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
local sam = assert(client:userTimeline("u-sam"))
assert(#sam == 2 and sam[1].post.id == "p-reef" and not sam[1].repostedBy, "Profiles list their own posts")
local maya = assert(client:userTimeline("u-maya"))
assert(maya[1].post.id == "p-eames" and maya[2].post.id == "p-small" and maya[2].repostedBy.name == "Maya Chen",
    "Profiles include reposts at the time they were reposted")
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
assert(err.code == "unauthenticated", "The home timeline needs a session")
_, err = client:activity()
assert(err.code == "unauthenticated", "Connect needs a session")
_, err = client:createPost { title = "Hi", content = "There" }
assert(err.code == "unauthenticated")
_, err = client:toggleLike("p-reef", "post")
assert(err.code == "unauthenticated")

-- Sign in, follow, write and react.
_, err = client:signIn("demo@example.invalid", "wrong-password")
assert(err.status == 401 and not client:getSession(), "Wrong passwords must not sign in")
local session = assert(client:signIn(" demo@example.invalid ", "password123"))
assert(session.user.id == "u-ada" and session.user.name == "Ada Reef")
-- Home: Ada's own posts, Maya's and Sam's posts, and what they reposted.
local timeline, cursor = assert(client:homeTimeline { limit = 4 })
assert(#timeline == 4 and type(cursor) == "table" and cursor.version == 1, "A full page preserves both source cursors")
assert(timeline[1].post.id == "p-reef" and timeline[2].post.id == "p-eames")
assert(timeline[3].post.id == "p-small" and timeline[3].repostedBy.id == "u-maya", "Reposts appear in the timeline")
assert(timeline[4].post.id == "p-arms" and timeline[4].repostedBy.name == "Maya Chen")
local rest = assert(client:homeTimeline { limit = 4, before = cursor })
assert(rest[1].post.id == "p-hello" and rest[2].post.id == "p-pivot", "Your own posts are on your timeline")
local seen = {}
for _, item in ipairs(timeline) do seen[item.post.id] = true end
for _, item in ipairs(rest) do assert(not seen[item.post.id], "Pages do not overlap") end
-- Connect: who followed Ada and who liked, reposted or replied to her post.
local activity = assert(client:activity())
local kinds = {}
for _, item in ipairs(activity) do
    kinds[#kinds + 1] = item.kind .. ":" .. item.name
    assert(item.userId ~= "u-ada", "Your own actions are not news")
end
assert(table.concat(kinds, ",") == "like:Noor Haddad,repost:Jules Park,comment:Jules Park,like:Maya Chen,"
    .. "comment:Maya Chen,follow:Jules Park,follow:Maya Chen", table.concat(kinds, ","))
assert(activity[3].body == "Hello from the website side." and activity[1].post.id == "p-hello")
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

-- Each composer type enforces its own required fields and ignores hidden images.
_, err = client:createPost { postType = "text", title = "Title", content = "   " }
assert(err.code == "validation" and err.message == "Write something before publishing.")
_, err = client:createPost { postType = "photo", content = "Caption without a photo" }
assert(err.code == "validation" and err.message == "Choose a photo to share.")
_, err = client:createPost { postType = "quote", quoteSource = "Someone" }
assert(err.code == "validation" and err.message == "Quote text is required.")
_, err = client:createPost { postType = "link", title = "A link" }
assert(err.code == "validation" and err.message == "URL is required.")
_, err = client:createPost { postType = "link", title = string.rep("x", 257), linkUrl = "https://example.com" }
assert(err.code == "validation" and err.message == "Keep the title under 256 characters.")
local link = assert(client:createPost { postType = "link", title = "A link", linkUrl = "https://example.com",
    photo = { jpeg = "invalid hidden photo" } })
assert(link.linkUrl == "https://example.com" and link.content == "" and not link.imageId,
    "Links support an optional description and never upload a hidden photo")
local plain = assert(client:createPost { postType = "text", title = "Text", content = "Body",
    photo = { jpeg = "invalid hidden photo" } })
assert(not plain.imageId, "Text posts never upload a hidden photo")
local bare = assert(client:createPost { postType = "quote", content = "Just the words" })
assert(bare.quoteSource == "" and not bare.userText and not bare.imageId, "Only the quote text is required")
local illustrated = assert(client:createPost { postType = "quote", content = "Stay curious.",
    quoteSource = "A friend", userText = "A good reminder.", photo = { jpeg = image } })
assert(illustrated.quoteSource == "A friend" and illustrated.userText == "A good reminder." and illustrated.imageId
    and client:getImage(illustrated.imageId) == image, "Quote background photos and thoughts round-trip")
local captionless = assert(client:createPost { postType = "photo", photo = { jpeg = image } })
assert(captionless.content == "" and captionless.imageId, "Photo captions are optional")

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
_, err = fresh:signUp("New Person", "demo@example.invalid", "password123", nil, "new-person")
assert(err.status == 409, "Existing emails are rejected by the service")
session = assert(fresh:signUp("New Person", "new@example.invalid", "password123", "password123", "new-person"))
assert(session.user.id == "new-person", "The account name is the account ID")
local own = assert(fresh:getProfile(session.user.id))
assert(own.exists and own.username == "New Person" and own.handle == "new-person" and own.posts == 0)

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
assert(decoded[3].method == "orderDesc" and decoded[3].attribute == "$id")
assert(decoded[4].method == "limit" and decoded[4].values[1] == 3)
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

-- Profile photos live in profiles.avatarId, shared with the website and Chat.
local photos = Octopus.mock { clock = clock }
local lookups, transport = 0, photos.transport
photos.transport = function(request) lookups = lookups + 1; return transport(request) end
local avatars = assert(photos:avatars { "u-jules", "u-nobody" })
assert(avatars["u-nobody"] == false, "People without a profile have no photo")
assert(lookups == 1 and photos:avatars { "u-nobody" } and lookups == 1, "Photo lookups are batched and cached")
assert(not photos:setAvatar(image), "Setting a photo needs a session")
assert(photos:signIn("demo@example.invalid", "password123"))
local mine = assert(photos:setAvatar(image))
assert(mine.avatarId and photos:getImage(mine.avatarId) == image, "The photo is uploaded")
assert(photos:avatars({ "u-ada" })["u-ada"] == mine.avatarId and photos:getProfile("u-ada").avatarId == mine.avatarId)
assert(photos:searchPeople("ada")[1].avatarId == mine.avatarId, "Search results carry photos")

print("Octopus API: guest reads, home and profile timelines with reposts, connect activity, sessions, posts of every type, likes, reposts, comments, follows, profiles, profile photos, photos and HTTP queries passed")
