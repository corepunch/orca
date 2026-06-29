#include <SpriteKit/SpriteKit.h>

// SKLabelNode_Render
HANDLER(SKLabelNode, SKNode, Render) {
  return FALSE;
}

// SKLabelNode_Create
HANDLER(SKLabelNode, Object, Create) {
  pSKLabelNode->_node = GetSKNode(hObject);
  return FALSE;
}
