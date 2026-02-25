#include <code/sprite/SKPublic.h>
#include <code/ui/UIPublic.h>
#include <code/renderer/r_public.h>

enum {
    kSKView_scene,
    kSKView_referenceWidth,
    kSKView_referenceHeight,
    kSKView_numCvars,
};

static struct property_desc const
SKView_properties[kSKView_numCvars] =
{
    { 0x2063cb13, "scene", PROPERTY_NODE, NULL },
    { 0x805c0e06, "referenceWidth", PROPERTY_NUMBER, NULL },
    { 0xd217ca99, "referenceHeight", PROPERTY_NUMBER, NULL },
};

struct SKView {
    struct property *properties[kSKView_numCvars];
};

static struct node const *
find_viewport(struct node const *node) {
    if (!node || node->subclass_id == kSKView)
        return node;
    return find_viewport(node->parent);
}

struct vector2
SKNode_referencesize(struct node const *node)
{
    struct node const *viewport = find_viewport(node);
    struct vector2 value = { 0, 0 };
    if (viewport) {
        struct SKView const *view = viewport->subclass.data;
        vec_t const w = propertyL_float(view->properties[kSKView_referenceWidth], 0);
        vec_t const h = propertyL_float(view->properties[kSKView_referenceHeight], 0);
        value.x = node_frame(viewport, kRect3_Width) - w;
        value.y = node_frame(viewport, kRect3_Height) - h;
        return value;
    } else {
        return value;
    }
}

static struct node *
SKView_scene(struct node const *node)
{
    struct SKView const *view = node->subclass.data;
    struct node *scene = NULL;
    property_value(view->properties[kSKView_scene], PROPERTY_NODE, &scene);
    return scene;
}

static void
SKNode_draw(struct node *node,
            struct matrix4 const *projection)
{
    struct viewdef viewdef;
    viewdef_new(&viewdef, projection);
    struct SKNodeData const *data = SKNodeData(node);
    if (data && data->draw) {
        data->draw(node, &viewdef);
    }
    FOR_EACH_LIST(struct node, child, node->children) {
        SKNode_draw(child, projection);
    }
}

static struct image *
SKView_foregroundcontent(struct node *node,
                         float backing_scale)
{
    struct node *scene = SKView_scene(node);
    struct matrix4 projection;
    float const w = node_frame(node, kRect3_Width);
    float const h = node_frame(node, kRect3_Height);
    if (scene) {
        matrix4_ortho(&projection, 0, w, 0, h, -1, 1);
        SKNode_draw(scene, &projection);
    }
    return NULL;
}

void
SKView_init(struct node *node, struct UIViewData *data)
{
    node->subclass.data = mem_alloc(sizeof(struct SKView), MEM_NODE);
    node->subclass.desc = SKView_properties;
    node->subclass.num_properties = kSKView_numCvars;
    node->children_type = kSKNode;
    data->foreground_content = SKView_foregroundcontent;
}
