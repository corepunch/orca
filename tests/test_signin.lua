local signin = require "orca.signin"
assert(type(signin.supported()) == "boolean")
assert(signin.poll() == nil, "An empty native queue must return nil")
local ok, message = signin.open("http://example.invalid/login", "appwrite-callback-test")
assert(ok == nil and type(message) == "string", "Sign-in pages must use https")
ok, message = signin.open("https://example.invalid/login", "Not A Scheme")
assert(ok == nil and type(message) == "string", "Callback schemes must be lowercase URL schemes")
if not signin.supported() then
    ok, message = signin.open("https://example.invalid/login", "appwrite-callback-test")
    assert(ok == nil and type(message) == "string" and #message > 0)
    assert(signin.poll() == nil, "Unsupported requests must not enqueue a fake callback")
end
print("Sign-in module contract passed")
