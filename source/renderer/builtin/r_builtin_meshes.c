#include "../r_local.h"
#include <math.h>

#define QUAD(a, b, c, d) a, d, c, d, a, b,
#define SPHERE_SEGMENTS 16
#define ROUNDED_VERTICES 8

static DRAWVERT
R_MakeVertex(float x,
             float y,
             float z,
             float u,
             float v,
             float nx,
             float ny,
             float nz)
{
  DRAWVERT vertex;
  memset(&vertex, 0, sizeof(DRAWVERT));
  VEC3_Set(&vertex.xyz, x, y, z);
  VEC2_Set(&vertex.texcoord[0], u, v);
  VEC3_Set(&vertex.normal, nx, ny, nz);
  vertex.color.r = 255;
  vertex.color.g = 255;
  vertex.color.b = 255;
  vertex.color.a = 255;
  return vertex;
}

HRESULT
Model_CreatePlane(float w, float h, struct model** ppModel)
{
  DRAWSURF dsurf;
  DRAWVERT vertices[] = {
    R_MakeVertex(-w * 0.5f, -h * 0.5f, 0, 0, 0, 0, 0, 1),
    R_MakeVertex(w * 0.5f, -h * 0.5f, 0, 1, 0, 0, 0, 1),
    R_MakeVertex(-w * 0.5f, h * 0.5f, 0, 0, 1, 0, 0, 1),
    R_MakeVertex(w * 0.5f, h * 0.5f, 0, 1, 1, 0, 0, 1),
  };
  DRAWINDEX indices[] = { QUAD(0, 1, 2, 3) };
  DRAWSURFATTR attr[] = { VERTEX_SEMANTIC_POSITION,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_TEXCOORD0,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_NORMAL,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_COLOR,
                          VERTEX_ATTR_DATATYPE_UINT8 |
                            VERTEX_ATTR_DATATYPE_NORMALIZED,
                          VERTEX_SEMANTIC_COUNT };

  dsurf.vertices = vertices;
  dsurf.indices = indices;
  dsurf.neighbors = NULL;
  dsurf.numVertices = sizeof(vertices) / sizeof(DRAWVERT);
  dsurf.numIndices = sizeof(indices) / sizeof(DRAWINDEX);
  dsurf.numSubmeshes = 0;

  return Model_Create(attr, &dsurf, ppModel);
}

HRESULT
Model_CreatePlaneXZ(float w, float h, struct model** ppModel)
{
  DRAWSURF dsurf;
  DRAWVERT vertices[] = {
    R_MakeVertex(-w * 0.5f, 0, -h * 0.5f, 1, 1, 0, 1, 0),
    R_MakeVertex(w * 0.5f, 0, -h * 0.5f, 0, 1, 0, 1, 0),
    R_MakeVertex(-w * 0.5f, 0, h * 0.5f, 1, 0, 0, 1, 0),
    R_MakeVertex(w * 0.5f, 0, h * 0.5f, 0, 0, 0, 1, 0),
  };
  DRAWINDEX indices[] = { QUAD(0, 1, 2, 3) };
  DRAWSURFATTR attr[] = { VERTEX_SEMANTIC_POSITION,
    VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_TEXCOORD0,
    VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_NORMAL,
    VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_COLOR,
    VERTEX_ATTR_DATATYPE_UINT8 |
    VERTEX_ATTR_DATATYPE_NORMALIZED,
    VERTEX_SEMANTIC_COUNT };
  
  dsurf.vertices = vertices;
  dsurf.indices = indices;
  dsurf.neighbors = NULL;
  dsurf.numVertices = sizeof(vertices) / sizeof(DRAWVERT);
  dsurf.numIndices = sizeof(indices) / sizeof(DRAWINDEX);
  dsurf.numSubmeshes = 0;
  
  return Model_Create(attr, &dsurf, ppModel);
}


HRESULT
Model_CreateBox(float w, float h, float d, struct model** ppModel)
{
  DRAWSURF dsurf;
  DRAWVERT vertices[] = {
    R_MakeVertex(-w, -h, d, 0, 0, 0, 0, 1),
    R_MakeVertex(w, -h, d, 1, 0, 0, 0, 1),
    R_MakeVertex(-w, h, d, 0, 1, 0, 0, 1),
    R_MakeVertex(w, h, d, 1, 1, 0, 0, 1),

    R_MakeVertex(w, -h, d, 0, 0, 1, 0, 0),
    R_MakeVertex(w, -h, -d, 1, 0, 1, 0, 0),
    R_MakeVertex(w, h, d, 0, 1, 1, 0, 0),
    R_MakeVertex(w, h, -d, 1, 1, 1, 0, 0),

    R_MakeVertex(w, -h, -d, 0, 0, 0, 0, -1),
    R_MakeVertex(-w, -h, -d, 1, 0, 0, 0, -1),
    R_MakeVertex(w, h, -d, 0, 1, 0, 0, -1),
    R_MakeVertex(-w, h, -d, 1, 1, 0, 0, -1),

    R_MakeVertex(-w, -h, -d, 0, 0, -1, 0, 0),
    R_MakeVertex(-w, -h, d, 1, 0, -1, 0, 0),
    R_MakeVertex(-w, h, -d, 0, 1, -1, 0, 0),
    R_MakeVertex(-w, h, d, 1, 1, -1, 0, 0),

    R_MakeVertex(-w, -h, -d, 0, 0, 0, -1, 0),
    R_MakeVertex(w, -h, -d, 1, 0, 0, -1, 0),
    R_MakeVertex(-w, -h, d, 0, 1, 0, -1, 0),
    R_MakeVertex(w, -h, d, 1, 1, 0, -1, 0),

    R_MakeVertex(-w, h, d, 0, 0, 0, 1, 0),
    R_MakeVertex(w, h, d, 1, 0, 0, 1, 0),
    R_MakeVertex(-w, h, -d, 0, 1, 0, 1, 0),
    R_MakeVertex(w, h, -d, 1, 1, 0, 1, 0),
  };
  DRAWINDEX indices[] = { QUAD(0, 1, 2, 3) QUAD(4, 5, 6, 7) QUAD(8, 9, 10, 11)
                            QUAD(12, 13, 14, 15) QUAD(16, 17, 18, 19)
                              QUAD(20, 21, 22, 23) };
  DRAWSURFATTR attr[] = { VERTEX_SEMANTIC_POSITION,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_TEXCOORD0,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_NORMAL,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_COLOR,
                          VERTEX_ATTR_DATATYPE_UINT8 |
                            VERTEX_ATTR_DATATYPE_NORMALIZED,
                          VERTEX_SEMANTIC_COUNT };

  dsurf.vertices = vertices;
  dsurf.indices = indices;
  dsurf.neighbors = NULL;
  dsurf.numVertices = sizeof(vertices) / sizeof(DRAWVERT);
  dsurf.numIndices = sizeof(indices) / sizeof(DRAWINDEX);
  dsurf.numSubmeshes = 0;

  return Model_Create(attr, &dsurf, ppModel);
}

struct model*
Model_CreateError(void)
{
  float x = 0.1f;
  float y = 0.1f;
  float z = 0.1f;

  DRAWSURF dsurf;
  DRAWVERT vertices[] = {
    R_MakeVertex(x, 0, 0, 0, 0, 0, 0, 1),  // 0
    R_MakeVertex(0, y, 0, 0, 0, 0, 0, 1),  // 1
    R_MakeVertex(-x, 0, 0, 0, 0, 0, 0, 1), // 2
    R_MakeVertex(0, -y, 0, 0, 0, 0, 0, 1), // 3
    R_MakeVertex(0, 0, z, 0, 0, 0, 0, 1),  // 4
    R_MakeVertex(0, 0, -z, 0, 0, 0, 0, 1), // 5
  };
  DRAWINDEX indices[] = {
    4, 0, 1, 4, 1, 2, 4, 2, 3, 4, 3, 0, 5, 1, 0, 5, 2, 1, 5, 3, 1, 5, 0, 3,
  };
  DRAWSURFATTR attr[] = { VERTEX_SEMANTIC_POSITION,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_TEXCOORD0,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_NORMAL,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_COLOR,
                          VERTEX_ATTR_DATATYPE_UINT8 |
                            VERTEX_ATTR_DATATYPE_NORMALIZED,
                          VERTEX_SEMANTIC_COUNT };

  dsurf.vertices = vertices;
  dsurf.indices = indices;
  dsurf.neighbors = NULL;
  dsurf.numVertices = sizeof(vertices) / sizeof(DRAWVERT);
  dsurf.numIndices = sizeof(indices) / sizeof(DRAWINDEX);
  dsurf.numSubmeshes = 0;

  struct model* model = ZeroAlloc(sizeof(struct model));

  Model_Init(model, attr, &dsurf);

  return model;
}

HRESULT
Model_CreateSphere(float radius, struct model** ppModel)
{
#define segsx (SPHERE_SEGMENTS * 2)
#define segsy SPHERE_SEGMENTS
  uint32_t const start = 0;
  uint32_t cvert = 0;
  uint32_t cidx = 0;
  DRAWVERT vertices[(segsx + 1) * (segsy + 1)];
  DRAWINDEX indices[segsx * segsy * 6];
  DRAWSURF dsurf;
  DRAWSURFATTR attr[] = { VERTEX_SEMANTIC_POSITION,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_TEXCOORD0,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_NORMAL,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_COLOR,
                          VERTEX_ATTR_DATATYPE_UINT8 |
                            VERTEX_ATTR_DATATYPE_NORMALIZED,
                          VERTEX_SEMANTIC_COUNT };

  for (uint32_t y = 0; y <= segsy; y++) {
    float angley = y * M_PI / segsy;
    float r = sin(angley);
    float h = cos(angley);
    for (uint32_t x = 0; x <= segsx; x++) {
      float anglex = 2 * x * M_PI / segsx;
      float xValue = cos(anglex) * r;
      float zValue = sin(anglex) * r;
      float yValue = h;

      // Add the vertex to your vertices array
      vertices[cvert++] = R_MakeVertex(xValue * radius,
                                       yValue * radius,
                                       zValue * radius,
                                       (float)x / segsx,
                                       1.0f - (float)y / segsy,
                                       xValue,
                                       yValue,
                                       zValue);
    }
  }
  for (uint32_t y = 0; y < segsy; y++) {
    for (uint32_t x = 0; x < segsx; x++) {
      uint32_t a = x + y * (segsx + 1) + start;
      uint32_t s = (segsx + 1);
      DRAWINDEX quad[] = { a, a + s + 1, a + s, a + s + 1, a, a + 1 };
      memcpy(indices + cidx, quad, sizeof(quad));
      cidx += 6;
    }
  }

  dsurf.vertices = vertices;
  dsurf.indices = indices;
  dsurf.neighbors = NULL;
  dsurf.numVertices = (uint32_t)sizeof(vertices) / sizeof(DRAWVERT);
  dsurf.numIndices = (uint32_t)sizeof(indices) / sizeof(DRAWINDEX);
  dsurf.numSubmeshes = 0;
  return Model_Create(attr, &dsurf, ppModel);
}

DRAWVERT
vertex_new2(float x, float y, float u, float v)
{
  DRAWVERT vertex;
  memset(&vertex, 0, sizeof(DRAWVERT));
  VEC3_Set(&vertex.xyz, x, y, 0);
  VEC2_Set(&vertex.texcoord[0], u, v);
  vertex.color.r = 255;
  vertex.color.g = 255;
  vertex.color.b = 255;
  vertex.color.a = 255;
  return vertex;
}

HRESULT
Model_CreateRectangle(struct rect const* f,
                      struct rect const* uv,
                      enum vertex_order vertex_order,
                      struct model** ppModel)
{
  struct rect const d = uv ? *uv : (struct rect){ 0, 0, 1, 1 };
  DRAWSURF dsurf;
  DRAWVERT vertices[] = {
    vertex_new2(f->x, f->y, d.x, d.y + d.height),
    vertex_new2(f->x + f->width, f->y, d.x + d.width, d.y + d.height),
    vertex_new2(f->x, f->y + f->height, d.x, d.y),
    vertex_new2(f->x + f->width, f->y + f->height, d.x + d.width, d.y),
  };
  DRAWINDEX indices[] = { 3, 1, 0, 0, 2, 3 };
  DRAWSURFATTR attr[] = { VERTEX_SEMANTIC_POSITION,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_TEXCOORD0,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_COLOR,
                          VERTEX_ATTR_DATATYPE_UINT8 |
                            VERTEX_ATTR_DATATYPE_NORMALIZED,
                          VERTEX_SEMANTIC_COUNT };

  dsurf.vertices = vertices;
  dsurf.indices = indices;
  dsurf.neighbors = NULL;
  dsurf.numVertices = sizeof(vertices) / sizeof(DRAWVERT);
  dsurf.numIndices = sizeof(indices) / sizeof(DRAWINDEX);
  dsurf.numSubmeshes = 0;

  return Model_Create(attr, &dsurf, ppModel);
}

// Creates a rounded rectangle mesh for shader-based rendering with variable radius.
// Generates a mesh in 0...1 coordinate space with corners collapsed to single points.
// The mesh uses texcoord[0] to store offset directions for shader-based corner expansion.
HRESULT
Model_CreateRoundedRectangle(struct model** ppModel)
{
  #define ADD_TRIANGLE(a, b, c) tris[iidx++] = a; tris[iidx++] = b; tris[iidx++] = c;
  
  // 4 corners * ROUNDED_VERTICES + 4 center vertices (one per corner)
  DRAWVERT verts[4 * ROUNDED_VERTICES + 4];
  DRAWINDEX tris[4 * (ROUNDED_VERTICES + 4) * 3];
  DRAWVERT v;
  uint32_t vidx = 0, iidx = 0;
  vec2_t crn[] = {{1,1}, {0,1}, {0,0}, {1,0}};
  
  // Generate all 4 corners in a single loop
  for (uint32_t c = 0; c < 4; c++) {
    vec2_t d = { 1 - crn[c].x * 2, 1 - crn[c].y * 2 };
    // Add center vertex for this corner (for the rounded corner fan)
    memset(&v, 0, sizeof(DRAWVERT));
    VEC3_Set(&v.xyz, crn[c].x, crn[c].y, 0);
    VEC2_Set(&v.texcoord[0], d.x, d.y);
    *(int*)&v.color = -1;
    verts[vidx++] = v;

    // Generate rounded corner vertices
    for (uint32_t i = 0; i < ROUNDED_VERTICES; i++) {
      float angle = (float)i / (float)(ROUNDED_VERTICES - 1) * M_PI_2; // 0 to PI/2
      memset(&v, 0, sizeof(DRAWVERT));
      VEC3_Set(&v.xyz, crn[c].x, crn[c].y, 0);
      switch(c) {
        case 0: VEC2_Set(&v.texcoord[0], d.x+cos(angle), d.y+sin(angle)); break;
        case 1: VEC2_Set(&v.texcoord[0], d.x-sin(angle), d.y+cos(angle)); break;
        case 2: VEC2_Set(&v.texcoord[0], d.x-cos(angle), d.y-sin(angle)); break;
        case 3: VEC2_Set(&v.texcoord[0], d.x+sin(angle), d.y-cos(angle)); break;
      }
      *(int*)&v.color = -1;
      verts[vidx++] = v;
    }
  }
  // Generate indices around the rectangle perimeter
  for (uint32_t a = 0, b = 1; a < 4; a++, b = (b + 1) % 4) {
    uint32_t centerIdx = a * (ROUNDED_VERTICES + 1);
    uint32_t cornerStart = centerIdx + 1;
    uint32_t nextCornerStart = b * (ROUNDED_VERTICES + 1);
    // Create fan triangles for the rounded corner
    for (uint32_t i = 0; i < ROUNDED_VERTICES - 1; i++) {
      ADD_TRIANGLE(centerIdx, cornerStart + i, cornerStart + i + 1);
    }
    // Connect last vertex of this corner to first vertex of next corner
    ADD_TRIANGLE(centerIdx, cornerStart + ROUNDED_VERTICES - 1, nextCornerStart + 1);
    ADD_TRIANGLE(centerIdx, nextCornerStart, nextCornerStart + 1);
  }

  // Generate base rectangle
  ADD_TRIANGLE(3 * (ROUNDED_VERTICES + 1), 1 * (ROUNDED_VERTICES + 1), 0 * (ROUNDED_VERTICES + 1));
  ADD_TRIANGLE(1 * (ROUNDED_VERTICES + 1), 2 * (ROUNDED_VERTICES + 1), 3 * (ROUNDED_VERTICES + 1));

  DRAWSURFATTR attr[] = { VERTEX_SEMANTIC_POSITION,
    VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_TEXCOORD0,
    VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_COLOR,
    VERTEX_ATTR_DATATYPE_UINT8 |
    VERTEX_ATTR_DATATYPE_NORMALIZED,
    VERTEX_SEMANTIC_COUNT };
  
  DRAWSURF dsurf;
  dsurf.vertices = verts;
  dsurf.indices = tris;
  dsurf.neighbors = NULL;
  dsurf.numVertices = vidx;
  dsurf.numIndices = iidx;
  dsurf.numSubmeshes = 0;
  
  return Model_Create(attr, &dsurf, ppModel);
  
  #undef ADD_TRIANGLE
}

// Creates a rounded border (outline) mesh for shader-based rendering with variable radius.
// Similar to Model_CreateRoundedRectangle but only generates the border, not the filled interior.
// The mesh uses texcoord[0] to store offset directions for shader-based corner expansion.
HRESULT
Model_CreateRoundedBorder(struct model** ppModel)
{
  #define ADD_TRIANGLE(a, b, c) tris[iidx++] = a; tris[iidx++] = b; tris[iidx++] = c;
  
  // For a border, we need both inner and outer vertices for each position
  // 4 corners * ROUNDED_VERTICES vertices * 2 (inner + outer) + 4 center vertices * 2
  DRAWVERT verts[4 * ROUNDED_VERTICES * 2 + 4 * 2];
  DRAWINDEX tris[4 * (ROUNDED_VERTICES * 2 + 4) * 3];
  DRAWVERT v;
  uint32_t vidx = 0, iidx = 0;
  vec2_t crn[] = {{1,1}, {0,1}, {0,0}, {1,0}};
  
  // Generate all 4 corners in a single loop
  for (uint32_t c = 0; c < 4; c++) {
    vec2_t d = { 1 - crn[c].x * 2, 1 - crn[c].y * 2 };
    
    // Add outer center vertex for this corner
    memset(&v, 0, sizeof(DRAWVERT));
    VEC3_Set(&v.xyz, crn[c].x, crn[c].y, 0);
    VEC2_Set(&v.texcoord[0], d.x, d.y);
    *(int*)&v.color = -1;
    verts[vidx++] = v;
    
    // Add inner center vertex for this corner (texcoord[1] could mark this as inner in shader)
    memset(&v, 0, sizeof(DRAWVERT));
    VEC3_Set(&v.xyz, crn[c].x, crn[c].y, 0);
    VEC2_Set(&v.texcoord[0], d.x, d.y);
    VEC2_Set(&v.texcoord[1], -1, -1); // Mark as inner vertex
    *(int*)&v.color = -1;
    verts[vidx++] = v;

    // Generate rounded corner vertices (outer then inner for each angle)
    for (uint32_t i = 0; i < ROUNDED_VERTICES; i++) {
      float angle = (float)i / (float)(ROUNDED_VERTICES - 1) * M_PI_2; // 0 to PI/2
      
      // Outer vertex
      memset(&v, 0, sizeof(DRAWVERT));
      VEC3_Set(&v.xyz, crn[c].x, crn[c].y, 0);
      switch(c) {
        case 0: VEC2_Set(&v.texcoord[0], d.x+cos(angle), d.y+sin(angle)); break;
        case 1: VEC2_Set(&v.texcoord[0], d.x-sin(angle), d.y+cos(angle)); break;
        case 2: VEC2_Set(&v.texcoord[0], d.x-cos(angle), d.y-sin(angle)); break;
        case 3: VEC2_Set(&v.texcoord[0], d.x+sin(angle), d.y-cos(angle)); break;
      }
      *(int*)&v.color = -1;
      verts[vidx++] = v;
      
      // Inner vertex (same direction but marked with texcoord[1])
      memset(&v, 0, sizeof(DRAWVERT));
      VEC3_Set(&v.xyz, crn[c].x, crn[c].y, 0);
      switch(c) {
        case 0: VEC2_Set(&v.texcoord[0], d.x+cos(angle), d.y+sin(angle)); break;
        case 1: VEC2_Set(&v.texcoord[0], d.x-sin(angle), d.y+cos(angle)); break;
        case 2: VEC2_Set(&v.texcoord[0], d.x-cos(angle), d.y-sin(angle)); break;
        case 3: VEC2_Set(&v.texcoord[0], d.x+sin(angle), d.y-cos(angle)); break;
      }
      VEC2_Set(&v.texcoord[1], -1, -1); // Mark as inner vertex
      *(int*)&v.color = -1;
      verts[vidx++] = v;
    }
  }
  
  // Generate quad strips for the border around the rectangle perimeter
  for (uint32_t a = 0, b = 1; a < 4; a++, b = (b + 1) % 4) {
    uint32_t baseIdx = a * (ROUNDED_VERTICES * 2 + 2);
    uint32_t outerCenter = baseIdx;
    uint32_t innerCenter = baseIdx + 1;
    uint32_t cornerStart = baseIdx + 2;
    uint32_t nextBaseIdx = b * (ROUNDED_VERTICES * 2 + 2);
    uint32_t nextOuterCenter = nextBaseIdx;
    uint32_t nextInnerCenter = nextBaseIdx + 1;
    uint32_t nextCornerStart = nextBaseIdx + 2;
    
    // Create quad strip for the rounded corner
    for (uint32_t i = 0; i < ROUNDED_VERTICES - 1; i++) {
      uint32_t outerCurr = cornerStart + i * 2;
      uint32_t innerCurr = cornerStart + i * 2 + 1;
      uint32_t outerNext = cornerStart + (i + 1) * 2;
      uint32_t innerNext = cornerStart + (i + 1) * 2 + 1;
      
      // Two triangles forming a quad
      ADD_TRIANGLE(outerCurr, innerCurr, outerNext);
      ADD_TRIANGLE(innerCurr, innerNext, outerNext);
    }
    
    // Connect last vertex of this corner to first vertex of next corner
    uint32_t outerLast = cornerStart + (ROUNDED_VERTICES - 1) * 2;
    uint32_t innerLast = cornerStart + (ROUNDED_VERTICES - 1) * 2 + 1;
    uint32_t outerFirst = nextCornerStart;
    uint32_t innerFirst = nextCornerStart + 1;
    
    ADD_TRIANGLE(outerLast, innerLast, outerFirst);
    ADD_TRIANGLE(innerLast, innerFirst, outerFirst);
  }

  DRAWSURFATTR attr[] = { VERTEX_SEMANTIC_POSITION,
    VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_TEXCOORD0,
    VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_TEXCOORD1,
    VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_COLOR,
    VERTEX_ATTR_DATATYPE_UINT8 |
    VERTEX_ATTR_DATATYPE_NORMALIZED,
    VERTEX_SEMANTIC_COUNT };
  
  DRAWSURF dsurf;
  dsurf.vertices = verts;
  dsurf.indices = tris;
  dsurf.neighbors = NULL;
  dsurf.numVertices = vidx;
  dsurf.numIndices = iidx;
  dsurf.numSubmeshes = 0;
  
  return Model_Create(attr, &dsurf, ppModel);
  
  #undef ADD_TRIANGLE
}
