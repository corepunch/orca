#include "SceneKit.h"

HANDLER(Scene, UpdateMatrix)
{
  struct Node3D* view = GetNode3D(hObject);

  view->_opacity = GetNode(hObject)->Opacity;
  view->Matrix = MAT4_Identity();

  FOR_EACH_CHILD(hObject, OBJ_SendMessageW, kMsgUpdateMatrix, 0,
                 &(struct UpdateMatrixMsgArgs){
    .parent = view->Matrix,
    .opacity = view->_opacity,
  });

  return TRUE;
}
