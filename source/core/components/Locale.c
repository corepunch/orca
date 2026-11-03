#include <source/core/core_local.h>
#include <libxml/tree.h>

typedef struct _LOCALE
{
  struct _LOCALE* next;
  uint32_t ident;
  LOCALE_TYPE type;
  char text[1];
}* PLOCALE;

static PLOCALE locales;

typedef void (*FORMATPROC)(LPSTR szOutput, uint32_t dwLength);

typedef struct _FORMATARG
{
  lpcString_t szString;
  FORMATPROC fnFormatProc;
} FORMATARG, *PFORMATARG;

#define VARIABLE(NAME) { #NAME, NAME }
#define FMTF(NAME, FORMAT, VALUE)                                              \
void NAME(LPSTR szOutput, uint32_t dwLength)                                 \
{                                                                            \
snprintf(szOutput, dwLength, FORMAT, VALUE);                               \
}

FMTF(data_FirstName, "%s", "John");
FMTF(data_LastName, "%s", "Doe");
FMTF(data_Age, "%s", "30");
FMTF(data_Email, "%s", "john.doe@example.com");
FMTF(data_Greeting, "%s", "Hello, World!");

static FORMATARG args[] = {
  VARIABLE(data_FirstName),
  VARIABLE(data_LastName),
  VARIABLE(data_Age),
  VARIABLE(data_Email),
  VARIABLE(data_Greeting),
  
  { NULL }
};

static FORMATPROC
FindFormatProc(lpcString_t szTag, size_t dwLength)
{
  for (PFORMATARG fmt = args; fmt->szString; fmt++) {
    if (!strncmp(fmt->szString, szTag, dwLength)) {
      return fmt->fnFormatProc;
    }
  }
  return NULL;
}

static void
formatstring(lpcString_t szFormat, LPSTR szOutput, uint32_t dwLength)
{
  lpcString_t szTag = strchr(szFormat, '<');
  *szOutput = 0;
  if (!szTag) {
    strncpy(szOutput, szFormat, dwLength);
  } else {
    size_t dwSkip = MIN(dwLength, szTag - szFormat);
    strncpy(szOutput, szFormat, dwSkip);
    szOutput += dwSkip;
    dwLength -= dwSkip;
    if (dwLength == 0) {
      // TODO: ran out of space, use realloc()
      return;
    }
    ++szTag; // skip <
    lpcString_t szEnd = strchr(szTag, '>');
    FORMATPROC fnProc = FindFormatProc(szTag, szEnd - szTag);
    if (fnProc) {
      fnProc(szOutput, dwLength);
      while (*szOutput) {
        szOutput++;
        dwLength--;
      }
    }
    formatstring(++szEnd, szOutput, dwLength);
  }
}

lpcString_t
Loc_GetString(lpcString_t szName, LOCALE_TYPE type)
{
  uint32_t ident = fnv1a32(szName);
  FOR_EACH_LIST(struct _LOCALE, locale, locales)
  {
    if (locale->ident == ident && locale->type == type) {
      if (type == LOC_TEXT) {
        static char tmp[1024];
        formatstring(locale->text, tmp, sizeof(tmp));
        return tmp;
      } else {
        return locale->text;
      }
    }
  }
  return szName;
}

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

lpcString_t
core_GetPreferredLanguage(void)
{
  static char language[128];
  char const *override = getenv("ORCA_LANGUAGE");
  if (override && *override) return override;
#ifdef __APPLE__
  CFBundleRef bundle = CFBundleGetMainBundle();
  CFArrayRef supported = bundle ? CFBundleCopyBundleLocalizations(bundle) : NULL;
  CFArrayRef preferred = supported && CFArrayGetCount(supported) > 0
      ? CFBundleCopyPreferredLocalizationsFromArray(supported) : CFLocaleCopyPreferredLanguages();
  if (supported) CFRelease(supported);
  if (preferred && CFArrayGetCount(preferred) > 0) {
    CFStringRef first = CFArrayGetValueAtIndex(preferred, 0);
    bool_t copied = CFStringGetCString(first, language, sizeof(language), kCFStringEncodingUTF8);
    CFRelease(preferred);
    if (copied && *language) return language;
  } else if (preferred) CFRelease(preferred);
#endif
  char const *names[] = { "LANGUAGE", "LC_ALL", "LC_MESSAGES", "LANG" };
  FOR_LOOP(i, 4) {
    char const *value = getenv(names[i]);
    if (value && *value) {
      snprintf(language, sizeof(language), "%s", value);
      char *end = strpbrk(language, ".:@");
      if (end) *end = 0;
      for (char *c = language; *c; c++) if (*c == '_') *c = '-';
      return language;
    }
  }
  return "en";
}

static lpcString_t
lookup_locale(struct Object *object, lpcString_t key)
{
  struct Locale *locale = GetLocale(object);
  FOR_LOOP(i, locale ? locale->NumEntries : 0) {
    struct LocaleEntry *entry = &locale->Entries[i];
    if (entry->Key && entry->Value && !strcmp(entry->Key, key)) return entry->Value;
  }
  return NULL;
}

lpcString_t
core_GetLocalizedString(struct Object *object, lpcString_t key)
{
  if (!key) return "";
  for (struct Object *scope = object; scope; scope = OBJ_GetParent(scope)) {
    lpcString_t value = lookup_locale(scope, key);
    if (value) return value;
    FOR_EACH_OBJECT(child, scope) {
      value = lookup_locale(child, key);
      if (value) return value;
    }
  }
  return Loc_GetString(key, LOC_TEXT);
}
