#include <source/core/core_local.h>
#include <include/api.h>
#include <include/codegen.h>

struct collection_observer {
  struct collection_observer *next;
  struct collection_observer *globalNext;
  struct Object *source;
  struct Object *target;
};

static struct collection_observer *s_collection_observers;

bool_t
core_ObserveCollection(struct Object *source, struct Object *observer)
{
  struct DataObject *data = source ? GetDataObject(source) : NULL;
  if (!data || !observer || source == observer) return FALSE;
  for (struct collection_observer *it = data->_collectionObservers; it; it = it->next)
    if (it->target == observer) return TRUE;
  struct collection_observer *entry = ZeroAlloc(sizeof(*entry));
  entry->source = source;
  entry->target = observer;
  entry->next = data->_collectionObservers;
  data->_collectionObservers = entry;
  entry->globalNext = s_collection_observers;
  s_collection_observers = entry;
  return TRUE;
}

void
core_UnobserveCollection(struct Object *source, struct Object *observer)
{
  struct DataObject *data = source ? GetDataObject(source) : NULL;
  if (!data) return;
  struct collection_observer **entry = &data->_collectionObservers;
  while (*entry) {
    struct collection_observer *current = *entry;
    if (current->target != observer) { entry = &current->next; continue; }
    *entry = current->next;
    struct collection_observer **global = &s_collection_observers;
    while (*global && *global != current) global = &(*global)->globalNext;
    if (*global) *global = current->globalNext;
    free(current);
    return;
  }
}

void
OBJ_ClearCollectionObservers(struct Object *object)
{
  struct collection_observer *entry = s_collection_observers;
  while (entry) {
    struct collection_observer *next = entry->globalNext;
    if (entry->target == object || entry->source == object)
      core_UnobserveCollection(entry->source, entry->target);
    entry = next;
  }
}

static void
DispatchCollectionChanged(struct Object *object, struct DataObject *data)
{
  uint32_t version = data->_collectionVersion;
  size_t count = 0;
  for (struct collection_observer *it = data->_collectionObservers; it; it = it->next) count++;
  if (!count) return;
  struct Object **targets = malloc(count * sizeof(*targets));
  if (!targets) return;
  size_t index = 0;
  OBJ_AddRef(object);
  for (struct collection_observer *it = data->_collectionObservers; it; it = it->next) {
    targets[index++] = it->target;
    OBJ_AddRef(it->target);
  }
  // Snapshot receivers: a callback may unsubscribe itself or mutate the source.
  for (size_t i = 0; i < count; i++) {
    for (struct collection_observer *it = data->_collectionObservers; it; it = it->next) {
      if (it->target != targets[i]) continue;
      _SendMessage(targets[i], DataObject, CollectionChanged, .Source = object, .Version = version);
      break;
    }
    OBJ_ReleaseRef(targets[i]);
  }
  free(targets);
  OBJ_ReleaseRef(object);
}

void
OBJ_NotifyCollectionChanged(struct Object *object)
{
  struct DataObject *data = GetDataObject(object);
  if (!data) return;
  data->_collectionVersion++;
  if (data->_updateDepth) { data->_collectionPending = TRUE; return; }
  DispatchCollectionChanged(object, data);
}

void
OBJ_BeginUpdate(struct Object *object)
{
  struct DataObject *data = GetDataObject(object);
  if (data) data->_updateDepth++;
}

void
OBJ_EndUpdate(struct Object *object)
{
  struct DataObject *data = GetDataObject(object);
  if (!data || !data->_updateDepth || --data->_updateDepth || !data->_collectionPending) return;
  data->_collectionPending = FALSE;
  DispatchCollectionChanged(object, data);
}

HANDLER(DataContext, DataContext, GetData) {
  struct Object *data = OBJ_FindByPath(hObject, pGetData->Path ? pGetData->Path : "");
  return (LRESULT)GetDataObject(data);
}

HANDLER(DataObject, Object, Start) {
  return FALSE;
}

HANDLER(DataObject, Object, Destroy) {
  OBJ_ClearCollectionObservers(hObject);
  return FALSE;
}
