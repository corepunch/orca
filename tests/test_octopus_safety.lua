local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
filesystem.init("apps/octopus")

local Octopus = require "Octopus/api/octopus"
local function clock() return 1761400000 end

-- Guests cannot report, block or delete.
local transport, state = require("Octopus/api/mock").new { clock = clock }
local client = assert(Octopus.client { mode = "mock", transport = transport, projectId = "octopus-mock", clock = clock })
local _, err = client:reportContent { userId = "u-sam", reason = "spam" }
assert(err.code == "unauthenticated", "Reports need a session")
_, err = client:setUserBlocked("u-sam", true)
assert(err.code == "unauthenticated", "Blocks need a session")
assert(not client:isBlocked("u-sam"), "Guests block nobody")

assert(client:signIn("demo@example.invalid", "password123"))
local me = client:currentUserId()

-- Reports name a person, and optionally one of their posts or comments.
_, err = client:reportContent { userId = "u-sam", reason = "rude" }
assert(err.code == "validation", "Unknown reasons are refused before sending")
_, err = client:reportContent { userId = me, reason = "spam" }
assert(err.code == "validation", "Readers cannot report themselves")
_, err = client:reportContent { userId = "u-sam", reason = "spam", postId = "p-reef", commentId = "c-welcome" }
assert(err.code == "validation", "One post or comment per report")
_, err = client:reportContent { userId = "u-maya", reason = "spam", postId = "p-reef" }
assert(err.code == "not_found", "The post must belong to the reported person")
local report = assert(client:reportContent { userId = "u-sam", reason = "harassment", details = "Unkind", postId = "p-reef" })
assert(report.submitted and state.reports[1].post == "p-reef" and state.reports[1].details == "Unkind"
    and state.reports[1].reporter == me, "A post report reaches the service with its details")
local comments = assert(client:listComments("p-hello"))
local theirs
for _, comment in ipairs(comments) do if comment.authorId ~= me then theirs = comment end end
assert(client:reportContent { userId = theirs.authorId, reason = "other", commentId = theirs.id })
assert(state.reports[2].comment == theirs.id, "A comment report names the comment")

-- Blocking hides a person's posts, reposts, comments and profile.
local function has(posts, author)
    for _, item in ipairs(posts) do if (item.post or item).authorId == author then return true end end
    return false
end
assert(has(assert(client:listPosts { limit = 20 }), "u-sam"), "Sam posts before the block")
_, err = client:setUserBlocked(me, true)
assert(err.code == "validation", "Readers cannot block themselves")
assert(client:setUserBlocked("u-sam", true))
assert(client:isBlocked("u-sam"))
assert(not has(assert(client:listPosts { limit = 20 }), "u-sam"), "Blocked authors leave post lists")
assert(not has(assert(client:discoverTimeline()), "u-sam"), "Blocked authors leave discover")
assert(not has(assert(client:userTimeline("u-maya")), "u-sam"), "Reposts of blocked authors are hidden")
assert(#assert(client:searchPeople("sam")) == 0, "Blocked people leave search")
for _, comment in ipairs(assert(client:listComments("p-hello"))) do
    assert(comment.authorId ~= "u-sam", "Blocked people's comments are hidden")
end
for _, item in ipairs(assert(client:activity())) do assert(item.userId ~= "u-sam", "Blocked people leave connect") end
local blocks = assert(client:listBlocks())
assert(#blocks == 1 and blocks[1].userId == "u-sam" and blocks[1].name == "Sam Rivera", "The block list names people")

-- Blocks belong to the account, not the device.
assert(client:signOut())
assert(has(assert(client:listPosts { limit = 20 }), "u-sam"), "Guests see everyone again")
assert(client:signIn("demo@example.invalid", "password123"))
assert(not client:isBlocked("u-sam"), "The block list is not loaded while building pages")
assert(not has(assert(client:listPosts { limit = 20 }), "u-sam") and client:isBlocked("u-sam"),
    "Blocks return with the account on the first list")
assert(client:setUserBlocked("u-sam", false))
assert(not client:isBlocked("u-sam") and #assert(client:listBlocks()) == 0)
assert(has(assert(client:listPosts { limit = 20 }), "u-sam"), "Unblocking restores posts")

-- Account deletion needs explicit confirmation and ends the session.
_, err = client:deleteAccount("yes")
assert(err.code == "validation" and client:getSession(), "Deletion needs the DELETE confirmation")
local deleted = assert(client:deleteAccount("DELETE"))
assert(deleted.accepted and not client:getSession() and state.deleted[me], "Deletion signs the device out")
assert(not has(assert(client:listPosts { limit = 20 }), me), "The account's posts are gone")
print("test_octopus_safety: ok")
