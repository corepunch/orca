#include <include/api.h>
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

FMTF(data_FrontRightTireTemperature, "%s", "52");
FMTF(data_FrontRightTirePressure, "%s", "0.0");
FMTF(data_RearRightTirePressure, "%s", "0.0");
FMTF(data_RearRightTireTemperature, "%s", "52");
FMTF(data_RearLeftTirePressure, "%s", "0.0");
FMTF(data_RearLeftTireTemperature, "%s", "52");
FMTF(data_FrontLeftTirePressure, "%s", "0.0");
FMTF(data_FrontLeftTireTemperature, "%s", "52");
FMTF(data_ElectricRatio, "%s", "24");
FMTF(data_ElectricRatioSinceReset, "%s", "24");
FMTF(data_ElectricRatioSinceStart, "%s", "24");
FMTF(unit_Percent, "%s", "%");
FMTF(data_remainingHADDistance, "%s", "120");
FMTF(unit_Consumption, "%s", "1/100 km");
FMTF(unit_ConsumptionElectric, "%s", "kWh/100 km");
FMTF(unit_SwitchingConsumption, "%s", "1/100 km");
FMTF(unit_SwitchingConsumptionElectric, "%s", "kWh/100 km");
FMTF(unit_Distance, "%s", "km");
FMTF(unit_Duration, "%s", "min");
FMTF(unit_DurationTimeSinceLastBreak, "%s", "min");
FMTF(unit_DurationSinceStart, "%s", "min");
FMTF(unit_DurationSinceReset, "%s", "min");
FMTF(unit_DurationZeroEmissionSinceStart, "%s", "min");
FMTF(unit_DurationZeroEmissionSinceReset, "%s", "min");
FMTF(data_OutsideTemperature, "%s", "13");
FMTF(data_CurrentTime, "%s", "12:30");
FMTF(unit_MeridianUnitCurrentTime, "%s", "");
FMTF(unit_Speed, "%s", "km/h");
FMTF(unit_Temperature, "%s", "°C");
FMTF(unit_RevMeter, "%s", "x1000/min");
FMTF(data_DigitalSpeedoAdditional, "%s", "222");
FMTF(data_ConventionalRange, "%s", "- - - -");
FMTF(data_SetSpeed, "%s", "150");

FMTF(data_electricRatioSinceStart, "%s", "000");
FMTF(data_AverageSpeedSinceStart, "%s", "53");
FMTF(data_ConsumptionAverageConventionalSinceStart, "%s", "12.6");
FMTF(data_ConsumptionAverageElectricSinceStart, "%s", "123");
FMTF(data_DurationSinceStart, "%s", "24:53");
FMTF(data_OdometerSinceStart, "%s", "978");
FMTF(data_OdometerZeroEmissionSinceStart, "%s", "35");
FMTF(data_DurationZeroEmissionSinceStart, "%s", "3:16");

FORMATARG args[] = {
  //    TireMonitoring_Var2
  VARIABLE(data_FrontRightTireTemperature),
  VARIABLE(data_FrontRightTirePressure),
  VARIABLE(data_RearRightTirePressure),
  VARIABLE(data_RearRightTireTemperature),
  VARIABLE(data_RearLeftTirePressure),
  VARIABLE(data_RearLeftTireTemperature),
  VARIABLE(data_FrontLeftTirePressure),
  VARIABLE(data_FrontLeftTireTemperature),
  //    ElectricRatio_Var1
  VARIABLE(data_ElectricRatio),
  VARIABLE(data_ElectricRatioSinceReset),
  VARIABLE(data_ElectricRatioSinceStart),
  //    Misc
  VARIABLE(unit_Percent),
  VARIABLE(data_remainingHADDistance),
  VARIABLE(unit_Consumption),
  VARIABLE(unit_ConsumptionElectric),
  VARIABLE(unit_SwitchingConsumption),
  VARIABLE(unit_SwitchingConsumptionElectric),
  VARIABLE(unit_Distance),
  VARIABLE(unit_Duration),
  VARIABLE(unit_DurationTimeSinceLastBreak),
  VARIABLE(unit_DurationSinceStart),
  VARIABLE(unit_DurationSinceReset),
  VARIABLE(unit_DurationZeroEmissionSinceStart),
  VARIABLE(unit_DurationZeroEmissionSinceReset),
  VARIABLE(data_OutsideTemperature),
  VARIABLE(data_CurrentTime),
  VARIABLE(unit_MeridianUnitCurrentTime),
  VARIABLE(unit_Speed),
  VARIABLE(unit_Temperature),
  VARIABLE(data_DigitalSpeedoAdditional),
  VARIABLE(data_ConventionalRange),
  VARIABLE(data_SetSpeed),
  VARIABLE(unit_RevMeter),
  //    SinceStart_Var9
  VARIABLE(data_electricRatioSinceStart),
  VARIABLE(data_AverageSpeedSinceStart),
  VARIABLE(data_ConsumptionAverageConventionalSinceStart),
  VARIABLE(data_ConsumptionAverageElectricSinceStart),
  VARIABLE(data_DurationSinceStart),
  VARIABLE(data_OdometerSinceStart),
  VARIABLE(data_OdometerZeroEmissionSinceStart),
  VARIABLE(data_DurationZeroEmissionSinceStart),

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

bool_t
Locale_Load(xmlNodePtr xml)
{
  if (!xmlStrcmp(xml->name, XMLSTR("Locale"))) {
    xmlForEach(entry, xml)
    {
      xmlWith(xmlChar, name, xmlGetProp(entry, XMLSTR("Key")), xmlFree)
      {
        xmlWith(xmlChar, type, xmlGetProp(entry, XMLSTR("Type")), xmlFree)
        {
          xmlWith(xmlChar, content, xmlNodeGetContent(entry), xmlFree)
          {
            PLOCALE locale =
              ZeroAlloc(sizeof(struct _LOCALE) + xmlStrlen(content));
            locale->ident = fnv1a32((lpcString_t)name);
            if (!xmlStrcmp(type, XMLSTR("TEXT"))) {
              locale->type = LOC_TEXT;
            } else if (!xmlStrcmp(type, XMLSTR("RESOURCE"))) {
              locale->type = LOC_RESOURCE;
            } else {
              locale->type = LOC_UNDEFINED;
            }
            memcpy(locale->text, content, xmlStrlen(content));
            ADD_TO_LIST(locale, locales);
          }
        }
      }
    }
    return TRUE;
  } else {
    Con_Error("Localization object have tag \"Locale\"");
    return FALSE;
  }
}

static int f_load_localization(lua_State* L)
{
	lpcString_t source = luaL_checkstring(L, 1);
  bool_t loaded = FALSE;
  xmlWith(struct _xmlDoc, doc, FS_LoadXML(source), xmlFreeDoc)
  {
    loaded = Locale_Load(xmlDocGetRootElement(doc));
  }
  lua_pushboolean(L, loaded);
  return 1;
}

int w_localization_shutdown(lua_State* L)
{
  Con_Error("Shutting down localization");
  FOR_EACH_LIST(struct _LOCALE, loc, locales)
  {
    free(loc);
  }
  return 0;
}

static luaL_Reg const lib_files[] = { { "load", f_load_localization },
                                      { NULL, NULL } };

ORCA_API int luaopen_orca_localization(lua_State* L)
{
  Con_Error("Initializing localization");
  
  locales = 0;
  
//  ALLOCATE_MODULE(L, IID_LOCALIZATION, struct localization);

  luaL_newlib(L, lib_files);

  API_MODULE_SHUTDOWN(L, w_localization_shutdown);

  return 1;
}
