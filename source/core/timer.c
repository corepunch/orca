#include "core_local.h"

struct timer
{
  uint32_t duration;
  uint32_t id;
  longTime_t nextExecution;
  shortStr_t callbackName;
  struct timer* next;
};

int OBJAPI(SetTimer)
{
  static uint32_t counter = 0;
  luaX_parsefield(lpcString_t, name, 2, luaL_optstring, NULL);
  luaX_parsefield(uint32_t, duration, 2, luaL_checknumber);
  struct timer* timer = ZeroAlloc(sizeof(struct timer));
  if (name) {
    strncpy(timer->callbackName, name, sizeof(shortStr_t));
  }
  timer->duration = MAX(duration, 1);
  timer->nextExecution = core.realtime + duration;
  timer->id = ++counter;
  OBJ_AddTimer(self, timer);
  lua_pushinteger(L, timer->id);
  return 1;
}

void
OBJ_UpdateTimers(lpObject_t hobj)
{
  FOR_EACH_LIST(struct timer, timer, _GetTimers(hobj))
  {
    if (timer->nextExecution <= core.realtime) {
      timer->nextExecution += timer->duration;
      WI_PostMessageW(hobj, kEventTimer, timer->id, timer->callbackName);
    }
  }
}

void
OBJ_AddTimer(lpObject_t hobj, struct timer* ptimer)
{
  ADD_TO_LIST(ptimer, _GetTimers(hobj));
}

void
OBJ_ReleaseTimers(lpObject_t hobj)
{
  FOR_EACH_LIST(struct timer, timer, _GetTimers(hobj))
  {
    free(timer);
  }
}
