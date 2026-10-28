#include <include/api.h>
#include <include/orca.h>

#define MAX_DATANAME 64
#define API_TYPE_RESPONSE "Response"
#define API_TYPE_SERVICECLIENT "ServiceClient"

// #define DEBUG_FETCH

typedef struct
{
  size_t size;
  size_t header_size;
  long code;
  char data[];
} response_t;

#ifndef __EMSCRIPTEN__

void
SZ_Init(struct AXbuffer* buf, void* data, int length)
{
  memset(buf, 0, sizeof(*buf));
  buf->data = data;
  buf->maxsize = length;
}

void
SZ_Clear(struct AXbuffer* buf)
{
  buf->cursize = 0;
}

void*
SZ_GetSpace(struct AXbuffer* buf, int length)
{
  if (buf->cursize + length > buf->maxsize)
    SZ_Clear(buf);
  return (LPSTR)buf->data + (buf->cursize += length) - length;
}

static void net_close_socket(int net_socket)
{
  axNetClose(net_socket);
}

static int net_connect(lpcString_t addr, int port)
{
  int sock = axNetSocket(AX_NET_AF_IPV4, AX_NET_SOCK_TCP);
  if (sock < 0)
    return -1;
  if (!axNetConnect(sock, addr, (uint16_t)port)) {
    axNetClose(sock);
    return -1;
  }
  return sock;
}

static int net_packet(int net_socket, struct AXbuffer* net_message)
{
  int n = axNetRecv(net_socket, net_message->data, net_message->maxsize);
  if (n > 0) {
    net_message->cursize = n;
  }
  return n;
}

static void
service_read_server_message(struct network_service* service,
                            struct AXbuffer* net_message)
{
}

void
_ServiceRead(struct network_service* service)
{
  struct AXbuffer net_message;
  byte_t net_message_buffer[MAX_MSGLEN];
  SZ_Init(&net_message, net_message_buffer, MAX_MSGLEN);
  do {
    switch (net_packet(service->sockfd, &net_message)) {
      case 0:
        Con_Error(API_TYPE_SERVICECLIENT " dropped connection");
        return;
      case -1:
        Con_Error(API_TYPE_SERVICECLIENT "::GetPacket() error");
        return;
      default:
        service_read_server_message(service, &net_message);
        SZ_Clear(&net_message);
        break;
    }
  } while (!service->stopped);
}

typedef void (*reader_t)(void*);

HRESULT
StartService(struct network_service* sv, SERVICEREADPROC read)
{
  sv->sockfd = net_connect(sv->ipaddr, sv->port);
  sv->read = read;
  // CL_RequestBaseline(sv->net_socket);
  //        sv->thread = thread_add((reader_t)read, sv);
  return NOERROR;
}

HRESULT
StopService(struct network_service* service)
{
  service->stopped = 1;
  if (service->sockfd != -1) {
    net_close_socket(service->sockfd);
  }
  return NOERROR;
}

#include <curl/curl.h>
#include <source/network/network.h>

#define INITIAL_BUFFER_SIZE 1024

typedef struct
{
  LPSTR data;      // Dynamic response buffer
  size_t size;     // Current size of the buffer
  size_t capacity; // Capacity of the buffer
} buffer_t;

typedef struct request_s
{
  CURL* easy_handle;
  CURLM* multi_handle;
  struct curl_slist* headers;
  buffer_t response;
  buffer_t header;
  struct request_s* next;
  struct request_s* previous;
  CURLcode result;
  long code;
  bool_t done;
} request_t;

static CURLM* fetch_pool;
static request_t* fetch_requests;
static int curl_initialized;

static void
free_request(request_t* req)
{
  if (req->multi_handle) {
    curl_multi_remove_handle(req->multi_handle, req->easy_handle);
  }
  if (req->previous) req->previous->next = req->next;
  else if (fetch_requests == req) fetch_requests = req->next;
  if (req->next) req->next->previous = req->previous;
  if (req->easy_handle) curl_easy_cleanup(req->easy_handle);
  curl_slist_free_all(req->headers);
  free(req->header.data);
  free(req->response.data);
  free(req);
}

static request_t*
alloc_request(void)
{
  request_t* req = calloc(1, sizeof(*req));
  if (!req) return NULL;
  req->next = fetch_requests;
  if (fetch_requests) fetch_requests->previous = req;
  fetch_requests = req;
  return req;
}

/* The multi handle owns connection, DNS and TLS caches across requests. All
   fetches run on the event-loop thread; completion belongs to the easy handle,
   never to the aggregate number of transfers still running. */
static void
poll_requests(void)
{
  if (!fetch_pool) return;
  int running;
  CURLMcode result = curl_multi_perform(fetch_pool, &running);
  if (result != CURLM_OK) {
    for (request_t* req = fetch_requests; req; req = req->next) {
      req->result = CURLE_FAILED_INIT;
      req->done = TRUE;
    }
    return;
  }
  CURLMsg* message;
  while ((message = curl_multi_info_read(fetch_pool, &running))) {
    if (message->msg != CURLMSG_DONE) continue;
    request_t* req = NULL;
    curl_easy_getinfo(message->easy_handle, CURLINFO_PRIVATE, &req);
    if (!req) continue;
    req->result = message->data.result;
    req->done = TRUE;
    curl_easy_getinfo(message->easy_handle, CURLINFO_RESPONSE_CODE, &req->code);
  }
}

static bool_t
start_request(request_t* req)
{
  NET_Init();
  if (!curl_initialized) return FALSE;
  if (!fetch_pool) fetch_pool = curl_multi_init();
  if (!fetch_pool) return FALSE;
  curl_easy_setopt(req->easy_handle, CURLOPT_PRIVATE, req);
  curl_easy_setopt(req->easy_handle, CURLOPT_ACCEPT_ENCODING, "");
  curl_easy_setopt(req->easy_handle, CURLOPT_CONNECTTIMEOUT, 15L);
  curl_easy_setopt(req->easy_handle, CURLOPT_TIMEOUT, 120L);
  curl_easy_setopt(req->easy_handle, CURLOPT_NOSIGNAL, 1L);
  if (curl_multi_add_handle(fetch_pool, req->easy_handle) != CURLM_OK) return FALSE;
  req->multi_handle = fetch_pool;
  poll_requests();
  return TRUE;
}

static int
collect_request(lua_State* L)
{
  request_t** owner = lua_touserdata(L, 1);
  if (*owner) free_request(*owner);
  *owner = NULL;
  return 0;
}

void
alloc_buffer(buffer_t* buffer)
{
  buffer->data = malloc(INITIAL_BUFFER_SIZE);
  buffer->size = 0;
  buffer->capacity = INITIAL_BUFFER_SIZE;
}

static size_t
write_callback(void* ptr, size_t size, size_t nmemb, buffer_t* buffer)
{
  if (size && nmemb > SIZE_MAX / size) return 0;
  size_t total_size = size * nmemb;
  if (total_size >= SIZE_MAX - buffer->size) return 0;
  size_t needed = buffer->size + total_size + 1;
  if (needed > buffer->capacity) {
    size_t capacity = needed <= SIZE_MAX / 2 ? needed * 2 : needed;
    void* data = realloc(buffer->data, capacity);
    if (!data) return 0;
    buffer->data = data;
    buffer->capacity = capacity;
  }
  memcpy(buffer->data + buffer->size, ptr, total_size);
  buffer->size += total_size;
  return total_size;
}

static size_t
header_callback(void* ptr, size_t size, size_t nmemb, buffer_t* buffer)
{
  size_t length = size * nmemb;
  // Redirects and interim responses must not contribute headers to the final response.
  if (length >= 5 && !memcmp(ptr, "HTTP/", 5)) buffer->size = 0;
  return write_callback(ptr, size, nmemb, buffer);
}

#define HTTP_OK 200
#define HTTP_CREATED 201
#define HTTP_NO_CONTENT 204

#define HTTP_UNAUTHORIZED 401
#define HTTP_FORBIDDEN 403
#define HTTP_NOT_FOUND 404
#define HTTP_INTERNAL_SERVER_ERROR 500

static int
resume_fetch_http(lua_State* L, int status, lua_KContext ctx)
{
  request_t** owner = (request_t**)ctx;
  request_t* req = *owner;
  poll_requests();
  if (!req->done) return lua_yieldk(L, 0, ctx, resume_fetch_http);
  if (req->result != CURLE_OK) {
    CURLcode result = req->result;
    free_request(req);
    *owner = NULL;
    return luaL_error(L, "HTTP transport error: %s", curl_easy_strerror(result));
  }

  response_t* response =
    lua_newuserdata(L, sizeof(response_t) + req->response.size + req->header.size + 2);
  luaL_setmetatable(L, API_TYPE_RESPONSE);
  response->size = req->response.size;
  response->header_size = req->header.size;
  response->code = req->code;
  memcpy(response->data, req->response.data, req->response.size);
  response->data[response->size] = '\0';
  memcpy(response->data + response->size + 1, req->header.data, req->header.size);
  response->data[response->size + response->header_size + 1] = '\0';
  free_request(req);
  *owner = NULL;

  switch (response->code) {
    case HTTP_OK:
    case HTTP_CREATED:
    case HTTP_NO_CONTENT:
      lua_pushinteger(L, response->code);
      return 2;
    default:
      return lua_error(L);
  }
}

// Function to fetch HTTP/HTTPS content using libcurl and Lua coroutine
int fetch_http(lua_State* L)
{
  lpcString_t url = luaL_checkstring(L, 1); // URL passed as the first argument
  
#ifdef DEBUG_FETCH
  Con_Error("Fetching %s", url);
#endif
  
  if (!lua_isyieldable(L)) return luaL_error(L, "network.fetch requires a coroutine");
  lua_settop(L, 2);
  /* Keep the owner on the suspended C frame. Unreachable coroutines then
     cancel their curl handle during GC, including argument/setup failures. */
  request_t** owner = lua_newuserdata(L, sizeof(*owner));
  *owner = NULL;
  if (luaL_newmetatable(L, "orca.network.Request")) {
    lua_pushcfunction(L, collect_request);
    lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
  request_t* req = alloc_request();
  if (!req) return luaL_error(L, "Failed to allocate HTTP request");
  *owner = req;
  alloc_buffer(&req->response);
  alloc_buffer(&req->header);
  if (!req->response.data || !req->header.data) {
    return luaL_error(L, "Failed to allocate HTTP response buffers");
  }
  
  // Initialize libcurl
  if (!(req->easy_handle = curl_easy_init())) {
    return luaL_error(L, "curl_easy_init() failed");
  }
  
  bool_t noloadcookies = FALSE;
  
  if (lua_type(L, 2) == LUA_TTABLE) {
    luaX_parsefield(lpcString_t, method, 2, luaL_optstring, NULL);
    luaX_parsefield(bool_t, nocookies, 2, lua_toboolean);
    lua_getfield(L, 2, "headers");
    if (lua_type(L, -1) == LUA_TTABLE) {
      lua_pushnil(L);
      while (lua_next(L, -2)) {
        lpcString_t key = luaL_checkstring(L, -2);
        lpcString_t value = luaL_checkstring(L, -1);
        size_t entry_size = strlen(key) + strlen(value) + 3;
        char* entry = malloc(entry_size);
        if (!entry) return luaL_error(L, "Failed to allocate HTTP header");
        snprintf(entry, entry_size, "%s: %s", key, value);
        struct curl_slist* headers = curl_slist_append(req->headers, entry);
        free(entry);
        if (!headers) return luaL_error(L, "Failed to append HTTP header");
        req->headers = headers;
        lua_pop(L, 1);
      }
      lua_pop(L, 1);
    } else {
      lua_pop(L, 1);
    }
    if (!method || !strcmp(method, "GET")) {
      curl_easy_setopt(req->easy_handle, CURLOPT_HTTPGET, 1L);
    } else if (!strcmp(method, "POST")) {
      curl_easy_setopt(req->easy_handle, CURLOPT_POST, 1L);
    } else {
      curl_easy_setopt(req->easy_handle, CURLOPT_CUSTOMREQUEST, method);
    }
    lua_getfield(L, 2, "body");
    size_t body_size = 0;
    char const *body = luaL_optlstring(L, -1, NULL, &body_size);
    if (body) {
      // Preserve embedded NUL bytes and own the payload while fetch yields.
      curl_easy_setopt(req->easy_handle, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)body_size);
      curl_easy_setopt(req->easy_handle, CURLOPT_COPYPOSTFIELDS, body);
    }
    lua_pop(L, 1);
    noloadcookies = nocookies;
  }
  
  // Set curl options
  curl_easy_setopt(req->easy_handle, CURLOPT_URL, url); // Set the URL
                                                        //  curl_easy_setopt(req->easy_handle, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(req->easy_handle,
                   CURLOPT_WRITEFUNCTION,
                   write_callback); // Write callback function
  curl_easy_setopt(
                   req->easy_handle, CURLOPT_WRITEDATA, &req->response); // Buffer for response
  curl_easy_setopt(req->easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(req->easy_handle, CURLOPT_HTTPHEADER, req->headers);
  curl_easy_setopt(req->easy_handle, CURLOPT_HEADERFUNCTION, header_callback);
  curl_easy_setopt(req->easy_handle, CURLOPT_HEADERDATA, &req->header);
  
  if (!noloadcookies) {
    curl_easy_setopt(req->easy_handle,
                     CURLOPT_COOKIEFILE,
                     "/tmp/cookies.txt"); // Load cookies from this file
    curl_easy_setopt(req->easy_handle,
                     CURLOPT_COOKIEJAR,
                     "/tmp/cookies.txt"); // Save cookies to this file
  }
  
  //  print_cookies(req->easy_handle);
  
  // Enable SSL support for HTTPS
  if (strncmp(url, "https://", 8) == 0) {
    curl_easy_setopt(req->easy_handle,
                     CURLOPT_SSL_VERIFYPEER,
                     1L); // Enable peer verification (for SSL)
    curl_easy_setopt(req->easy_handle,
                     CURLOPT_SSL_VERIFYHOST,
                     2L); // Check the host against the certificate
  }
  
  if (!start_request(req)) return luaL_error(L, "Failed to start HTTP request");
  return resume_fetch_http(L, LUA_OK, (lua_KContext)owner);
}

ORCA_API void
NET_Init(void)
{
  if (curl_initialized) return;
  Con_Printf("Initializing network");
  if (curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK) curl_initialized = 1;
}

ORCA_API void
NET_Shutdown(void)
{
  if (!curl_initialized) return;
  Con_Printf("Shutting down network");
  while (fetch_requests) {
    request_t* req = fetch_requests;
    fetch_requests = req->next;
    if (req->multi_handle) curl_multi_remove_handle(fetch_pool, req->easy_handle);
    if (req->easy_handle) curl_easy_cleanup(req->easy_handle);
    req->easy_handle = NULL;
    req->multi_handle = NULL;
    req->next = req->previous = NULL;
    req->result = CURLE_ABORTED_BY_CALLBACK;
    req->done = TRUE;
  }
  if (fetch_pool) curl_multi_cleanup(fetch_pool);
  fetch_pool = NULL;
  curl_global_cleanup();
  curl_initialized = 0;
}

/* ----------------------------------------------------------------------- *
 * C-callable non-Lua fetch API (libcurl backend)
 * ----------------------------------------------------------------------- */

#include <source/network/network.h>

ORCA_API fetch_handle_t
NET_FetchBegin(const char *url)
{
  if (!url || !*url) return NULL;

  NET_Init();

  request_t *req = alloc_request();
  if (!req) return NULL;
  alloc_buffer(&req->response);
  alloc_buffer(&req->header);

  req->easy_handle = curl_easy_init();
  if (!req->easy_handle) { free_request(req); return NULL; }

  if (!req->response.data || !req->header.data) { free_request(req); return NULL; }

  curl_easy_setopt(req->easy_handle, CURLOPT_URL, url);
  curl_easy_setopt(req->easy_handle, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(req->easy_handle, CURLOPT_WRITEDATA, &req->response);
  curl_easy_setopt(req->easy_handle, CURLOPT_FOLLOWLOCATION, 1L);

  if (strncmp(url, "https://", 8) == 0) {
    curl_easy_setopt(req->easy_handle, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(req->easy_handle, CURLOPT_SSL_VERIFYHOST, 2L);
  }

  if (!start_request(req)) { free_request(req); return NULL; }
  return (fetch_handle_t)req;
}

ORCA_API bool_t
NET_FetchPoll(fetch_handle_t handle)
{
  if (!handle) return FALSE;
  poll_requests();
  return !((request_t*)handle)->done;
}

ORCA_API long
NET_FetchFinish(fetch_handle_t handle, void **data_out, size_t *size_out)
{
  if (data_out) *data_out = NULL;
  if (size_out) *size_out = 0;
  if (!handle) return -1;
  request_t* req = (request_t*)handle;
  long code = -1;
  if (data_out && size_out && req->done && req->result == CURLE_OK) {
    *data_out = req->response.data;
    *size_out = req->response.size;
    ((char*)*data_out)[*size_out] = '\0';
    req->response.data = NULL;
    code = req->code;
  }
  free_request(req);
  return code;
}

ORCA_API void
NET_FetchCancel(fetch_handle_t handle)
{
  if (handle) free_request((request_t*)handle);
}

#else // __EMSCRIPTEN__

#include <emscripten.h>
#include <emscripten/fetch.h>

#define MAX_FETCH_HEADERS 32

typedef struct
{
  int done;
  int success;
  int cancelled;
  emscripten_fetch_t* fetch;
} webgl_fetch_state_t;

static void
on_fetch_success(emscripten_fetch_t* fetch)
{
  webgl_fetch_state_t* state = (webgl_fetch_state_t*)fetch->userData;
  state->fetch = fetch;
  state->success = 1;
  state->done = 1;
  if (state->cancelled) {
    emscripten_fetch_close(fetch);
    free(state);
  }
}

static void
on_fetch_error(emscripten_fetch_t* fetch)
{
  webgl_fetch_state_t* state = (webgl_fetch_state_t*)fetch->userData;
  state->fetch = fetch;
  state->success = 0;
  state->done = 1;
  if (state->cancelled) {
    emscripten_fetch_close(fetch);
    free(state);
  }
}

#define HTTP_OK 200
#define HTTP_CREATED 201
#define HTTP_NO_CONTENT 204

/* Push the completed fetch result onto the Lua stack and return. */
static int
fetch_http_finish(lua_State* L, webgl_fetch_state_t* state)
{
  emscripten_fetch_t* fetch = state->fetch;
  free(state);

  size_t size = (size_t)fetch->numBytes;
  response_t* response = (response_t*)lua_newuserdata(L, sizeof(response_t) + size + 1);
  luaL_setmetatable(L, API_TYPE_RESPONSE);
  response->size = size;
  response->header_size = 0;
  response->code = (long)fetch->status;
  if (size > 0) {
    memcpy(response->data, fetch->data, size);
  }
  response->data[size] = '\0';

  emscripten_fetch_close(fetch);

  lua_pushinteger(L, response->code);

  switch (response->code) {
    case HTTP_OK:
    case HTTP_CREATED:
    case HTTP_NO_CONTENT:
      return 2;
    default:
      return lua_error(L);
  }
}

/* lua_yieldk continuation: re-yield until the fetch callback fires, then
   finish.  network.fetch() must be called from a Lua coroutine (orca.async). */
static int
fetch_http_resume(lua_State* L, int status, lua_KContext ctx)
{
  webgl_fetch_state_t* state = (webgl_fetch_state_t*)ctx;
  if (!state->done) {
    return lua_yieldk(L, 0, ctx, fetch_http_resume);
  }
  return fetch_http_finish(L, state);
}

int fetch_http(lua_State* L)
{
  lpcString_t url = luaL_checkstring(L, 1);

#ifdef DEBUG_FETCH
  Con_Error("Fetching %s", url);
#endif

  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = on_fetch_success;
  attr.onerror = on_fetch_error;

  // Collect header key/value pairs.  emscripten_fetch needs a
  // NULL-terminated array of alternating "Key", "Value" strings.
  const char* header_keys[MAX_FETCH_HEADERS];
  const char* header_vals[MAX_FETCH_HEADERS];
  char* header_buf = NULL;
  int n_headers = 0;

  const char* body = NULL;
  size_t body_size = 0;

  if (lua_type(L, 2) == LUA_TTABLE) {
    lua_getfield(L, 2, "method");
    if (!lua_isnil(L, -1)) {
      strncpy(attr.requestMethod, luaL_checkstring(L, -1),
              sizeof(attr.requestMethod) - 1);
      attr.requestMethod[sizeof(attr.requestMethod) - 1] = '\0';
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "body");
    if (lua_isstring(L, -1)) {
      size_t len = 0;
      body = lua_tolstring(L, -1, &len);
      body_size = len;
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "headers");
    if (lua_type(L, -1) == LUA_TTABLE) {
      // First pass: count headers and total string length.
      size_t total_len = 0;
      lua_pushnil(L);
      while (lua_next(L, -2) && n_headers < MAX_FETCH_HEADERS) {
        total_len += strlen(luaL_checkstring(L, -2)) + 1;
        total_len += strlen(luaL_checkstring(L, -1)) + 1;
        n_headers++;
        lua_pop(L, 1);
      }
      // Second pass: copy strings into a single buffer.
      if (n_headers > 0) {
        header_buf = (char*)malloc(total_len);
        if (!header_buf) {
          lua_pop(L, 1);
          return luaL_error(L, "out of memory allocating header buffer");
        }
        char* p = header_buf;
        int i = 0;
        lua_pushnil(L);
        while (lua_next(L, -2) && i < n_headers) {
          lpcString_t k = luaL_checkstring(L, -2);
          lpcString_t v = luaL_checkstring(L, -1);
          size_t klen = strlen(k) + 1;
          size_t vlen = strlen(v) + 1;
          memcpy(p, k, klen); header_keys[i] = p; p += klen;
          memcpy(p, v, vlen); header_vals[i] = p; p += vlen;
          i++;
          lua_pop(L, 1);
        }
      }
    }
    lua_pop(L, 1);
  }

  // Build the NULL-terminated requestHeaders array expected by emscripten_fetch.
  const char* req_headers[MAX_FETCH_HEADERS * 2 + 1];
  for (int i = 0; i < n_headers; i++) {
    req_headers[i * 2]     = header_keys[i];
    req_headers[i * 2 + 1] = header_vals[i];
  }
  req_headers[n_headers * 2] = NULL;
  if (n_headers > 0) {
    attr.requestHeaders = (const char* const*)req_headers;
  }

  if (body) {
    attr.requestData = body;
    attr.requestDataSize = body_size;
  }

  webgl_fetch_state_t* state = (webgl_fetch_state_t*)malloc(sizeof(webgl_fetch_state_t));
  if (!state) {
    free(header_buf);
    return luaL_error(L, "out of memory");
  }
  memset(state, 0, sizeof(webgl_fetch_state_t));
  attr.userData = state;

  emscripten_fetch(&attr, url);
  /* emscripten_fetch has processed requestHeaders and requestData before
     returning, so header_buf is no longer needed. */
  free(header_buf);

  /* Yield the Lua coroutine until the XHR callback fires.  Each resume
     from kMsgResumeCoroutine checks state->done via fetch_http_resume. */
  if (!state->done) {
    return lua_yieldk(L, 0, (lua_KContext)state, fetch_http_resume);
  }
  return fetch_http_finish(L, state);
}

ORCA_API void
NET_Init(void)
{
  Con_Printf("Initializing network");
}

ORCA_API void
NET_Shutdown(void)
{
  Con_Printf("Shutting down network");
}

/* ----------------------------------------------------------------------- *
 * C-callable non-Lua fetch API (Emscripten / WebGL backend)
 * ----------------------------------------------------------------------- */

#include <source/network/network.h>

ORCA_API fetch_handle_t
NET_FetchBegin(const char *url)
{
  if (!url || !*url) return NULL;

  webgl_fetch_state_t *state = malloc(sizeof(webgl_fetch_state_t));
  if (!state) return NULL;
  memset(state, 0, sizeof(*state));

  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess  = on_fetch_success;
  attr.onerror    = on_fetch_error;
  attr.userData   = state;

  emscripten_fetch(&attr, url);

  return (fetch_handle_t)state;
}

ORCA_API bool_t
NET_FetchPoll(fetch_handle_t handle)
{
  if (!handle) return FALSE;
  return !((webgl_fetch_state_t *)handle)->done;
}

ORCA_API long
NET_FetchFinish(fetch_handle_t handle, void **data_out, size_t *size_out)
{
  if (!handle || !data_out || !size_out) {
    if (data_out) *data_out = NULL;
    if (size_out) *size_out = 0;
    if (handle) {
      webgl_fetch_state_t *state = (webgl_fetch_state_t *)handle;
      if (state->fetch) emscripten_fetch_close(state->fetch);
      free(state);
    }
    return -1;
  }

  webgl_fetch_state_t *state = (webgl_fetch_state_t *)handle;
  emscripten_fetch_t  *fetch = state->fetch;
  long code = fetch ? (long)fetch->status : -1;

  if (fetch && fetch->numBytes > 0) {
    *size_out = (size_t)fetch->numBytes;
    *data_out = malloc(*size_out + 1);
    if (*data_out) {
      memcpy(*data_out, fetch->data, *size_out);
      ((char *)*data_out)[*size_out] = '\0';
    } else {
      *size_out = 0;
      code = -1;
    }
    emscripten_fetch_close(fetch);
  } else {
    *data_out = NULL;
    *size_out = 0;
    if (fetch) emscripten_fetch_close(fetch);
  }

  free(state);
  return code;
}

ORCA_API void
NET_FetchCancel(fetch_handle_t handle)
{
  if (!handle) return;
  webgl_fetch_state_t *state = (webgl_fetch_state_t *)handle;
  if (state->done) {
    /* Callback already fired — safe to close and free immediately. */
    if (state->fetch) emscripten_fetch_close(state->fetch);
    free(state);
  } else {
    /* Fetch is still in-flight; mark cancelled and let the callback
       close the fetch handle and free state once it completes. */
    state->cancelled = 1;
  }
}

#endif // __EMSCRIPTEN__

int f_response_text(lua_State* L)
{
  response_t* res = luaL_checkudata(L, 1, API_TYPE_RESPONSE);
  lua_pushlstring(L, res->data, res->size); // Push body
  return 1;
}

int f_response_json(lua_State* L)
{
  void push_json_value(lua_State * L, lpcString_t* s);
  response_t* res = luaL_checkudata(L, 1, API_TYPE_RESPONSE);
  lpcString_t tmp = res->data;
  push_json_value(L, &tmp);
  return 1;
}

int f_response_size(lua_State* L)
{
  response_t* res = luaL_checkudata(L, 1, API_TYPE_RESPONSE);
  lua_pushinteger(L, res->size);
  return 1;
}

int f_response_code(lua_State* L)
{
  response_t* res = luaL_checkudata(L, 1, API_TYPE_RESPONSE);
  lua_pushinteger(L, res->code);
  return 1;
}

int f_response_headers(lua_State* L)
{
  response_t* res = luaL_checkudata(L, 1, API_TYPE_RESPONSE);
  char const *cursor = res->data + res->size + 1, *end = cursor + res->header_size;
  lua_newtable(L);
  while (cursor < end) {
    char const *line = memchr(cursor, '\n', end - cursor);
    if (!line) line = end;
    char const *colon = memchr(cursor, ':', line - cursor);
    if (colon) {
      size_t length = colon - cursor;
      char *name = malloc(length + 1);
      if (!name) return luaL_error(L, "Failed to allocate HTTP header name");
      for (size_t i = 0; i < length; i++)
        name[i] = cursor[i] >= 'A' && cursor[i] <= 'Z' ? cursor[i] + ('a' - 'A') : cursor[i];
      name[length] = '\0';
      char const *value = colon + 1, *value_end = line;
      while (value < value_end && (*value == ' ' || *value == '\t')) value++;
      while (value_end > value && (value_end[-1] == '\r' || value_end[-1] == ' ' || value_end[-1] == '\t')) value_end--;
      lua_getfield(L, -1, name);
      if (!lua_istable(L, -1)) { lua_pop(L, 1); lua_newtable(L); }
      lua_pushlstring(L, value, value_end - value);
      lua_rawseti(L, -2, lua_rawlen(L, -2) + 1);
      lua_setfield(L, -2, name);
      free(name);
    }
    cursor = line < end ? line + 1 : end;
  }
  return 1;
}

#ifndef __EMSCRIPTEN__
#include <include/renderer.h>
int f_response_image(lua_State* L)
{
  response_t* res = luaL_checkudata(L, 1, API_TYPE_RESPONSE);
  /*handle_t h = */R_LoadImageFromMemory(res->data, (uint32_t)res->size);
  //  lua_pushlightuserdata(L, h);
  //  luaX_pushObject(L, R_LoadImageFromMemory(L, res->data, (uint32_t)res->size));
  return 1;
}
#else
int f_response_image(lua_State* L)
{
  return luaL_error(L, "response:image() is not supported in WebGL builds");
}
#endif

static luaL_Reg const lib_response[] = { { "__tostring", f_response_text },
  { "text", f_response_text },
  { "image", f_response_image },
  { "json", f_response_json },
  { "size", f_response_size },
  { "code", f_response_code },
  { "headers", f_response_headers },
  { NULL, NULL } };

int f_network_shutdown(lua_State* L)
{
  NET_Shutdown();
  return 0;
}

// Register the function with Lua
ORCA_API int luaopen_orca_network(lua_State* L)
{
  static luaL_Reg const functions[] = { { "fetch", fetch_http },
    { NULL, NULL } };
  
  NET_Init();
  
  luaL_newlib(L, functions);
  
  API_MODULE_SHUTDOWN(L, f_network_shutdown);
  
  luaL_newmetatable(L, API_TYPE_RESPONSE);
  luaL_setfuncs(L, lib_response, 0);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_setfield(L, -2, API_TYPE_RESPONSE);
  
  const char *lua_code =
  "local function url_encode(str)\n"
  "  if not str then return \"\" end\n"
  "  str = string.gsub(str, \"([^%w%-%.%_%~])\", function(c)\n"
  "    return string.format(\"%%%02X\", string.byte(c))\n"
  "  end)\n"
  "  return str\n"
  "end\n"
  "return url_encode\n";

  if (luaL_dostring(L, lua_code)) {
    return luaL_error(L, "Can't load 'url_encode' function: %s", lua_tostring(L, -1));
  } else {
    lua_setfield(L, -2, "url");  /* assuming table at -2 */
  }

  return 1;
}
