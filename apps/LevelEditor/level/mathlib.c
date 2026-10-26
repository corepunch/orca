#include "types.h"

vec_t VectorNormalize(const vec3_t in, vec3_t out)
{
	vec_t length = sqrt(in[0] * in[0] + in[1] * in[1] + in[2] * in[2]);
	if (length == 0)
	{
		Vector3Clear(out);
		return 0;
	}
	vec_t ilength = 1.0 / length;
	out[0] = in[0] * ilength;
	out[1] = in[1] * ilength;
	out[2] = in[2] * ilength;
	return length;
}

int PlaneFromPoints(vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c)
{
	vec3_t d1, d2;
	Vector3Subtract(b, a, d1);
	Vector3Subtract(c, a, d2);
	Vector3CrossProduct(d2, d1, plane);

	if (VectorNormalize(plane, plane) == 0)
	{
		return 0;
	}
	plane[3] = Vector3DotProduct(a, plane);
	return 1;
}

vec_t Vector3Distance(vec3_t const a, vec3_t const b)
{
	vec3_t c;
	Vector3Subtract(a, b, c);
	return sqrt(Vector3DotProduct(c, c));
}

void GetVector3FromColor(color_t normal, vec3_t out)
{
	int8_t const *n = (int8_t const *)&normal;
	out[0] = n[0] / 128.0f;
	out[1] = n[1] / 128.0f;
	out[2] = n[2] / 128.0f;
}

void Vector3Lerp4(vec3_t const a, vec3_t const b, vec3_t const c, vec3_t const d, vec2_t const uv, vec3_t out)
{
	// vec3_t tmp1, tmp2, out2;
	// Vector3Lerp(a, b, uv[1], tmp1);
	// Vector3Lerp(c, d, uv[1], tmp2);
	// Vector3Lerp(tmp1, tmp2, uv[0], out2);
	if (uv[0] < uv[1])
	{
		vec3_t ab, db, tmp;
		Vector3Subtract(a, b, ab);
		Vector3Subtract(d, b, db);
		Vector3MA(b, uv[0], db, tmp);
		Vector3MA(tmp, 1 - uv[1], ab, out);
	}
	else
	{
		vec3_t ac, dc, tmp;
		Vector3Subtract(a, c, ac);
		Vector3Subtract(d, c, dc);
		Vector3MA(c, uv[1], dc, tmp);
		Vector3MA(tmp, 1 - uv[0], ac, out);
	}
}

void ColorLerp4(color_t a, color_t b, color_t c, color_t d, vec2_t const uv, vec3_t out)
{
	vec3_t _a, _b, _c, _d;
	GetVector3FromColor(a, _a);
	GetVector3FromColor(b, _b);
	GetVector3FromColor(c, _c);
	GetVector3FromColor(d, _d);
	Vector3Lerp4(_a, _b, _c, _d, uv, out);
}
