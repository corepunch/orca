#include <source/core/core_local.h>

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

// Global property parser callback, set once on Load and reused for all ApplyStateGroup calls.
static propertyParser_t _parser;

// ============================================================================
// INTERNAL HELPERS (formerly in state_manager.c)
// ============================================================================

static void
_ReleaseStateGroup(PSTATEGRP stategroup)
{
  if (stategroup->next) {
    _ReleaseStateGroup(stategroup->next);
  }
  xmlFree(stategroup->name);
  free(stategroup);
}

static void
_ReleaseDoc(struct StateManagerController* sm)
{
  if (sm->doc) {
    xmlFreeDoc((xmlDocPtr)sm->doc);
    sm->doc = NULL;
  }
  if (sm->stategroups) {
    _ReleaseStateGroup((PSTATEGRP)sm->stategroups);
    sm->stategroups = NULL;
  }
  sm->initialized = FALSE;
}

static void
_InitStateGroups(struct StateManagerController* sm, lpObject_t object)
{
  xmlDocPtr doc = (xmlDocPtr)sm->doc;
  if (!doc)
    return;
  xmlForEach(stateGroupObject, xmlDocGetRootElement(doc)) {
    if (xmlStrcmp(stateGroupObject->name, XMLSTR("StateGroup")))
      continue;
    xmlCharPtr contprop = xmlGetProp(stateGroupObject, XMLSTR("ControllerProperty"));
    if (!xmlStrcmp(contprop, XMLSTR("StateToolsEditStateEnabled"))) {
      xmlFree(contprop);
      continue;
    }
    if (contprop == NULL) {
      Con_Error("No ControllerProperty %s", doc->URL);
      continue;
    }
    PSTATEGRP stateGroup = ZeroAlloc(sizeof(struct _STATGROUP));
    if (SUCCEEDED(OBJ_FindShortProperty(
          object, (lpcString_t)contprop, &stateGroup->property))) {
      PROP_SetFlag(stateGroup->property, PF_USED_IN_STATE_MANAGER);
    } else {
#ifdef LOG_MISSING_PROPERTIES
      Con_Error("%s: can't find property \"%s\" in object \"%s\"",
                 (lpcString_t)doc->URL,
                 contprop,
                 object->name);
#endif
    }
    stateGroup->xml = stateGroupObject;
    stateGroup->name = contprop;
    PSTATEGRP list = (PSTATEGRP)sm->stategroups;
    ADD_TO_LIST(stateGroup, list);
    sm->stategroups = list;
  }
}

static void
_ApplyStateGroup(lpObject_t object,
                 xmlCharPtr controllerProperty,
                 xmlNodePtr stateObject,
                 bool_t debug)
{
  WITH(xmlChar, Path, xmlGetProp(stateObject, XMLSTR("Path")), xmlFree)
  {
    object = OBJ_FindByPath(object, (lpcString_t)Path);
    if (!object) {
#ifdef LOG_MISSING_OBJECTS
      Con_Error("state manager: can't find child object \"%s\"",
                 (lpcString_t)Path);
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

    WITH(xmlChar, value, xmlNodeGetContent((xmlNode*)attr), xmlFree)
    {
      lua_State* L = core.L;
      if (FAILED(_parser(L, object, (lpcString_t)attr->name, (lpcString_t)value))) {
        Con_Error("state_manager: undefined property type %s=\"%s\" in object \"%s\"",
                  (lpcString_t)attr->name,
                  (lpcString_t)value,
                  OBJ_GetName(object));
      }
    }
  }

  xmlForEach(substate, stateObject)
  {
    if (!xmlStrcmp(substate->name, XMLSTR("State"))) {
      _ApplyStateGroup(object, controllerProperty, substate, debug);
    }
  }
}

static void
_HandleControllerChange(struct StateManagerController* sm,
                        lpObject_t hobj,
                        lpProperty_t prop)
{
  FOR_EACH_LIST(struct _STATGROUP, stategroup, (PSTATEGRP)sm->stategroups)
  {
    if (stategroup->property != prop)
      continue;
    xmlForEach(state, stategroup->xml)
    {
      if (xmlStrcmp(state->name, XMLSTR("State")))
        continue;
      WITH(xmlChar, value, xmlGetProp(state, stategroup->name), xmlFree)
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
            _ApplyStateGroup(hobj, stategroup->name, state, debug);
            next = NULL; // stop iterating states once a match is applied
          }
        } else if (fabs(pval - atof((lpcString_t)value)) < 0.01f) {
          _ApplyStateGroup(hobj, stategroup->name, state, debug);
          next = NULL; // stop iterating states once a match is applied
        }
      }
    }
  }
}

// ============================================================================
// COMPONENT HANDLERS
// ============================================================================

HANDLER(StateManagerController, Object, Create) {
  pStateManagerController->doc         = NULL;
  pStateManagerController->stategroups = NULL;
  pStateManagerController->initialized = FALSE;
  return FALSE;
}

HANDLER(StateManagerController, Object, Release) {
  _ReleaseDoc(pStateManagerController);
  return FALSE;
}

// Lazily resolve state groups on the first Start (matching former SM_EnsureStateManagerInitialized).
HANDLER(StateManagerController, Object, Start) {
  if (pStateManagerController->doc && !pStateManagerController->initialized) {
    _InitStateGroups(pStateManagerController, hObject);
    pStateManagerController->initialized = TRUE;
  }
  return FALSE;
}

// Replace SM_ReadStateManager: install a new XML document into the component.
// The caller must use OBJ_SendMessageW directly:
//   OBJ_SendMessageW(obj, ID_StateManagerController_Load,
//                    (wParam_t)(uintptr_t)doc, (lParam_t)(uintptr_t)parser);
// wParam carries the xmlDocPtr; lParam carries the propertyParser_t callback.
// In this HANDLER, lParam arrives as the 4th argument (pLoad), so we recover
// it via (uintptr_t)pLoad.
HANDLER(StateManagerController, StateManagerController, Load) {
  _ReleaseDoc(pStateManagerController);
  pStateManagerController->doc = (void*)(uintptr_t)wParam;
  // pLoad == (StateManagerController_LoadMsgPtr)lParam; recover parser from it
  if (pLoad) {
    _parser = (propertyParser_t)(uintptr_t)(void*)pLoad;
  }
  return TRUE;
}

// Replace SM_HandleControllerChange: called from property_events when a
// PF_USED_IN_STATE_MANAGER property changes value.
// wParam = lpProperty_t
HANDLER(StateManagerController, StateManagerController, ControllerChanged) {
  if (!pStateManagerController->initialized) {
    _InitStateGroups(pStateManagerController, hObject);
    pStateManagerController->initialized = TRUE;
  }
  _HandleControllerChange(pStateManagerController, hObject, (lpProperty_t)(uintptr_t)wParam);
  return FALSE;
}
