local crypto = require "orca.crypto"

local function hex(s) return (s:gsub(".", function(c) return string.format("%02x", c:byte()) end)) end
local function unhex(h) return (h:gsub("..", function(x) return string.char(tonumber(x, 16)) end)) end
local function fails(code, result, err)
    assert(result == nil and type(err) == "table" and err.code == code,
        "Expected " .. code .. ", got " .. tostring(err and err.code or result))
end

-- Random bytes
local r1, r2 = assert(crypto.random(32)), assert(crypto.random(32))
assert(#r1 == 32 and r1 ~= r2, "Random output must have the requested size and differ")
assert(#assert(crypto.random(65536)) == 65536)
fails("invalid_size", crypto.random(0))
fails("invalid_size", crypto.random(65537))

-- BLAKE2b (RFC 7693 appendix A and the reference implementation's empty input)
assert(hex(crypto.hash("abc", 64)) == "ba80a53f981c4d0d6a2797b69f12f6e94c212f14685ac4b74b12bb6fdbffa2d1"
    .. "7d87c5392aab792dc252d5de4533cc9518d38aa8dbf1925ab92386edd4009923")
assert(hex(crypto.hash("", 64)) == "786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419"
    .. "d25e1031afee585313896444934eb04b903a685b1448b755d56f701afe9be2ce")
assert(#crypto.hash("abc") == 32, "The default hash size is 32 bytes")
fails("invalid_size", crypto.hash("abc", 0))
fails("invalid_size", crypto.hash("abc", 65))

-- X25519 (RFC 7748 section 6.1)
local aliceSecret = unhex("77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a")
local alicePublic = unhex("8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a")
local bobSecret = unhex("5dab087e624a8a4b79e17f8b83800ee66f3bb1292618b6fd1c2f8b27ff88e0eb")
local bobPublic = unhex("de9edb7d7b7dc1b4d35b61c2ece435373f8343c85b78674dadfc7e146f882b4f")
local raw = unhex("4a5d9d5ba4ce2de1728e3bf480350f25e07e21c947d19e3376f09b3c1e161742")
assert(crypto.boxPublicKey(aliceSecret) == alicePublic)
assert(crypto.boxPublicKey(bobSecret) == bobPublic)
fails("invalid_key", crypto.boxPublicKey("short"))

local context = "orca-chat/v1\0room"
local aliceKey = assert(crypto.sharedKey(aliceSecret, bobPublic, context))
assert(aliceKey == assert(crypto.sharedKey(bobSecret, alicePublic, context)), "Both parties must derive the same key")
-- alicePublic (0x85...) sorts before bobPublic (0xde...)
assert(aliceKey == crypto.hash(raw .. alicePublic .. bobPublic .. context), "sharedKey must hash raw || sorted publics || context")
assert(aliceKey ~= crypto.sharedKey(aliceSecret, bobPublic, context .. "x"), "The context must change the key")
fails("invalid_key", crypto.sharedKey(aliceSecret, string.rep("\0", 32), context))
fails("invalid_key", crypto.sharedKey(aliceSecret, "short", context))
fails("invalid_context", crypto.sharedKey(aliceSecret, bobPublic, string.rep("c", 1025)))

local s1, p1 = assert(crypto.boxKeypair())
local s2, p2 = assert(crypto.boxKeypair())
assert(#s1 == 32 and #p1 == 32 and crypto.boxPublicKey(s1) == p1 and p1 ~= p2)
assert(crypto.sharedKey(s1, p2, "") == crypto.sharedKey(s2, p1, ""))

-- XChaCha20-Poly1305
local key = assert(crypto.random(32))
local message = "hello\0world"
local sealed = assert(crypto.seal(key, message, "ad"))
assert(#sealed == #message + 40, "Sealed data is nonce + ciphertext + tag")
assert(sealed ~= crypto.seal(key, message, "ad"), "Every seal uses a fresh nonce")
assert(not sealed:find(message, 1, true), "Plaintext must not appear in sealed data")
assert(crypto.open(key, sealed, "ad") == message)
assert(crypto.open(key, assert(crypto.seal(key, "", "")), "") == "", "Empty plaintexts round-trip")
fails("authentication_failed", crypto.open(key, sealed, "other"))
fails("authentication_failed", crypto.open(assert(crypto.random(32)), sealed, "ad"))
for _, at in ipairs({ 1, 24, 25, #message + 24, #sealed }) do
    local flipped = sealed:sub(1, at - 1) .. string.char(sealed:byte(at) ~ 1) .. sealed:sub(at + 1)
    fails("authentication_failed", crypto.open(key, flipped, "ad"))
end
fails("authentication_failed", crypto.open(key, sealed:sub(1, 39), "ad"))
fails("authentication_failed", crypto.open(key, sealed:sub(1, -2), "ad"))
fails("invalid_key", crypto.seal("short", message, ""))
fails("invalid_key", crypto.open("short", sealed, ""))
fails("too_large", crypto.seal(key, "", string.rep("a", 65537)))

-- Ed25519 (RFC 8032 section 7.1, test 1 and test 2)
local seed = unhex("9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60")
local public = unhex("d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a")
local signature = unhex("e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555"
    .. "fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b")
assert(crypto.sign(seed .. public, "") == signature)
assert(crypto.verify(public, "", signature) == true)
local seed2 = unhex("4ccd089b28ff96da9db6c346ec114e0f5b8a319f35aba624da8cf6ed4fb8a6fb")
local public2 = unhex("3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c")
local signature2 = unhex("92a009a9f0d4cab8720e820b5f642540a2b27b5416503f8fb3762223ebdb69da"
    .. "085ac1e43e15996e458f3613d0f11d8c387b2eaeb4302aeeb00d291612bb0c00")
assert(crypto.sign(seed2 .. public2, "\x72") == signature2)
assert(crypto.verify(public2, "\x72", signature2) == true)
assert(crypto.verify(public2, "\x73", signature2) == false, "A different message must fail")
assert(crypto.verify(public, "\x72", signature2) == false, "A different key must fail")
assert(crypto.verify(public2, "\x72", signature2:sub(1, 63)) == false, "A short signature must fail")
fails("invalid_key", crypto.sign(seed, ""))

local signSecret, signPublic = assert(crypto.signKeypair())
assert(#signSecret == 64 and #signPublic == 32 and signSecret:sub(33) == signPublic)
assert(crypto.verify(signPublic, "envelope", assert(crypto.sign(signSecret, "envelope"))))

-- Constant-time comparison
assert(crypto.equal("abc", "abc") == true)
assert(crypto.equal("abc", "abd") == false)
assert(crypto.equal("abc", "ab") == false)
assert(crypto.equal("", "") == true)

print("Crypto tests passed")
