#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include "../platform.h"

longTime_t
WI_GetMilliseconds(void)
{
  struct timeval tp;
  struct timezone tzp;
  static long secbase;

  gettimeofday(&tp, &tzp);

  if (!secbase) {
    secbase = tp.tv_sec;
    return tp.tv_usec / 1000;
  }

  return (tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000;
}

void
WI_Sleep(longTime_t msec)
{
  struct timespec ts;

  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;

  nanosleep(&ts, &ts);
}

typedef struct
{
  char const *name;
  uint32_t keynum;
} keyname_t;

keyname_t keynames[] = { { "tab", WI_KEY_TAB },
  { "enter", WI_KEY_ENTER },
  { "escape", WI_KEY_ESCAPE },
  { "space", WI_KEY_SPACE },
  { "backspace", WI_KEY_BACKSPACE },
  { "up", WI_KEY_UPARROW },
  { "down", WI_KEY_DOWNARROW },
  { "left", WI_KEY_LEFTARROW },
  { "right", WI_KEY_RIGHTARROW },
  
  { "alt", WI_KEY_ALT },
  { "ctrl", WI_KEY_CTRL },
  { "shift", WI_KEY_SHIFT },
  
  { "f1", WI_KEY_F1 },
  { "f2", WI_KEY_F2 },
  { "f3", WI_KEY_F3 },
  { "f4", WI_KEY_F4 },
  { "f5", WI_KEY_F5 },
  { "f6", WI_KEY_F6 },
  { "f7", WI_KEY_F7 },
  { "f8", WI_KEY_F8 },
  { "f9", WI_KEY_F9 },
  { "f10", WI_KEY_F10 },
  { "f11", WI_KEY_F11 },
  { "f12", WI_KEY_F12 },
  
  { "ins", WI_KEY_INS },
  { "del", WI_KEY_DEL },
  { "pgdn", WI_KEY_PGDN },
  { "pgup", WI_KEY_PGUP },
  { "home", WI_KEY_HOME },
  { "end", WI_KEY_END },
  
  { "mouse1", WI_KEY_MOUSE1 },
  { "mouse2", WI_KEY_MOUSE2 },
  { "mouse3", WI_KEY_MOUSE3 },
  
  { "joy1", WI_KEY_JOY1 },
  { "joy2", WI_KEY_JOY2 },
  { "joy3", WI_KEY_JOY3 },
  { "joy4", WI_KEY_JOY4 },
  
  { "aux1", WI_KEY_AUX1 },
  { "aux2", WI_KEY_AUX2 },
  { "aux3", WI_KEY_AUX3 },
  { "aux4", WI_KEY_AUX4 },
  { "aux5", WI_KEY_AUX5 },
  { "aux6", WI_KEY_AUX6 },
  { "aux7", WI_KEY_AUX7 },
  { "aux8", WI_KEY_AUX8 },
  { "aux9", WI_KEY_AUX9 },
  { "aux10", WI_KEY_AUX10 },
  { "aux11", WI_KEY_AUX11 },
  { "aux12", WI_KEY_AUX12 },
  { "aux13", WI_KEY_AUX13 },
  { "aux14", WI_KEY_AUX14 },
  { "aux15", WI_KEY_AUX15 },
  { "aux16", WI_KEY_AUX16 },
  { "aux17", WI_KEY_AUX17 },
  { "aux18", WI_KEY_AUX18 },
  { "aux19", WI_KEY_AUX19 },
  { "aux20", WI_KEY_AUX20 },
  { "aux21", WI_KEY_AUX21 },
  { "aux22", WI_KEY_AUX22 },
  { "aux23", WI_KEY_AUX23 },
  { "aux24", WI_KEY_AUX24 },
  { "aux25", WI_KEY_AUX25 },
  { "aux26", WI_KEY_AUX26 },
  { "aux27", WI_KEY_AUX27 },
  { "aux28", WI_KEY_AUX28 },
  { "aux29", WI_KEY_AUX29 },
  { "aux30", WI_KEY_AUX30 },
  { "aux31", WI_KEY_AUX31 },
  { "aux32", WI_KEY_AUX32 },
  
  { "kp_home", WI_KEY_KP_HOME },
  { "kp_uparrow", WI_KEY_KP_UPARROW },
  { "kp_pgup", WI_KEY_KP_PGUP },
  { "kp_leftarrow", WI_KEY_KP_LEFTARROW },
  { "kp_5", WI_KEY_KP_5 },
  { "kp_rightarrow", WI_KEY_KP_RIGHTARROW },
  { "kp_end", WI_KEY_KP_END },
  { "kp_downarrow", WI_KEY_KP_DOWNARROW },
  { "kp_pgdn", WI_KEY_KP_PGDN },
  { "kp_enter", WI_KEY_KP_ENTER },
  { "kp_ins", WI_KEY_KP_INS },
  { "kp_del", WI_KEY_KP_DEL },
  { "kp_slash", WI_KEY_KP_SLASH },
  { "kp_minus", WI_KEY_KP_MINUS },
  { "kp_plus", WI_KEY_KP_PLUS },
  
  { "mwheelup", WI_KEY_MWHEELUP },
  { "mwheeldown", WI_KEY_MWHEELDOWN },
  
  { "pause", WI_KEY_PAUSE },
  
  { "semicolon",
    ';' }, // because a raw semicolon seperates commands
  
  { NULL, 0 } };

char keyshift[256];

char const *
WI_KeynumToString(uint32_t keynum)
{
  keyname_t* kn;
  static char tinystr[2];
  keynum = keynum&0xff;
  if (keynum == -1)
    return "<KEY NOT FOUND>";
  if (keynum > 32 && keynum < 127) { // printable ascii
    tinystr[0] = keynum;
    tinystr[1] = 0;
    return tinystr;
  }
  for (kn = keynames; kn->name; kn++)
    if (keynum == kn->keynum)
      return kn->name;
  return "<UNKNOWN KEYNUM>";
}
