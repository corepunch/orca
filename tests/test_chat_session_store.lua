package.path = "apps/chat/?.lua;" .. package.path
local Store = require "session_store"
local json = require "api.json"
local records = {}
local native = {
    supported = function() return true end,
    get = function(key) return records[key] end,
    set = function(key, value) records[key] = value; return true end,
    delete = function(key) records[key] = nil; return true end,
}
local config = { endpoint = "https://fra.cloud.appwrite.io/v1", projectId = "project-one" }
local first = assert(Store.new(config, native))
assert(first.load() == nil)
assert(first.save { Cookie = "a_session_project-one=private-cookie" })
local restarted = assert(Store.new(config, native))
assert(restarted.load().Cookie == "a_session_project-one=private-cookie")
assert(Store.new({ endpoint = config.endpoint .. "/", projectId = config.projectId }, native).load().Cookie)
assert(Store.new({ endpoint = config.endpoint, projectId = "another" }, native).load() == nil)
assert(Store.new({ endpoint = "https://nyc.cloud.appwrite.io/v1", projectId = config.projectId }, native).load() == nil)
for _, value in ipairs {
    { Cookie = "a_session_other=secret" }, { Authorization = "Bearer jwt" },
    { Cookie = "a_session_project-one=secret; extra=bad" },
    { ["X-Appwrite-Session"] = "bad\r\nheader" },
    { Cookie = "a_session_project-one=secret", password = "never-store" },
} do assert(not first.save(value)) end
local key = next(records)
records[key] = "malformed JSON"
assert(restarted.load() == nil and records[key] == nil)
records[key] = json.encode { Cookie = "a_session_other=secret" }
assert(restarted.load() == nil and records[key] == nil)
assert(first.save { ["X-Appwrite-Session"] = "private-secret" })
assert(restarted.load()["X-Appwrite-Session"] == "private-secret")
assert(restarted.clear() and first.load() == nil)
native.get = function() return nil, { code = "locked" } end
local value, err = first.load()
assert(value == nil and err.code == "locked")
native.supported = function() return false end
assert(Store.new(config, native) == nil)
print("Chat secure session store checks passed")
