#include <include/api.h>
#include <media/media.h>
#include <stdlib.h>
#include "media_native.h"

static int media_push_error(lua_State *L, struct MediaError const *error) {
  lua_pushnil(L);
  lua_createtable(L, 0, 2);
  lua_pushstring(L, error->code); lua_setfield(L, -2, "code");
  lua_pushstring(L, error->message); lua_setfield(L, -2, "message");
  return 2;
}

static int media_push_photo(lua_State *L, struct MediaPhoto *photo) {
  lua_createtable(L, 0, 10);
  lua_pushlstring(L, (char const *)photo->jpeg, photo->byte_count); lua_setfield(L, -2, "jpeg");
  lua_pushlstring(L, (char const *)photo->thumbnail, photo->thumbnail_byte_count); lua_setfield(L, -2, "thumbnail");
  lua_pushinteger(L, photo->width); lua_setfield(L, -2, "width");
  lua_pushinteger(L, photo->height); lua_setfield(L, -2, "height");
  lua_pushinteger(L, photo->byte_count); lua_setfield(L, -2, "byte_count");
  lua_pushinteger(L, photo->thumbnail_width); lua_setfield(L, -2, "thumbnail_width");
  lua_pushinteger(L, photo->thumbnail_height); lua_setfield(L, -2, "thumbnail_height");
  lua_pushinteger(L, photo->thumbnail_byte_count); lua_setfield(L, -2, "thumbnail_byte_count");
  lua_pushnumber(L, photo->quality); lua_setfield(L, -2, "quality");
  lua_pushliteral(L, "image/jpeg"); lua_setfield(L, -2, "mime_type");
  free(photo->jpeg);
  free(photo->thumbnail);
  return 1;
}

int media_ProcessPhoto(lua_State *L, char const *bytes) {
  size_t size;
  lua_tolstring(L, 1, &size);
  if (!size || size > MEDIA_INPUT_LIMIT) {
    struct MediaError error = { size ? "input_too_large" : "invalid_image", "Choose a still image no larger than 32 MiB." };
    return media_push_error(L, &error);
  }
  struct MediaPhoto photo = {0};
  struct MediaError error = {0};
  if (!Media_ProcessPhoto(bytes, size, &photo, &error)) return media_push_error(L, &error);
  return media_push_photo(L, &photo);
}

int media_PickPhoto(lua_State *L) {
  struct MediaPhoto photo = {0};
  struct MediaError error = {0};
  if (!Media_PickPhoto(&photo, &error)) return media_push_error(L, &error);
  return media_push_photo(L, &photo);
}

#if !defined(__APPLE__)
int Media_ProcessPhoto(void const *bytes, size_t size, struct MediaPhoto *photo, struct MediaError *error) {
  (void)bytes; (void)size; (void)photo;
  *error = (struct MediaError){ "unsupported", "Photo processing is available on Apple platforms." };
  return 0;
}

int Media_PickPhoto(struct MediaPhoto *photo, struct MediaError *error) {
  (void)photo;
  *error = (struct MediaError){ "unsupported", "Photo picking is available on macOS." };
  return 0;
}
#endif

#include <media/media_export.c>
