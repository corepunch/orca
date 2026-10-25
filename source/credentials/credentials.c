#include <include/api.h>
#include <credentials/credentials.h>
#include <string.h>

static int credentials_error(lua_State *L, char const *code, char const *message) {
  lua_pushnil(L);
  lua_createtable(L, 0, 2);
  lua_pushstring(L, code); lua_setfield(L, -2, "code");
  lua_pushstring(L, message); lua_setfield(L, -2, "message");
  return 2;
}

static int credentials_valid_key(lua_State *L, char const *key) {
  size_t length;
  lua_tolstring(L, 1, &length);
  return length > 0 && length <= 1024 && !memchr(key, '\0', length);
}

int credentials_Supported(lua_State *L) {
#if defined(__APPLE__)
  lua_pushboolean(L, 1);
#else
  lua_pushboolean(L, 0);
#endif
  return 1;
}

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>

static CFMutableDictionaryRef credentials_query(char const *key) {
  CFStringRef account = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
  if (!account) return NULL;
  CFBundleRef bundle = CFBundleGetMainBundle();
  CFStringRef service = bundle ? CFBundleGetIdentifier(bundle) : NULL;
  if (!service) service = CFSTR("org.orca.credentials");
  CFMutableDictionaryRef query = CFDictionaryCreateMutable(NULL, 0,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(query, kSecClass, kSecClassGenericPassword);
  CFDictionarySetValue(query, kSecAttrService, service);
  CFDictionarySetValue(query, kSecAttrAccount, account);
  CFDictionarySetValue(query, kSecAttrSynchronizable, kCFBooleanFalse);
  CFRelease(account);
  return query;
}

static int credentials_status(lua_State *L, OSStatus status) {
  if (status == errSecInteractionNotAllowed || status == errSecNotAvailable)
    return credentials_error(L, "locked", "Secure credential storage is currently unavailable. Unlock the device and try again.");
  return credentials_error(L, "storage_error", "The system could not access secure credential storage.");
}
#endif

int credentials_Get(lua_State *L, char const *key) {
  if (!credentials_valid_key(L, key))
    return credentials_error(L, "invalid_key", "A nonempty credential key of at most 1024 bytes is required.");
#if defined(__APPLE__)
  CFMutableDictionaryRef query = credentials_query(key);
  if (!query) return credentials_error(L, "invalid_key", "The credential key must be UTF-8.");
  CFDictionarySetValue(query, kSecReturnData, kCFBooleanTrue);
  CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitOne);
  CFTypeRef data = NULL;
  OSStatus status = SecItemCopyMatching(query, &data);
  CFRelease(query);
  if (status == errSecItemNotFound) { lua_pushnil(L); return 1; }
  if (status != errSecSuccess) return credentials_status(L, status);
  if (!data || CFGetTypeID(data) != CFDataGetTypeID()) {
    if (data) CFRelease(data);
    return credentials_error(L, "storage_error", "The system returned an invalid credential.");
  }
  lua_pushlstring(L, (char const *)CFDataGetBytePtr(data), (size_t)CFDataGetLength(data));
  CFRelease(data);
  return 1;
#else
  return credentials_error(L, "unsupported", "Secure credential storage is available on Apple platforms.");
#endif
}

int credentials_Set(lua_State *L, char const *key, char const *value) {
  if (!credentials_valid_key(L, key))
    return credentials_error(L, "invalid_key", "A nonempty credential key of at most 1024 bytes is required.");
  size_t length;
  lua_tolstring(L, 2, &length);
  if (length > 65536)
    return credentials_error(L, "value_too_large", "Credentials must not exceed 64 KiB.");
#if defined(__APPLE__)
  CFMutableDictionaryRef query = credentials_query(key);
  if (!query) return credentials_error(L, "invalid_key", "The credential key must be UTF-8.");
  CFDataRef data = CFDataCreate(NULL, (UInt8 const *)value, (CFIndex)length);
  CFMutableDictionaryRef attributes = CFDictionaryCreateMutable(NULL, 0,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(attributes, kSecValueData, data);
  CFDictionarySetValue(attributes, kSecAttrAccessible, kSecAttrAccessibleWhenUnlockedThisDeviceOnly);
  OSStatus status = SecItemUpdate(query, attributes);
  if (status == errSecItemNotFound) {
    CFDictionarySetValue(query, kSecValueData, data);
    CFDictionarySetValue(query, kSecAttrAccessible, kSecAttrAccessibleWhenUnlockedThisDeviceOnly);
    status = SecItemAdd(query, NULL);
    if (status == errSecDuplicateItem) {
      CFDictionaryRemoveValue(query, kSecValueData);
      CFDictionaryRemoveValue(query, kSecAttrAccessible);
      status = SecItemUpdate(query, attributes);
    }
  }
  CFRelease(attributes);
  CFRelease(data);
  CFRelease(query);
  if (status != errSecSuccess) return credentials_status(L, status);
  lua_pushboolean(L, 1);
  return 1;
#else
  (void)value;
  return credentials_error(L, "unsupported", "Secure credential storage is available on Apple platforms.");
#endif
}

int credentials_Delete(lua_State *L, char const *key) {
  if (!credentials_valid_key(L, key))
    return credentials_error(L, "invalid_key", "A nonempty credential key of at most 1024 bytes is required.");
#if defined(__APPLE__)
  CFMutableDictionaryRef query = credentials_query(key);
  if (!query) return credentials_error(L, "invalid_key", "The credential key must be UTF-8.");
  OSStatus status = SecItemDelete(query);
  CFRelease(query);
  if (status != errSecSuccess && status != errSecItemNotFound) return credentials_status(L, status);
  lua_pushboolean(L, 1);
  return 1;
#else
  return credentials_error(L, "unsupported", "Secure credential storage is available on Apple platforms.");
#endif
}

#include <credentials/credentials_export.c>
