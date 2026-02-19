typedef struct
{
  uint32_t identifier; // == IDMESHHEADER
  uint32_t numVertices;
  uint32_t numIndices;
  uint32_t numSubmeshes;
  uint32_t numAttributes;
} dmeshheader;

static int triangle3_normal2(struct triangle3 const* t, struct vec3* n)
{
  struct vec3 n_a = VEC3_Sub(&t->a, &t->b);
  struct vec3 n_b = VEC3_Sub(&t->a, &t->c);
  *n = VEC3_Cross(&n_a, &n_b);
  if (VEC3_Len(n) < EPSILON) {
    return 0;
  }
  VEC3_Normalize(n);
  return 1;
}

struct normal_worker
{
  struct vec3 normal;
  struct vec3 tan1;
  struct vec3 tan2;
  int normalsPerVtx;
};

HRESULT
Model_GenerateTangents(PDRAWSURF model)
{
  struct normal_worker* wrk =
  ZeroAlloc(sizeof(struct normal_worker) * model->numVertices);
  if (wrk == NULL)
    return E_OUTOFMEMORY;
  for (int t = 0; t < model->numIndices;) {
    PDRAWVERT v1 = &model->vertices[model->indices[t++]];
    PDRAWVERT v2 = &model->vertices[model->indices[t++]];
    PDRAWVERT v3 = &model->vertices[model->indices[t++]];
    
    struct vec3 n1;
    struct vec3 const* n2 = &n1;
    struct vec3 const* n3 = &n1;
    
    if (!triangle3_normal2(
                           &(struct triangle3){
                             .a = v1->xyz,
                             .b = v2->xyz,
                             .c = v3->xyz,
                           },
                           &n1)) {
                             continue;
                           }
    
    float const x1 = v2->xyz.x - v1->xyz.x;
    float const x2 = v3->xyz.x - v1->xyz.x;
    float const y1 = v2->xyz.y - v1->xyz.y;
    float const y2 = v3->xyz.y - v1->xyz.y;
    float const z1 = v2->xyz.z - v1->xyz.z;
    float const z2 = v3->xyz.z - v1->xyz.z;
    
    float const s1 = v2->texcoord[0].x - v1->texcoord[0].x;
    float const s2 = v3->texcoord[0].x - v1->texcoord[0].x;
    float const t1 = v2->texcoord[0].y - v1->texcoord[0].y;
    float const t2 = v3->texcoord[0].y - v1->texcoord[0].y;
    
    float area = (s1 * t2 - s2 * t1);
    
    if (area == 0.0f) {
      area = 1.0f;
    }
    
    float const r = 1.0f / area;
    
    float const sx = ((t2 * x1 - t1 * x2) * r);
    float const sy = ((t2 * y1 - t1 * y2) * r);
    float const sz = ((t2 * z1 - t1 * z2) * r);
    
    float const tx = ((s1 * x2 - s2 * x1) * r);
    float const ty = ((s1 * y2 - s2 * y1) * r);
    float const tz = ((s1 * z2 - s2 * z1) * r);
    
    wrk[v1 - model->vertices].tan1.x += sx;
    wrk[v1 - model->vertices].tan1.y += sy;
    wrk[v1 - model->vertices].tan1.z += sz;
    wrk[v2 - model->vertices].tan1.x += sx;
    wrk[v2 - model->vertices].tan1.y += sy;
    wrk[v2 - model->vertices].tan1.z += sz;
    wrk[v3 - model->vertices].tan1.x += sx;
    wrk[v3 - model->vertices].tan1.y += sy;
    wrk[v3 - model->vertices].tan1.z += sz;
    wrk[v1 - model->vertices].tan2.x += tx;
    wrk[v1 - model->vertices].tan2.y += ty;
    wrk[v1 - model->vertices].tan2.z += tz;
    wrk[v2 - model->vertices].tan2.x += tx;
    wrk[v2 - model->vertices].tan2.y += ty;
    wrk[v2 - model->vertices].tan2.z += tz;
    wrk[v3 - model->vertices].tan2.x += tx;
    wrk[v3 - model->vertices].tan2.y += ty;
    wrk[v3 - model->vertices].tan2.z += tz;
    wrk[v1 - model->vertices].normal.x += n1.x;
    wrk[v1 - model->vertices].normal.y += n1.y;
    wrk[v1 - model->vertices].normal.z += n1.z;
    wrk[v2 - model->vertices].normal.x += n2->x;
    wrk[v2 - model->vertices].normal.y += n2->y;
    wrk[v2 - model->vertices].normal.z += n2->z;
    wrk[v3 - model->vertices].normal.x += n3->x;
    wrk[v3 - model->vertices].normal.y += n3->y;
    wrk[v3 - model->vertices].normal.z += n3->z;
    wrk[v1 - model->vertices].normalsPerVtx++;
    wrk[v2 - model->vertices].normalsPerVtx++;
    wrk[v3 - model->vertices].normalsPerVtx++;
  }
  
  FOR_LOOP(i, model->numVertices)
  {
    struct vec3 const* n2 = &wrk[i].normal;
    struct vec3 const* t = &wrk[i].tan1;
    struct vec3 const* t2 = &wrk[i].tan2;
    int const normalsPerVtx = wrk[i].normalsPerVtx;
    PDRAWVERT vertex = &model->vertices[i];
    
    if (normalsPerVtx == 0) {
      continue;
    }
    
    // Gram-Schmidt orthogonalize
    struct vec3 temp, temp1, n, t3;
    
    vertex->normal = VEC3_Scale(n2, 1.0f / normalsPerVtx);
    
    // calc tangent
    temp1 = VEC3_Scale(&n, VEC3_Dot(&n, t));
    t3 = VEC3_Sub(t, &temp1);
    VEC3_Normalize(&t3);
    vertex->tangent = t3;
    
    // calc binormal
    temp = VEC3_Cross(&n, t);
    VEC3_Normalize(&temp);
    float const dp = VEC3_Dot(&temp, t2);
    float const sig = (dp < 0.0f) ? -1.0f : 1.0f;
    vertex->binormal = VEC3_Scale(&temp, sig);
  }
  
  free(wrk);
  
  return S_OK;
}
