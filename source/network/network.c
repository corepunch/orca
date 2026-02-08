#include <include/api.h>
#include <include/orca.h>

#define MAX_DATANAME 64
#define API_TYPE_RESPONSE "Response"
#define API_TYPE_SERVICECLIENT "ServiceClient"

// #define DEBUG_FETCH

void
SZ_Init(struct WI_Buffer* buf, void* data, int length)
{
  memset(buf, 0, sizeof(*buf));
  buf->data = data;
  buf->maxsize = length;
}

void
SZ_Clear(struct WI_Buffer* buf)
{
  buf->cursize = 0;
}

void*
SZ_GetSpace(struct WI_Buffer* buf, int length)
{
  if (buf->cursize + length > buf->maxsize)
    SZ_Clear(buf);
  return (LPSTR)buf->data + (buf->cursize += length) - length;
}

int net_open_socket(int port);
void
net_close_socket(int net_socket);
int net_accept(int net_socket);
int net_connect(lpcString_t addr, int port);
int net_packet(int net_socket, struct WI_Buffer* net_message);
int net_send_packet(int net_socket, struct WI_Buffer* net_message);
int net_set_nonblocking(int sockfd);
bool_t
net_has_no_error(void);

static void
service_read_server_message(struct network_service* service,
                            struct WI_Buffer* net_message)
{
}

void
_ServiceRead(struct network_service* service)
{
  struct WI_Buffer net_message;
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

#define INITIAL_BUFFER_SIZE 1024

typedef struct
{
  size_t size;
  long code;
  char data[];
} response_t;

typedef struct
{
  LPSTR data;      // Dynamic response buffer
  size_t size;     // Current size of the buffer
  size_t capacity; // Capacity of the buffer
} buffer_t;

// Struct to hold the curl handle and dynamic response buffer
typedef struct
{
  CURL* easy_handle;
  CURLM* multi_handle;
  buffer_t response;
  buffer_t header;
} request_t;

void
free_request(request_t* req)
{
  free(req->header.data);
  free(req->response.data);
  free(req);
}

void
alloc_buffer(buffer_t* buffer)
{
  buffer->data = malloc(INITIAL_BUFFER_SIZE);
  buffer->size = 0;
  buffer->capacity = INITIAL_BUFFER_SIZE;
}

// Helper function to grow the response buffer as needed
void
grow_response_buffer(buffer_t* buffer, size_t additional_size)
{
  // If we need more space for the response, resize the buffer
  if (buffer->size + additional_size > buffer->capacity) {
    buffer->capacity =
    (buffer->size + additional_size) * 2; // Double the capacity
    buffer->data = realloc(buffer->data, buffer->capacity);
    if (!buffer->data) {
      // Handle memory allocation failure
      Con_Error("Failed to allocate memory for response buffer");
      exit(1);
    }
  }
}

// Callback function to write data into the response buffer
size_t
write_callback(void* ptr, size_t size, size_t nmemb, buffer_t* buffer)
{
  size_t total_size = size * nmemb;
  grow_response_buffer(buffer, total_size);
  memcpy(buffer->data + buffer->size, ptr, total_size);
  buffer->size += total_size;
  return total_size;
}

#define HTTP_OK 200
#define HTTP_CREATED 201
#define HTTP_NO_CONTENT 204
#define HTTP_BAD_REQUEST 400
#define HTTP_UNAUTHORIZED 401
#define HTTP_FORBIDDEN 403
#define HTTP_NOT_FOUND 404
#define HTTP_INTERNAL_SERVER_ERROR 500

// Helper function to resume the Lua coroutine with the result
int resume_fetch_http(lua_State* L, int status, lua_KContext ctx)
{
  request_t* req = (request_t*)ctx;
  int running_handles;
  // Perform the transfer
  CURLMcode res = curl_multi_perform(req->multi_handle, &running_handles);
  if (res != CURLM_OK) {
    curl_multi_cleanup(req->multi_handle);
    curl_easy_cleanup(req->easy_handle);
    free_request(req);
    return luaL_error(
                      L, "curl_multi_perform error: %s", curl_multi_strerror(res));
  }
  // If the transfer is not complete, yield again
  if (running_handles > 0) {
    //    curl_multi_wait(req->multi_handle, NULL, 0, 1000, NULL);
    return lua_yieldk(L, 0, (lua_KContext)req, resume_fetch_http);
  }
  // Transfer completed
  CURLMsg* msg;
  while ((msg = curl_multi_info_read(req->multi_handle, &running_handles))) {
    if (msg->msg == CURLMSG_DONE) {
      long response_code;
      curl_easy_getinfo(
                        req->easy_handle, CURLINFO_RESPONSE_CODE, &response_code);
      
      // Null-terminate the response
      req->response.data[req->response.size] = '\0';
      
      // Null-terminate the header
      req->header.data[req->header.size] = '\0';
      
      //      puts(req->header.data);
      
      // Push response data to Lua
      response_t* response =
      lua_newuserdata(L, req->response.size + sizeof(response_t));
      luaL_setmetatable(L, API_TYPE_RESPONSE);
      response->size = req->response.size;
      response->code = response_code;
      memcpy(response->data, req->response.data, req->response.size);
      
      curl_multi_remove_handle(req->multi_handle, req->easy_handle);
      curl_easy_cleanup(req->easy_handle);
      curl_multi_cleanup(req->multi_handle);
      free_request(req);
      
      switch (response_code) {
        case HTTP_OK:
        case HTTP_CREATED:
        case HTTP_NO_CONTENT:
          // Push HTTP status code
          lua_pushinteger(L, response_code);
          return 2;
        default:
          return lua_error(L);
      }
    }
  }
  
  return 0;
}

// Function to fetch HTTP/HTTPS content using libcurl and Lua coroutine
int fetch_http(lua_State* L)
{
  lpcString_t url = luaL_checkstring(L, 1); // URL passed as the first argument
  
#ifdef DEBUG_FETCH
  Con_Error("Fetching %s", url);
#endif
  
  //  CURLcode res;
  // Allocate the initial request structure
  request_t* req = malloc(sizeof(request_t));
  alloc_buffer(&req->response);
  alloc_buffer(&req->header);
  
  // Initialize libcurl
  if (!(req->easy_handle = curl_easy_init())) {
    free_request(req);
    return luaL_error(L, "curl_easy_init() failed");
  }
  
  struct curl_slist* headers = NULL;
  bool_t noloadcookies = FALSE;
  
  if (lua_type(L, 2) == LUA_TTABLE) {
    luaX_parsefield(lpcString_t, body, 2, luaL_optstring, NULL);
    luaX_parsefield(lpcString_t, method, 2, luaL_optstring, NULL);
    luaX_parsefield(bool_t, nocookies, 2, lua_toboolean);
    lua_getfield(L, 2, "headers");
    if (lua_type(L, -1) == LUA_TTABLE) {
      lua_pushnil(L);
      while (lua_next(L, -2)) {
        static char entry[1024] = { 0 };
        lpcString_t key = luaL_checkstring(L, -2);
        lpcString_t value = luaL_checkstring(L, -1);
        snprintf(entry, sizeof(entry), "%s: %s", key, value);
        headers = curl_slist_append(headers, entry);
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
    if (body) {
      curl_easy_setopt(req->easy_handle, CURLOPT_POSTFIELDS, body);
    }
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
  curl_easy_setopt(req->easy_handle, CURLOPT_HTTPHEADER, headers);
  //  curl_easy_setopt(req->easy_handle, CURLOPT_HEADERFUNCTION,
  // write_callback);   curl_easy_setopt(req->easy_handle,
  // CURLOPT_HEADERDATA,
  //&req->header);
  
  if (!noloadcookies) {
    curl_easy_setopt(req->easy_handle,
                     CURLOPT_COOKIEFILE,
                     "/tmp/cookies.txt"); // Load cookies from this file
  }
  curl_easy_setopt(req->easy_handle,
                   CURLOPT_COOKIEJAR,
                   "/tmp/cookies.txt"); // Save cookies to this file
  
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
  
  if (!(req->multi_handle = curl_multi_init())) {
    curl_easy_cleanup(req->easy_handle);
    free_request(req);
    return luaL_error(L, "curl_multi_init() failed");
  }
  curl_multi_add_handle(req->multi_handle, req->easy_handle);
  
  // Start the transfer
  int running_handles;
  curl_multi_perform(req->multi_handle, &running_handles);
  
  // If the transfer is not complete, yield
  if (running_handles > 0) {
    return lua_yieldk(L, 0, (lua_KContext)req, resume_fetch_http);
  }
  
  // Cleanup in case of immediate completion
  curl_easy_cleanup(req->easy_handle);
  curl_multi_cleanup(req->multi_handle);
  
  free_request(req);
  
  return 0;
}

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

#include <include/renderer.h>
int f_response_image(lua_State* L)
{
  response_t* res = luaL_checkudata(L, 1, API_TYPE_RESPONSE);
  /*handle_t h = */R_LoadImageFromMemory(L, res->data, (uint32_t)res->size);
  //  lua_pushlightuserdata(L, h);
  //  luaX_pushObject(L, R_LoadImageFromMemory(L, res->data, (uint32_t)res->size));
  return 1;
}

static luaL_Reg const lib_response[] = { { "__tostring", f_response_text },
  { "text", f_response_text },
  { "image", f_response_image },
  { "json", f_response_json },
  { "size", f_response_size },
  { "code", f_response_code },
  { NULL, NULL } };

static void
Net_Init(void)
{
  Con_Error("Initializing network");
  
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

static void
Net_Shutdown(void)
{
  Con_Error("Shutting down network");
  
  curl_global_cleanup();
}

int f_network_shutdown(lua_State* L)
{
  Net_Shutdown();
  return 0;
}

// Register the function with Lua
ORCA_API int luaopen_orca_network(lua_State* L)
{
  static luaL_Reg const functions[] = { { "fetch", fetch_http },
    { NULL, NULL } };
  
  Net_Init();
  
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
