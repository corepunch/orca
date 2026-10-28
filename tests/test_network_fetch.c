/* Run via tests/test_network_headers.py --native-binary build/bin/test_network_fetch. */
#include <include/orca.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

/* The C fetch API is exported but has no public aggregate header yet. */
extern void NET_Init(void);
extern void NET_Shutdown(void);
extern void* NET_FetchBegin(char const* url);
extern bool_t NET_FetchPoll(void* handle);
extern long NET_FetchFinish(void* handle, void** data, size_t* size);
extern void NET_FetchCancel(void* handle);

static void*
fetch(char const* base, char const* path)
{
  char url[256];
  snprintf(url, sizeof(url), "%s%s", base, path);
  void* handle = NET_FetchBegin(url);
  assert(handle);
  return handle;
}

static void
complete(void* handle)
{
  time_t deadline = time(NULL) + 10;
  while (NET_FetchPoll(handle)) {
    assert(time(NULL) < deadline);
    usleep(1000);
  }
}

int
main(int argc, char** argv)
{
  assert(argc == 2);
  void* slow = fetch(argv[1], "/hold");
  void* fast = fetch(argv[1], "/fast");
  complete(fast);
  void* data = NULL;
  size_t size = 0;
  assert(NET_FetchFinish(fast, &data, &size) == 200);
  assert(size && strstr(data, "/fast"));
  free(data);
  assert(NET_FetchPoll(slow));
  NET_FetchCancel(slow);

  fast = fetch(argv[1], "/binary");
  complete(fast);
  static unsigned char const binary[] = {0, 'n', 'a', 't', 'i', 'v', 'e', 255, 0};
  assert(NET_FetchFinish(fast, &data, &size) == 200);
  assert(size == sizeof(binary) && !memcmp(data, binary, size));
  free(data);

  fast = fetch(argv[1], "/drop");
  complete(fast);
  assert(NET_FetchFinish(fast, &data, &size) == -1);
  assert(!data && !size);

  slow = fetch(argv[1], "/hold");
  NET_Shutdown();
  assert(!NET_FetchPoll(slow));
  assert(NET_FetchFinish(slow, &data, &size) == -1);
  assert(!data && !size);

  fast = fetch(argv[1], "/recovered");
  complete(fast);
  assert(NET_FetchFinish(fast, &data, &size) == 200);
  assert(size && strstr(data, "/recovered"));
  free(data);
  NET_FetchCancel(NULL);
  NET_Shutdown();
  puts("PASS: native C fetch concurrency, gzip binary, cancellation, shutdown and failure recovery");
  return 0;
}
