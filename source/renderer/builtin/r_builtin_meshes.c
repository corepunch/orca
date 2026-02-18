#include "../r_local.h"

#define QUAD(a, b, c, d) a, d, c, d, a, b,
#define SPHERE_SEGMENTS 16
#define ROUNDED_VERTICES 16  // Number of vertices per corner in rounded rectangle mesh

static DRAWVERT
R_MakeVertex(float x, float y, float z, float u, float v, float nx, float ny, float nz)
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

static DRAWVERT
R_MakeVertexWithWeight(float x, float y, float z, float u, float v, float nx, float ny, float nz, float wx, float wy, float wz)
{
  DRAWVERT vertex;
  memset(&vertex, 0, sizeof(DRAWVERT));
  VEC3_Set(&vertex.xyz, x, y, z);
  VEC2_Set(&vertex.texcoord[0], u, v);
  VEC3_Set(&vertex.normal, nx, ny, nz);
  VEC4_Set(&vertex.weight, wx, wy, wz, 0.0f);
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
      float a = (float)i / (float)(ROUNDED_VERTICES - 1) * M_PI_2; // 0 to PI/2
      float x = cos(a), y = sin(a);
      vec2_t b[] = {{x,y}, {-y,x}, {-x,-y}, {y,-x}};
      memset(&v, 0, sizeof(DRAWVERT));
      *(int*)&v.color = -1;
      VEC3_Set(&v.xyz, crn[c].x, crn[c].y, 0);
      VEC2_Set(&v.texcoord[0], d.x+b[c].x, d.y+b[c].y);
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

  DRAWSURFATTR attr[] = {
    VERTEX_SEMANTIC_POSITION, VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_TEXCOORD0, VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_COLOR, VERTEX_ATTR_DATATYPE_UINT8 | VERTEX_ATTR_DATATYPE_NORMALIZED,
    VERTEX_SEMANTIC_COUNT };
  
  DRAWSURF dsurf;
  dsurf.vertices = verts;
  dsurf.indices = tris;
  dsurf.neighbors = NULL;
  dsurf.numVertices = vidx;
  dsurf.numIndices = iidx;
  dsurf.numSubmeshes = 0;
  
  return Model_Create(attr, &dsurf, ppModel);
}

HRESULT
Model_CreateRoundedBorder(struct model** ppModel)
{
  DRAWVERT verts[8 * ROUNDED_VERTICES];
  DRAWINDEX tris[24 * ROUNDED_VERTICES];
  DRAWVERT v;
  uint32_t vidx = 0, iidx = 0;
  vec2_t crn[] = {{1,1}, {0,1}, {0,0}, {1,0}};
  
  // Generate all 4 corners in a single loop
  for (uint32_t c = 0; c < 4; c++) {
    vec2_t d = { 1 - crn[c].x * 2, 1 - crn[c].y * 2 };
    // Generate rounded corner vertices
    for (uint32_t i = 0; i < ROUNDED_VERTICES; i++) {
      float a = (float)i / (float)(ROUNDED_VERTICES - 1) * M_PI_2; // 0 to PI/2
      float x = cos(a), y = sin(a);
      vec2_t b[] = {{x,y}, {-y,x}, {-x,-y}, {y,-x}};
      memset(&v, 0, sizeof(DRAWVERT));
      *(int*)&v.color = -1;
      VEC3_Set(&v.xyz, crn[c].x, crn[c].y, 0);
      VEC2_Set(&v.texcoord[0], d.x+b[c].x, d.y+b[c].y);
      VEC2_Set(&v.texcoord[1], 0, 0);
      verts[vidx++] = v;
      VEC2_Set(&v.texcoord[1], b[c].x, b[c].y);
      verts[vidx++] = v;
    }
  }
  // Generate indices around the rectangle perimeter
  for (uint32_t i = 0, d = 8 * ROUNDED_VERTICES; i < d; i++) {
    ADD_TRIANGLE(i, (i + 1) % d, (i + 2) % d);
  }
  
  DRAWSURFATTR attr[] = {
    VERTEX_SEMANTIC_POSITION, VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_TEXCOORD0, VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_TEXCOORD1, VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_COLOR, VERTEX_ATTR_DATATYPE_UINT8 | VERTEX_ATTR_DATATYPE_NORMALIZED,
    VERTEX_SEMANTIC_COUNT };
  
  DRAWSURF dsurf;
  dsurf.vertices = verts;
  dsurf.indices = tris;
  dsurf.neighbors = NULL;
  dsurf.numVertices = vidx;
  dsurf.numIndices = iidx;
  dsurf.numSubmeshes = 0;
  
  return Model_Create(attr, &dsurf, ppModel);
}

HRESULT
Model_CreateRoundedBox(float width, float height, float depth, float radius, struct model** ppModel)
{
  if (radius <= 0.0f) return Model_CreateBox(width * 0.5f, height * 0.5f, depth * 0.5f, ppModel);
  
  float maxRadius = width < height ? (width < depth ? width : depth) : (height < depth ? height : depth);
  maxRadius *= 0.5f;
  if (radius > maxRadius) radius = maxRadius;

  #define SEGS 8
  
  float boxWidth = width - 2.0f * radius;
  float boxHeight = height - 2.0f * radius;
  float boxDepth = depth - 2.0f * radius;
  
  // Calculate vertex and index counts
  // 8 corners * (SEGS+1)^2 vertices per corner
  // 12 edges * (SEGS+1) * 2 vertices per edge
  // 6 faces * 4 vertices per face
  uint32_t cornerVerts = 8 * (SEGS + 1) * (SEGS + 1);
  uint32_t edgeVerts = 12 * (SEGS + 1) * 2;
  uint32_t faceVerts = 6 * 4;
  uint32_t maxVertices = cornerVerts + edgeVerts + faceVerts;
  
  uint32_t cornerIndices = 8 * SEGS * SEGS * 6;
  uint32_t edgeIndices = 12 * SEGS * 6;
  uint32_t faceIndices = 6 * 6;
  uint32_t maxIndices = cornerIndices + edgeIndices + faceIndices;
  
  DRAWVERT verts[maxVertices];
  DRAWINDEX tris[maxIndices];
  uint32_t vidx = 0, iidx = 0;
  
  // Define 8 corner positions (centers of the corner spheres)
  vec3_t corners[8] = {
    { boxWidth * 0.5f,  boxHeight * 0.5f,  boxDepth * 0.5f},  // 0: +X +Y +Z
    {-boxWidth * 0.5f,  boxHeight * 0.5f,  boxDepth * 0.5f},  // 1: -X +Y +Z
    {-boxWidth * 0.5f, -boxHeight * 0.5f,  boxDepth * 0.5f},  // 2: -X -Y +Z
    { boxWidth * 0.5f, -boxHeight * 0.5f,  boxDepth * 0.5f},  // 3: +X -Y +Z
    { boxWidth * 0.5f,  boxHeight * 0.5f, -boxDepth * 0.5f},  // 4: +X +Y -Z
    {-boxWidth * 0.5f,  boxHeight * 0.5f, -boxDepth * 0.5f},  // 5: -X +Y -Z
    {-boxWidth * 0.5f, -boxHeight * 0.5f, -boxDepth * 0.5f},  // 6: -X -Y -Z
    { boxWidth * 0.5f, -boxHeight * 0.5f, -boxDepth * 0.5f},  // 7: +X -Y -Z
  };
  
  // Define which octant each corner sphere occupies
  int octants[8][3] = {
    {1, 1, 1}, {-1, 1, 1}, {-1, -1, 1}, {1, -1, 1},
    {1, 1, -1}, {-1, 1, -1}, {-1, -1, -1}, {1, -1, -1}
  };
  
  // Generate vertices for each corner
  for (uint32_t corner = 0; corner < 8; corner++) {
    uint32_t cornerStart = vidx;
    
    for (uint32_t i = 0; i <= SEGS; i++) {
      float phi = (float)i / (float)SEGS * M_PI_2;
      
      for (uint32_t j = 0; j <= SEGS; j++) {
        float theta = (float)j / (float)SEGS * M_PI_2;
        
        // Compute sphere point in octant (normalized direction from corner center)
        float x = sin(phi) * cos(theta);
        float y = cos(phi);
        float z = sin(phi) * sin(theta);
        
        // Apply octant sign
        x *= octants[corner][0];
        y *= octants[corner][1];
        z *= octants[corner][2];
        
        // Position at corner center (to be offset by weight * radius in shader)
        float px = corners[corner].x;
        float py = corners[corner].y;
        float pz = corners[corner].z;
        
        float u = (float)j / (float)SEGS;
        float v = (float)i / (float)SEGS;
        
        // Store offset direction in weight field (shader will multiply by radius)
        verts[vidx++] = R_MakeVertexWithWeight(px, py, pz, u, v, x, y, z, x, y, z);
      }
    }
    
    // Generate indices for this corner
    for (uint32_t i = 0; i < SEGS; i++) {
      for (uint32_t j = 0; j < SEGS; j++) {
        uint32_t base = cornerStart + i * (SEGS + 1) + j;
        
        tris[iidx++] = base;
        tris[iidx++] = base + 1;
        tris[iidx++] = base + SEGS + 1;
        
        tris[iidx++] = base + 1;
        tris[iidx++] = base + SEGS + 2;
        tris[iidx++] = base + SEGS + 1;
      }
    }
  }
  
  // Generate 12 edges (cylinders connecting corners)
  // Edge definition: [corner1, corner2, axis (0=X, 1=Y, 2=Z)]
  int edges[12][3] = {
    // Front face (Z+) edges
    {0, 1, 0}, {1, 2, 1}, {2, 3, 0}, {3, 0, 1},
    // Back face (Z-) edges
    {4, 5, 0}, {5, 6, 1}, {6, 7, 0}, {7, 4, 1},
    // Connecting edges
    {0, 4, 2}, {1, 5, 2}, {2, 6, 2}, {3, 7, 2}
  };
  
  for (uint32_t e = 0; e < 12; e++) {
    int c1 = edges[e][0];
    int c2 = edges[e][1];
    int axis = edges[e][2];
    uint32_t edgeStart = vidx;
    
    // Determine the plane perpendicular to the edge
    for (uint32_t seg = 0; seg <= SEGS; seg++) {
      float t = (float)seg / (float)SEGS;
      
      // Interpolate along edge
      vec3_t edgePos = {
        corners[c1].x + t * (corners[c2].x - corners[c1].x),
        corners[c1].y + t * (corners[c2].y - corners[c1].y),
        corners[c1].z + t * (corners[c2].z - corners[c1].z)
      };
      
      // Create two vertices for the edge cylinder
      vec3_t offset1 = {0, 0, 0};
      vec3_t offset2 = {0, 0, 0};
      
      if (axis == 0) { // Edge along X
        offset1.y = octants[c1][1];
        offset1.z = octants[c1][2];
        offset2.y = octants[c1][1];
        offset2.z = 0;
      } else if (axis == 1) { // Edge along Y
        offset1.x = octants[c1][0];
        offset1.z = octants[c1][2];
        offset2.x = 0;
        offset2.z = octants[c1][2];
      } else { // Edge along Z
        offset1.x = octants[c1][0];
        offset1.y = octants[c1][1];
        offset2.x = octants[c1][0];
        offset2.y = 0;
      }
      
      verts[vidx++] = R_MakeVertexWithWeight(edgePos.x, edgePos.y, edgePos.z, 
                                              t, 0, offset1.x, offset1.y, offset1.z,
                                              offset1.x, offset1.y, offset1.z);
      verts[vidx++] = R_MakeVertexWithWeight(edgePos.x, edgePos.y, edgePos.z,
                                              t, 1, offset2.x, offset2.y, offset2.z,
                                              offset2.x, offset2.y, offset2.z);
    }
    
    // Generate indices for edge
    for (uint32_t seg = 0; seg < SEGS; seg++) {
      uint32_t base = edgeStart + seg * 2;
      
      tris[iidx++] = base;
      tris[iidx++] = base + 2;
      tris[iidx++] = base + 1;
      
      tris[iidx++] = base + 1;
      tris[iidx++] = base + 2;
      tris[iidx++] = base + 3;
    }
  }
  
  // Generate 6 flat faces
  // Face definition: [corner indices (4), normal axis]
  struct { int corners[4]; vec3_t normal; vec3_t weight; } faces[6] = {
    {{0, 3, 2, 1}, {0, 0, 1}, {0, 0, 0}},   // Front (+Z)
    {{4, 5, 6, 7}, {0, 0, -1}, {0, 0, 0}},  // Back (-Z)
    {{0, 1, 5, 4}, {0, 1, 0}, {0, 0, 0}},   // Top (+Y)
    {{3, 7, 6, 2}, {0, -1, 0}, {0, 0, 0}},  // Bottom (-Y)
    {{0, 4, 7, 3}, {1, 0, 0}, {0, 0, 0}},   // Right (+X)
    {{1, 2, 6, 5}, {-1, 0, 0}, {0, 0, 0}}   // Left (-X)
  };
  
  for (uint32_t f = 0; f < 6; f++) {
    uint32_t faceStart = vidx;
    
    for (uint32_t v = 0; v < 4; v++) {
      int ci = faces[f].corners[v];
      vec3_t pos = corners[ci];
      float u = (v == 1 || v == 2) ? 1.0f : 0.0f;
      float fv = (v == 2 || v == 3) ? 1.0f : 0.0f;
      
      verts[vidx++] = R_MakeVertexWithWeight(pos.x, pos.y, pos.z, u, fv,
                                              faces[f].normal.x, faces[f].normal.y, faces[f].normal.z,
                                              0, 0, 0); // No weight offset for flat faces
    }
    
    // Two triangles for the face
    tris[iidx++] = faceStart + 0;
    tris[iidx++] = faceStart + 1;
    tris[iidx++] = faceStart + 2;
    
    tris[iidx++] = faceStart + 0;
    tris[iidx++] = faceStart + 2;
    tris[iidx++] = faceStart + 3;
  }
  
  DRAWSURFATTR attr[] = {
    VERTEX_SEMANTIC_POSITION, VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_TEXCOORD0, VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_NORMAL, VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_WEIGHT, VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_COLOR, VERTEX_ATTR_DATATYPE_UINT8 | VERTEX_ATTR_DATATYPE_NORMALIZED,
    VERTEX_SEMANTIC_COUNT
  };

  DRAWSURF dsurf;
  dsurf.vertices = verts;
  dsurf.indices = tris;
  dsurf.submeshes = NULL;
  dsurf.neighbors = NULL;
  dsurf.numVertices = vidx;
  dsurf.numIndices = iidx;
  dsurf.numSubmeshes = 0;

  return Model_Create(attr, &dsurf, ppModel);

  #undef SEGS
}

HRESULT
Model_CreateCapsule(float width, float height, float depth, struct model** ppModel)
{
//  if (cylindriness < 0.0f) cylindriness = 0.0f;
//  if (cylindriness > 1.0f) cylindriness = 1.0f;
//  if (cylindriness < 0.001f) return Model_CreateBox(width * 0.5f, height * 0.5f, depth * 0.5f, ppModel);

  #define SEGS 16
  #define RINGS 8

  float cylindriness = 0.99f;
  float minDim = (height < depth) ? height : depth;
  float radius = minDim * 0.5f * cylindriness;
  float cylinderLength = width - 2.0f * radius;
  float heightScale = height / minDim;
  float depthScale = depth / minDim;

  uint32_t numRings = RINGS * 2 + 4;
  uint32_t numVertices = numRings * (SEGS + 1);
  uint32_t numQuadRows = numRings - 1;
  uint32_t numIndices = numQuadRows * SEGS * 6;

  DRAWVERT verts[numVertices];
  DRAWINDEX tris[numIndices];
  uint32_t vidx = 0, iidx = 0;

  // Generate right hemisphere vertices (positive X)
  for (uint32_t ring = 0; ring <= RINGS; ring++) {
    float phi = (float)ring / (float)RINGS * M_PI_2;
    float x = cos(phi) * radius + cylinderLength * 0.5f;
    float ringRadius = sin(phi) * radius;

    for (uint32_t seg = 0; seg <= SEGS; seg++) {
      float theta = (float)seg / (float)SEGS * 2.0f * M_PI;
      float y = cos(theta) * ringRadius * heightScale;
      float z = sin(theta) * ringRadius * depthScale;
      float nx = cos(phi);
      float ny = cos(theta) * sin(phi);
      float nz = sin(theta) * sin(phi);
      float u = (float)seg / (float)SEGS;
      float v = (float)ring / (float)RINGS * 0.25f;

      verts[vidx++] = R_MakeVertex(x, y, z, u, v, nx, ny, nz);
    }
  }

  // Generate cylinder body vertices (2 rings)
  for (uint32_t i = 0; i < 2; i++) {
    float x = (i == 0) ? cylinderLength * 0.5f : -cylinderLength * 0.5f;

    for (uint32_t seg = 0; seg <= SEGS; seg++) {
      float theta = (float)seg / (float)SEGS * 2.0f * M_PI;
      float y = cos(theta) * radius * heightScale;
      float z = sin(theta) * radius * depthScale;
      float ny = cos(theta);
      float nz = sin(theta);
      float u = (float)seg / (float)SEGS;
      float v = 0.25f + (i == 0 ? 0.0f : 0.5f);

      verts[vidx++] = R_MakeVertex(x, y, z, u, v, 0, ny, nz);
    }
  }

  // Generate left hemisphere vertices (negative X)
  for (uint32_t ring = 0; ring <= RINGS; ring++) {
    float phi = M_PI_2 + (float)ring / (float)RINGS * M_PI_2;
    float x = cos(phi) * radius - cylinderLength * 0.5f;
    float ringRadius = sin(phi) * radius;

    for (uint32_t seg = 0; seg <= SEGS; seg++) {
      float theta = (float)seg / (float)SEGS * 2.0f * M_PI;
      float y = cos(theta) * ringRadius * heightScale;
      float z = sin(theta) * ringRadius * depthScale;
      float nx = cos(phi);
      float ny = cos(theta) * sin(phi);
      float nz = sin(theta) * sin(phi);
      float u = (float)seg / (float)SEGS;
      float v = 0.75f + (float)ring / (float)RINGS * 0.25f;

      verts[vidx++] = R_MakeVertex(x, y, z, u, v, nx, ny, nz);
    }
  }

  // Generate indices for all quads
  for (uint32_t i = 0; i < numQuadRows; i++) {
    for (uint32_t s = 0; s < SEGS; s++) {
      uint32_t base = i * (SEGS + 1) + s;
      tris[iidx++] = base;
      tris[iidx++] = base + SEGS + 1;
      tris[iidx++] = base + 1;
      tris[iidx++] = base + 1;
      tris[iidx++] = base + SEGS + 1;
      tris[iidx++] = base + SEGS + 2;
    }
  }

  DRAWSURFATTR attr[] = {
    VERTEX_SEMANTIC_POSITION, VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_TEXCOORD0, VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_NORMAL, VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_COLOR, VERTEX_ATTR_DATATYPE_UINT8 | VERTEX_ATTR_DATATYPE_NORMALIZED,
    VERTEX_SEMANTIC_COUNT
  };

  DRAWSURF dsurf;
  dsurf.vertices = verts;
  dsurf.indices = tris;
  dsurf.submeshes = NULL;
  dsurf.neighbors = NULL;
  dsurf.numVertices = vidx;
  dsurf.numIndices = iidx;
  dsurf.numSubmeshes = 0;
  
//  vec2_t sizex={0};
//  vec2_t sizey={0};
//  FOR_LOOP(i, vidx) {
//    sizex.x = fmin(sizex.x, verts[i].xyz.x);
//    sizex.y = fmax(sizex.y, verts[i].xyz.x);
//    sizey.x = fmin(sizey.x, verts[i].xyz.y);
//    sizey.y = fmax(sizey.y, verts[i].xyz.y);
//  }

  return Model_Create(attr, &dsurf, ppModel);

  #undef SEGS
  #undef RINGS
}
