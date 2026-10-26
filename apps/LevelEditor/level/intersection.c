#include "types.h"

int GetIntersection(line_t const *line, triangle_t const *triangle, vec3_t out) {
    vec3_t dir[2], n, lineDirection, linePoint, pc;
    vec3_t side[3], point[3], cross[3];
    Vector3Subtract(triangle->b, triangle->a, side[0]);
    Vector3Subtract(triangle->c, triangle->b, side[1]);
    Vector3Subtract(triangle->a, triangle->c, side[2]);
    Vector3CrossProduct(side[0], side[1], n);
    Vector3Subtract(line->a, triangle->a, dir[0]);
    Vector3Subtract(line->b, triangle->a, dir[1]);
    float r1 = Vector3DotProduct(n, dir[0]);
    float r2 = Vector3DotProduct(n, dir[1]);
    if (r1 < 0 || r2 > 0) return 0;
    Vector3Subtract(line->a, line->b, lineDirection);
    Vector3Scale(lineDirection, r1 / (r2 - r1), linePoint);
    Vector3Add(line->a, linePoint, pc);
    Vector3Subtract(pc, triangle->a, point[0]);
    Vector3Subtract(pc, triangle->b, point[1]);
    Vector3Subtract(pc, triangle->c, point[2]);
    Vector3CrossProduct(side[0], point[0], cross[0]);
    Vector3CrossProduct(side[1], point[1], cross[1]);
    Vector3CrossProduct(side[2], point[2], cross[2]);
    if (Vector3DotProduct(n, cross[0]) < 0) return 0;
    if (Vector3DotProduct(n, cross[1]) < 0) return 0;
    if (Vector3DotProduct(n, cross[2]) < 0) return 0;
    if (out) Vector3Copy(pc, out);
    return 1;
}

int PointOnPlane(line_t const *line, triangle_t const *triangle, vec3_t out) {
    vec3_t dir[2], n, lineDirection, linePoint, pc;
    vec3_t side[3];
    Vector3Subtract(triangle->b, triangle->a, side[0]);
    Vector3Subtract(triangle->c, triangle->b, side[1]);
    Vector3Subtract(triangle->a, triangle->c, side[2]);
    Vector3CrossProduct(side[0], side[1], n);
    Vector3Subtract(line->a, triangle->a, dir[0]);
    Vector3Subtract(line->b, triangle->a, dir[1]);
    float r1 = Vector3DotProduct(n, dir[0]);
    float r2 = Vector3DotProduct(n, dir[1]);
    if (r1 < 0 || r2 > 0) return 0;
    Vector3Subtract(line->a, line->b, lineDirection);
    Vector3Scale(lineDirection, r1 / (r2 - r1), linePoint);
    Vector3Add(line->a, linePoint, pc);
    if (out) Vector3Copy(pc, out);
    return 1;
}
