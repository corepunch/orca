#include "core_local.h"

#include <libxml/parser.h>
#include <libxml/tree.h>

#define IGNORE_OBJECT_ATTR(attr)                                               \
!xmlStrcmp(attr->name, XMLSTR("IDInImportSource")) ||                         \
!xmlStrcmp(attr->name, XMLSTR("GpuResourceMemoryType")) ||                    \
!xmlStrcmp(attr->name, XMLSTR("IsDisabled")) ||                               \
!xmlStrcmp(attr->name, XMLSTR("PlaceholderTemplate")) ||                      \
!xmlStrcmp(attr->name, XMLSTR("Kanzi.OnPropertyChangedTrigger"))

typedef xmlChar* xmlCharPtr;

typedef struct _STATGROUP
{
  lpProperty_t property;
  xmlNodePtr xml;
  xmlCharPtr name;
  struct _STATGROUP* next;
}* PSTATEGRP;

typedef struct state_manager
{
  xmlDocPtr doc;
  lpObject_t object;
  PSTATEGRP stategroups;
}* PSTATEMAN;

static propertyParser_t _parser;

HRESULT
SM_ReadStateManager(lua_State* L, lpObject_t hobj, xmlDocPtr doc, propertyParser_t p)
{
  if (!doc) {
    return E_INVALIDARG;
  }
  _parser = p;
  SM_ReleaseStateManager(hobj);
  PSTATEMAN stateman = ZeroAlloc(sizeof(struct state_manager));
  stateman->doc = doc;
  _GetStateManager(hobj) = stateman;
  return NOERROR;
}

static void
SM_ReleaseStateGroup(PSTATEGRP stategroup)
{
  if (stategroup->next) {
    SM_ReleaseStateGroup(stategroup->next);
  }
  xmlFree(stategroup->name);
  free(stategroup);
}

void
SM_ReleaseStateManager(lpObject_t hobj)
{
  PSTATEMAN stateman = _GetStateManager(hobj);
  if (stateman) {
    xmlFreeDoc(stateman->doc);
    if (stateman->stategroups)
      SM_ReleaseStateGroup(stateman->stategroups);
    free(stateman);
  }
}

lpcString_t
SM_GetFileName(PSTATEMAN stateman)
{
  return (lpcString_t)stateman->doc->URL;
}

void
SM_InitStateManager(PSTATEMAN stateman, lpObject_t object)
{
  stateman->object = object;
  if (stateman->doc == NULL)
    return;
  xmlForEach(stateGroupObject, xmlDocGetRootElement(stateman->doc)) {
    if (xmlStrcmp(stateGroupObject->name, XMLSTR("StateGroup")))
      continue;
    xmlCharPtr contprop = xmlGetProp(stateGroupObject, XMLSTR("ControllerProperty"));
    if (!xmlStrcmp(contprop, XMLSTR("StateToolsEditStateEnabled"))) {
      xmlFree(contprop);
      continue;
    }
    if (contprop == NULL) {
      Con_Error("No ControllerProperty %s", stateman->doc->URL);
      continue;
    }
    PSTATEGRP stateGroup = ZeroAlloc(sizeof(struct _STATGROUP));
    if (SUCCEEDED(OBJ_FindShortProperty(
          object, (lpcString_t)contprop, &stateGroup->property))) {
      PROP_SetFlag(stateGroup->property, PF_USED_IN_STATE_MANAGER);
    } else {
#ifdef LOG_MISSING_PROPERTIES
      Con_Error("%s: can't find property \"%s\" in object \"%s\"",
                 stateman->filename,
                 contprop,
                 object->name);
#endif
    }
    stateGroup->xml = stateGroupObject;
    stateGroup->name = contprop;
    ADD_TO_LIST(stateGroup, stateman->stategroups);
  }
}

void
SM_ApplyStateGroup(PSTATEMAN stateman,
                   xmlCharPtr controllerProperty,
                   xmlNodePtr stateObject,
                   bool_t debug)
{
  lpObject_t object = stateman->object;

  xmlWith(xmlChar, Path, xmlGetProp(stateObject, XMLSTR("Path")), xmlFree)
  {
    object = OBJ_FindByPath(object, (lpcString_t)Path);
    if (!object) {
#ifdef LOG_MISSING_OBJECTS
      Con_Error("%s: can't find child object \"%s\" of \"%s\"",
                 stateman->filename,
                 (lpcString_t)Path,
                 object->name);
#endif
      return;
    }
  }

  if (!object)
    return;

  FOR_EACH_LIST(xmlAttr, attr, stateObject->properties)
  {
    if (!xmlStrcmp(attr->name, XMLSTR("Path")) ||
        !xmlStrcmp(attr->name, XMLSTR("Name")) ||
        !xmlStrcmp(attr->name, XMLSTR("id")) ||
        !xmlStrcmp(attr->name, BAD_CAST controllerProperty) ||
        IGNORE_OBJECT_ATTR(attr)) {
      continue;
    }

    xmlWith(xmlChar, value, xmlNodeGetContent((xmlNode*)attr), xmlFree)
    {
      lua_State* L = core.L;
      if (FAILED(_parser(L, object, (lpcString_t)attr->name, (lpcString_t)value))) {
        Con_Error("%s: undefined property type %s=\"%s\" in object \"%s\"",
                  stateman->doc->URL,
                  (lpcString_t)attr->name,
                  (lpcString_t)value,
                  OBJ_GetName(object));
      }
    }
  }

  xmlForEach(substate, stateObject)
  {
    if (!xmlStrcmp(substate->name, XMLSTR("State"))) {
      SM_ApplyStateGroup(stateman, controllerProperty, substate, debug);
    }
  }
}

void
SM_HandleControllerChange(lpObject_t hobj, lpProperty_t prop)
{
  PSTATEMAN stateman = _GetStateManager(hobj);
//  if (strstr(stateman->filename, "StatusArea_IAA_Variant_2D")) {
//    float *n = PROP_GetValue(prop);
//    int a=0;
//  }
  FOR_EACH_LIST(struct _STATGROUP, stategroup, stateman->stategroups)
  {
    if (stategroup->property != prop)
      continue;
    xmlForEach(state, stategroup->xml)
    {
      if (xmlStrcmp(state->name, XMLSTR("State")))
        continue;
      xmlWith(xmlChar, value, xmlGetProp(state, stategroup->name), xmlFree)
      {
        lpProperty_t p = stategroup->property;
        float pval = *(float*)PROP_GetValue(p);
        if (PROP_GetType(p) < kDataTypeFloat) {
          pval = *(int*)PROP_GetValue(p);
        }
        bool_t debug = xmlHasProp(stategroup->xml, XMLSTR("Debug")) != NULL;
        if (debug) {
          Con_Error("Processing stategroup %s with value %d", stategroup->name, (int)pval);
        }
        if (PROP_GetType(p) == kDataTypeBool && isalpha(*value)) {
          if (!pval == !xmlStrcmp(value, XMLSTR("false"))) {
            SM_ApplyStateGroup(stateman, stategroup->name, state, debug);
            next = NULL;
          }
        } else if (fabs(pval - atof((lpcString_t)value)) < 0.01f) {
          SM_ApplyStateGroup(stateman, stategroup->name, state, debug);
          next = NULL;
        }
      }
    }
  }
}

void
SM_EnsureStateManagerInitialized(lpObject_t object)
{
  if (_GetStateManager(object) &&
      !(OBJ_GetFlags(object) & OF_STATEMANAGER_INITIALIZED)) {
    SM_InitStateManager(_GetStateManager(object), object);
    OBJ_SetFlags(object, OBJ_GetFlags(object) | OF_STATEMANAGER_INITIALIZED);
  }
}
