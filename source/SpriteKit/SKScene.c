#include <code/api/api.h>
#include <code/sprite/SKPublic.h>
#include <code/common/common.h>
#include <code/renderer/r_public.h>

enum {
    kSKScene_camera,
    kSKScene_renderPass,
    kSKScene_numCvars,
};

static struct property_desc const SKScene_properties[kSKScene_numCvars] = {
    { 0x9ebe984e, "camera", PROPERTY_NODE, NULL },
    { 0x0c2884a0, "renderPass", PROPERTY_NODE, NULL },
};

struct SKScene {
    struct property *properties[kSKScene_numCvars];
};

struct node *
SKScene_camera(struct node const *node)
{
    struct SKScene const *scene = node->subclass.data;
    struct node *camera = NULL;
    property_value(scene->properties[kSKScene_camera], PROPERTY_NODE, &camera);
    return camera;
}

struct node *
SKScene_renderpass(struct node const *node)
{
    struct SKScene const *SKScene = node->subclass.data;
    struct node *render_pass = NULL;
    property_value(SKScene->properties[kSKScene_renderPass], PROPERTY_NODE, &render_pass);
    return render_pass;
}

void
SKScene_init(struct node *node, struct SKNodeData *data)
{
    node->subclass.data = mem_alloc(sizeof(struct SKScene), MEM_NODE);
    node->subclass.desc = SKScene_properties;
    node->subclass.num_properties = kSKScene_numCvars;
}
