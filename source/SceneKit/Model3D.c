#include <include/orca.h>
#include <include/renderer.h>

#include "SceneKit.h"

void
_CollectUniforms(lpObject_t hObject,
                 struct Material* mat,
                 struct ViewEntity* ent,
                 struct uniform* u,
                 uint32_t blendMode)
{
  ent->blendMode = (enum blend_mode)kBlendModeAlphaAutomatic;
  ent->uniforms = u;
  ent->numUniforms = OBJ_GetUniforms(hObject, u);

  if (mat) {
    ent->shader = mat->Shader;
    if (mat->BlendMode != -1) {
      ent->blendMode = (enum blend_mode)mat->BlendMode;
    }
    ent->numUniforms += OBJ_GetUniforms(CMP_GetObject(mat), u + ent->numUniforms);
  }
  if (blendMode != -1) {
    ent->blendMode = (enum blend_mode)blendMode;
  }
}

//#include "../renderer/r_local.h"

HANDLER(Model3D, Render)
{
  static struct uniform u[MAX_UNIFORMS];

//  int a = 0;
//  for (lpObject_t obj = OBJ_GetParent(hObject); obj; obj = OBJ_GetParent(obj)) {
//    if (GetModel3D(obj)) {
//      a = 1;
//    }
//  }
//  if (!a) {
//    GetNode3D(hObject)->RenderTransform.rotation.y = OBJ_GetWindow(hObject)->frame * 0.05;
//  }
  
  if (!pModel3D->Mesh)
    return FALSE;

  struct ViewEntity ent = {
    .debugName = OBJ_GetName(hObject),
    .mesh = pModel3D->Mesh,
    .material.opacity = GetNode3D(hObject)->_opacity,
    .matrix = GetNode3D(hObject)->Matrix,
  };

  const int blend = OBJ_GetInteger(hObject, ID_Material_BlendMode, -1);
  _CollectUniforms(hObject, pModel3D->Material, &ent, u, blend);

  //	if (OBJ_CheckName(hObject, "new_Hills_right")) {
  //		lpProperty_t p= OBJ_FindProperty2(hObject, "ActiveContext");
  //		uint32_t b= fnv1a32("ActiveContext");
  //		FOR_LOOP(i, ent.numUniforms) {
  //			Con_Error("%u %f", ent.u[i].Identifier,
  // ent.u[i].Value[0]);
  //		}
  //		int a=0;
  //	}

  R_DrawEntity(pRender, &ent);

  return TRUE;
}
