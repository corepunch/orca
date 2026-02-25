#include <code/client/node.h>
#include <code/sprite/SKPublic.h>
#include <code/common/common.h>
#include <code/renderer/r_public.h>

void SKScene_init(struct node *, struct SKNodeData *);
void SKSpriteNode_init(struct node *, struct SKNodeData *);
void SKLabelNode_init(struct node *, struct SKNodeData *);

static struct property_desc const
SKNode_properties[kSKNode_numCvars] =
{
    { 0x42edcab4, "anchor", PROPERTY_NUMBER, NULL, },
};

struct SKNodeData const *
SKNodeData(struct node const *node)
{
    return NODE_CLASS(node, SKNode) ? node->class_data.data : NULL;
}

void
SKNode_init(struct node *node)
{
    struct property *property = propertyL_new_of_type(PROPERTY_TRANSFORM2,
                                         node_get_param_hash(NODE_layoutTransform));
    ADD_TO_LIST(property, *node->properties);
    node->properties[NODE_layoutTransform] = property;
    node->class_data.data = mem_alloc(sizeof(struct SKNodeData), MEM_NODE);
    node->class_data.desc = SKNode_properties;
    node->class_data.num_properties = kSKNode_numCvars;

    switch (node->subclass_id) {
        case kSKScene: SKScene_init(node, node->class_data.data); break;
        case kSKSpriteNode: SKSpriteNode_init(node, node->class_data.data); break;
        case kSKLabelNode: SKLabelNode_init(node, node->class_data.data); break;
        default: break;
    }
}

void
SKNode_updatematrix(struct node* node,
                    struct matrix4 const *parent,
                    float opacity)
{
    if (!NODE_CLASS(node, SKNode) || node_is_hidden(node))
        return;
    
    float const node_opacity = propertyL_float(NODE_PARAM(node, opacity), 1);
    
    struct matrix4 matrix, layoutMatrix, renderMatrix;
    struct transform2 layoutTransform, renderTransform;
    struct vector2 pivot = { 0 };

    transform2_identity(&layoutTransform);
    transform2_identity(&renderTransform);
    
    property_value(NODE_PARAM(node, layoutTransform), PROPERTY_TRANSFORM2, &layoutTransform);
    property_value(NODE_PARAM(node, renderTransform), PROPERTY_TRANSFORM2, &renderTransform);
    property_value(NODE_PARAM (node, pivot), PROPERTY_VECTOR2, &pivot);

    transform2_to_matrix4(&layoutTransform, &pivot, &layoutMatrix);
    transform2_to_matrix4(&renderTransform, &pivot, &renderMatrix);
    matrix4_multiply(&layoutMatrix, &renderMatrix, &matrix);

    if (parent) {
        matrix4_multiply(parent, &matrix, &node->frame.matrix);
    } else {
        node->frame.matrix = matrix;
    }

    node->frame.opacity = node_opacity * opacity;

    FOR_EACH_LIST(struct node, child, node->children) {
        if (NODE_CLASS(child, SKNode)) {
            SKNode_updatematrix(child, &node->frame.matrix, node->frame.opacity);
        }
    }
}

bool
SKNode_containspoint(struct node const *node,
                     struct vector2 const *pnt)
{
    if (!SKNodeData(node)->sprite_rect)
        return false;
    struct rect const rect = SKNodeData(node)->sprite_rect(node);
    struct matrix4 inv;
    struct vector3 out, point = { pnt->x, pnt->y, 0 };
    matrix4_inverse(&node->frame.matrix, &inv);
    matrix4_multiply_vector3(&inv, &point, &out);
    return rect_contains_point(&rect, (struct vector2 const *)&out);
}

static luaL_Reg const lib_spritekit[] = {
    { NULL, NULL }
};

CALL_OBJECT_TYPE(Node, kSKNode, kUndefined);
CALL_OBJECT_TYPE(Scene, kSKNode, kSKScene);
CALL_OBJECT_TYPE(SpriteNode, kSKNode, kSKSpriteNode);
CALL_OBJECT_TYPE(View, kUIView, kSKView);
CALL_OBJECT_TYPE(LabelNode, kSKNode, kSKLabelNode);

int luaopen_SpriteKit(lua_State *L) {
    luaL_newlib(L, lib_spritekit);

    ADD_LOCAL_OBJECT_TYPE(Node);
    ADD_LOCAL_OBJECT_TYPE(Scene);
    ADD_LOCAL_OBJECT_TYPE(SpriteNode);
    ADD_LOCAL_OBJECT_TYPE(View);
    ADD_LOCAL_OBJECT_TYPE(LabelNode);

    return 1;
}
