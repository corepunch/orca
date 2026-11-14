#include <include/api.h>
#include <crypto/crypto.h>
#include <monocypher.h>
#include <optional/monocypher-ed25519.h>
#include <stdlib.h>
#include <string.h>

#if defined(__APPLE__) || defined(__ANDROID__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#define CRYPTO_ARC4RANDOM 1
#elif defined(__linux__)
#include <errno.h>
#include <sys/random.h>
#elif defined(__EMSCRIPTEN__)
#include <unistd.h>
#endif

#define CRYPTO_MAX_DATA (16u << 20)
#define CRYPTO_MAX_AD (64u << 10)
#define CRYPTO_NONCE 24
#define CRYPTO_TAG 16

static int crypto_error(lua_State *L, char const *code, char const *message) {
  lua_pushnil(L);
  lua_createtable(L, 0, 2);
  lua_pushstring(L, code); lua_setfield(L, -2, "code");
  lua_pushstring(L, message); lua_setfield(L, -2, "message");
  return 2;
}

static uint8_t const *crypto_bytes(lua_State *L, int index, size_t *size) {
  return (uint8_t const *)luaL_checklstring(L, index, size);
}

// Fills buffer from the operating system; never falls back to a weak source.
static int crypto_fill(uint8_t *buffer, size_t size) {
#if defined(CRYPTO_ARC4RANDOM)
  arc4random_buf(buffer, size);
  return TRUE;
#elif defined(__linux__)
  while (size) {
    ssize_t n = getrandom(buffer, size, 0);
    if (n < 0 && errno == EINTR) continue;
    if (n <= 0) return FALSE;
    buffer += n, size -= (size_t)n;
  }
  return TRUE;
#elif defined(__EMSCRIPTEN__)
  for (size_t n; size; buffer += n, size -= n) {
    n = size < 256 ? size : 256;
    if (getentropy(buffer, n)) return FALSE;
  }
  return TRUE;
#else
  (void)buffer, (void)size;
  return FALSE;
#endif
}

static int crypto_no_random(lua_State *L) {
  return crypto_error(L, "unsupported", "This platform has no secure random number source.");
}

static int crypto_push_wiped(lua_State *L, uint8_t *bytes, size_t size) {
  lua_pushlstring(L, (char const *)bytes, size);
  crypto_wipe(bytes, size);
  return 1;
}

int crypto_Random(lua_State *L, int size) {
  if (size < 1 || size > 65536)
    return crypto_error(L, "invalid_size", "Random sizes range from 1 to 65536 bytes.");
  uint8_t *buffer = malloc((size_t)size);
  if (!buffer) return luaL_error(L, "out of memory");
  if (!crypto_fill(buffer, (size_t)size)) return free(buffer), crypto_no_random(L);
  crypto_push_wiped(L, buffer, (size_t)size);
  free(buffer);
  return 1;
}

int crypto_BoxKeypair(lua_State *L) {
  uint8_t secret[32], public_key[32];
  if (!crypto_fill(secret, sizeof(secret))) return crypto_no_random(L);
  crypto_x25519_public_key(public_key, secret);
  crypto_push_wiped(L, secret, sizeof(secret));
  lua_pushlstring(L, (char const *)public_key, sizeof(public_key));
  return 2;
}

int crypto_BoxPublicKey(lua_State *L, char const *secret_arg) {
  size_t size;
  uint8_t const *secret = crypto_bytes(L, 1, &size);
  (void)secret_arg;
  if (size != 32) return crypto_error(L, "invalid_key", "X25519 secret keys are 32 bytes.");
  uint8_t public_key[32];
  crypto_x25519_public_key(public_key, secret);
  lua_pushlstring(L, (char const *)public_key, sizeof(public_key));
  return 1;
}

int crypto_SharedKey(lua_State *L, char const *secret_arg, char const *peer_arg, char const *context_arg) {
  size_t secret_size, peer_size, context_size;
  uint8_t const *secret = crypto_bytes(L, 1, &secret_size);
  uint8_t const *peer = crypto_bytes(L, 2, &peer_size);
  uint8_t const *context = crypto_bytes(L, 3, &context_size);
  (void)secret_arg, (void)peer_arg, (void)context_arg;
  if (secret_size != 32 || peer_size != 32)
    return crypto_error(L, "invalid_key", "X25519 keys are 32 bytes.");
  if (context_size > 1024)
    return crypto_error(L, "invalid_context", "Contexts are at most 1024 bytes.");
  uint8_t own[32], raw[32], key[32], zero[32] = { 0 };
  crypto_x25519_public_key(own, secret);
  crypto_x25519(raw, secret, peer);
  if (!crypto_verify32(raw, zero))
    return crypto_wipe(raw, sizeof(raw)), crypto_error(L, "invalid_key", "The peer public key is not usable.");
  int own_first = memcmp(own, peer, 32) <= 0;
  crypto_blake2b_ctx ctx;
  crypto_blake2b_init(&ctx, sizeof(key));
  crypto_blake2b_update(&ctx, raw, sizeof(raw));
  crypto_blake2b_update(&ctx, own_first ? own : peer, 32);
  crypto_blake2b_update(&ctx, own_first ? peer : own, 32);
  crypto_blake2b_update(&ctx, context, context_size);
  crypto_blake2b_final(&ctx, key);
  crypto_wipe(raw, sizeof(raw));
  return crypto_push_wiped(L, key, sizeof(key));
}

int crypto_Seal(lua_State *L, char const *key_arg, char const *plaintext_arg, char const *ad_arg) {
  size_t key_size, size, ad_size;
  uint8_t const *key = crypto_bytes(L, 1, &key_size);
  uint8_t const *plaintext = crypto_bytes(L, 2, &size);
  uint8_t const *ad = crypto_bytes(L, 3, &ad_size);
  (void)key_arg, (void)plaintext_arg, (void)ad_arg;
  if (key_size != 32) return crypto_error(L, "invalid_key", "Symmetric keys are 32 bytes.");
  if (size > CRYPTO_MAX_DATA) return crypto_error(L, "too_large", "Plaintexts are at most 16 MiB.");
  if (ad_size > CRYPTO_MAX_AD) return crypto_error(L, "too_large", "Associated data is at most 64 KiB.");
  size_t total = CRYPTO_NONCE + size + CRYPTO_TAG;
  uint8_t *out = malloc(total);
  if (!out) return luaL_error(L, "out of memory");
  if (!crypto_fill(out, CRYPTO_NONCE)) return free(out), crypto_no_random(L);
  crypto_aead_lock(out + CRYPTO_NONCE, out + CRYPTO_NONCE + size, key, out, ad, ad_size, plaintext, size);
  lua_pushlstring(L, (char const *)out, total);
  free(out);
  return 1;
}

int crypto_Open(lua_State *L, char const *key_arg, char const *sealed_arg, char const *ad_arg) {
  size_t key_size, total, ad_size;
  uint8_t const *key = crypto_bytes(L, 1, &key_size);
  uint8_t const *sealed = crypto_bytes(L, 2, &total);
  uint8_t const *ad = crypto_bytes(L, 3, &ad_size);
  (void)key_arg, (void)sealed_arg, (void)ad_arg;
  if (key_size != 32) return crypto_error(L, "invalid_key", "Symmetric keys are 32 bytes.");
  if (ad_size > CRYPTO_MAX_AD) return crypto_error(L, "too_large", "Associated data is at most 64 KiB.");
  if (total < CRYPTO_NONCE + CRYPTO_TAG || total - CRYPTO_NONCE - CRYPTO_TAG > CRYPTO_MAX_DATA)
    return crypto_error(L, "authentication_failed", "The sealed data could not be verified.");
  size_t size = total - CRYPTO_NONCE - CRYPTO_TAG;
  uint8_t *out = malloc(size ? size : 1);
  if (!out) return luaL_error(L, "out of memory");
  if (crypto_aead_unlock(out, sealed + CRYPTO_NONCE + size, key, sealed, ad, ad_size, sealed + CRYPTO_NONCE, size))
    return free(out), crypto_error(L, "authentication_failed", "The sealed data could not be verified.");
  crypto_push_wiped(L, out, size);
  free(out);
  return 1;
}

int crypto_SignKeypair(lua_State *L) {
  uint8_t seed[32], secret[64], public_key[32];
  if (!crypto_fill(seed, sizeof(seed))) return crypto_no_random(L);
  crypto_ed25519_key_pair(secret, public_key, seed); // wipes seed
  crypto_push_wiped(L, secret, sizeof(secret));
  lua_pushlstring(L, (char const *)public_key, sizeof(public_key));
  return 2;
}

int crypto_Sign(lua_State *L, char const *secret_arg, char const *message_arg) {
  size_t secret_size, size;
  uint8_t const *secret = crypto_bytes(L, 1, &secret_size);
  uint8_t const *message = crypto_bytes(L, 2, &size);
  (void)secret_arg, (void)message_arg;
  if (secret_size != 64) return crypto_error(L, "invalid_key", "Ed25519 secret keys are 64 bytes.");
  if (size > CRYPTO_MAX_DATA) return crypto_error(L, "too_large", "Messages are at most 16 MiB.");
  uint8_t signature[64];
  crypto_ed25519_sign(signature, secret, message, size);
  lua_pushlstring(L, (char const *)signature, sizeof(signature));
  return 1;
}

int crypto_Verify(lua_State *L, char const *public_arg, char const *message_arg, char const *signature_arg) {
  size_t public_size, size, signature_size;
  uint8_t const *public_key = crypto_bytes(L, 1, &public_size);
  uint8_t const *message = crypto_bytes(L, 2, &size);
  uint8_t const *signature = crypto_bytes(L, 3, &signature_size);
  (void)public_arg, (void)message_arg, (void)signature_arg;
  lua_pushboolean(L, public_size == 32 && signature_size == 64 && size <= CRYPTO_MAX_DATA
                     && !crypto_ed25519_check(signature, public_key, message, size));
  return 1;
}

int crypto_Hash(lua_State *L, char const *data_arg) {
  size_t size;
  uint8_t const *data = crypto_bytes(L, 1, &size);
  lua_Integer hash_size = luaL_optinteger(L, 2, 32);
  (void)data_arg;
  if (hash_size < 1 || hash_size > 64) return crypto_error(L, "invalid_size", "Hash sizes range from 1 to 64 bytes.");
  if (size > CRYPTO_MAX_DATA) return crypto_error(L, "too_large", "Inputs are at most 16 MiB.");
  uint8_t hash[64];
  crypto_blake2b(hash, (size_t)hash_size, data, size);
  lua_pushlstring(L, (char const *)hash, (size_t)hash_size);
  return 1;
}

int crypto_Equal(lua_State *L, char const *a_arg, char const *b_arg) {
  size_t a_size, b_size;
  uint8_t const *a = crypto_bytes(L, 1, &a_size);
  uint8_t const *b = crypto_bytes(L, 2, &b_size);
  (void)a_arg, (void)b_arg;
  uint8_t diff = 0;
  if (a_size == b_size) for (size_t i = 0; i < a_size; i++) diff |= a[i] ^ b[i];
  lua_pushboolean(L, a_size == b_size && !diff);
  return 1;
}

#include <crypto/crypto_export.c>
