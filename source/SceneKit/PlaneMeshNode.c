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

  struct Node3D* node3D = GetNode3D(hObject);
  struct ViewEntity entity = {
    .mesh = BOX_PTR(Mesh, MD_PLANE),
    .material.opacity = node3D ? node3D->_opacity : 1.0f,
    .matrix = node3D ? node3D->Matrix : MAT4_Identity(),
    .bbox = BOX3_FromRect(((struct rect){0, 0,
      pPlaneMeshNode->PlaneWidth, pPlaneMeshNode->PlaneHeight})),
  };
  struct Material *mat = mod->Material ? mod->Material : pPlaneMeshNode->MeshMaterial;
  const int blend = OBJ_GetInteger(hObject, ID_Material_BlendMode, kBlendModeAlpha);
  _CollectUniforms(hObject, mat, &entity, uniforms, blend);

  // Workaround for PixelAlignedTexture
  if (pPlaneMeshNode->PlaneWidth == 0) {
    entity.mesh = BOX_PTR(Mesh, MD_DOT);
  }

  R_DrawEntity(pRender, &entity);

  return TRUE;
}
