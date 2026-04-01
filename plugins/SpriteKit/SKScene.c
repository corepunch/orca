#include "SpriteKit.h"

HANDLER(SKScene, UpdateMatrix)
{
  SKNodePtr node = GetSKNode(hObject);

  node->_opacity = GetNode(hObject)->Opacity;
  node->Matrix = MAT4_Identity();

  FOR_EACH_CHILD(hObject, _SendMessage, Node, UpdateMatrix,
                   .parent = node->Matrix,
                   .opacity = node->_opacity,
                 );

  return TRUE;
}
