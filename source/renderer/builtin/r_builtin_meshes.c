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
  if (radius <= 0.0f)
    return Model_CreateBox(width * 0.5f, height * 0.5f, depth * 0.5f, ppModel);
  
  float maxRadius = fminf(fminf(width, height), depth) * 0.5f;
  if (radius > maxRadius) radius = maxRadius;
  
  // Half-extents of the inner box (corner sphere centers live here)
  float hx = (width  - 2.0f * radius) * 0.5f;
  float hy = (height - 2.0f * radius) * 0.5f;
  float hz = (depth  - 2.0f * radius) * 0.5f;
  
  // Latitude/longitude subdivision. More lats needed to represent
  // the flat face regions cleanly; we duplicate seams at ±face boundaries.
  //
  // We sample a full sphere in [0, PI] x [0, 2*PI] but inject extra
  // rings/columns at the 6 "crease" angles so edges stay sharp.
  //
  //   phi   breaks at: asin(hy/R)  and  PI - asin(hy/R)   (top/bottom caps)
  //   theta breaks at: atan2(hz,hx), PI-..., PI+..., 2PI-...  (4 vertical creases)
  //
  // Each break duplicates a ring/column (two verts at same angle, different
  // "which box face" membership) so UVs and normals can be face-local.
  
#define LAT_SEGS   16   // per-quadrant latitude  segments
#define LON_SEGS   16   // per-quadrant longitude segments
  
  // Total rings  = 4 quadrants * LAT_SEGS + 1, plus 2 duplicate crease rings
  // Total cols   = 4 quadrants * LON_SEGS + 1, plus 4 duplicate crease cols
  // We keep it simple: just use a flat grid and remap each (i,j) sample.
  
#define RINGS (4 * LAT_SEGS + 1 + 2)   // +2 for top/bottom crease dupes
#define COLS  (4 * (LON_SEGS + 1) + 3)   // +4 for 4 side crease dupes
  
  uint32_t maxVertices = (uint32_t)(RINGS * COLS);
  uint32_t maxIndices  = (uint32_t)((RINGS - 1) * (COLS - 1) * 6);
  
  DRAWVERT  *verts = malloc(sizeof(DRAWVERT)  * maxVertices);
  DRAWINDEX *tris  = malloc(sizeof(DRAWINDEX) * maxIndices);
  if (!verts || !tris) { free(verts); free(tris); return E_OUTOFMEMORY; }
  
  uint32_t vidx = 0, iidx = 0;
  
  // Actually, the crease angle measured from the equator is asin(hy/radius),
  // so from the north pole it is PI/2 - asin(hy/radius).
  float phi_top    = (float)M_PI_2 - asinf(fminf(hy / radius, 1.0f));
  float phi_bot    = (float)M_PI   - phi_top;
  
  // Collect phi samples: evenly spaced in [0,phi_top], duplicate at phi_top,
  // evenly spaced across the middle band, duplicate at phi_bot, then to PI.
  // For simplicity we generate them into a small array.
  float phi_samples[RINGS];
  int   nrings = 0;
  {
    // Top cap: LAT_SEGS+1 values in [0, phi_top]
    for (int i = 0; i <= LAT_SEGS; i++)
      phi_samples[nrings++] = phi_top * ((float)i / LAT_SEGS);
    
    // Duplicate phi_top (crease: same angle, different face ownership)
    phi_samples[nrings++] = phi_top;
    
    // Middle band: LAT_SEGS-1 interior values strictly between phi_top and phi_bot
    // (2*LAT_SEGS samples spanning the middle, excluding endpoints already added)
    for (int i = 1; i < 2 * LAT_SEGS; i++)
      phi_samples[nrings++] = phi_top + (phi_bot - phi_top) * ((float)i / (2 * LAT_SEGS));
    
    // Duplicate phi_bot
    phi_samples[nrings++] = phi_bot;
    
    // Bottom cap: LAT_SEGS+1 values in [phi_bot, PI]
    for (int i = 0; i <= LAT_SEGS; i++)
      phi_samples[nrings++] = phi_bot + ((float)M_PI - phi_bot) * ((float)i / LAT_SEGS);
  }
  assert(nrings == RINGS);
  
  // Collect theta samples: evenly spaced per quadrant, duplicate at each crease.
  // Crease angles (measured from +X in XZ plane):
  float th[4];
  th[0] = atan2f(hz, hx);           // first quadrant crease (+X/+Z boundary)
  th[1] = (float)M_PI - th[0];      // second quadrant
  th[2] = (float)M_PI + th[0];      // third quadrant
  th[3] = 2.0f * (float)M_PI - th[0]; // fourth quadrant
  
  float theta_samples[COLS];
  int   ncols = 0;
  {
    float quad_start[4] = { 0.0f, (float)M_PI_2, (float)M_PI, 1.5f * (float)M_PI };
    float quad_end[4]   = { (float)M_PI_2, (float)M_PI, 1.5f * (float)M_PI, 2.0f * (float)M_PI };
    
    for (int q = 0; q < 4; q++) {
      // LON_SEGS+1 values per quadrant
      for (int j = 0; j <= LON_SEGS; j++)
        theta_samples[ncols++] = quad_start[q] + (quad_end[q] - quad_start[q]) * ((float)j / LON_SEGS);
      // Duplicate at quadrant boundary (except the very last one closes the loop)
      if (q < 3)
        theta_samples[ncols++] = quad_end[q];
    }
  }
  assert(ncols == COLS);
  
  // ---------------------------------------------------------------------------
  // Core mapping: sphere direction -> rounded-box surface point + normal
  // ---------------------------------------------------------------------------
  // Given unit sphere dir (nx, ny, nz):
  //   center = (clamp(nx, -hx, hx), clamp(ny, -hy, hy), clamp(nz, -hz, hz))
  //   position = center + radius * (nx, ny, nz)
  //   normal   = (nx, ny, nz)   [already unit length from sphere]
  //
  // This is the SDF-derived formula for a rounded box. It works because:
  //   - Inside a corner octant: all three clamps are at their max -> corner sphere
  //   - Inside an edge band:    one clamp is interior     -> cylinder cap
  //   - On a flat face:         two clamps are interior   -> flat region
  // ---------------------------------------------------------------------------
  
  for (int ri = 0; ri < nrings; ri++) {
    float phi   = phi_samples[ri];
    float sin_p = sinf(phi);
    float cos_p = cosf(phi);
    
    for (int ci = 0; ci < ncols; ci++) {
      float theta = theta_samples[ci];
      
      // Unit sphere point (Y-up convention, theta in XZ)
      float nx = sin_p * cosf(theta);
      float ny = cos_p;
      float nz = sin_p * sinf(theta);
      
      // Clamp to inner box extents -> nearest point on inner box surface
      float cx = fmaxf(-hx, fminf(hx, nx * 1e9f)); // large scale to get sign
      float cy = fmaxf(-hy, fminf(hy, ny * 1e9f));
      float cz = fmaxf(-hz, fminf(hz, nz * 1e9f));
      // (we just want sign/clamp, not actual scale; see cleaner version below)
      
      // Cleaner: the inner-box center is just sign(n) * h, clamped
      cx = fmaxf(-hx, fminf(hx, (nx < 0 ? -1e9f : 1e9f)));
      cy = fmaxf(-hy, fminf(hy, (ny < 0 ? -1e9f : 1e9f)));
      cz = fmaxf(-hz, fminf(hz, (nz < 0 ? -1e9f : 1e9f)));
      
      // Surface position
      float px = cx + radius * nx;
      float py = cy + radius * ny;
      float pz = cz + radius * nz;
      
      // UV: spherical, normalized to [0,1]
      float u = theta / (2.0f * (float)M_PI);
      float v = phi   / (float)M_PI;
      
      verts[vidx++] = R_MakeVertexWithWeight(px, py, pz, u, v,
                                             nx, ny, nz,   // normal
                                             nx, ny, nz);  // weight (sphere dir)
    }
  }
  
  // Generate indices as a simple grid (skip degenerate quads at seam)
  for (int ri = 0; ri < nrings - 1; ri++) {
    for (int ci = 0; ci < ncols - 1; ci++) {
      uint32_t a = (uint32_t)( ri      * ncols + ci    );
      uint32_t b = (uint32_t)( ri      * ncols + ci + 1);
      uint32_t c = (uint32_t)((ri + 1) * ncols + ci    );
      uint32_t d = (uint32_t)((ri + 1) * ncols + ci + 1);
      
      tris[iidx++] = a; tris[iidx++] = b; tris[iidx++] = c;
      tris[iidx++] = b; tris[iidx++] = d; tris[iidx++] = c;
    }
  }
  
  DRAWSURFATTR attr[] = {
    VERTEX_SEMANTIC_POSITION,  VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_TEXCOORD0, VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_NORMAL,    VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_WEIGHT,    VERTEX_ATTR_DATATYPE_FLOAT32,
    VERTEX_SEMANTIC_COLOR,     VERTEX_ATTR_DATATYPE_UINT8 | VERTEX_ATTR_DATATYPE_NORMALIZED,
    VERTEX_SEMANTIC_COUNT
  };
  
  DRAWSURF dsurf = {
    .vertices    = verts,
    .indices     = tris,
    .submeshes   = NULL,
    .neighbors   = NULL,
    .numVertices = vidx,
    .numIndices  = iidx,
    .numSubmeshes = 0,
  };
  
  HRESULT hr = Model_Create(attr, &dsurf, ppModel);
  free(verts);
  free(tris);
  return hr;
  
#undef LAT_SEGS
#undef LON_SEGS
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
