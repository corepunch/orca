#include <source/core/core_local.h>
#include <math.h>

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

// Declare parse_property (defined in core_main.c; used the same way as in StyleController).
extern int parse_property(lua_State* L,
                          const char* str,
                          struct PropertyType const* prop,
                          void* valueptr);

// Walk StateManager's StateGroup children and set PF_USED_IN_STATE_MANAGER on
// each property referenced by a ControllerProperty attribute.
static void
_InitControllerProperties(struct StateManagerController* smc, lpObject_t host)
{
  if (!smc->StateManager) return;
  lpObject_t smObj = CMP_GetObject(smc->StateManager);
  if (!smObj) return;
  FOR_EACH_OBJECT(sgObj, smObj) {
    struct StateGroup* sg = GetStateGroup(sgObj);
    if (!sg || !sg->ControllerProperty || !sg->ControllerProperty[0]) continue;
    lpProperty_t prop = NULL;
    if (SUCCEEDED(OBJ_FindShortProperty(host, sg->ControllerProperty, &prop))) {
      PROP_SetFlag(prop, PF_USED_IN_STATE_MANAGER);
    }
  }
}

// Apply all StatePropertySetter children of `stateObj` to `target`.
static void
_ApplyState(lpObject_t host, lpObject_t target, lpObject_t stateObj)
{
  FOR_EACH_OBJECT(setterObj, stateObj) {
    struct StatePropertySetter* sp = GetStatePropertySetter(setterObj);
    if (!sp || !sp->Property || !sp->Property[0] || !sp->Value) continue;
    lpProperty_t hprop = NULL;
    if (FAILED(OBJ_FindShortProperty(target, sp->Property, &hprop))) continue;
    if (PROP_GetType(hprop) == kDataTypeString) {
      PROP_SetStringValue(hprop, sp->Value);
    } else {
      char buf[MAX_PROPERTY_STRING] = { 0 };
      parse_property(core.L, sp->Value, PROP_GetDesc(hprop), buf);
      PROP_SetValue(hprop, buf);
    }
  }
  // Recurse into nested State children (sub-states targeting child objects).
  FOR_EACH_OBJECT(child, stateObj) {
    struct State* sub = GetState(child);
    if (!sub) continue;
    lpObject_t subtarget = (sub->Path && sub->Path[0])
      ? OBJ_FindByPath(host, sub->Path) : host;
    if (subtarget) _ApplyState(host, subtarget, child);
  }
}

// Returns true if the controller property `prop` matches the State's Value string.
static bool_t
_StateMatches(lpProperty_t prop, lpcString_t value)
{
  if (!value || !value[0]) return FALSE;
  eDataType_t type = PROP_GetType(prop);
  if (type == kDataTypeBool && isalpha((unsigned char)*value)) {
    // Bool comparison: match "true"/"false" strings.
    float pval = (type < kDataTypeFloat)
      ? (float)*(int*)PROP_GetValue(prop)
      : *(float*)PROP_GetValue(prop);
    bool_t propTrue = (pval != 0.0f);
    bool_t valTrue  = (strcmp(value, "false") != 0);
    return propTrue == valTrue;
  }
  // Numeric comparison.
  float pval = (type < kDataTypeFloat)
    ? (float)*(int*)PROP_GetValue(prop)
    : *(float*)PROP_GetValue(prop);
  return fabsf(pval - (float)atof(value)) < 0.01f;
}

// ============================================================================
// COMPONENT HANDLERS
// ============================================================================

// On Start: mark all tracked controller properties so property_events.c knows
// to fire ControllerChanged when they change.  Also mark the StateManager
// property itself with PF_USED_IN_TRIGGER so Object.PropertyChanged fires
// when the StateManager is replaced at runtime.
HANDLER(StateManagerController, Object, Start) {
  _InitControllerProperties(pStateManagerController, hObject);
  lpProperty_t smProp = PROP_FindByLongID(OBJ_GetProperties(hObject),
                                          ID_StateManagerController_StateManager);
  if (smProp) PROP_SetFlag(smProp, PF_USED_IN_TRIGGER);
  return FALSE;
}

// When the StateManager object property is changed, re-initialise the tracked
// property flags against the new StateManager object.
HANDLER(StateManagerController, Object, PropertyChanged) {
  if (pPropertyChanged->Property &&
      PROP_GetLongIdentifier(pPropertyChanged->Property) ==
        ID_StateManagerController_StateManager) {
    _InitControllerProperties(pStateManagerController, hObject);
  }
  return FALSE;
}

// Dispatched by property_events.c whenever a PF_USED_IN_STATE_MANAGER property
// changes value.  Refresh tracked controller-property bindings first so that
// runtime StateManager swaps are handled even if Object.PropertyChanged for the
// StateManager property was not dispatched (e.g., assigned before Start ran).
HANDLER(StateManagerController, StateManagerController, ControllerChanged) {
  _InitControllerProperties(pStateManagerController, hObject);
  if (!pStateManagerController->StateManager) return FALSE;
  lpObject_t smObj = CMP_GetObject(pStateManagerController->StateManager);
  if (!smObj) return FALSE;

  lpProperty_t prop = pControllerChanged->Property;
  if (!prop) return FALSE;
  lpcString_t propName = PROP_GetName(prop);

  FOR_EACH_OBJECT(sgObj, smObj) {
    struct StateGroup* sg = GetStateGroup(sgObj);
    if (!sg || !sg->ControllerProperty) continue;
    if (strcmp(sg->ControllerProperty, propName) != 0) continue;

    // Find the first State whose Value matches the current property value.
    FOR_EACH_OBJECT(stateObj, sgObj) {
      struct State* st = GetState(stateObj);
      if (!st) continue;
      if (!_StateMatches(prop, st->Value)) continue;
      // Resolve optional target path.
      lpObject_t target = (st->Path && st->Path[0])
        ? OBJ_FindByPath(hObject, st->Path) : hObject;
      if (target) _ApplyState(hObject, target, stateObj);
      break; // only apply the first matching state per StateGroup
    }
  }
  return FALSE;
}

