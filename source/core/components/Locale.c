#include <source/core/core_local.h>
#include <libxml/tree.h>
#include <ctype.h>

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

/* "US" from "en_US.UTF-8" or "en-US"; FALSE when the tag names no region. */
static bool_t
region_of_tag(char const *tag, char region[3])
{
  for (char const *c = tag; c && *c && *c != '.' && *c != '@'; c++) {
    if ((*c == '_' || *c == '-') && isalpha((unsigned char)c[1]) && isalpha((unsigned char)c[2]) &&
        (!c[3] || strchr("._-@", c[3]))) {
      region[0] = (char)toupper((unsigned char)c[1]);
      region[1] = (char)toupper((unsigned char)c[2]);
      region[2] = 0;
      return TRUE;
    }
  }
  return FALSE;
}

static char const *
region_override(void)
{
  char const *value = getenv("ORCA_REGION");
  return value && *value ? value : NULL;
}

lpcString_t
core_GetRegion(void)
{
  static char region[8];
  char const *override = region_override();
  if (override) {
    snprintf(region, sizeof(region), "%s", override);
    for (char *c = region; *c; c++) *c = (char)toupper((unsigned char)*c);
    return region;
  }
#ifdef __APPLE__
  CFLocaleRef locale = CFLocaleCopyCurrent();
  CFStringRef code = locale ? CFLocaleGetValue(locale, kCFLocaleCountryCode) : NULL;
  bool_t copied = code && CFStringGetCString(code, region, sizeof(region), kCFStringEncodingUTF8);
  if (locale) CFRelease(locale);
  if (copied && *region) return region;
#endif
  char const *names[] = { "LC_ALL", "LC_MEASUREMENT", "LANG" };
  FOR_LOOP(i, 3) if (region_of_tag(getenv(names[i]), region)) return region;
  return "";
}

static lpcString_t
measurement_of_region(lpcString_t region)
{
  if (!strcmp(region, "US") || !strcmp(region, "LR") || !strcmp(region, "MM")) return "us";
  return !strcmp(region, "GB") ? "uk" : "metric";
}

lpcString_t
core_GetMeasurementSystem(void)
{
#ifdef __APPLE__
  if (!region_override()) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFStringRef system = locale ? CFLocaleGetValue(locale, kCFLocaleMeasurementSystem) : NULL;
    lpcString_t result = NULL;
    if (system && CFStringCompare(system, CFSTR("U.S."), 0) == kCFCompareEqualTo) result = "us";
    else if (system && CFStringCompare(system, CFSTR("U.K."), 0) == kCFCompareEqualTo) result = "uk";
    else if (system) result = "metric";
    if (locale) CFRelease(locale);
    if (result) return result;
  }
#endif
  return measurement_of_region(core_GetRegion());
}

bool_t
core_Uses24HourClock(void)
{
#ifdef __APPLE__
  if (!region_override()) {
    // The locale's skeleton "j" becomes "HH" or "h a"; the person's
    // 24-Hour Time switch is folded into the current locale.
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFStringRef format = locale ? CFDateFormatterCreateDateFormatFromTemplate(NULL, CFSTR("j"), 0, locale) : NULL;
    char text[32] = "";
    bool_t copied = format && CFStringGetCString(format, text, sizeof(text), kCFStringEncodingUTF8);
    if (format) CFRelease(format);
    if (locale) CFRelease(locale);
    if (copied && *text) return !strpbrk(text, "ahK");
  }
#endif
  return strcmp(measurement_of_region(core_GetRegion()), "us") != 0;
}

/* Dynamic Type: the content size categories and iOS's Body text style at
 * each, in points (Apple's typography tables); large is 17. */
static struct { char const *name; float body; } const content_sizes[] = {
  { "extra-small", 14 }, { "small", 15 }, { "medium", 16 }, { "large", 17 }, { "extra-large", 19 },
  { "extra-extra-large", 21 }, { "extra-extra-extra-large", 23 }, { "accessibility-medium", 28 },
  { "accessibility-large", 33 }, { "accessibility-extra-large", 40 }, { "accessibility-extra-extra-large", 47 },
  { "accessibility-extra-extra-extra-large", 53 },
};
static char content_size[48] = "large";

lpcString_t
core_GetContentSizeCategory(void)
{
  char const *value = getenv("ORCA_CONTENT_SIZE");
  return value && *value ? value : content_size;
}

void
core_SetContentSizeCategory(lpcString_t category)
{
  snprintf(content_size, sizeof(content_size), "%s", category && *category ? category : "large");
}

float
core_GetTextScale(void)
{
  lpcString_t category = core_GetContentSizeCategory();
  FOR_LOOP(i, sizeof(content_sizes) / sizeof(*content_sizes))
    if (!strcmp(content_sizes[i].name, category)) return content_sizes[i].body / 17;
  return 1;
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
