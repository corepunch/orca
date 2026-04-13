#include <source/core/core_local.h>

// Forward declaration from localization.c
void Loc_AddEntry(lpcString_t key, lpcString_t value, LOCALE_TYPE type);

// Map LocaleEntryType enum to LOCALE_TYPE
static LOCALE_TYPE
_ToLocaleType(enum LocaleEntryType t)
{
  switch (t) {
    case kLocaleEntryTypeText:     return LOC_TEXT;
    case kLocaleEntryTypeResource: return LOC_RESOURCE;
    default:                       return LOC_UNDEFINED;
  }
}

HANDLER(Locale, Object, Start) {
  for (int i = 0; i < pLocale->NumEntries; i++) {
    struct Entry const *e = &pLocale->Entries[i];
    Loc_AddEntry(e->Key, e->Value, _ToLocaleType(e->Type));
  }
  return FALSE;
}
