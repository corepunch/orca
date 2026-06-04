#ifndef __OBJECT_INTERNAL_H__
#define __OBJECT_INTERNAL_H__

#include <source/core/core_local.h>

struct component
{
  struct component* next;
  struct ClassDesc const *pcls;
  struct Object *pobj;
  char pUserData[];
};

struct Object
{
  LPSTR Name;
  uint32_t identifier;
  uint32_t refcount;

  struct Object *parent;
  struct Object *children;
  struct Object *next;

  LPSTR SourceFile;
  LPSTR TextContent;

  /* Component list — fallback for classes not yet in a typedata block.
   * Migrated to typedata (storage-struct="true/Name" on root + module prefix):
   *   UIKit/Node2D     → struct UINode2D       (prefix="UI")
   *   SceneKit/Node3D  → struct SceneNode3D    (prefix="Scene")
   *   SceneKit/RenderPass → struct SceneRenderPass
   *   SpriteKit/SKNode → struct SKData         (explicit name)
   *
   * Still on component list:
   *   UIKit:      Action, Trigger, StyleSheet, StyleRule, StyleController,
   *               StateManager, StateGroup, State, StateManagerController,
   *               ResourceDictionary, Locale, DataObject, Binding, BindingExpression,
   *               Node (abstract — never instantiated directly)
   *   core:       SendMessageAction, Setter, HideAction,
   *               OnPropertyChangedTrigger, OnAttachedTrigger, EventTrigger,
   *               AnimationCurve, AnimationClip, AnimationPlayer
   *   SceneKit:   Viewport3D → struct SceneViewport3D { Node2D; Viewport3D; }
   *   SpriteKit:  SKView    → struct SKViewData       { Node2D; SKView; }
   *   renderer:   Texture, Image, RenderTargetTexture, CubeMapTexture, IOSurfaceTexture,
   *               VertexShader, FragmentShader, Shader, Material, Mesh, FontFamily,
   *               Trajectory, Timeline
   *   filesystem: Project, Directory, Package, Entry, Library, Workspace,
   *               Locale*, Tag, LocaleReferenceItem, ThemeDefaultValuesDictionary
   */
  struct component* components;
  struct Property* properties;

  uint32_t alias;
  uint32_t unique;
  uint32_t flags;

  uint32_t class_id;            /* concrete class — Button, TextBlock, … */
  struct ClassDesc const *type; /* pointer to concrete ClassDesc */

  longTime_t dirty;

  char typedata[];              /* storage family data — UINode2D, SceneNode3D, … */
};

/* True when the object uses the monolithic typedata block for storage.
 * False for classes without a registered TypedataOffset (e.g. Viewport3D). */
static INLINE bool_t
OBJ_UsesTypedata(struct Object const *object)
{
  return object &&
         object->type &&
         object->type->TypedataOffset != UINT32_MAX;
}

/* Return a pointer to the per-class data for class_id on this object.
 * For typedata objects: typedata + TypedataOffset of the matching ancestor.
 * For component objects: the component's pUserData block. */
static INLINE void *
OBJ_GetTypedata(struct Object *object, uint32_t class_id)
{
  if (!object) return NULL;
  if (OBJ_UsesTypedata(object)) {
    extern struct game core;
    FOR_LOOP(i, MAX_CLASSES) {
      struct ClassDesc const *cls = core.classes[i];
      if (cls && cls->ClassID == class_id && cls->TypedataOffset != UINT32_MAX)
        return object->typedata + cls->TypedataOffset;
    }
  }
  return OBJ_GetComponent(object, class_id);
}

void OBJ_ApplyInheritedProperties(struct Object *object);
void OBJ_PropagateInheritedProperty(struct Object *object, struct Property *property);

#endif /* __OBJECT_INTERNAL_H__ */
