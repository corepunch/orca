#include <code/api/api.h>
#include <code/sprite/SKPublic.h>
#include <code/ui/UIPublic.h>
#include <code/common/common.h>
#include <code/renderer/r_public.h>

enum {
    kSKSpriteNode_animation,
    kSKSpriteNode_animation2,
    kSKSpriteNode_image,
    kSKSpriteNode_freezeFrame,
    kSKSpriteNode_blendMode,
    kSKSpriteNode_rect,
    kSKSpriteNode_uvRect,
    kSKSpriteNode_numCvars,
};

void BlendMode(lua_State *L, int index, struct property *property);

static struct property_desc const
SKSpriteNodeFields[kSKSpriteNode_numCvars] =
{
    { 0xe145ee5d, "animation", PROPERTY_SPRITEANIMATION, NULL },
    { 0x101658bd, "animation2", PROPERTY_SPRITEANIMATION, NULL },
    { 0xb35135fa, "image", PROPERTY_IMAGE, NULL },
    { 0x4594e273, "freezeFrame", PROPERTY_NUMBER, NULL },
    { 0x615978cb, "blendMode", PROPERTY_STRING, property_desc_make_enum, BlendMode },
    { 0xeae44d07, "rect", PROPERTY_RECTANGLE, NULL },
    { 0x688243e0, "uvRect", PROPERTY_RECTANGLE, NULL },
};

struct SKSpriteNodeData {
    struct property *properties[kSKSpriteNode_numCvars];
};

struct SKSpriteNodeFormat {
    struct sprite_animation const *animation;
    struct sprite_animation const *animation2;
    struct sprite sprite;
    struct image *image;
    enum blend_mode blendMode;
};

static struct sprite const *
animation_sprite(struct sprite_animation const *animation,
                 int freezeFrame)
{
    int const frame = (int)(frame_start_time * animation->framerate / 1000);
    int const index = (freezeFrame >= 0 ? freezeFrame : frame) % animation->num_frames;
    return &animation->frames[index];
}

void
SKNode_anchor(struct node const *node,
              struct rect *rect)
{
    int const anchor = propertyL_int(SKNODE_PARAM(node, anchor), 0);
    if (anchor & kSKNodeAnchorRight) {
        rect->x += SKNode_referencesize(node).x;
    }
    if (anchor & kSKNodeAnchorTop) {
        rect->y += SKNode_referencesize(node).y;
    }
}

static struct rect
sprite_rect(struct node const *node,
            struct sprite const *sprite)
{
    struct rect rect = sprite->rect;
    rect.x = -rect.x;
    rect.y = -rect.y;
    SKNode_anchor(node, &rect);
    return rect;
}

static struct SKSpriteNodeFormat
SKSpriteNode_compile(struct SKSpriteNodeData const *sprite)
{
    struct SKSpriteNodeFormat fmt;
    memset(&fmt, 0, sizeof(struct SKSpriteNodeFormat));
    property_value(sprite->properties[kSKSpriteNode_image], PROPERTY_IMAGE, &fmt.image);
    property_value(sprite->properties[kSKSpriteNode_animation], PROPERTY_SPRITEANIMATION, &fmt.animation);
    property_value(sprite->properties[kSKSpriteNode_animation2], PROPERTY_SPRITEANIMATION, &fmt.animation2);
    fmt.blendMode = propertyL_int(sprite->properties[kSKSpriteNode_blendMode], BLEND_MODE_INHERIT);
    if (fmt.animation && fmt.animation->num_frames > 0) {
        int const freeze_frame = propertyL_int(sprite->properties[kSKSpriteNode_freezeFrame], -1);
        fmt.sprite = *animation_sprite(fmt.animation, freeze_frame);
        fmt.image = fmt.animation->image;
    } else if (fmt.image) {
        fmt.sprite.uv = (struct rect) { 0, 0, 1, 1 };
        property_value(sprite->properties[kSKSpriteNode_uvRect], PROPERTY_RECTANGLE, &fmt.sprite.uv);
        
        fmt.sprite.rect = (struct rect) { 0, 0, fmt.image->size.width * fmt.sprite.uv.width, fmt.image->size.height * fmt.sprite.uv.height };
        property_value(sprite->properties[kSKSpriteNode_rect], PROPERTY_RECTANGLE, &fmt.sprite.rect);
        
        fmt.sprite.rect.x = -fmt.sprite.rect.x;
        fmt.sprite.rect.y = -fmt.sprite.rect.y;
    }
    return fmt;
}

static struct model *
sprite_model(struct node const *node,
             struct sprite const *sprite)
{
    struct rect const frame = sprite_rect(node, sprite);
    struct rect const uv = {
        .x = sprite->uv.x,
        .y = sprite->uv.y + sprite->uv.height,
        .width = sprite->uv.width,
        .height = -sprite->uv.height
    };
    return re.model_rectangle(&frame, &uv, VERTEX_ORDER_CCW);
}

static struct model *empty_model() {
    return re.model_rectangle(&(struct rect) {0,0,5,5}, NULL, VERTEX_ORDER_DEFAULT);
}

static void
SKSpriteNode_updategeometry(struct node *node,
                            struct SKSpriteNodeFormat const *spritenode)
{
    struct SKNodeData *nodedata = node->class_data.data;
    if (node->geometry) {
        re.model_release(node->geometry);
        node->geometry = NULL;
    }
    if (nodedata->geometry2) {
        re.model_release(nodedata->geometry2);
        nodedata->geometry2 = NULL;
    }
    node->geometry = spritenode->image ? sprite_model(node, &spritenode->sprite) : empty_model();
    if (spritenode->animation2) {
        struct sprite const *sprite = animation_sprite(spritenode->animation2, -1);
        ((struct SKNodeData *)node->class_data.data)->geometry2 = sprite_model(node, sprite);
    }
}

static void
SKSpriteNode_draw(struct node *node,
                  struct viewdef *viewdef)
{
    struct viewentity viewentity;
    struct SKSpriteNodeFormat fmt = SKSpriteNode_compile(node->subclass.data);
    struct image const *image = fmt.animation ? fmt.animation->image : fmt.image;
    SKSpriteNode_updategeometry(node, &fmt);
    if (SKNodeData(node)->geometry2) {
        UIView_viewentity(node, &viewentity, fmt.animation2->image, SKNodeData(node)->geometry2);
        re.entity_draw(viewdef, &viewentity);
    }
    UIView_viewentity(node, &viewentity, image, node->geometry);
    re.entity_draw(viewdef, &viewentity);
}

static struct rect
SKSpriteNode_spriterect(struct node const *node)
{
    struct SKSpriteNodeFormat fmt = SKSpriteNode_compile(node->subclass.data);
    if (fmt.image) {
        return sprite_rect(node, &fmt.sprite);
    } else {
        return (struct rect) { 0, 0, 0, 0 };
    }
}

void
SKSpriteNode_init(struct node *node,
                  struct SKNodeData *data)
{
    node->subclass.data = mem_alloc(sizeof(struct SKSpriteNodeData), MEM_NODE);
    node->subclass.desc = SKSpriteNodeFields;
    node->subclass.num_properties = kSKSpriteNode_numCvars;
    
    data->sprite_rect = SKSpriteNode_spriterect;
    data->draw = SKSpriteNode_draw;
}
