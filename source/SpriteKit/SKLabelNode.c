#include <source/SpriteKit/SpriteKit.h>

HANDLER(SKLabelNode, Render) {
  return FALSE;
}

HANDLER(SKLabelNode, Create) {
  pSKLabelNode->_node = GetSKNode(hObject);
  return FALSE;
}
