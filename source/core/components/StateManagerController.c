#include <source/core/core_local.h>
#include <source/core/animation.h>
#include <source/core/property/property_internal.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

// Walk StateManager's StateGroup children and set PF_USED_IN_STATE_MANAGER on
// each property referenced by a ControllerProperty attribute.
static void
_InitControllerProperties(struct StateManagerController* smc, struct Object *host)
{
  if (!smc->StateManager) return;
  struct Object *smObj = CMP_GetObject(smc->StateManager);
  if (!smObj) return;
  FOR_EACH_OBJECT(sgObj, smObj) {
    struct StateGroup* sg = GetStateGroup(sgObj);
    if (!sg || !sg->ControllerProperty || !sg->ControllerProperty[0]) continue;
    struct Property *prop = OBJ_FindShortProperty(host, fnv1a32(sg->ControllerProperty));
    if (prop) {
      PROP_SetFlag(prop, PF_USED_IN_STATE_MANAGER);
    }
  }
}

// Apply all attached properties of `stateObj` to `target`.
// Attached properties are stored as native C properties on the State object
// with PF_PROPERTY_TYPE NOT set (project properties, not component-owned).
// This mirrors how StyleController._ApplyStyleRule works.
static void
_ApplyState(struct Object *host, struct Object *target, struct Object *stateObj)
{
  for (struct Property *rp = OBJ_GetProperties(stateObj); rp; rp = PROP_GetNext(rp)) {
    // Skip State's own component properties (Value, Path, etc.)
    if (PROP_GetFlags(rp) & PF_PROPERTY_TYPE) continue;
    if (PROP_IsNull(rp)) continue;
    struct PropertyType const *pdesc = PROP_GetDesc(rp);
    if (!pdesc || !pdesc->Name) continue;
    struct Property *hprop = OBJ_FindShortProperty(target, fnv1a32(pdesc->Name));
    if (hprop) {
      PROP_SetValue(hprop, PROP_GetValue(rp));
    }
  }
  // Recurse into nested State children (sub-states targeting child objects).
  FOR_EACH_OBJECT(child, stateObj) {
    struct State* sub = GetState(child);
    if (!sub) continue;
    struct Object *subtarget = (sub->Path && sub->Path[0])
      ? OBJ_FindByPath(host, sub->Path) : host;
    if (subtarget) _ApplyState(host, subtarget, child);
  }
}

// Returns true if the controller property matches the State's Value string.
static bool_t
_StateMatches(struct Property *prop, lpcString_t value)
{
  if (!value || !value[0]) return FALSE;
  void const *slot = PROP_GetValue(prop);
  if (!slot) return FALSE;
  eDataType_t type = PROP_GetType(prop);
  if (type == kDataTypeString) {
    char const *current = *(char const *const *)slot;
    return current && !strcmp(current, value);
  }
  if (type == kDataTypeBool && (!strcmp(value, "true") || !strcmp(value, "false")))
    return (*(bool_t const *)slot != FALSE) == !strcmp(value, "true");
  if (type != kDataTypeBool && type != kDataTypeInt &&
      type != kDataTypeEnum && type != kDataTypeFloat) {
    Con_Error("StateManager: unsupported controller property type for %s", PROP_GetName(prop));
    return FALSE;
  }
  char *end;
  errno = 0;
  double expected = strtod(value, &end);
  while (isspace((unsigned char)*end)) end++;
  if (end == value || *end || errno == ERANGE || !isfinite(expected) ||
      (type == kDataTypeBool && expected != 0.0 && expected != 1.0)) {
    Con_Error("StateManager: invalid state value '%s' for %s", value, PROP_GetName(prop));
    return FALSE;
  }
  double current = type == kDataTypeFloat ? *(float const *)slot : *(int const *)slot;
  return isfinite(current) && fabs(current - expected) < 0.01;
}

struct state_track {
  struct state_track *next;
  struct Object *target;
  uint32_t identifier;
  size_t size;
  void *from, *to;
  char *stringValue;
  bool_t wanted, acquired, fromValid;
};

struct state_group_runtime {
  struct state_group_runtime *next;
  struct Object *definition, *state, *failedState;
  struct state_track *tracks;
  uint32_t start;
  float duration;
  enum InterpolationMode interpolation;
  enum Easing easing;
  bool_t initialized, transitioning, failed;
};

struct state_manager_runtime {
  struct Object *host, *definition;
  struct state_group_runtime *groups;
  bool_t busy, pending, disposed;
};

static bool_t
_ManagedGroup(struct Object *object)
{
  if (!GetStateGroup(object)) return FALSE;
  FOR_EACH_OBJECT(child, object) if (GetStateTransition(child)) return TRUE;
  return FALSE;
}

static struct Property *
_TrackProperty(struct state_track *track)
{
  return OBJ_FindLongProperty(track->target, track->identifier);
}

static void
_FreeTrack(struct state_manager_runtime *runtime, struct state_group_runtime *group,
           struct state_track *track, bool_t restore)
{
  struct Property *property = _TrackProperty(track);
  if (restore && property) PROP_EndStateOverride(property, group);
  if (track->target != runtime->host) OBJ_ReleaseRef(track->target);
  free(track->from);
  free(track->to);
  free(track->stringValue);
  free(track);
}

static void
_FreeGroup(struct state_manager_runtime *runtime, struct state_group_runtime *group)
{
  core_UnobserveCollection(group->definition, runtime->host);
  while (group->tracks) {
    struct state_track *track = group->tracks;
    group->tracks = track->next;
    _FreeTrack(runtime, group, track, TRUE);
  }
  OBJ_ReleaseRef(group->definition);
  if (group->state) OBJ_ReleaseRef(group->state);
  free(group);
}

static void
_ClearGroups(struct state_manager_runtime *runtime)
{
  if (runtime->definition) {
    core_UnobserveCollection(runtime->definition, runtime->host);
  }
  while (runtime->groups) {
    struct state_group_runtime *group = runtime->groups;
    runtime->groups = group->next;
    _FreeGroup(runtime, group);
  }
  if (runtime->definition) OBJ_ReleaseRef(runtime->definition);
  runtime->definition = NULL;
}

static bool_t
_Mounted(struct Object *host, struct Object *target)
{
  for (struct Object *object = target; object; object = OBJ_GetParent(object))
    if (object == host) return TRUE;
  return FALSE;
}

static struct state_track *
_FindTrack(struct state_track *tracks, struct Object *target, uint32_t identifier)
{
  for (; tracks; tracks = tracks->next)
    if (tracks->target == target && tracks->identifier == identifier) return tracks;
  return NULL;
}

static bool_t
_CollectTracks(struct state_manager_runtime *runtime, struct Object *target,
               struct Object *state, struct state_track **tracks)
{
  if (!_Mounted(runtime->host, target))
    return Con_Error("StateManager: managed targets must belong to the host subtree"), FALSE;
  FOR_EACH_PROPERTY(source, OBJ_GetProperties(state)) {
    if ((PROP_GetFlags(source) & PF_PROPERTY_TYPE) || PROP_IsNull(source)) continue;
    uint32_t identifier = PROP_GetLongIdentifier(source);
    struct Property *property = OBJ_FindLongProperty(target, identifier);
    if (!property)
      return Con_Error("StateManager: missing target property %s", PROP_GetName(source)), FALSE;
    if (PROP_GetFlags(property) & PF_USED_IN_STATE_MANAGER)
      return Con_Error("StateManager: a managed state cannot override a controller property"), FALSE;
    if (_FindTrack(*tracks, target, identifier))
      return Con_Error("StateManager: duplicate managed target %s", PROP_GetName(property)), FALSE;
    if (PROP_GetType(source) != PROP_GetType(property) || PROP_GetSize(source) != PROP_GetSize(property))
      return Con_Error("StateManager: incompatible target %s", PROP_GetName(property)), FALSE;
    struct state_track *track = calloc(1, sizeof(*track));
    if (!track) return Con_Error("StateManager: cannot allocate transition track"), FALSE;
    track->size = PROP_GetSize(property);
    track->from = malloc(track->size);
    track->to = malloc(track->size);
    if (!track->from || !track->to) {
      free(track->from); free(track->to); free(track);
      return Con_Error("StateManager: cannot allocate transition values"), FALSE;
    }
    track->target = target;
    if (target != runtime->host) OBJ_AddRef(target);
    track->identifier = identifier;
    track->wanted = TRUE;
    void const *presented = PROP_GetValue(property);
    track->fromValid = presented != NULL;
    if (presented) memcpy(track->from, presented, track->size);
    memcpy(track->to, PROP_GetValue(source), track->size);
    if (PROP_GetType(property) == kDataTypeString) {
      char const *value = *(char const *const *)track->to;
      track->stringValue = value ? strdup(value) : NULL;
      if (value && !track->stringValue) {
        _FreeTrack(runtime, NULL, track, FALSE);
        return Con_Error("StateManager: cannot copy transition string"), FALSE;
      }
      memcpy(track->to, &track->stringValue, sizeof(track->stringValue));
    }
    track->next = *tracks;
    *tracks = track;
  }
  FOR_EACH_OBJECT(child, state) {
    struct State *nested = GetState(child);
    if (!nested) continue;
    struct Object *next = nested->Path && *nested->Path
      ? OBJ_FindByPath(runtime->host, nested->Path) : runtime->host;
    if (!next)
      return Con_Error("StateManager: missing managed target path %s", nested->Path), FALSE;
    if (!_CollectTracks(runtime, next, child, tracks)) return FALSE;
  }
  return TRUE;
}

static bool_t
_FilterMatches(char const *filter, char const *value)
{
  return !filter || !*filter || !strcmp(filter, "*") || !strcmp(filter, value);
}

static struct StateTransition *
_Transition(struct state_group_runtime *group, struct Object *state)
{
  struct State *previous = group->state ? GetState(group->state) : NULL;
  struct State *next = state ? GetState(state) : NULL;
  char const *from = previous && previous->Value ? previous->Value : "";
  char const *to = next && next->Value ? next->Value : "";
  struct StateTransition *best = NULL;
  int priority = -1;
  FOR_EACH_OBJECT(child, group->definition) {
    struct StateTransition *transition = GetStateTransition(child);
    if (!transition || !_FilterMatches(transition->From, from) || !_FilterMatches(transition->To, to))
      continue;
    int specificity = (transition->From && *transition->From && strcmp(transition->From, "*") != 0)
                    + (transition->To && *transition->To && strcmp(transition->To, "*") != 0);
    if (specificity > priority) { best = transition; priority = specificity; }
  }
  return best;
}

static bool_t
_Interpolated(struct Property *property)
{
  if (PROP_GetType(property) == kDataTypeFloat) return TRUE;
  if (PROP_GetType(property) != kDataTypeStruct) return FALSE;
  char const *type = PROP_GetUserData(property);
  return type && (!strcmp(type, "Vector2D") || !strcmp(type, "Vector3D") ||
                  !strcmp(type, "Vector4D") || !strcmp(type, "Transform2D") ||
                  !strcmp(type, "Transform3D"));
}

static void
_TickGroup(struct state_manager_runtime *runtime, struct state_group_runtime *group,
           uint32_t now, bool_t immediate)
{
  float progress = immediate || !group->duration ? 1.0f
    : fminf(1, (uint32_t)(now - group->start) / group->duration);
  float eased = anim_interpolate(group->interpolation, group->easing, progress);
  struct state_track **link = &group->tracks;
  while (*link && !runtime->disposed) {
    struct state_track *track = *link;
    struct Property *property = _TrackProperty(track);
    if (!property || !_Mounted(runtime->host, track->target)) {
      *link = track->next;
      _FreeTrack(runtime, group, track, TRUE);
      continue;
    }
    void const *destination = track->wanted ? track->to : PROP_GetStateBaseValue(property, group);
    if (!destination) {
      if (track->wanted) Con_Error("StateManager: transition target lost its property override");
      *link = track->next;
      _FreeTrack(runtime, group, track, !track->wanted);
      continue;
    }
    if (progress < 1 && track->fromValid && _Interpolated(property)) {
      float *value = malloc(track->size);
      if (!value) {
        Con_Error("StateManager: cannot allocate interpolated value");
        progress = 1;
        PROP_SetStateOverride(property, group, destination);
      } else {
        for (size_t i = 0; i < track->size / sizeof(float); i++) {
          float from, to;
          memcpy(&from, (char *)track->from + i * sizeof(float), sizeof(float));
          memcpy(&to, (char const *)destination + i * sizeof(float), sizeof(float));
          value[i] = from + (to - from) * eased;
        }
        PROP_SetStateOverride(property, group, value);
        free(value);
      }
    } else if (progress >= 1 || track->identifier != ID_Node_Visible ||
               PROP_GetType(property) != kDataTypeBool || *(bool_t const *)destination) {
      PROP_SetStateOverride(property, group, destination);
    }
    if (progress >= 1 && !track->wanted) {
      *link = track->next;
      _FreeTrack(runtime, group, track, TRUE);
    } else link = &track->next;
  }
  group->transitioning = progress < 1;
}

static bool_t
_ChangeGroup(struct state_manager_runtime *runtime, struct state_group_runtime *group,
             struct Object *state, bool_t reducedMotion, uint32_t now)
{
  struct StateTransition *transition = _Transition(group, state);
  if (transition && (!isfinite(transition->Duration) || transition->Duration < 0 ||
                     transition->Duration > UINT32_MAX / 1000.0)) {
    Con_Error("StateManager: transition duration must be finite, nonnegative seconds");
    return FALSE;
  }
  struct state_track *desired = NULL;
  struct State *selected = state ? GetState(state) : NULL;
  struct Object *target = selected && selected->Path && *selected->Path
    ? OBJ_FindByPath(runtime->host, selected->Path) : runtime->host;
  if (!target) {
    Con_Error("StateManager: missing managed target path %s", selected->Path);
    return FALSE;
  }
  bool_t valid = !state || _CollectTracks(runtime, target, state, &desired);
  for (struct state_track *track = desired; valid && track; track = track->next) {
    if (_FindTrack(group->tracks, track->target, track->identifier)) continue;
    valid = PROP_BeginStateOverride(_TrackProperty(track), group);
    track->acquired = valid;
  }
  if (!valid) {
    while (desired) {
      struct state_track *track = desired;
      desired = track->next;
      _FreeTrack(runtime, group, track, track->acquired);
    }
    return FALSE;
  }
  while (group->tracks) {
    struct state_track *track = group->tracks;
    group->tracks = track->next;
    if (_FindTrack(desired, track->target, track->identifier)) {
      _FreeTrack(runtime, group, track, FALSE);
    } else {
      struct Property *property = _TrackProperty(track);
      if (!property) {
        _FreeTrack(runtime, group, track, FALSE);
        continue;
      }
      void const *presented = PROP_GetValue(property);
      track->fromValid = presented != NULL;
      if (presented) memcpy(track->from, presented, track->size);
      track->wanted = FALSE;
      track->next = desired;
      desired = track;
    }
  }
  group->tracks = desired;
  group->duration = !group->initialized || reducedMotion || !transition ? 0 : transition->Duration * 1000;
  group->interpolation = transition ? transition->Interpolation : kInterpolationModeCubic;
  group->easing = transition ? transition->Easing : kEasingInOut;
  group->start = now;
  if (state) OBJ_AddRef(state);
  if (group->state) OBJ_ReleaseRef(group->state);
  group->state = state;
  group->initialized = TRUE;
  _TickGroup(runtime, group, now, reducedMotion);
  return TRUE;
}

static void
_ServiceManaged(struct StateManagerController *controller, struct Object *host)
{
  if (!(OBJ_GetFlags(host) & OF_STARTED)) return;
  struct state_manager_runtime *runtime = controller->_runtime;
  if (runtime && runtime->disposed) return;
  if (runtime && runtime->busy) { runtime->pending = TRUE; return; }
  if (!runtime) {
    runtime = controller->_runtime = calloc(1, sizeof(*runtime));
    if (!runtime) { Con_Error("StateManager: cannot allocate managed runtime"); return; }
    runtime->host = host;
  }
  runtime->busy = TRUE;
  bool_t moving = FALSE;
  do {
    runtime->pending = FALSE;
    struct Object *definition = controller->StateManager ? CMP_GetObject(controller->StateManager) : NULL;
    if (runtime->definition != definition) {
      if (definition) OBJ_AddRef(definition);
      _ClearGroups(runtime);
      runtime->definition = definition;
    }
    if (runtime->disposed) break;
    struct state_group_runtime **link = &runtime->groups;
    while (*link) {
      struct state_group_runtime *group = *link;
      if (OBJ_GetParent(group->definition) != definition || !_ManagedGroup(group->definition)) {
        *link = group->next;
        _FreeGroup(runtime, group);
      } else link = &group->next;
    }
    if (definition) FOR_EACH_OBJECT(child, definition) {
      if (!_ManagedGroup(child)) continue;
      struct state_group_runtime *group = runtime->groups;
      while (group && group->definition != child) group = group->next;
      if (group) continue;
      group = calloc(1, sizeof(*group));
      if (!group) { Con_Error("StateManager: cannot allocate managed group"); continue; }
      group->definition = child;
      OBJ_AddRef(child);
      if (!GetDataObject(child)) OBJ_AddComponent(child, ID_DataObject);
      if (!core_ObserveCollection(child, host))
        Con_Error("StateManager: cannot observe managed state membership");
      if (!GetDataObject(definition)) OBJ_AddComponent(definition, ID_DataObject);
      if (!core_ObserveCollection(definition, host))
        Con_Error("StateManager: cannot observe managed definition membership");
      group->next = runtime->groups;
      runtime->groups = group;
    }
    moving = FALSE;
    uint32_t now = ANIM_GetTime();
    for (struct state_group_runtime *group = runtime->groups; group && !runtime->disposed; group = group->next) {
      struct StateGroup *description = GetStateGroup(group->definition);
      struct Property *property = description->ControllerProperty
        ? OBJ_FindShortProperty(host, fnv1a32(description->ControllerProperty)) : NULL;
      if (!property) {
        if (!group->failed) Con_Error("StateManager: missing managed controller property");
        group->failed = TRUE;
        continue;
      }
      PROP_SetFlag(property, PF_USED_IN_STATE_MANAGER);
      PROP_Update(property);
      struct Object *state = NULL;
      FOR_EACH_OBJECT(child, group->definition) {
        struct State *candidate = GetState(child);
        if (candidate && _StateMatches(property, candidate->Value)) { state = child; break; }
      }
      if ((!group->initialized || group->state != state) &&
          (!group->failed || group->failedState != state)) {
        group->failed = !_ChangeGroup(runtime, group, state, controller->ReducedMotion, now);
        group->failedState = state;
      } else if (group->transitioning) {
        _TickGroup(runtime, group, now, controller->ReducedMotion);
      } else {
        _TickGroup(runtime, group, now, TRUE);
      }
      moving |= group->transitioning;
    }
    if (!runtime->disposed) {
      struct Property *status = OBJ_FindLongProperty(host, ID_StateManagerController_IsTransitioning);
      if (status) PROP_SetValue(status, &moving);
    }
  } while (runtime->pending && !runtime->disposed);
  runtime->busy = FALSE;
  if (runtime->disposed) {
    _ClearGroups(runtime);
    free(runtime);
  } else if (moving) OBJ_RequestAnimate(host);
}

HANDLER(StateManagerController, DataObject, CollectionChanged) {
  _InitControllerProperties(pStateManagerController, hObject);
  _ServiceManaged(pStateManagerController, hObject);
  return FALSE;
}

// ============================================================================
// COMPONENT HANDLERS
// ============================================================================

// On Start: mark all tracked controller properties so property_events.c knows
// to fire ControllerChanged when they change.  Also mark the StateManager
// property itself with PF_USED_IN_TRIGGER so Object.PropertyChanged fires
// when the StateManager is replaced at runtime.
// StateManagerController_Start
HANDLER(StateManagerController, Object, Start) {
  _InitControllerProperties(pStateManagerController, hObject);
  struct Property *smProp = OBJ_FindLongProperty(hObject, ID_StateManagerController_StateManager);
  if (smProp) PROP_SetFlag(smProp, PF_USED_IN_TRIGGER);
  struct Property *motion = OBJ_FindLongProperty(hObject, ID_StateManagerController_ReducedMotion);
  if (motion) PROP_SetFlag(motion, PF_USED_IN_TRIGGER);
  _ServiceManaged(pStateManagerController, hObject);
  return FALSE;
}

// When the StateManager object property is changed, re-initialise the tracked
// property flags against the new StateManager object.
// StateManagerController_PropertyChanged
HANDLER(StateManagerController, Object, PropertyChanged) {
  if (pPropertyChanged->Property &&
      PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_StateManagerController_StateManager) {
    _InitControllerProperties(pStateManagerController, hObject);
    _ServiceManaged(pStateManagerController, hObject);
  } else if (pPropertyChanged->Property &&
             PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_StateManagerController_ReducedMotion) {
    _ServiceManaged(pStateManagerController, hObject);
  }
  return FALSE;
}

// Dispatched by property_events.c whenever a PF_USED_IN_STATE_MANAGER property
// changes value.  Refresh tracked controller-property bindings first so that
// runtime StateManager swaps are handled even if Object.PropertyChanged for the
// StateManager property was not dispatched (e.g., assigned before Start ran).
// StateManagerController_ControllerChanged
HANDLER(StateManagerController, StateManagerController, ControllerChanged) {
  _InitControllerProperties(pStateManagerController, hObject);
  _ServiceManaged(pStateManagerController, hObject);
  if (!pStateManagerController->StateManager) return FALSE;
  struct Object *smObj = CMP_GetObject(pStateManagerController->StateManager);
  if (!smObj) return FALSE;

  struct Property *prop = pControllerChanged->Property;
  if (!prop) return FALSE;
  lpcString_t propName = PROP_GetName(prop);

  FOR_EACH_OBJECT(sgObj, smObj) {
    struct StateGroup* sg = GetStateGroup(sgObj);
    if (!sg || !sg->ControllerProperty) continue;
    if (_ManagedGroup(sgObj)) continue;
    if (strcmp(sg->ControllerProperty, propName) != 0) continue;

    // Find the first State whose Value matches the current property value.
    FOR_EACH_OBJECT(stateObj, sgObj) {
      struct State* st = GetState(stateObj);
      if (!st) continue;
      if (!_StateMatches(prop, st->Value)) continue;
      // Resolve optional target path.
      struct Object *target = (st->Path && st->Path[0])
        ? OBJ_FindByPath(hObject, st->Path) : hObject;
      if (target) _ApplyState(hObject, target, stateObj);
      break; // only apply the first matching state per StateGroup
    }
  }
  return FALSE;
}

HANDLER(StateManagerController, Object, Animate) {
  _ServiceManaged(pStateManagerController, hObject);
  return FALSE;
}

HANDLER(StateManagerController, Object, Release) {
  struct state_manager_runtime *runtime = pStateManagerController->_runtime;
  if (!runtime) return FALSE;
  pStateManagerController->_runtime = NULL;
  runtime->disposed = TRUE;
  if (!runtime->busy) {
    _ClearGroups(runtime);
    free(runtime);
  }
  return FALSE;
}
