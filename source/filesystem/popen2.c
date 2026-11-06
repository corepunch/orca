#include <include/api.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

typedef struct {
  int write_fd;  // stdin of child process
  int read_fd;   // stdout of child process
  pid_t pid;
} LuaProcess;

static int l_process_gc(lua_State *L) {
  LuaProcess *lp = lua_touserdata(L, 1);
  
  // Close pipes if still open
  if (lp->write_fd >= 0) {
    close(lp->write_fd);
    lp->write_fd = -1;
  }
  if (lp->read_fd >= 0) {
    close(lp->read_fd);
    lp->read_fd = -1;
  }
  
  // Kill and wait for child process if still alive
  if (lp->pid > 0) {
    kill(lp->pid, SIGTERM);
    // Give process a chance to terminate gracefully
    usleep(100000); // 100ms
    
    int status;
    if (waitpid(lp->pid, &status, WNOHANG) == 0) {
      // Process still alive, force kill
      kill(lp->pid, SIGKILL);
      waitpid(lp->pid, &status, 0);
    }
    lp->pid = -1;
  }
  
  return 0;
}

static int l_process_write(lua_State *L) {
  LuaProcess *lp = luaL_checkudata(L, 1, "Process");
  size_t len;
  const char *s = luaL_checklstring(L, 2, &len);
  
  if (lp->write_fd < 0) return luaL_error(L, "process stdin closed");
  
  ssize_t written = write(lp->write_fd, s, len);
  if (written < 0) return luaL_error(L, "write failed: %s", strerror(errno));
  
  lua_pushinteger(L, written);
  return 1;
}

static int l_process_read(lua_State *L) {
  LuaProcess *lp = luaL_checkudata(L, 1, "Process");
  lua_Integer len = luaL_optinteger(L, 2, 4096);
  if (lp->read_fd < 0 || len <= 0) return luaL_error(L, "invalid process or length");
  
  char *buf = malloc(len + 1);
  if (!buf) return luaL_error(L, "out of memory");
  
  size_t total = 0, buf_size = len;
  const char *end_response = "</response>";

  while (1) {
    if (total + len > buf_size) {
      buf_size *= 2;
      char *new_buf = realloc(buf, buf_size + 1);
      if (!new_buf) { free(buf); return luaL_error(L, "out of memory"); }
      buf = new_buf;
    }
    
    ssize_t n = read(lp->read_fd, buf + total, len);
    if (n <= 0) { free(buf); return luaL_error(L, n ? "read failed" : "incomplete XML"); }
    
    total += n;
    buf[total] = '\0';
    if (total >= strlen(end_response) && strstr(buf, end_response)) break;
  }
  
  /* Skip anything the child printed to stdout before its reply. */
  char *start = strstr(buf, "<?xml");
  if (!start) start = strstr(buf, "<response");
  if (!start) start = buf;
  lua_pushlstring(L, start, total - (start - buf));
  free(buf);
  return 1;
}

static int l_process_fetch(lua_State *L) {
  lua_getglobal(L, "string");
  lua_getfield(L, -1, "format");
  lua_remove(L, -2); // leave string.format on top
  lua_insert(L, 2); // move string.format below fmt + args
  int nargs = lua_gettop(L) - 2;  // everything after string.format
  if (lua_pcall(L, nargs, 1, 0) != LUA_OK) return lua_error(L);
  lua_pushcfunction(L, l_process_write);
  lua_pushvalue(L, 1);
  lua_pushvalue(L, 2);
  lua_call(L, 2, 0);
  lua_getglobal(L, "require");
  lua_pushstring(L, "orca.parsers.xml");
  lua_call(L, 1, 1);
  lua_getfield(L, -1, "parse");
  lua_pushcfunction(L, l_process_read);
  lua_pushvalue(L, 1);
  lua_call(L, 1, 1);
  lua_call(L, 1, 1);
  return 1;
}

static int l_process_get(lua_State *L) {
  lua_pushfstring(L, "get/%s", luaL_checkstring(L, 2));
  lua_replace(L, 2);
  return l_process_fetch(L);
}

static int l_process_post(lua_State *L) {
  lua_pushfstring(L, "post/%s", luaL_checkstring(L, 2));
  lua_replace(L, 2);
  return l_process_fetch(L);
}

static int l_process_put(lua_State *L) {
  lua_pushfstring(L, "put/%s", luaL_checkstring(L, 2));
  lua_replace(L, 2);
  return l_process_fetch(L);
}

static int l_process_delete(lua_State *L) {
  lua_pushfstring(L, "delete/%s", luaL_checkstring(L, 2));
  lua_replace(L, 2);
  return l_process_fetch(L);
}

//static int l_process_kill(lua_State *L) {
//  LuaProcess *lp = luaL_checkudata(L, 1, "Process");
//  lua_Integer sig = luaL_optinteger(L, 2, SIGTERM);
//  
//  if (lp->pid <= 0) return luaL_error(L, "process not running");
//  
//  if (kill(lp->pid, (int)sig) < 0) {
//    return luaL_error(L, "kill failed: %s", strerror(errno));
//  }
//  
//  return 0;
//}

static int l_process_wait(lua_State *L) {
  LuaProcess *lp = luaL_checkudata(L, 1, "Process");
  
  if (lp->pid <= 0) return luaL_error(L, "process not running");
  
  int status;
  if (waitpid(lp->pid, &status, 0) < 0) {
    return luaL_error(L, "waitpid failed: %s", strerror(errno));
  }
  
  lp->pid = -1; // Mark as no longer running
  
  if (WIFEXITED(status)) {
    lua_pushinteger(L, WEXITSTATUS(status));
    lua_pushstring(L, "exit");
  } else if (WIFSIGNALED(status)) {
    lua_pushinteger(L, WTERMSIG(status));
    lua_pushstring(L, "signal");
  } else {
    lua_pushinteger(L, status);
    lua_pushstring(L, "unknown");
  }
  
  return 2;
}

//static int l_process_close_stdin(lua_State *L) {
//  LuaProcess *lp = luaL_checkudata(L, 1, "Process");
//  
//  if (lp->write_fd >= 0) {
//    close(lp->write_fd);
//    lp->write_fd = -1;
//  }
//  
//  return 0;
//}
//
//static int l_process_close_stdout(lua_State *L) {
//  LuaProcess *lp = luaL_checkudata(L, 1, "Process");
//  
//  if (lp->read_fd >= 0) {
//    close(lp->read_fd);
//    lp->read_fd = -1;
//  }
//  
//  return 0;
//}
//
//static int l_process_pid(lua_State *L) {
//  LuaProcess *lp = luaL_checkudata(L, 1, "Process");
//  lua_pushinteger(L, lp->pid);
//  return 1;
//}

static int l_popen(lua_State *L) {
  int nargs = lua_gettop(L);
  if (nargs < 1) {
    return luaL_error(L, "popen requires at least one argument");
  }
  
  // Calculate total length needed for command string
  size_t total_len = 0;
  for (int i = 1; i <= nargs; i++) {
    size_t len;
    luaL_checklstring(L, i, &len);
    total_len += len;
    if (i < nargs) total_len++; // Space between args
  }
  total_len++; // Null terminator
  
  // Build command string
  char *cmd = malloc(total_len);
  if (!cmd) {
    return luaL_error(L, "out of memory");
  }
  
  cmd[0] = '\0';
  for (int i = 1; i <= nargs; i++) {
    size_t len;
    const char *arg = luaL_checklstring(L, i, &len);
    strcat(cmd, arg);
    if (i < nargs) {
      strcat(cmd, " ");
    }
  }
  
  int inpipe[2], outpipe[2];
  
  if (pipe(inpipe) < 0 || pipe(outpipe) < 0) {
    free(cmd);
    return luaL_error(L, "pipe failed: %s", strerror(errno));
  }
  
  pid_t pid = fork();
  if (pid == -1) {
    close(inpipe[0]); close(inpipe[1]);
    close(outpipe[0]); close(outpipe[1]);
    free(cmd);
    return luaL_error(L, "fork failed: %s", strerror(errno));
  }
  
  if (pid == 0) {
    // Child process
    dup2(inpipe[0], STDIN_FILENO);
    dup2(outpipe[1], STDOUT_FILENO);
    close(inpipe[0]); close(inpipe[1]);
    close(outpipe[0]); close(outpipe[1]);
    
    execl("/bin/sh", "sh", "-c", cmd, (char*)NULL);
    _exit(127);
  }
  
  // Parent process - free command string as we no longer need it
  free(cmd);
  
  // Close unused pipe ends
  close(inpipe[0]);   // Close read end of input pipe
  close(outpipe[1]);  // Close write end of output pipe
  
  LuaProcess *lp = lua_newuserdata(L, sizeof(LuaProcess));
  lp->write_fd = inpipe[1];   // Write to child's stdin
  lp->read_fd = outpipe[0];   // Read from child's stdout
  lp->pid = pid;
  
  luaL_getmetatable(L, "Process");
  lua_setmetatable(L, -2);
  
  return 1;
}

static const luaL_Reg process_methods[] = {
  {"write", l_process_write},
  {"read", l_process_read},
  {"get", l_process_get},
  {"put", l_process_put},
  {"post", l_process_post},
  {"delete", l_process_delete},
//  {"kill", l_process_kill},
  {"wait", l_process_wait},
//  {"close_stdin", l_process_close_stdin},
//  {"close_stdout", l_process_close_stdout},
//  {"pid", l_process_pid},
  {NULL, NULL}
};

static const luaL_Reg pipe_funcs[] = {
  {"popen", l_popen},
  {NULL, NULL}
};

int luaopen_orca_pipe(lua_State *L) {
  // Create Process metatable
  luaL_newmetatable(L, "Process");
  lua_pushcfunction(L, l_process_gc);
  lua_setfield(L, -2, "__gc");
  luaL_newlib(L, process_methods);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);
  
  // Create main library table
  luaL_newlib(L, pipe_funcs);
  return 1;
}

/* Native shared XML transport. Unlike the legacy Lua pipe helper this never
 * invokes a shell, and bounds every exchange so a failed child cannot hang UI. */
#include <include/orca.h>
#include <filesystem/filesystem.h>
#include <filesystem/filesystem_properties.h>
#include <sys/socket.h>
#include <poll.h>
#include <fcntl.h>
#include <time.h>

struct xml_process {
  int fd;
  pid_t pid;
  char *reply;
};

static int64_t
process_now(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static const char *
process_error(struct ProcessConnection *connection, const char *message)
{
  PROP_SetStringValue(OBJ_FindLongProperty(CMP_GetObject(connection),
      ID_ProcessConnection_ErrorMessage), message);
  return NULL;
}

ORCA_API void
ProcessConnection_Close(struct ProcessConnection *connection)
{
  struct xml_process *process = connection ? connection->_process : NULL;
  if (!process) return;
  close(process->fd);
  if (process->pid > 0) {
    kill(process->pid, SIGTERM);
    if (waitpid(process->pid, NULL, WNOHANG) == 0) {
      kill(process->pid, SIGKILL);
      while (waitpid(process->pid, NULL, 0) < 0 && errno == EINTR) {}
    }
  }
  free(process->reply);
  free(process);
  connection->_process = NULL;
}

static bool_t
process_open(struct ProcessConnection *connection)
{
  if (connection->_process) return TRUE;
  if (!connection->Executable || !*connection->Executable)
    return process_error(connection, "No executable configured"), FALSE;
  int sockets[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0)
    return process_error(connection, strerror(errno)), FALSE;
  fcntl(sockets[0], F_SETFD, FD_CLOEXEC);
  fcntl(sockets[1], F_SETFD, FD_CLOEXEC);
  char **argv = calloc((size_t)connection->NumArguments + 2, sizeof(char *));
  struct xml_process *process = calloc(1, sizeof(*process));
  if (!argv || !process) {
    close(sockets[0]); close(sockets[1]); free(argv); free(process);
    return process_error(connection, "Out of memory"), FALSE;
  }
  argv[0] = (char *)connection->Executable;
  for (int i = 0; i < connection->NumArguments; i++)
    argv[i + 1] = (char *)(connection->Arguments[i].Value ? connection->Arguments[i].Value : "");
  pid_t pid = fork();
  if (pid == 0) {
    dup2(sockets[1], STDIN_FILENO);
    dup2(sockets[1], STDOUT_FILENO);
    close(sockets[0]); close(sockets[1]);
    execvp(argv[0], argv);
    _exit(127);
  }
  free(argv);
  close(sockets[1]);
  if (pid < 0) {
    close(sockets[0]); free(process);
    return process_error(connection, strerror(errno)), FALSE;
  }
#ifdef SO_NOSIGPIPE
  int no_sigpipe = 1;
  setsockopt(sockets[0], SOL_SOCKET, SO_NOSIGPIPE, &no_sigpipe, sizeof(no_sigpipe));
#endif
  fcntl(sockets[0], F_SETFL, fcntl(sockets[0], F_GETFL, 0) | O_NONBLOCK);
  process->fd = sockets[0];
  process->pid = pid;
  connection->_process = process;
  return TRUE;
}

static bool_t
process_ready(int fd, short events, int64_t deadline)
{
  struct pollfd pfd = { .fd = fd, .events = events };
  for (;;) {
    int64_t remaining = deadline - process_now();
    if (remaining <= 0) { errno = ETIMEDOUT; return FALSE; }
    int n = poll(&pfd, 1, (int)remaining);
    if (n > 0) return (pfd.revents & events) != 0;
    if (n < 0 && errno == EINTR) continue;
    if (n == 0) errno = ETIMEDOUT;
    return FALSE;
  }
}

ORCA_API const char *
ProcessConnection_Request(struct ProcessConnection *connection, const char *request)
{
  if (!connection || !request) return NULL;
  if (strchr(request, '\n') || strchr(request, '\r'))
    return process_error(connection, "A request must be a single line");
  if (!process_open(connection)) return NULL;
  struct xml_process *process = connection->_process;
  free(process->reply);
  process->reply = NULL;
  int64_t deadline = process_now() + (connection->Timeout > 0 ? connection->Timeout : 10000);
  size_t len = strlen(request), sent = 0;
  char *line = malloc(len + 2);
  if (!line) return process_error(connection, "Out of memory");
  memcpy(line, request, len); line[len++] = '\n'; line[len] = 0;
  while (sent < len) {
    if (!process_ready(process->fd, POLLOUT, deadline)) break;
#ifdef MSG_NOSIGNAL
    ssize_t n = send(process->fd, line + sent, len - sent, MSG_NOSIGNAL);
#else
    ssize_t n = send(process->fd, line + sent, len - sent, 0);
#endif
    if (n < 0 && (errno == EINTR || errno == EAGAIN)) continue;
    if (n <= 0) break;
    sent += n;
  }
  free(line);
  if (sent != len) {
    process_error(connection, "Could not write request before deadline");
    ProcessConnection_Close(connection);
    return NULL;
  }
  size_t size = 4096, used = 0;
  char *reply = malloc(size + 1);
  if (!reply) return process_error(connection, "Out of memory");
  const char *marker = connection->EndMarker && *connection->EndMarker
    ? connection->EndMarker : "</response>";
  for (;;) {
    if (used == size) {
      if (size >= 16 * 1024 * 1024) break;
      size *= 2;
      char *larger = realloc(reply, size + 1);
      if (!larger) break;
      reply = larger;
    }
    if (!process_ready(process->fd, POLLIN, deadline)) break;
    ssize_t n = read(process->fd, reply + used, size - used);
    if (n < 0 && (errno == EINTR || errno == EAGAIN)) continue;
    if (n <= 0) break;
    used += n; reply[used] = 0;
    char *end = strstr(reply, marker);
    if (!end) continue;
    end[strlen(marker)] = 0;
    char *start = strstr(reply, "<?xml");
    if (!start) start = strstr(reply, "<response");
    if (start && start != reply) memmove(reply, start, strlen(start) + 1);
    process->reply = reply;
    process_error(connection, "");
    return reply;
  }
  free(reply);
  process_error(connection, "Incomplete XML response, closed connection, or request deadline exceeded");
  ProcessConnection_Close(connection);
  return NULL;
}

HANDLER(ProcessConnection, Object, Release) {
  ProcessConnection_Close(pProcessConnection);
  return FALSE;
}
