#include <stdio.h>
#include <string.h>

#include <include/orcadef.h>
#include <include/shared.h>

void VEC2_Set(struct vec2* v, float x, float y)
{
  v->x = x;
  v->y = y;
}

struct vec2 VEC2_Scale(struct vec2 const* v, float s)
{
  return (struct vec2){ .x = v->x * s, .y = v->y * s };
}

struct vec2 VEC2_Add(struct vec2 const* a, struct vec2 const* b)
{
  return (struct vec2){
    .x = a->x + b->x,
    .y = a->y + b->y,
  };
}

struct vec2 VEC2_Mul(struct vec2 const* a, struct vec2 const* b)
{
  return (struct vec2){
    .x = a->x * b->x,
    .y = a->y * b->y,
  };
}

struct vec2 VEC2_Div(struct vec2 const* a, struct vec2 const* b)
{
  return (struct vec2){
    .x = a->x / b->x,
    .y = a->y / b->y,
  };
}

struct vec2 VEC2_Sub(struct vec2 const* a, struct vec2 const* b)
{
  return (struct vec2){
    .x = a->x - b->x,
    .y = a->y - b->y,
  };
}

struct vec2 VEC2_Unm(struct vec2 const* v)
{
  return (struct vec2){
    .x = -v->x,
    .y = -v->y,
  };
}

float VEC2_Dot(struct vec2 const* a, struct vec2 const* b)
{
  return a->x * b->x + a->y * b->y;
}

float VEC2_Lengthsq(struct vec2 const* vec)
{
  return VEC2_Dot(vec, vec);
}

float VEC2_Len(struct vec2 const* vec)
{
  return sqrtf(VEC2_Lengthsq(vec));
}

float VEC2_Distance(struct vec2 const* a, struct vec2 const* b)
{
  struct vec2 const dist = VEC2_Sub(a, b);
  return VEC2_Len(&dist);
}

void VEC2_Normalize(struct vec2* v)
{
  *v = VEC2_Scale(v, 1 / VEC2_Len(v));
}

struct vec2 VEC2_Lerp(struct vec2 const* a, struct vec2 const* b, float t)
{
  return (struct vec2){
    .x = a->x * (1 - t) + b->x * t,
    .y = a->y * (1 - t) + b->y * t,
  };
}

struct vec2 VEC2_Mad(struct vec2 const* v, float s, struct vec2 const* b)
{
  return (struct vec2){
    .x = v->x + b->x * s,
    .y = v->y + b->y * s,
  };
}

float VEC3_Dot(struct vec3 const* a, struct vec3 const* b)
{
  return a->x * b->x + a->y * b->y + a->z * b->z;
}

float VEC3_Lengthsq(struct vec3 const* vec)
{
  return VEC3_Dot(vec, vec);
}

float VEC3_Len(struct vec3 const* vec)
{
  return sqrtf(VEC3_Lengthsq(vec));
}

struct vec3 VEC3_Bezier(struct vec3 const* a,
            struct vec3 const* b,
            struct vec3 const* c,
            struct vec3 const* d,
            float t)
{
  float const inverseFactor = 1 - t;
  float const inverseFactorTimesTwo = inverseFactor * inverseFactor;
  float const factorTimes2 = t * t;
  float const factor1 = inverseFactorTimesTwo * inverseFactor;
  float const factor2 = 3 * t * inverseFactorTimesTwo;
  float const factor3 = 3 * factorTimes2 * inverseFactor;
  float const factor4 = factorTimes2 * t;

  return (struct vec3){
    a->x * factor1 + b->x * factor2 + c->x * factor3 + d->x * factor4,
    a->y * factor1 + b->y * factor2 + c->y * factor3 + d->y * factor4,
    a->z * factor1 + b->z * factor2 + c->z * factor3 + d->z * factor4,
  };
}

struct vec3 VEC3_Hermite(struct vec3 const* a,
             struct vec3 const* b,
             struct vec3 const* c,
             struct vec3 const* d,
             float t)
{
  float const factorTimes2 = t * t;
  float const factor1 = factorTimes2 * (2 * t - 3) + 1;
  float const factor2 = factorTimes2 * (t - 2) + t;
  float const factor3 = factorTimes2 * (t - 1);
  float const factor4 = factorTimes2 * (3 - 2 * t);

  return (struct vec3){
    a->x * factor1 + b->x * factor2 + c->x * factor3 + d->x * factor4,
    a->y * factor1 + b->y * factor2 + c->y * factor3 + d->y * factor4,
    a->z * factor1 + b->z * factor2 + c->z * factor3 + d->z * factor4,
  };
}

struct vec3 VEC3_Lerp(struct vec3 const* a, struct vec3 const* b, float t)
{
  return (struct vec3){ .x = a->x * (1 - t) + b->x * t,
                        .y = a->y * (1 - t) + b->y * t,
                        .z = a->z * (1 - t) + b->z * t };
}

struct vec3 VEC3_Cross(struct vec3 const* a, struct vec3 const* b)
{
  return (struct vec3){ .x = a->y * b->z - a->z * b->y,
                        .y = a->z * b->x - a->x * b->z,
                        .z = a->x * b->y - a->y * b->x };
}

struct vec3 VEC3_Sub(struct vec3 const* a, struct vec3 const* b)
{
  return (struct vec3){ .x = a->x - b->x, .y = a->y - b->y, .z = a->z - b->z };
}

struct vec3 VEC3_Add(struct vec3 const* a, struct vec3 const* b)
{
  return (struct vec3){ .x = a->x + b->x, .y = a->y + b->y, .z = a->z + b->z };
}

struct vec3 VEC3_Mad(struct vec3 const* v, float s, struct vec3 const* b)
{
  return (struct vec3){ .x = v->x + b->x * s,
                        .y = v->y + b->y * s,
                        .z = v->z + b->z * s };
}

struct vec3 VEC3_Mul(struct vec3 const* a, struct vec3 const* b)
{
  return (struct vec3){ .x = a->x * b->x, .y = a->y * b->y, .z = a->z * b->z };
}

struct vec3 VEC3_Scale(struct vec3 const* v, float s)
{
  return (struct vec3){ .x = v->x * s, .y = v->y * s, .z = v->z * s };
}

void VEC3_Normalize(struct vec3* v)
{
  *v = VEC3_Scale(v, 1 / VEC3_Len(v));
}

void VEC3_Set(struct vec3* v, float x, float y, float z)
{
  v->x = x;
  v->y = y;
  v->z = z;
}

void VEC3_Clear(struct vec3* v)
{
  VEC3_Set(v, 0, 0, 0);
}

struct vec3 VEC3_Unm(struct vec3 const* v)
{
  return (struct vec3){ .x = -v->x, .y = -v->y, .z = -v->z };
}

float VEC3_Distance(struct vec3 const* a, struct vec3 const* b)
{
  struct vec3 const dist = VEC3_Sub(a, b);
  return VEC3_Len(&dist);
}

void VEC4_Set(struct vec4* v, float x, float y, float z, float w)
{
  v->x = x;
  v->y = y;
  v->z = z;
  v->w = w;
}

struct vec4 VEC4_Scale(struct vec4 const* v, float s)
{
  return (
    struct vec4){ .x = v->x * s, .y = v->y * s, .z = v->z * s, .w = v->w * s };
}

struct vec4 VEC4_Add(struct vec4 const* a, struct vec4 const* b)
{
  return (struct vec4){
    .x = a->x + b->x, .y = a->y + b->y, .z = a->z + b->z, .w = a->w + b->w
  };
}

struct vec4 VEC4_Unm(struct vec4 const* v)
{
  return (struct vec4){ .x = -v->x, .y = -v->y, .z = -v->z, .w = -v->w };
}

struct vec4 VEC4_Lerp(struct vec4 const* a, struct vec4 const* b, float t)
{
  return (struct vec4){
    .x = a->x * (1 - t) + b->x * t,
    .y = a->y * (1 - t) + b->y * t,
    .z = a->z * (1 - t) + b->z * t,
    .w = a->w * (1 - t) + b->w * t
  };
}

struct vec2
BOX2_Center(struct box2 const* box)
{
  return (struct vec2){
    (box->min.x + box->max.x) * 0.5f,
    (box->min.y + box->max.y) * 0.5f,
  };
}

void
BOX2_MoveTo(struct box2* box, struct vec2 const* newCenterLoc)
{
  struct vec2 center = BOX2_Center(box);
  box->min.x += newCenterLoc->x - center.x;
  box->max.x += newCenterLoc->x - center.x;
  box->min.y += newCenterLoc->y - center.y;
  box->max.y += newCenterLoc->y - center.y;
}

unsigned int
BOX2_ContainsPoint(struct box2 const* box, struct vec2 const* point)
{
  if (point->x < box->min.x)
    return 0;
  if (point->y < box->min.y)
    return 0;
  if (point->x >= box->max.x)
    return 0;
  if (point->y >= box->max.y)
    return 0;
  return 1;
}

struct vec3
BOX3_Center(struct box3 const* box)
{
  return (struct vec3){
    (box->min.x + box->max.x) * 0.5f,
    (box->min.y + box->max.y) * 0.5f,
    (box->min.z + box->max.z) * 0.5f,
  };
}

unsigned int
RECT_Contains(struct rect const* rect, struct vec2 const* point)
{
  if (rect->x > point->x)
    return 0;
  if (rect->y > point->y)
    return 0;
  if (rect->x + rect->width <= point->x)
    return 0;
  if (rect->y + rect->height <= point->y)
    return 0;
  return 1;
}

struct rect
RECT_Scale(struct rect const* rect, float scale)
{
  struct rect const screen = {
    .x = rect->x * scale,
    .y = rect->y * scale,
    .width = rect->width * scale,
    .height = rect->height * scale,
  };
  return screen;
}

struct vec2
RECT_Center(struct rect const* rect)
{
  struct vec2 const center = {
    rect->x + rect->width * 0.5f,
    rect->y + rect->height * 0.5f,
  };
  return center;
}

struct rect
RECT_Fit(struct rect const *small, struct vec2 const *big) {
  float scaleX = small->width / big->x;
  float scaleY = small->height / big->y;
  float scale = (scaleX < scaleY) ? scaleX : scaleY; // pick smaller
  return (struct rect) {
    .x = small->x + (small->width - big->x * scale) / 2.0f,
    .y = small->y + (small->height - big->y * scale) / 2.0f,
    .width  = big->x * scale,
    .height = big->y * scale,
  };
}

struct frustum3
frustum_Calculate(struct mat4 const* matrix)
{
  struct frustum3 out={0};
  
  out.right.a = matrix->v[3] - matrix->v[0];
  out.right.b = matrix->v[7] - matrix->v[4];
  out.right.c = matrix->v[11] - matrix->v[8];
  out.right.d = matrix->v[15] - matrix->v[12];

  plane3_Normalize(&out.right);

  out.left.a = matrix->v[3] + matrix->v[0];
  out.left.b = matrix->v[7] + matrix->v[4];
  out.left.c = matrix->v[11] + matrix->v[8];
  out.left.d = matrix->v[15] + matrix->v[12];

  plane3_Normalize(&out.left);

  out.bottom.a = matrix->v[3] + matrix->v[1];
  out.bottom.b = matrix->v[7] + matrix->v[5];
  out.bottom.c = matrix->v[11] + matrix->v[9];
  out.bottom.d = matrix->v[15] + matrix->v[13];

  plane3_Normalize(&out.bottom);

  out.top.a = matrix->v[3] - matrix->v[1];
  out.top.b = matrix->v[7] - matrix->v[5];
  out.top.c = matrix->v[11] - matrix->v[9];
  out.top.d = matrix->v[15] - matrix->v[13];

  plane3_Normalize(&out.top);

  out.back.a = matrix->v[3] - matrix->v[2];
  out.back.b = matrix->v[7] - matrix->v[6];
  out.back.c = matrix->v[11] - matrix->v[10];
  out.back.d = matrix->v[15] - matrix->v[14];

  plane3_Normalize(&out.back);

  out.front.a = matrix->v[3] + matrix->v[2];
  out.front.b = matrix->v[7] + matrix->v[6];
  out.front.c = matrix->v[11] + matrix->v[10];
  out.front.d = matrix->v[15] + matrix->v[14];

  plane3_Normalize(&out.front);
  
  return out;
}

unsigned int
frustum_ContainsSphere(struct frustum3 const* frustum,
                       struct sphere3 const* sphere)
{
  struct plane3 const *planes = (struct plane3 const *)frustum;
  for (unsigned i = 0; i < FRUSTUM_NUM_PLANES; i++) {
    if (plane3_MultiplyVector3D(planes + i, &sphere->center) <= -sphere->radius) {
      return 0;
    }
  }
  return 1;
}

unsigned int
frustum_ContainsPoint(struct frustum3 const* frustum,
                      struct vec3 const* point)
{
  return frustum_ContainsSphere(frustum, &(struct sphere3){ .center = *point, .radius = 0 });
}

unsigned int
frustum_ContainsBox(struct frustum3 const* frustum,
                    struct box3 const* box,
                    struct mat4 const* matrix)
{
  struct plane3 const *planes = (struct plane3 const *)frustum;
  struct vec3 const points[] = {
    MAT4_MultiplyVector3D(matrix, &(struct vec3){ box->min.x, box->min.y, box->min.z }),
    MAT4_MultiplyVector3D(matrix, &(struct vec3){ box->max.x, box->min.y, box->min.z }),
    MAT4_MultiplyVector3D(matrix, &(struct vec3){ box->min.x, box->max.y, box->min.z }),
    MAT4_MultiplyVector3D(matrix, &(struct vec3){ box->max.x, box->max.y, box->min.z }),
    MAT4_MultiplyVector3D(matrix, &(struct vec3){ box->min.x, box->min.y, box->max.z }),
    MAT4_MultiplyVector3D(matrix, &(struct vec3){ box->max.x, box->min.y, box->max.z }),
    MAT4_MultiplyVector3D(matrix, &(struct vec3){ box->min.x, box->max.y, box->max.z }),
    MAT4_MultiplyVector3D(matrix, &(struct vec3){ box->max.x, box->max.y, box->max.z }),
  };
  for (unsigned i = 0; i < FRUSTUM_NUM_PLANES; i++) {
    for (unsigned j = 0; j < sizeof(points) / sizeof(*points); j++) {
      if (plane3_MultiplyVector3D(planes + i, points + j) > 0) {
        goto next_cycle;
      }
    }
    return 0;
  next_cycle:;
  }
  return 1;
}

unsigned int
frustum_ContainsAABox(struct frustum3 const* frustum,
                      struct box3 const* box)
{
  struct plane3 const *planes = (struct plane3 const *)frustum;
  struct vec3 const points[] = {
    { box->min.x, box->min.y, box->min.z },
    { box->max.x, box->min.y, box->min.z },
    { box->min.x, box->max.y, box->min.z },
    { box->max.x, box->max.y, box->min.z },
    { box->min.x, box->min.y, box->max.z },
    { box->max.x, box->min.y, box->max.z },
    { box->min.x, box->max.y, box->max.z },
    { box->max.x, box->max.y, box->max.z },
  };
  for (unsigned i = 0; i < FRUSTUM_NUM_PLANES; i++) {
    for (unsigned j = 0; j < sizeof(points) / sizeof(*points); j++) {
      if (plane3_MultiplyVector3D(planes + i, points + j) > 0) {
        goto next_cycle;
      }
    }
    return 0;
  next_cycle:;
  }
  return 1;
}

unsigned int line3_intersect_sphere3(struct line3 const* line, struct sphere3 const* sphere, struct vec3* output)
{
  float cx = sphere->center.x;
  float cy = sphere->center.y;
  float cz = sphere->center.z;
  float px = line->a.x;
  float py = line->a.y;
  float pz = line->a.z;
  float vx = line->b.x - px;
  float vy = line->b.y - py;
  float vz = line->b.z - pz;
  float A = vx * vx + vy * vy + vz * vz;
  float B = 2.0f * (px * vx + py * vy + pz * vz - vx * cx - vy * cy - vz * cz);
  float C = px * px - 2 * px * cx + cx * cx + py * py - 2 * py * cy + cy * cy +
            pz * pz - 2 * pz * cz + cz * cz - sphere->radius * sphere->radius;
  float D = B * B - 4 * A * C;

  if (D < 0) {
    return 0;
  } else if (!output) {
    return 1;
  }

  float t1 = (-B - sqrtf(D)) / (2.0f * A);

  struct vec3 solution1 = { line->a.x * (1 - t1) + t1 * line->b.x,
                            line->a.y * (1 - t1) + t1 * line->b.y,
                            line->a.z * (1 - t1) + t1 * line->b.z };

  if (D == 0) {
    *output = solution1;
    return 1;
  }

  float t2 = (-B + sqrtf(D)) / (2.0f * A);
  struct vec3 solution2 = { line->a.x * (1 - t2) + t2 * line->b.x,
                            line->a.y * (1 - t2) + t2 * line->b.y,
                            line->a.z * (1 - t2) + t2 * line->b.z };

  if (fabs(t1 - 0.5f) < fabs(t2 - 0.5f)) {
    *output = solution1;
    return 1;
  }

  *output = solution2;
  return 1;
}

unsigned int line3_intersect_plane3(struct line3 const* line, struct plane3 const* plane, struct vec3* output)
{
  struct vec3 direction = VEC3_Sub(&line->b, &line->a);
  float dotProduct = VEC3_Dot(&direction, (struct vec3 const *)plane);
  // Check if the line and plane are parallel
  if (fabs(dotProduct) < EPSILON) {
    return 0;
  }
  if (output) {
    float t = -plane3_MultiplyVector3D(plane, &line->a) / dotProduct;
    *output = VEC3_Mad(&line->a, t, &direction);
  }
  return 1;
}

static inline float
DotNormal(struct vec3 const* normal,
          struct vec3 const* a,
          struct vec3 const* b,
          struct vec3 const* c)
{
  struct vec3 const rcross = triangle3_normal(&(const struct triangle3){ *a, *b, *c });
  return VEC3_Dot(normal, &rcross);
}

unsigned int line3_intersect_triangle(struct line3 const* line,
                         struct triangle3 const* triangle,
                         struct vec3* output)
{
  struct vec3 const normal = triangle3_normal(triangle);
  struct vec3 const diff1 = VEC3_Sub(&line->a, &triangle->a);
  struct vec3 const diff2 = VEC3_Sub(&line->b, &triangle->a);
  float const r1 = VEC3_Dot(&normal, &diff1);
  float const r2 = VEC3_Dot(&normal, &diff2);
  if ((r1 > 0) == (r2 > 0))
    return 0;
  struct vec3 const distance = VEC3_Sub(&line->a, &line->b);
  struct vec3 const pc = VEC3_Mad(&line->a, r1 / (r2 - r1), &distance);
  if (DotNormal(&normal, &triangle->a, &triangle->b, &pc) < 0)
    return 0;
  if (DotNormal(&normal, &triangle->b, &triangle->c, &pc) < 0)
    return 0;
  if (DotNormal(&normal, &triangle->c, &triangle->a, &pc) < 0)
    return 0;
  if (output)
    *output = pc;
  return 1;
}
//
// bool_t intersect_triangle(
//    in Ray R, in vec3 A, in vec3 B, in vec3 C, out float t,
//    out float u, out float v, out vec3 N
//) {
//   vec3 E1 = B-A;
//   vec3 E2 = C-A;
//         N = cross(E1,E2);
//   float det = -dot(R.Dir, N);
//   float invdet = 1.0f/det;
//   vec3 AO  = R.Origin - A;
//   vec3 DAO = cross(AO, R.Dir);
//   u =  dot(E2,DAO) * invdet;
//   v = -dot(E1,DAO) * invdet;
//   t =  dot(AO,N)  * invdet;
//   return (det >= 1e-6 && t >= 0.0f && u >= 0.0f && v >= 0.0f && (u+v)
//   <= 1.0f);
//}

unsigned int line3_intersect_box3(struct line3 const* line, struct box3 const* box, struct vec3* output)
{
  float st, et, fst = 0, fet = 1;
  float const* bmin = &box->min.x;
  float const* bmax = &box->max.x;
  float const* si = &line->a.x;
  float const* ei = &line->b.x;
  for (int i = 0; i < 3; i++) {
    if (*si < *ei) {
      if (*si > *bmax || *ei < *bmin)
        return 0;
      float di = *ei - *si;
      st = (*si < *bmin) ? (*bmin - *si) / di : 0;
      et = (*ei > *bmax) ? (*bmax - *si) / di : 1;
    } else {
      if (*ei > *bmax || *si < *bmin)
        return 0;
      float di = *ei - *si;
      st = (*si > *bmax) ? (*bmax - *si) / di : 0;
      et = (*ei < *bmin) ? (*bmin - *si) / di : 1;
    }

    if (st > fst)
      fst = st;
    if (et < fet)
      fet = et;
    if (fet < fst)
      return 0;
    bmin++;
    bmax++;
    si++;
    ei++;
  }

  if (output) {
    *output = VEC3_Lerp(&line->a, &line->b, fst);
  }
  return 1;
}

struct mat3
MAT3_Identity(void)
{
  struct mat3 m={0};
  m.v[0] = 1;
  m.v[1] = 0;
  m.v[2] = 0;
  m.v[3] = 0;
  m.v[4] = 1;
  m.v[5] = 0;
  m.v[6] = 0;
  m.v[7] = 0;
  m.v[8] = 1;
  return m;
}

struct mat3
MAT3_Normal(struct mat4 const* modelview)
{
  struct mat4 inverse = MAT4_Inverse(modelview);
  float const m33[9] = { inverse.v[0], inverse.v[1], inverse.v[2],
                         inverse.v[4], inverse.v[5], inverse.v[6],
                         inverse.v[8], inverse.v[9], inverse.v[10] };
  struct mat3 out;
  memcpy(&out, m33, sizeof(m33));
  return out;
}

void
MAT3_Translate(struct mat3* m, struct vec2 const* v)
{
  m->v[6] += m->v[0] * v->x + m->v[3] * v->y;
  m->v[7] += m->v[1] * v->x + m->v[4] * v->y;
}

void
MAT3_Scale(struct mat3* m, struct vec2 const* v)
{
  m->v[0] *= v->x;
  m->v[1] *= v->x;
  m->v[3] *= v->y;
  m->v[4] *= v->y;
}

struct mat4
MAT4_Identity(void)
{
  struct mat4 m;
  m.v[0] = 1;
  m.v[1] = 0;
  m.v[2] = 0;
  m.v[3] = 0;
  m.v[4] = 0;
  m.v[5] = 1;
  m.v[6] = 0;
  m.v[7] = 0;
  m.v[8] = 0;
  m.v[9] = 0;
  m.v[10] = 1;
  m.v[11] = 0;
  m.v[12] = 0;
  m.v[13] = 0;
  m.v[14] = 0;
  m.v[15] = 1;
  return m;
}

void
MAT4_Translate(struct mat4* m, struct vec3 const* v)
{
  m->v[12] += m->v[0] * v->x + m->v[4] * v->y + m->v[8] * v->z;
  m->v[13] += m->v[1] * v->x + m->v[5] * v->y + m->v[9] * v->z;
  m->v[14] += m->v[2] * v->x + m->v[6] * v->y + m->v[10] * v->z;
  m->v[15] += m->v[3] * v->x + m->v[7] * v->y + m->v[11] * v->z;
}

void
MAT4_Scale(struct mat4* m, struct vec3 const* v)
{
  m->v[0] *= v->x;
  m->v[1] *= v->x;
  m->v[2] *= v->x;
  m->v[4] *= v->y;
  m->v[5] *= v->y;
  m->v[6] *= v->y;
  m->v[8] *= v->z;
  m->v[9] *= v->z;
  m->v[10] *= v->z;
}

struct mat4
MAT4_Transpose(struct mat4 const* m)
{
  struct mat4 out = {0};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      out.v[i * 4 + j] = m->v[j * 4 + i];
    }
  }
  return out;
}

void
MAT4_Rotate4(struct mat4* m, struct vec4 const* quat)
{
  struct mat4 r, tmp;

  float fTx = 2.0f * quat->x;
  float fTy = 2.0f * quat->y;
  float fTz = 2.0f * quat->z;
  float fTwx = fTx * quat->w;
  float fTwy = fTy * quat->w;
  float fTwz = fTz * quat->w;
  float fTxx = fTx * quat->x;
  float fTxy = fTy * quat->x;
  float fTxz = fTz * quat->x;
  float fTyy = fTy * quat->y;
  float fTyz = fTz * quat->y;
  float fTzz = fTz * quat->z;

  r = MAT4_Identity();

  r.v[0] = 1.0f - (fTyy + fTzz);
  r.v[4] = fTxy - fTwz;
  r.v[8] = fTxz + fTwy;
  r.v[1] = fTxy + fTwz;
  r.v[5] = 1.0f - (fTxx + fTzz);
  r.v[9] = fTyz - fTwx;
  r.v[2] = fTxz - fTwy;
  r.v[6] = fTyz + fTwx;
  r.v[10] = 1.0f - (fTxx + fTyy);

  tmp = MAT4_Multiply(m, &r);
  *m = tmp;
}

struct mat4
MAT4_Perspective(float radians, float aspect, float znear, float zfar) {
  struct mat4 m = {0};
  double cotan = 1.0 / tan(radians);
  m.v[0] = cotan / aspect;
  m.v[1] = 0.0f;
  m.v[2] = 0.0f;
  m.v[3] = 0.0f;
  m.v[4] = 0.0f;
  m.v[5] = cotan;
  m.v[6] = 0.0f;
  m.v[7] = 0.0f;
  m.v[8] = 0.0f;
  m.v[9] = 0.0f;
  m.v[10] = -(znear + zfar) / (zfar - znear);
  m.v[11] = -1.0f;
  m.v[12] = 0.0f;
  m.v[13] = 0.0f;
  m.v[14] = -(2.0f * znear * zfar) / (zfar - znear);
  m.v[15] = 0.0f;
  return m;
}

struct mat4
MAT4_Ortho(float left,
           float right,
           float bottom,
           float top,
           float znear,
           float zfar)
{
  struct mat4 m = {0};
  float const width = right - left;
  float const invheight = top - bottom;
  float const clip = zfar - znear;
  m.v[0] = 2.0f / width;
  m.v[1] = 0.0f;
  m.v[2] = 0.0f;
  m.v[3] = 0.0f;
  m.v[4] = 0.0f;
  m.v[5] = 2.0f / invheight;
  m.v[6] = 0.0f;
  m.v[7] = 0.0f;
  m.v[8] = 0.0f;
  m.v[9] = 0.0f;
  m.v[10] = -2.0f / clip;
  m.v[11] = 0.0f;
  m.v[12] = -(left + right) / width;
  m.v[13] = -(top + bottom) / invheight;
  m.v[14] = -(znear + zfar) / clip;
  m.v[15] = 1.0f;
  return m;
}

struct mat4
MAT4_LookAt(struct vec3 const* eye,
            struct vec3 const* direction,
            struct vec3 const* up)
{
  struct mat4 m = {0};
  struct vec3 zaxis = VEC3_Unm(direction);
  struct vec3 xyz = VEC3_Unm(eye);
  struct vec3 xaxis = VEC3_Cross(up, &zaxis);
  struct vec3 yaxis = VEC3_Cross(&zaxis, &xaxis);

  VEC3_Normalize(&xaxis);
  VEC3_Normalize(&yaxis);
  VEC3_Normalize(&zaxis);

  m.v[0] = xaxis.x;
  m.v[1] = yaxis.x;
  m.v[2] = zaxis.x;
  m.v[3] = 0.0f;
  m.v[4] = xaxis.y;
  m.v[5] = yaxis.y;
  m.v[6] = zaxis.y;
  m.v[7] = 0.0f;
  m.v[8] = xaxis.z;
  m.v[9] = yaxis.z;
  m.v[10] = zaxis.z;
  m.v[11] = 0.0f;
  m.v[12] = 0.0f;
  m.v[13] = 0.0f;
  m.v[14] = 0.0f;
  m.v[15] = 1.0f;

  MAT4_Translate(&m, &xyz);
  
  return m;
}

#if 0 && !defined COMPILE_VM && defined __ARM_NEON
#include <arm_neon.h>

void
MAT4_Multiply(struct mat4 const * m1, struct mat4 const * m2, struct mat4 * out)
{
    // these are the columns A
    float32x4_t A0;
    float32x4_t A1;
    float32x4_t A2;
    float32x4_t A3;

    // these are the columns B
    float32x4_t B0;
    float32x4_t B1;
    float32x4_t B2;
    float32x4_t B3;

    // these are the columns C
    float32x4_t C0;
    float32x4_t C1;
    float32x4_t C2;
    float32x4_t C3;

    A0 = vld1q_f32(m1->v);
    A1 = vld1q_f32(m1->v + 4);
    A2 = vld1q_f32(m1->v + 8);
    A3 = vld1q_f32(m1->v + 12);

    // Zero accumulators for C values
    C0 = vmovq_n_f32(0);
    C1 = vmovq_n_f32(0);
    C2 = vmovq_n_f32(0);
    C3 = vmovq_n_f32(0);

    // Multiply accumulate in 4x1 blocks, i.e. each column in C
    B0 = vld1q_f32(m2->v);
    C0 = vfmaq_laneq_f32(C0, A0, B0, 0);
    C0 = vfmaq_laneq_f32(C0, A1, B0, 1);
    C0 = vfmaq_laneq_f32(C0, A2, B0, 2);
    C0 = vfmaq_laneq_f32(C0, A3, B0, 3);
    vst1q_f32(out->v, C0);

    B1 = vld1q_f32(m2->v + 4);
    C1 = vfmaq_laneq_f32(C1, A0, B1, 0);
    C1 = vfmaq_laneq_f32(C1, A1, B1, 1);
    C1 = vfmaq_laneq_f32(C1, A2, B1, 2);
    C1 = vfmaq_laneq_f32(C1, A3, B1, 3);
    vst1q_f32(out->v + 4, C1);

    B2 = vld1q_f32(m2->v + 8);
    C2 = vfmaq_laneq_f32(C2, A0, B2, 0);
    C2 = vfmaq_laneq_f32(C2, A1, B2, 1);
    C2 = vfmaq_laneq_f32(C2, A2, B2, 2);
    C2 = vfmaq_laneq_f32(C2, A3, B2, 3);
    vst1q_f32(out->v + 8, C2);

    B3 = vld1q_f32(m2->v + 12);
    C3 = vfmaq_laneq_f32(C3, A0, B3, 0);
    C3 = vfmaq_laneq_f32(C3, A1, B3, 1);
    C3 = vfmaq_laneq_f32(C3, A2, B3, 2);
    C3 = vfmaq_laneq_f32(C3, A3, B3, 3);
    vst1q_f32(out->v + 12, C3);
}
#else
struct mat4
MAT4_Multiply(struct mat4 const* m1, struct mat4 const* m2)
{
  struct mat4 out={0};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      out.v[i * 4 + j] = m2->v[i * 4 + 0] * m1->v[0 * 4 + j] +
                         m2->v[i * 4 + 1] * m1->v[1 * 4 + j] +
                         m2->v[i * 4 + 2] * m1->v[2 * 4 + j] +
                         m2->v[i * 4 + 3] * m1->v[3 * 4 + j];
    }
  }
  return out;
}
#endif

struct mat4
MAT4_Inverse(struct mat4 const* m)
{
  struct mat4 out = {0};
  float det, invDet;

  // 2x2 sub-determinants required to calculate 4x4 determinant
  float det2_01_01 = m->v[0] * m->v[5] - m->v[1] * m->v[4];
  float det2_01_02 = m->v[0] * m->v[6] - m->v[2] * m->v[4];
  float det2_01_03 = m->v[0] * m->v[7] - m->v[3] * m->v[4];
  float det2_01_12 = m->v[1] * m->v[6] - m->v[2] * m->v[5];
  float det2_01_13 = m->v[1] * m->v[7] - m->v[3] * m->v[5];
  float det2_01_23 = m->v[2] * m->v[7] - m->v[3] * m->v[6];

  // 3x3 sub-determinants required to calculate 4x4 determinant
  float det3_201_012 =
    m->v[8] * det2_01_12 - m->v[9] * det2_01_02 + m->v[10] * det2_01_01;
  float det3_201_013 =
    m->v[8] * det2_01_13 - m->v[9] * det2_01_03 + m->v[11] * det2_01_01;
  float det3_201_023 =
    m->v[8] * det2_01_23 - m->v[10] * det2_01_03 + m->v[11] * det2_01_02;
  float det3_201_123 =
    m->v[9] * det2_01_23 - m->v[10] * det2_01_13 + m->v[11] * det2_01_12;

  det = (-det3_201_123 * m->v[12] + det3_201_023 * m->v[13] -
         det3_201_013 * m->v[14] + det3_201_012 * m->v[15]);

  if (fabs(det) < EPSILON) {
    return (struct mat4){0};
  }

  invDet = 1.0f / det;

  // remaining 2x2 sub-determinants
  float det2_03_01 = m->v[0] * m->v[13] - m->v[1] * m->v[12];
  float det2_03_02 = m->v[0] * m->v[14] - m->v[2] * m->v[12];
  float det2_03_03 = m->v[0] * m->v[15] - m->v[3] * m->v[12];
  float det2_03_12 = m->v[1] * m->v[14] - m->v[2] * m->v[13];
  float det2_03_13 = m->v[1] * m->v[15] - m->v[3] * m->v[13];
  float det2_03_23 = m->v[2] * m->v[15] - m->v[3] * m->v[14];

  float det2_13_01 = m->v[4] * m->v[13] - m->v[5] * m->v[12];
  float det2_13_02 = m->v[4] * m->v[14] - m->v[6] * m->v[12];
  float det2_13_03 = m->v[4] * m->v[15] - m->v[7] * m->v[12];
  float det2_13_12 = m->v[5] * m->v[14] - m->v[6] * m->v[13];
  float det2_13_13 = m->v[5] * m->v[15] - m->v[7] * m->v[13];
  float det2_13_23 = m->v[6] * m->v[15] - m->v[7] * m->v[14];

  // remaining 3x3 sub-determinants
  float det3_203_012 =
    m->v[8] * det2_03_12 - m->v[9] * det2_03_02 + m->v[10] * det2_03_01;
  float det3_203_013 =
    m->v[8] * det2_03_13 - m->v[9] * det2_03_03 + m->v[11] * det2_03_01;
  float det3_203_023 =
    m->v[8] * det2_03_23 - m->v[10] * det2_03_03 + m->v[11] * det2_03_02;
  float det3_203_123 =
    m->v[9] * det2_03_23 - m->v[10] * det2_03_13 + m->v[11] * det2_03_12;

  float det3_213_012 =
    m->v[8] * det2_13_12 - m->v[9] * det2_13_02 + m->v[10] * det2_13_01;
  float det3_213_013 =
    m->v[8] * det2_13_13 - m->v[9] * det2_13_03 + m->v[11] * det2_13_01;
  float det3_213_023 =
    m->v[8] * det2_13_23 - m->v[10] * det2_13_03 + m->v[11] * det2_13_02;
  float det3_213_123 =
    m->v[9] * det2_13_23 - m->v[10] * det2_13_13 + m->v[11] * det2_13_12;

  float det3_301_012 =
    m->v[12] * det2_01_12 - m->v[13] * det2_01_02 + m->v[14] * det2_01_01;
  float det3_301_013 =
    m->v[12] * det2_01_13 - m->v[13] * det2_01_03 + m->v[15] * det2_01_01;
  float det3_301_023 =
    m->v[12] * det2_01_23 - m->v[14] * det2_01_03 + m->v[15] * det2_01_02;
  float det3_301_123 =
    m->v[13] * det2_01_23 - m->v[14] * det2_01_13 + m->v[15] * det2_01_12;

  out.v[0] = -det3_213_123 * invDet;
  out.v[4] = +det3_213_023 * invDet;
  out.v[8] = -det3_213_013 * invDet;
  out.v[12] = +det3_213_012 * invDet;
  out.v[1] = +det3_203_123 * invDet;
  out.v[5] = -det3_203_023 * invDet;
  out.v[9] = +det3_203_013 * invDet;
  out.v[13] = -det3_203_012 * invDet;
  out.v[2] = +det3_301_123 * invDet;
  out.v[6] = -det3_301_023 * invDet;
  out.v[10] = +det3_301_013 * invDet;
  out.v[14] = -det3_301_012 * invDet;
  out.v[3] = -det3_201_123 * invDet;
  out.v[7] = +det3_201_023 * invDet;
  out.v[11] = -det3_201_013 * invDet;
  out.v[15] = +det3_201_012 * invDet;

  return out;
}

struct vec3
MAT4_MultiplyVector3D(struct mat4 const* _m, struct vec3 const* v)
{
  float const* m = _m->v;
  float fInvW = 1.0f / (m[3] * v->x + m[7] * v->y + m[11] * v->z + m[15]);
  return (struct vec3){
    .x = (m[0] * v->x + m[4] * v->y + m[8] * v->z + m[12]) * fInvW,
    .y = (m[1] * v->x + m[5] * v->y + m[9] * v->z + m[13]) * fInvW,
    .z = (m[2] * v->x + m[6] * v->y + m[10] * v->z + m[14]) * fInvW,
  };
}

void
MAT4_Rotate(struct mat4* m, struct vec3 const* euler, enum RotationOrder order)
{
  float const DEG2RAD = 3.14159f / 180.f;

  struct mat4 rx, ry, rz, tmp, tmp2;

  rx = MAT4_Identity();
  ry = MAT4_Identity();
  rz = MAT4_Identity();

  rx.v[5] = rx.v[10] = cos(euler->x * DEG2RAD);
  rx.v[9] = rx.v[6] = sin(euler->x * DEG2RAD);
  rx.v[9] = -rx.v[9];

  ry.v[0] = ry.v[10] = cos(euler->y * DEG2RAD);
  ry.v[8] = ry.v[2] = sin(euler->y * DEG2RAD);
  ry.v[2] = -ry.v[2];

  rz.v[0] = rz.v[5] = cos(euler->z * DEG2RAD);
  rz.v[4] = rz.v[1] = sin(euler->z * DEG2RAD);
  rz.v[4] = -rz.v[4];

  switch (order) {
    case kRotationOrderXYZ:
      tmp = MAT4_Multiply(&rz, &ry);
      tmp2 = MAT4_Multiply(&tmp, &rx);
      break;
    case kRotationOrderXZY:
      tmp = MAT4_Multiply(&ry, &rz);
      tmp2 = MAT4_Multiply(&tmp, &rx);
      break;
    case kRotationOrderYXZ:
      tmp = MAT4_Multiply(&rz, &rx);
      tmp2 = MAT4_Multiply(&tmp, &ry);
      break;
    case kRotationOrderYZX:
      tmp = MAT4_Multiply(&rx, &rz);
      tmp2 = MAT4_Multiply(&tmp, &ry);
      break;
    case kRotationOrderZXY:
      tmp = MAT4_Multiply(&ry, &rx);
      tmp2 = MAT4_Multiply(&tmp, &rz);
      break;
    case kRotationOrderZYX:
      tmp = MAT4_Multiply(&rx, &ry);
      tmp2 = MAT4_Multiply(&tmp, &rz);
      break;
    default:
      tmp2 = MAT4_Identity();
      break;
  }

  *m = MAT4_Multiply(m, &tmp2);
}

void
MAT4_RotateQuat(struct mat4* m, struct quat const* quat)
{
  struct mat4 tmp, tmp2;
  struct vec3 zero = { 0, 0, 0 };
  tmp = *m;
  tmp2 = MAT4_FromRotationOrigin(quat, &zero);
  *m = MAT4_Multiply(&tmp, &tmp2);
}

struct mat4
MAT4_FromRotationOrigin(struct quat const* rotation,
                        struct vec3 const* origin)
{
  float const x = rotation->x;
  float const y = rotation->y;
  float const z = rotation->z;
  float const w = rotation->w;
  float const x2 = x + x;
  float const y2 = y + y;
  float const z2 = z + z;
  float const xx = x * x2;
  float const xy = x * y2;
  float const xz = x * z2;
  float const yy = y * y2;
  float const yz = y * z2;
  float const zz = z * z2;
  float const wx = w * x2;
  float const wy = w * y2;
  float const wz = w * z2;
  float const ox = origin->x;
  float const oy = origin->y;
  float const oz = origin->z;

  struct mat4 out = {0};
  out.v[0] = (1 - (yy + zz));
  out.v[1] = (xy + wz);
  out.v[2] = (xz - wy);
  out.v[3] = 0;
  out.v[4] = (xy - wz);
  out.v[5] = (1 - (xx + zz));
  out.v[6] = (yz + wx);
  out.v[7] = 0;
  out.v[8] = (xz + wy);
  out.v[9] = (yz - wx);
  out.v[10] = (1 - (xx + yy));
  out.v[11] = 0;
  out.v[12] = ox - (out.v[0] * ox + out.v[4] * oy + out.v[8] * oz);
  out.v[13] = oy - (out.v[1] * ox + out.v[5] * oy + out.v[9] * oz);
  out.v[14] = oz - (out.v[2] * ox + out.v[6] * oy + out.v[10] * oz);
  out.v[15] = 1;
  return out;
}

struct mat4
MAT4_FromRotationTranslationScaleOrigin(struct quat const* q,
                                        struct vec3 const* v,
                                        struct vec3 const* s,
                                        struct vec3 const* o)
{
  float const x = q->x;
  float const y = q->y;
  float const z = q->z;
  float const w = q->w;
  float const x2 = x + x;
  float const y2 = y + y;
  float const z2 = z + z;
  float const xx = x * x2;
  float const xy = x * y2;
  float const xz = x * z2;
  float const yy = y * y2;
  float const yz = y * z2;
  float const zz = z * z2;
  float const wx = w * x2;
  float const wy = w * y2;
  float const wz = w * z2;
  float const sx = s->x;
  float const sy = s->y;
  float const sz = s->z;
  float const ox = o->x;
  float const oy = o->y;
  float const oz = o->z;
  float const out0 = (1 - (yy + zz)) * sx;
  float const out1 = (xy + wz) * sx;
  float const out2 = (xz - wy) * sx;
  float const out4 = (xy - wz) * sy;
  float const out5 = (1 - (xx + zz)) * sy;
  float const out6 = (yz + wx) * sy;
  float const out8 = (xz + wy) * sz;
  float const out9 = (yz - wx) * sz;
  float const out10 = (1 - (xx + yy)) * sz;

  struct mat4 out={0};
  out.v[0] = out0;
  out.v[1] = out1;
  out.v[2] = out2;
  out.v[3] = 0;
  out.v[4] = out4;
  out.v[5] = out5;
  out.v[6] = out6;
  out.v[7] = 0;
  out.v[8] = out8;
  out.v[9] = out9;
  out.v[10] = out10;
  out.v[11] = 0;
  out.v[12] = v->x + ox - (out0 * ox + out4 * oy + out8 * oz);
  out.v[13] = v->y + oy - (out1 * ox + out5 * oy + out9 * oz);
  out.v[14] = v->z + oz - (out2 * ox + out6 * oy + out10 * oz);
  out.v[15] = 1;
  return out;
}

struct mat4
MAT4_FromTranslation(struct vec3 const* v)
{
  struct mat4 out;
  out.v[0] = 1;
  out.v[1] = 0;
  out.v[2] = 0;
  out.v[3] = 0;
  out.v[4] = 0;
  out.v[5] = 1;
  out.v[6] = 0;
  out.v[7] = 0;
  out.v[8] = 0;
  out.v[9] = 0;
  out.v[10] = 1;
  out.v[11] = 0;
  out.v[12] = v->x;
  out.v[13] = v->y;
  out.v[14] = v->z;
  out.v[15] = 1;
  return out;
}

struct color COLOR_Parse(lpcString_t string) {
  int r = 255, g = 255, b = 255, a = 100;
  if (sscanf(string, "#%02x%02x%02x%02x", &r, &g, &b, &a) == 4) {
    return color_new(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
  } else if (sscanf(string, "#%02x%02x%02x/%d", &r, &g, &b, &a) == 4) {
    return color_new(r / 255.0f, g / 255.0f, b / 255.0f, a / 100.0f);
  } else if (sscanf(string, "#%02x%02x%02x", &r, &g, &b) == 3) {
    return color_new(r / 255.0f, g / 255.0f, b / 255.0f, 1.f);
  } if (sscanf(string, "#%01x%01x%01x", &r, &g, &b) == 3) {
    // Multiply each value by 17 to expand from 0-15 to 0-255
    return color_new((r * 17) / 255.0f, (g * 17) / 255.0f, (b * 17) / 255.0f, 1.f);
  } else {
    return color_new(1, 1, 1, 1);
  }
}

struct color COLOR_Lerp(struct color const* a, struct color const* b, float t) {
  return (struct color){
    .r = a->r * (1 - t) + b->r * t,
    .g = a->g * (1 - t) + b->g * t,
    .b = a->b * (1 - t) + b->b * t,
    .a = a->a * (1 - t) + b->a * t
  };
}

void
plane3_Normalize(struct plane3* p)
{
  // Here we calculate the magnitude of the normal to the plane (point A B C)
  // Remember that (A, B, C) is that same thing as the normal's (X, Y, Z).
  // To calculate magnitude you use the equation:  magnitude = sqrt( x^2 +
  // y^2 + z^2)
  float magnitude = (float)sqrt(p->a * p->a + p->b * p->b + p->c * p->c);

  // Then we divide the plane's values by it's magnitude.
  // This makes it easier to work with.
  p->a /= magnitude;
  p->b /= magnitude;
  p->c /= magnitude;
  p->d /= magnitude;
}

float
plane3_MultiplyVector3D(struct plane3 const* p, struct vec3 const* v)
{
  return p->a * v->x + p->b * v->y + p->c * v->z + p->d;
}

struct quat
QUAT_Slerp(struct quat const* a, struct quat const* b, float t)
{
  float ax = a->x, ay = a->y, az = a->z, aw = a->w;
  float bx = b->x, by = b->y, bz = b->z, bw = b->w;
  float omega, cosom, sinom, scale0, scale1;
  cosom = ax * bx + ay * by + az * bz + aw * bw;
  if (cosom < 0.0f) {
    cosom = -cosom;
    bx = -bx;
    by = -by;
    bz = -bz;
    bw = -bw;
  }
  if (1.0f - cosom > EPSILON) {
    omega = acosf(cosom);
    sinom = sinf(omega);
    scale0 = sinf((1.0f - t) * omega) / sinom;
    scale1 = sinf(t * omega) / sinom;
  } else {
    scale0 = 1.0f - t;
    scale1 = t;
  }
  return (struct quat){
    .x = scale0 * ax + scale1 * bx,
    .y = scale0 * ay + scale1 * by,
    .z = scale0 * az + scale1 * bz,
    .w = scale0 * aw + scale1 * bw,
  };
}

struct quat
QUAT_Sqlerp(struct quat const* a, struct quat const* b, struct quat const* c, struct quat const* d, float t)
{
  struct quat temp1 = QUAT_Slerp(a, d, t);
  struct quat temp2 = QUAT_Slerp(b, c, t);
  return QUAT_Slerp(&temp1, &temp2, 2 * t * (1 - t));
}

float
QUAT_DotProduct(struct quat const* left, struct quat const* right)
{
  return left->w * right->w + left->x * right->x + left->y * right->y +
         left->z * right->z;
}

float
QUAT_Length(struct quat const* param)
{
  return sqrt(QUAT_DotProduct(param, param));
}

struct quat
QUAT_Unm(struct quat const* param)
{
  return QUAT_Normalized(&(struct quat){
    .x = -param->x,
    .y = -param->y,
    .z = -param->z,
    .w = param->w,
  });
}

struct quat
QUAT_Normalized(struct quat const* param)
{
  struct quat r;
  float length = QUAT_Length(param);
  if (length < EPSILON)
    return *param;
  r.x = param->x / length;
  r.y = param->y / length;
  r.z = param->z / length;
  r.w = param->w / length;
  return r;
}

struct quat
QUAT_FromMatrix(struct mat4 const* mat)
{
  struct quat r;

  // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
  // article "Quaternion Calculus and Fast Animation".

  float fTrace = mat->v[0] + mat->v[5] + mat->v[10];
  float fRoot;

  if (fTrace > 0.0f) {
    // |w| > 1/2, may as well choose w > 1/2
    fRoot = sqrt(fTrace + 1.0f); // 2w
    r.w = 0.5f * fRoot;
    fRoot = 0.5f / fRoot; // 1/(4w)
    r.x = (mat->v[6] - mat->v[9]) * fRoot;
    r.y = (mat->v[8] - mat->v[2]) * fRoot;
    r.z = (mat->v[1] - mat->v[4]) * fRoot;
  } else {
    // |w| <= 1/2
    static int s_iNext[3] = { 1, 2, 0 };
    int i = 0;
    if (mat->v[5] > mat->v[0])
      i = 1;
    if (mat->v[10] > mat->v[i*4+i])
      i = 2;
    int j = s_iNext[i];
    int k = s_iNext[j];
    fRoot = sqrt(mat->v[i*4+i] - mat->v[j*4+j] - mat->v[k*4+k] + 1.0f);
    float* apkQuat[3] = { &r.x, &r.y, &r.z };
    *apkQuat[i] = 0.5f * fRoot;
    fRoot = 0.5f / fRoot;
    r.w = (mat->v[j*4+k] - mat->v[k*4+j]) * fRoot;
    *apkQuat[j] = (mat->v[i*4+j] + mat->v[j*4+i]) * fRoot;
    *apkQuat[k] = (mat->v[i*4+k] + mat->v[k*4+i]) * fRoot;
  }

  return QUAT_Normalized(&r);
}

struct quat
QUAT_FromEuler(struct vec3 const* euler, enum RotationOrder order)
{
  struct mat4 tmp;
  tmp = MAT4_Identity();
  MAT4_Rotate(&tmp, euler, order);
  return QUAT_FromMatrix(&tmp);
}

struct transform2
transform2_Identity(void)
{
  return (struct transform2){
    .translation = { 0, 0 },
    .rotation = 0,
    .scale = { 1, 1 },
  };
}

struct transform3
transform3_Identity(void)
{
  return (struct transform3){
    .translation = { 0, 0, 0 },
    .rotation = { 0, 0, 0 },
    .scale = { 1, 1, 1 },
  };
}

struct mat4
transform2_ToMatrix3D(struct transform2 const* transform,
                      struct vec2 const* pivot)
{
  struct transform3 tr = {
    { transform->translation.x, transform->translation.y, 0 },
    { 0, 0, transform->rotation },
    { transform->scale.x, transform->scale.y, 1 },
  };
  struct vec3 pv = { pivot->x, pivot->y, 0 };
  return transform3_ToMatrix3D(&tr, &pv);
}

struct mat4
transform3_ToMatrix3D(struct transform3 const* transform,
                      struct vec3 const* pivot)
{
  struct mat4 matrix = MAT4_Identity();
  MAT4_Translate(&matrix, &transform->translation);

  if (pivot) {
    MAT4_Translate(&matrix, pivot);
  }

  MAT4_Rotate(&matrix, &transform->rotation, kRotationOrderXYZ);
  MAT4_Scale(&matrix, &transform->scale);

  if (pivot) {
    struct vec3 np = VEC3_Unm(pivot);
    MAT4_Translate(&matrix, &np);
  }
  return matrix;
}

struct vec3
triangle3_normal(struct triangle3 const* triangle)
{
  struct vec3 const tside1 = VEC3_Sub(&triangle->b, &triangle->a);
  struct vec3 const tside2 = VEC3_Sub(&triangle->c, &triangle->a);
  return VEC3_Cross(&tside1, &tside2);
}

#define RectRight(rect) (rect).x + (rect).width
#define RectBottom(rect) (rect).y + (rect).height

void
calculate_ninepatch(struct vec2 const* framesize,
                    struct vec2 const* imgsize,
                    struct edges const* insets,
                    struct edges const* edges,
                    struct rect const* uv,
                    struct mat4* output)
{
  struct rect const frame = {
    -insets->left,
    -insets->top,
    framesize->x + (insets->left + insets->right),
    framesize->y + (insets->top + insets->bottom),
  };
  struct rect const sliced_frame = {
    frame.x + edges->left,
    frame.y + edges->top,
    frame.width - (edges->left + edges->right),
    frame.height - (edges->top + edges->bottom),
  };

  float const _x[4] = { frame.x,
                        fmin(sliced_frame.x, RectRight(frame)),
                        fmax(RectRight(sliced_frame),
                             fmin(sliced_frame.x, RectRight(frame))),
                        RectRight(frame) };

  float const _u[4] = { uv->x,
                        fmin(uv->x + edges->left / imgsize->x, RectRight(*uv)),
                        fmax(RectRight(*uv) - edges->right / imgsize->x,
                             fmin(uv->x + edges->left / imgsize->x,
                                  RectRight(*uv))),
                        RectRight(*uv) };

  float const _y[4] = {
    frame.y,
    fmin(sliced_frame.y, RectBottom(frame)),
    fmax(RectBottom(sliced_frame), fmin(sliced_frame.y, RectBottom(frame))),
    RectBottom(frame),
  };

  float const _v[4] = {
    uv->y,
    fmin(uv->y + edges->top / imgsize->y, RectBottom(*uv)),
    fmax(RectBottom(*uv) - edges->bottom / imgsize->y,
         fmin(uv->y + edges->top / imgsize->y, RectBottom(*uv))),
    RectBottom(*uv),
  };

  memcpy(&output->v[0], _x, sizeof(struct vec4));
  memcpy(&output->v[4], _y, sizeof(struct vec4));
  memcpy(&output->v[8], _u, sizeof(struct vec4));
  memcpy(&output->v[12], _v, sizeof(struct vec4));
}

#include <ctype.h>
#include <assert.h>

uint32_t
strlistidx(lpcString_t needle, lpcString_t haystack, lpcString_t *out)
{
  uint32_t index = 0;
  while (haystack) {
    lpcString_t next_comma = strchr(haystack, ',');
    size_t len =
    next_comma ? (size_t)(next_comma - haystack) : strlen(haystack);
    if (strncasecmp(needle, haystack, len) == 0 &&
        (needle[len] == '\0' || (out&&isspace(needle[len])))) {
      if (out) {
        *out = &needle[len];
      }
      return index;
    }
    haystack = next_comma ? next_comma + 1 : NULL;
    index++;
  }
  return -1;
}

lpcString_t strlistget(uint32_t index, lpcString_t haystack) {
  while (haystack) {
    lpcString_t next_comma = strchr(haystack, ',');
    size_t len =
    next_comma ? (size_t)(next_comma - haystack) : strlen(haystack);
    if (index == 0) {
      static char result[64];
      assert(len < sizeof(result) - 1);
      strncpy(result, haystack, len);
      result[len] = '\0';
      return result;
    }
    
    haystack = next_comma ? next_comma + 1 : NULL;
    index--;
  }
  return NULL; // Out of bounds
}

lpcString_t stristr(lpcString_t haystack, lpcString_t needle) {
  size_t needle_len = strlen(needle);
  if (needle_len == 0) {
    return haystack; // Empty needle matches everything
  }
  while (*haystack) {
    if (strncasecmp(haystack, needle, needle_len) == 0) {
      return haystack; // Match found
    }
    haystack++;
  }
  return NULL; // No match
}
