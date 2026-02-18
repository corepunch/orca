#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include "SceneKit.h"

void
_CollectUniforms(lpObject_t,
                 struct Material*,
                 struct ViewEntity*,
                 struct uniform*,
                 uint32_t);

HANDLER(PlaneMeshNode, Render)
{
  struct Model3D* mod = GetModel3D(hObject);
  // HACK: update with proper ignition animation
  if (!strcmp("PointerNeedle", OBJ_GetName(hObject))) {
    pPlaneMeshNode->PlaneHeight = 9.5f;
  }
  
  static struct uniform uniforms[MAX_UNIFORMS];

  struct ViewEntity entity = {
    .mesh = MESH_PLANE,
    .material.opacity = GetNode3D(hObject)->_opacity,
    .matrix = GetNode3D(hObject)->Matrix,
    .bbox = BOX3_FromRect(((struct rect){0, 0,
      pPlaneMeshNode->PlaneWidth, pPlaneMeshNode->PlaneHeight})),
  };
  struct Material *mat = mod->Material ? mod->Material : pPlaneMeshNode->MeshMaterial;
  const int blend = OBJ_GetInteger(hObject, ID_Material_BlendMode, kBlendModeAlpha);
  _CollectUniforms(hObject, mat, &entity, uniforms, blend);

  // Workaround for PixelAlignedTexture
  if (pPlaneMeshNode->PlaneWidth == 0) {
    entity.mesh = MESH_DOT;
  }

  R_DrawEntity(pRender, &entity);

  return TRUE;
}
