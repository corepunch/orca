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

#if defined(ORCA_IOS)
static int media_picker_gc(lua_State *L) {
  void **request = lua_touserdata(L, 1);
  if (*request) Media_ReleasePhotoPicker(*request);
  *request = NULL;
  return 0;
}

static int media_picker_resume(lua_State *L, int status, lua_KContext context) {
  (void)status;
  void **request = (void **)context;
  struct MediaPhoto photo = {0};
  struct MediaError error = {0};
  int result = Media_PollPickedPhoto(*request, &photo, &error);
  if (!result) return lua_yieldk(L, 0, context, media_picker_resume);
  Media_ReleasePhotoPicker(*request);
  *request = NULL;
  return result > 0 ? media_push_photo(L, &photo) : media_push_error(L, &error);
}
#endif

int media_PickPhoto(lua_State *L) {
#if defined(ORCA_IOS)
  struct MediaError error = {0};
  if (!lua_isyieldable(L)) {
    error = (struct MediaError){ "not_yieldable", "Photo selection must run in an application coroutine." };
    return media_push_error(L, &error);
  }
  // The suspended C call retains this userdata; GC cancels abandoned requests.
  void **request = lua_newuserdatauv(L, sizeof(*request), 0);
  *request = NULL;
  if (luaL_newmetatable(L, "orca.media.PhotoPicker")) {
    lua_pushcfunction(L, media_picker_gc); lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
  *request = Media_BeginPickPhoto(&error);
  if (!*request) return media_push_error(L, &error);
  return lua_yieldk(L, 0, (lua_KContext)request, media_picker_resume);
#else
  struct MediaPhoto photo = {0};
  struct MediaError error = {0};
  if (!Media_PickPhoto(&photo, &error)) return media_push_error(L, &error);
  return media_push_photo(L, &photo);
#endif
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
