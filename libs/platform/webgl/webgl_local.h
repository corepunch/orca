#ifndef __WEBGL_LOCAL__
#define __WEBGL_LOCAL__

#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define ZeroAlloc(size) calloc(1, size)

typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef char const *PCSTR;
typedef char PATHSTR[1024];
typedef struct WI_Size *PSIZE2;
typedef struct WI_Size const *PCSIZE2;
typedef struct WI_Message EVENT, *PEVENT;
typedef unsigned long TIME;

extern EMSCRIPTEN_WEBGL_CONTEXT_HANDLE g_webgl_ctx;
extern int g_canvas_width;
extern int g_canvas_height;

void webgl_register_callbacks(void);

#endif
