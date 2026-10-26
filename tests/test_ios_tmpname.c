#include <tools/ios/lua_config.h>
#include <assert.h>
#include <sys/stat.h>

int main(void) {
  char root[] = "/tmp/orca-ios-temp-XXXXXX";
  assert(mkdtemp(root));
  char directory[PATH_MAX];
  snprintf(directory, sizeof(directory), "%s/app container temporary files", root);
  assert(!mkdir(directory, 0700));
  assert(!setenv("TMPDIR", directory, 1));
  umask(0);

  char first[LUA_TMPNAMBUFSIZE], second[LUA_TMPNAMBUFSIZE];
  int error;
  lua_tmpnam(first, error);
  assert(!error && !strncmp(first, directory, strlen(directory)));
  struct stat info;
  assert(!stat(first, &info) && S_ISREG(info.st_mode) && (info.st_mode & 0777) == 0600);

  // Match the image cache's rename/write/read path, including binary NULs.
  char photo[PATH_MAX];
  assert(snprintf(photo, sizeof(photo), "%s.jpg", first) < (int)sizeof(photo));
  assert(!rename(first, photo));
  char const bytes[] = "\xff\xd8\xff\0photo\xff\xd9";
  FILE *file = fopen(photo, "wb");
  assert(file && fwrite(bytes, 1, sizeof(bytes), file) == sizeof(bytes));
  assert(!fclose(file));
  char restored[sizeof(bytes)];
  file = fopen(photo, "rb");
  assert(file && fread(restored, 1, sizeof(restored), file) == sizeof(restored));
  assert(!fclose(file) && !memcmp(bytes, restored, sizeof(bytes)));

  char trailing[PATH_MAX];
  assert(snprintf(trailing, sizeof(trailing), "%s/", directory) < (int)sizeof(trailing));
  assert(!setenv("TMPDIR", trailing, 1));
  lua_tmpnam(second, error);
  assert(!error && strcmp(first, second));
  assert(!unlink(photo) && !unlink(second));
  assert(!rmdir(directory));
  lua_tmpnam(second, error);
  assert(error); // An unavailable sandbox directory must not fall back to /tmp.

  char oversized[PATH_MAX + 1];
  memset(oversized, 'a', sizeof(oversized) - 1);
  oversized[0] = '/';
  oversized[sizeof(oversized) - 1] = 0;
  char const *invalid[] = { "", "relative/path", oversized };
  for (size_t i = 0; i < sizeof(invalid) / sizeof(*invalid); i++) {
    assert(!setenv("TMPDIR", invalid[i], 1));
    lua_tmpnam(second, error);
    assert(error);
  }
  assert(!unsetenv("TMPDIR"));
  lua_tmpnam(second, error);
  assert(error);
  assert(!rmdir(root));
  puts("PASS: iOS sandbox temporary files, private permissions, image cache writes and failure paths");
  return 0;
}
