#ifndef __MODULE_H__
#define __MODULE_H__

#include <include/orca.h>
#include <include/renderer.h>

enum {
  f___addsf3 = 0,
  f___subsf3 = 1,
  f___mulsf3 = 2,
  f___divsf3 = 3,
  f___gtsf2 = 4,
  f___gesf2 = 5,
  f___ltsf2 = 6,
  f___lesf2 = 7,
  f___eqsf2 = 8,
  f___nesf2 = 9,
  f___fixsfsi = 10,
  f___fixunssfsi = 11,
  f___floatsisf = 12,
  f___floatunsisf = 13,
  f___unordsf2 = 14,
  f___modsi3 = 15,
  f___divsi3 = 16,
  f___udivsi3 = 17,
  f___umodsi3 = 18,
  f___adddf3 = 19,
  f___muldf3 = 20,
  f___extendsfdf2 = 21,
  f___fixsfdi = 22,
  f___floatdisf = 23,
  f___floatundisf = 24,
  f___floatunsidf = 25,
  f___eqdf2 = 26,
  f___gedf2 = 27,
  f___truncdfsf2 = 28,
  f___divdf3 = 29,
  f___fixdfsi = 30,
  f___floatsidf = 31,
  f___gtdf2 = 32,
  f___ltdf2 = 33,
  f_abs = 34,
  f_floor = 35,
  f_isalnum = 36,
  f_isalpha = 37,
  f_iscntrl = 38,
  f_isdigit = 39,
  f_isgraph = 40,
  f_islower = 41,
  f_ispunct = 42,
  f_isspace = 43,
  f_isupper = 44,
  f_isxdigit = 45,
  f_round = 46,
  f_atof = 47,
  f_snprintf = 48,
  f_sprintf = 49,
  f_strchr = 50,
  f_strcmp = 51,
  f_strcoll = 52,
  f_strcpy = 53,
  f_strncpy = 54,
  f_strdup = 55,
  f_strerror = 56,
  f_strftime = 57,
  f_strlen = 58,
  f_strncmp = 59,
  f_strpbrk = 60,
  f_strspn = 61,
  f_strstr = 62,
  f_strtof = 63,
  f_strtok = 64,
  f_fopen = 65,
  f_fwrite = 66,
  f_fread = 67,
  f_fclose = 68,
  f_feof = 69,
  f_ferror = 70,
  f_fflush = 71,
  f_fgets = 72,
  f_fprintf = 73,
  f_fputc = 74,
  f_fputs = 75,
  f_freopen = 76,
  f_fseek = 77,
  f_ftell = 78,
  f_setvbuf = 79,
  f_remove = 80,
  f_rename = 81,
  f_tmpfile = 82,
  f_tmpnam = 83,
  f_system = 84,
  f_ungetc = 85,
  f_getc = 86,
  f_puts = 87,
  f_putchar = 88,
  f_read = 89,
  f_write = 90,
  f_getenv = 91,
  f_clearerr = 92,
  f_memchr = 93,
  f_memcmp = 94,
  f_memcpy = 95,
  f_frexpf = 96,
  f_floorf = 97,
  f_fmaxf = 98,
  f_fminf = 99,
  f_acosf = 100,
  f_asinf = 101,
  f_atan2f = 102,
  f_ceilf = 103,
  f_cosf = 104,
  f_expf = 105,
  f_fmax = 106,
  f_fmin = 107,
  f_fmodf = 108,
  f_log10f = 109,
  f_log2f = 110,
  f_logf = 111,
  f_tanf = 112,
  f_sqrtf = 113,
  f_sinf = 114,
  f_powf = 115,
  f_roundf = 116,
  f___assert_rtn = 117,
  f___error = 118,
  f_malloc = 119,
  f_realloc = 120,
  f_free = 121,
  f_memset = 122,
  f_clock = 123,
  f_difftime = 124,
  f_time = 125,
  f_gmtime = 126,
  f_localtime = 127,
  f_mktime = 128,
  f_setlocale = 129,
  f_localeconv = 130,
  f_exit = 131,
  f_abort = 132,
  f_tolower = 133,
  f_toupper = 134,
  f_setjmp = 135,
  f_longjmp = 136,
  f_accept = 137,
  f_bind = 138,
  f_close = 139,
  f_connect = 140,
  f_inet_addr = 141,
  f_listen = 142,
  f_perror = 143,
  f_printf = 144,
  f_socket = 145,
  f_unlink = 146,
  f_cos = 147,
  f_sin = 148,
  f_sqrt = 149,
  f_sscanf = 150,
};

typedef struct _PROPDEF {
  lpcString_t szName;
  DWORD dwIndex;
  PROP_TYPE Type;
  DWORD dwHandleType;
  LPVOID lpEditorValue;
  LPVOID lpRuntimeValue;
  lpcString_t lpEnumValues;
  DWORD dwFlags;
  DWORD dwSize;
  LPSTR pPrograms[ATTR_COUNT];
  BOOL bHasBinding;
  BOOL bIsUsedInBinding;
} *LPPROPDEF;

typedef struct _SPREADSHEETDEF {
  lpcString_t szName;
  HANDLE hIdentifier;
  DWORD dwNumRows;
  DWORD dwNumColumns;
} *LPSPREADSHEETDEF;

typedef struct _SHEETCELLDEF {
  HSTATEMAN stateman;
  SIZE2 sheetsize;
  DWORD sheetid;
  DWORD row;
  DWORD column;
  PROP_TYPE type;
  LPVOID data;
  struct {
    lpcString_t object, property;
  } target;
} *LPSHEETCELLDEF;

typedef void (*EnumSpreadheetProc)(LPSPREADSHEETDEF, LPVOID);
typedef void (*EnumPropertyProc)(LPPROPDEF, LPVOID);
typedef void (*EnumPropertyTypeProc)(lpcString_t, LPVOID);
typedef void (*EnumAliasProc)(lpcString_t, lpcString_t, LPVOID);

ORCA_API void    UI_Render(FLOAT stereo, HANDLE rt);
ORCA_API void    UI_EnumChildObjects(HOBJ, EnumChildProc, LPVOID);
ORCA_API BOOL    UI_EnumObjectProperties(HOBJ, EnumPropertyProc, LPVOID);
ORCA_API void    UI_EnumObjectPropertyTypes(HOBJ, EnumPropertyTypeProc, LPVOID);
ORCA_API void    UI_EnumStateGroups(HSTATEMAN, EnumSpreadheetProc, LPVOID);
ORCA_API BOOL    UI_GetStateCell(HSTATEMAN, HANDLE, DWORD, DWORD, LPSHEETCELLDEF);
ORCA_API void    UI_FilterObjects(HOBJ, lpcString_t, EnumChildProc, LPVOID);
ORCA_API BOOL    UI_GetObjectItem(HOBJ, LPOBJDEF);
ORCA_API BOOL    UI_RenameObject(HOBJ, lpcString_t);
ORCA_API HOBJ    UI_NewObject(HOBJ parent, lpcString_t name, DWORD type);
ORCA_API BOOL    UI_GetObjectProperty(HOBJ, lpcString_t, LPPROPDEF);
ORCA_API BOOL    UI_GetProperty(DWORD, LPPROPDEF);
ORCA_API void    UI_RefreshProperty(DWORD);
ORCA_API TIME    UI_GetTime(void);
ORCA_API lpcString_t UI_LocalizeString(lpcString_t, LOCALE_TYPE);
ORCA_API HRESULT UI_Save(lpcString_t);
ORCA_API BOOL    UI_GetObjectItem(HOBJ, LPOBJDEF);

#endif
