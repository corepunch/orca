#ifndef SKPublic_h
#define SKPublic_h

#include <code/client/node.h>

#define SKNODE_PARAM(VAR, NAME) class_property(&VAR->class_data, kSKNode_##NAME)

enum {
    kSKNode_anchor,
    kSKNode_numCvars,
};

enum {
    kSKNodeAnchorLeft = 1,
    kSKNodeAnchorRight = 2,
    kSKNodeAnchorTop = 4,
    kSKNodeAnchorBottom = 8,
    kSKNodeAnchorMiddle = 16,
};

bool SKNode_containspoint(struct node const *, struct vector2 const *);
void SKNode_updatematrix(struct node *, struct matrix4 const *, float);
struct SKNodeData const *SKNodeData(struct node const *);
struct vector2 SKNode_referencesize(struct node const *);

struct SKNodeData {
    struct property *parms[kSKNode_numCvars];
    struct rect (*sprite_rect)(struct node const *);
    void (*draw)(struct node *, struct viewdef *view);
    struct model *geometry2;
};

#endif
