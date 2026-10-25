local credentials = require "orca.credentials"
local function fails(code, operation, ...)
    local result, err = operation(...)
    assert(result == nil and type(err) == "table" and err.code == code, "Expected " .. code)
end

assert(type(credentials.supported()) == "boolean")
fails("invalid_key", credentials.get, "")
fails("invalid_key", credentials.set, "a\0b", "test")
fails("invalid_key", credentials.delete, string.rep("k", 1025))
fails("value_too_large", credentials.set, "orca-test", string.rep("v", 65537))

if not credentials.supported() then
    fails("unsupported", credentials.get, "orca-test")
    fails("unsupported", credentials.set, "orca-test", "test")
    fails("unsupported", credentials.delete, "orca-test")
else
    -- Use unique dummy entries and remove them even when an assertion fails.
    local key = "orca-credential-test:" .. tostring(os.time()) .. ":" .. tostring({})
    local ok, err = pcall(function()
        assert(credentials.get(key) == nil)
        assert(credentials.set(key, "test\0credential"))
        assert(credentials.get(key) == "test\0credential", "Binary values must survive a Keychain round trip")
        assert(credentials.set(key, "replacement"))
        assert(credentials.get(key) == "replacement", "Saving an existing key must replace it")
        assert(credentials.get(key .. ":other") == nil, "Credential keys must be isolated")
        assert(credentials.delete(key))
        local missing, readError = credentials.get(key)
        assert(missing == nil and readError == nil, "An absent credential must not be reported as a storage error")
        assert(credentials.delete(key), "Deleting an absent credential must succeed")
    end)
    local removed, removeError = credentials.delete(key)
    assert(removed, removeError and removeError.message)
    assert(ok, err)
end
print("Credential storage tests passed")
