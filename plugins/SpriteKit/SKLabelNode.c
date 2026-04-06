#include <plugins/SpriteKit/SpriteKit.h>

HANDLER(SKLabelNode, SKNode, Render) {
  return FALSE;
}

HANDLER(SKLabelNode, Object, Create) {
  pSKLabelNode->_node = GetSKNode(hObject);
  return FALSE;
}
