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
  
  char *buf = malloc(len);
  if (!buf) return luaL_error(L, "out of memory");
  
  size_t total = 0, buf_size = len;
  const char *end_response = "</response>";

  while (1) {
    if (total + len > buf_size) {
      buf_size *= 2;
      char *new_buf = realloc(buf, buf_size);
      if (!new_buf) { free(buf); return luaL_error(L, "out of memory"); }
      buf = new_buf;
    }
    
    ssize_t n = read(lp->read_fd, buf + total, len);
    if (n <= 0) { free(buf); return luaL_error(L, n ? "read failed" : "incomplete XML"); }
    
    total += n;
    buf[total] = '\0';
    if (total >= strlen(end_response) && strstr(buf, end_response)) break;
  }
  
//  Con_Error("%s", buf);
  
  lua_pushlstring(L, buf, total);
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
