#ifndef __QNX_LOCAL_H__
#define __QNX_LOCAL_H__

#include "../platform.h"

typedef void *HWND;
typedef char const *LPCSTR;
typedef uint32_t DWORD;
typedef unsigned long TIME;

typedef struct {
  int width;
  int height;
} SIZE2, *LPSIZE2;

typedef struct {
  float scaleX;
  float scaleY;
} METRICS, *LPMETRICS;

#endif
