#include "SceneKit.h"

HANDLER(Node3D, UpdateMatrix)
{
  struct mat4 renderMatrix;
  struct mat4 layoutMatrix;
  struct mat4 matrix;
  struct transform3 /*const*/* renderTransform3D = &(pNode3D->RenderTransform);
  struct transform3 const* layoutTransform3D = &(pNode3D->LayoutTransform);
  struct vec3 pivot = pNode3D->RenderTransformOrigin;
  struct vec3 contentOffset = pNode3D->ContentOffset;
  
  layoutMatrix = transform3_ToMatrix3D(layoutTransform3D, &(struct vec3){0});
  renderMatrix = transform3_ToMatrix3D(renderTransform3D, &pivot);

  matrix = MAT4_Multiply(&layoutMatrix, &renderMatrix);

  if (pUpdateMatrix->parent) {
    pNode3D->Matrix = MAT4_Multiply(pUpdateMatrix->parent, &matrix);
  } else {
    pNode3D->Matrix = matrix;
  }

  pNode3D->_opacity = GetNode(hObject)->Opacity * pUpdateMatrix->opacity;
  matrix = pNode3D->Matrix;

  MAT4_Translate(&matrix, &contentOffset);

  FOR_EACH_CHILD(hObject, OBJ_SendMessageW, kEventUpdateMatrix, 0,
                 &(UPDATEMATRIXSTRUCT){
                   .parent = &pNode3D->Matrix,
                   .opacity = pNode3D->_opacity,
                 });

  return TRUE;
}
