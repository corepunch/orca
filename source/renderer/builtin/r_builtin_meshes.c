#include "../r_local.h"

#define QUAD(a, b, c, d) a, d, c, d, a, b,
#define SPHERE_SEGMENTS 16
#define ROUNDED_VERTICES 16  // Number of vertices per corner in rounded rectangle mesh

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
// Generates a mesh in -1...1 coordinate space with center near 0 (top-right: 0.001..1.0).
// The mesh consists of a center vertex and 4 corners with ROUNDED_VERTICES each.
HRESULT
Model_CreateRoundedRectangle(struct model** ppModel)
{
  DRAWVERT vertices[4 * ROUNDED_VERTICES + 1];
  DRAWINDEX indices[4 * ROUNDED_VERTICES * 3];
  
  uint32_t vidx = 0;
  uint32_t iidx = 0;
  
  // Center vertex at origin (very close to 0)
  vertices[vidx++] = vertex_new2(0.0f, 0.0f, 0.5f, 0.5f);
  uint32_t centerIdx = 0;
  
  // Generate vertices for each corner
  // Top-right corner: x from 0.001 to 1.0, y from 0.001 to 1.0
  for (uint32_t i = 0; i < ROUNDED_VERTICES; i++) {
    float angle = (float)i / (float)(ROUNDED_VERTICES - 1) * M_PI_2; // 0 to PI/2
    float x = 0.001f + (1.0f - 0.001f) * cos(angle);
    float y = 0.001f + (1.0f - 0.001f) * sin(angle);
    float u = 0.5f + x * 0.5f;
    float v = 0.5f + y * 0.5f;
    vertices[vidx++] = vertex_new2(x, y, u, v);
  }
  
  // Top-left corner: x from -0.001 to -1.0, y from 0.001 to 1.0
  for (uint32_t i = 0; i < ROUNDED_VERTICES; i++) {
    float angle = (float)i / (float)(ROUNDED_VERTICES - 1) * M_PI_2; // 0 to PI/2
    float x = -0.001f - (1.0f - 0.001f) * sin(angle);
    float y = 0.001f + (1.0f - 0.001f) * cos(angle);
    float u = 0.5f + x * 0.5f;
    float v = 0.5f + y * 0.5f;
    vertices[vidx++] = vertex_new2(x, y, u, v);
  }
  
  // Bottom-left corner: x from -0.001 to -1.0, y from -0.001 to -1.0
  for (uint32_t i = 0; i < ROUNDED_VERTICES; i++) {
    float angle = (float)i / (float)(ROUNDED_VERTICES - 1) * M_PI_2; // 0 to PI/2
    float x = -0.001f - (1.0f - 0.001f) * cos(angle);
    float y = -0.001f - (1.0f - 0.001f) * sin(angle);
    float u = 0.5f + x * 0.5f;
    float v = 0.5f + y * 0.5f;
    vertices[vidx++] = vertex_new2(x, y, u, v);
  }
  
  // Bottom-right corner: x from 0.001 to 1.0, y from -0.001 to -1.0
  for (uint32_t i = 0; i < ROUNDED_VERTICES; i++) {
    float angle = (float)i / (float)(ROUNDED_VERTICES - 1) * M_PI_2; // 0 to PI/2
    float x = 0.001f + (1.0f - 0.001f) * sin(angle);
    float y = -0.001f - (1.0f - 0.001f) * cos(angle);
    float u = 0.5f + x * 0.5f;
    float v = 0.5f + y * 0.5f;
    vertices[vidx++] = vertex_new2(x, y, u, v);
  }
  
  // Generate indices for triangular fans (one per corner)
  for (uint32_t corner = 0; corner < 4; corner++) {
    uint32_t cornerStart = 1 + corner * ROUNDED_VERTICES;
    for (uint32_t i = 0; i < ROUNDED_VERTICES - 1; i++) {
      indices[iidx++] = centerIdx;
      indices[iidx++] = cornerStart + i;
      indices[iidx++] = cornerStart + i + 1;
    }
    // Connect last vertex of this corner to first vertex of next corner
    uint32_t nextCornerStart = 1 + ((corner + 1) % 4) * ROUNDED_VERTICES;
    indices[iidx++] = centerIdx;
    indices[iidx++] = cornerStart + ROUNDED_VERTICES - 1;
    indices[iidx++] = nextCornerStart;
  }
  
  DRAWSURFATTR attr[] = { VERTEX_SEMANTIC_POSITION,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_TEXCOORD0,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_COLOR,
                          VERTEX_ATTR_DATATYPE_UINT8 |
                            VERTEX_ATTR_DATATYPE_NORMALIZED,
                          VERTEX_SEMANTIC_COUNT };
  
  DRAWSURF dsurf;
  dsurf.vertices = vertices;
  dsurf.indices = indices;
  dsurf.neighbors = NULL;
  dsurf.numVertices = vidx;
  dsurf.numIndices = iidx;
  dsurf.numSubmeshes = 0;
  
  return Model_Create(attr, &dsurf, ppModel);
}
