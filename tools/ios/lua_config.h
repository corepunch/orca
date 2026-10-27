#ifndef ORCA_IOS_LUA_CONFIG_H
#define ORCA_IOS_LUA_CONFIG_H

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Lua's POSIX default uses /tmp, which is outside the iOS app sandbox.
#define LUA_TMPNAMBUFSIZE PATH_MAX
static inline int orca_lua_tmpnam(char *path, size_t capacity) {
  char const *directory = getenv("TMPDIR");
  if (!directory || directory[0] != '/') return 1;
  size_t length = strlen(directory);
  int size = snprintf(path, capacity, "%s%slua_XXXXXX", directory,
      directory[length - 1] == '/' ? "" : "/");
  if (size < 0 || (size_t)size >= capacity) return 1;
  int fd = mkstemp(path);
  if (fd == -1) return 1;
  close(fd);
  return 0;
}
#define lua_tmpnam(path, error) ((error) = orca_lua_tmpnam((path), sizeof(path)))

#endif
