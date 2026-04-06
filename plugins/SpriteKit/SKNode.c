#include "SpriteKit.h"

HANDLER(SKNode, Node, UpdateMatrix)
{
//  struct mat4 renderMatrix;
//  struct mat4 layoutMatrix;
  struct mat4 matrix;
//  struct transform2 const *renderTransform = &pSKNode->RenderTransform;
//  struct transform2 const *layoutTransform = &pSKNode->LayoutTransform;
//  struct vec2 pivot = pSKNode->RenderTransformOrigin;
//  struct vec2 center = {0};

//  layoutMatrix = transform2_ToMatrix3D(layoutTransform, &center);
//  renderMatrix = transform2_ToMatrix3D(renderTransform, &pivot);

//  matrix = MAT4_Multiply(&layoutMatrix, &renderMatrix);
  matrix = MAT4_FromTranslation(&(vec3_t){pSKNode->Position.x, pSKNode->Position.y});
  pSKNode->Matrix = MAT4_Multiply(&pUpdateMatrix->parent, &matrix);
  pSKNode->_opacity = GetNode(hObject)->Opacity * pUpdateMatrix->opacity;

  FOR_EACH_CHILD(hObject, _SendMessage, Node, UpdateMatrix,
                   .parent = pSKNode->Matrix,
                   .opacity = pSKNode->_opacity,
                 );

  return TRUE;
}
