#include "types.h"

#define PLANAR_EPSILON 0.001
#define ON_EPSILON 0.001

line_t lines[1000];
int num_lines = 0;

void Debug_AddLine(vec3_t const a, vec3_t const b)
{
	if (num_lines < 1000)
	{
		Vector3Copy(a, lines[num_lines].a);
		Vector3Copy(b, lines[num_lines].b);
		num_lines++;
	}
}

void Debug_GetLineAt(room_t const *room, int index, vec3_t a, vec3_t b)
{
	Vector3Copy(lines[index].a, a);
	Vector3Copy(lines[index].b, b);
}

void TraceAgainstFacet(traceWork_t *tr, facet_t *facet)
{
	int j;
	float d1, d2, d, f;
	vec3_t point;

	// ignore degenerate facets
	if (facet->numBoundaries < 3)
	{
		return;
	}

	float dist = facet->surface[3];

	// compare the trace endpoints against the facet plane
	d1 = Vector3DotProduct(tr->start, facet->surface) - dist;
	// if (d1 > -1 && d1 < 1)
	// {
	// 	return; // don't self intersect
	// }
	d2 = Vector3DotProduct(tr->end, facet->surface) - dist;
	// if (d2 > -1 && d2 < 1)
	// {
	// 	return; // don't self intersect
	// }

	// calculate the intersection fraction
	f = (d1 - ON_EPSILON) / (d1 - d2);
	if (f <= 0)
	{
		return;
	}
	if (f >= tr->trace->hitFraction)
	{
		return; // we have hit something earlier
	}

	// calculate the intersection point
	for (j = 0; j < 3; j++)
	{
		point[j] = tr->start[j] + f * (tr->end[j] - tr->start[j]);
	}

	// check the point against the facet boundaries
	for (j = 0; j < facet->numBoundaries; j++)
	{
		// adjust the plane distance apropriately for mins/maxs
		dist = facet->boundaries[j][3];

		d = Vector3DotProduct(point, facet->boundaries[j]);
		if (d > dist + ON_EPSILON)
		{
			break; // outside the bounds
		}
	}

	if (j != facet->numBoundaries)
	{
		return; // we are outside the bounds of the facet
	}

	// we hit this facet

	tr->trace->hitFraction = f;

	//	VectorCopy( facet->surface, tr->trace->plane.normal );
	//	tr->trace->plane.dist = facet->surface[3];
}

void TraceAgainstSurface(traceWork_t *tw, surfaceTest_t const *surf)
{
	for (int i = 0; i < surf->numFacets; i++)
	{
		TraceAgainstFacet(tw, surf->facets + i);
	}
}

void CM_GenerateBoundaryForPoints(float boundary[4], float plane[4], vec3_t a, vec3_t b)
{
	vec3_t d1;
	// make a perpendicular vector to the edge and the surface
	Vector3Subtract(b, a, d1);
	Vector3CrossProduct(plane, d1, boundary);
	Vector3Normalize(boundary);
	boundary[3] = Vector3DotProduct(a, boundary);
}

int CM_GenerateFacetFor3Points(facet_t *f, drawVert_t *a, drawVert_t *b, drawVert_t *c)
{
	// if we can't generate a valid plane for the points, ignore the facet
	if (!PlaneFromPoints(f->surface, a->xyz, b->xyz, c->xyz))
	{
		f->numBoundaries = 0;
		return 0;
	}

	// make boundaries
	f->numBoundaries = 3;

	CM_GenerateBoundaryForPoints(f->boundaries[0], f->surface, a->xyz, b->xyz);
	CM_GenerateBoundaryForPoints(f->boundaries[1], f->surface, b->xyz, c->xyz);
	CM_GenerateBoundaryForPoints(f->boundaries[2], f->surface, c->xyz, a->xyz);

	Vector3Copy(a->xyz, f->points[0]);
	Vector3Copy(b->xyz, f->points[1]);
	Vector3Copy(c->xyz, f->points[2]);

	// TextureMatrixFromPoints( f, a, b, c );

	return 1;
}

int CM_GenerateFacetFor4Points(facet_t *f, drawVert_t *a, drawVert_t *b, drawVert_t *c, drawVert_t *d)
{
	vec4_t plane;

	// if we can't generate a valid plane for the points, ignore the facet
	if (!PlaneFromPoints(f->surface, a->xyz, b->xyz, c->xyz))
	{
		f->numBoundaries = 0;
		return 0;
	}

	// if the fourth point is also on the plane, we can make a quad facet
	vec_t dist = Vector3DotProduct(d->xyz, f->surface) - f->surface[3];
	if (fabs(dist) > PLANAR_EPSILON)
	{
		f->numBoundaries = 0;
		return 0;
	}

	// make boundaries
	f->numBoundaries = 4;

	CM_GenerateBoundaryForPoints(f->boundaries[0], f->surface, a->xyz, b->xyz);
	CM_GenerateBoundaryForPoints(f->boundaries[1], f->surface, b->xyz, c->xyz);
	CM_GenerateBoundaryForPoints(f->boundaries[2], f->surface, c->xyz, d->xyz);
	CM_GenerateBoundaryForPoints(f->boundaries[3], f->surface, d->xyz, a->xyz);

	Vector3Copy(a->xyz, f->points[0]);
	Vector3Copy(b->xyz, f->points[1]);
	Vector3Copy(c->xyz, f->points[2]);
	Vector3Copy(d->xyz, f->points[3]);

	for (int i = 1; i < 4; i++)
	{
		if (!PlaneFromPoints(plane, f->points[i], f->points[(i + 1) % 4], f->points[(i + 2) % 4]))
		{
			f->numBoundaries = 0;
			return 0;
		}

		if (Vector3DotProduct(f->surface, plane) < 0.9)
		{
			f->numBoundaries = 0;
			return 0;
		}
	}

	// TextureMatrixFromPoints(f, a, b, c);

	return 1;
}

static float GetLightAt(int tri, vec2_t const uv, vec2_t const sm, dsurface_t const *dsurf, surfaceTest_t const *test)
{
	trace_t trace;
	traceWork_t tw;
	vec3_t start, end, tangent, binormal, n, tmp, normal;
	drawIndex_t const *ind = dsurf->indices;
	drawVert_t const *vert = dsurf->vertices;
	drawVert_t const *a = vert + ind[tri + 1];
	drawVert_t const *b = vert + ind[tri + 0];
	drawVert_t const *c = vert + ind[tri + 5];
	drawVert_t const *d = vert + ind[tri + 2];

	trace.hitFraction = 1;
	tw.trace = &trace;

	Vector3Lerp4(a->xyz, b->xyz, c->xyz, d->xyz, uv, start);
	ColorLerp4(a->normal, b->normal, c->normal, d->normal, uv, n);
	ColorLerp4(a->tangent, b->tangent, c->tangent, d->tangent, uv, tangent);
	ColorLerp4(a->binormal, b->binormal, c->binormal, d->binormal, uv, binormal);

	if (sm[0] > 0)
	{
		Vector3Lerp(n, tangent, sm[0], tmp);
	}
	else
	{
		vec3_t neg;
		Vector3Negate(tangent, neg);
		Vector3Lerp(n, neg, -sm[0], tmp);
	}

	if (sm[1] > 0)
	{
		Vector3Lerp(tmp, binormal, sm[1], normal);
	}
	else
	{
		vec3_t neg;
		Vector3Negate(binormal, neg);
		Vector3Lerp(tmp, neg, -sm[1], normal);
	}

	Vector3Normalize(normal);
	Vector3MA(start, 100, normal, end);
	Vector3Copy(end, trace.hit);
	Vector3MA(start, 0.5, normal, tw.start);
	Vector3Copy(end, tw.end);

	TraceAgainstSurface(&tw, test);

	// vec3_t hit;
	// Vector3Lerp(start, end, trace.hitFraction, hit);

	// Debug_AddLine(start, hit);

	return trace.hitFraction;
}

static surfaceTest_t Room_GetFacets(dsurface_t const *dsurf)
{
	int count = 0;
	surfaceTest_t test;
	test.numFacets = dsurf->numIndices / 3;
	test.facets = gi->Malloc(sizeof(facet_t) * test.numFacets, MEM_SCRIPT);
	for (int i = 0; i < test.numFacets; i++)
	{
		int ind = i * 3;
		int i1 = dsurf->indices[ind + 0];
		int i2 = dsurf->indices[ind + 1];
		int i3 = dsurf->indices[ind + 2];
		drawVert_t *v1 = &dsurf->vertices[i1];
		drawVert_t *v2 = &dsurf->vertices[i2];
		drawVert_t *v3 = &dsurf->vertices[i3];
		// try and make a quad out of two triangles
		if (i != test.numFacets - 1)
		{
			int i4 = dsurf->indices[ind + 3];
			int i5 = dsurf->indices[ind + 4];
			int i6 = dsurf->indices[ind + 5];
			if (i4 == i3 && i5 == i2)
			{
				drawVert_t *v4 = &dsurf->vertices[i6];
				if (CM_GenerateFacetFor4Points(&test.facets[count], v1, v2, v4, v3))
				{
					count++;
					i++; // skip next tri
					continue;
				}
			}
		}
		if (CM_GenerateFacetFor3Points(&test.facets[count], v1, v2, v3))
		{
			count++;
		}
	}

	test.numFacets = count;

	return test;
}

typedef color_t lightmap_t[LIGHTMAP_SIZE][LIGHTMAP_SIZE];

typedef struct {
	int x, y;
	dsurface_t const *dsurf;
	surfaceTest_t const *test;
	lightmap_t *bitmap;
	int threadid;
} aoThreadWorker_t;

static void AO_Thead(void *ptr)
{
	const float rays[] = {
			0.75f,
			0.5f,
			0.25f,
			// 0.f,
			-0.25f,
			-0.5f,
			-0.75f,
	};
	aoThreadWorker_t *wrk = ptr;
	vec2_t uv = { wrk->y / 4.0f + 0.125f, wrk->x / 4.0f + 0.125f };
	FOR_LOOP(idx, wrk->dsurf->numIndices / 6)
	{
		const int start = idx * 6;
		const int u = idx * 4;
		const int width = LIGHTMAP_SIZE - LIGHTMAP_RES;
		const int lmu = u % width;
		const int lmv = (u / width) * LIGHTMAP_RES;
		float s = 0.0f;
		FOR_LOOP(_x, 6)
		{
			float weight_x = 1.0f - fabs(rays[_x]);
			FOR_LOOP(_y, 6)
			{
				float weight_y = 1.0f - fabs(rays[_y]);
				vec2_t sm = { rays[_x], rays[_y] };
				s += GetLightAt(start, uv, sm, wrk->dsurf, wrk->test) * weight_x * weight_y;
			}
		}
		s *= 0.1f;
		color_t light = { s * 255, s * 255, s * 255, ~0 };
		(*wrk->bitmap)[lmv + wrk->y][lmu + wrk->x] = light;
	}
}

#define NUM_WORKERS LIGHTMAP_RES * LIGHTMAP_RES

image_t *Room_AllocateLightmap(dsurface_t const *dsurf)
{
	surfaceTest_t test = Room_GetFacets(dsurf);

	num_lines = 0;
	static lightmap_t roomBitmap;
	memset(roomBitmap, 0, sizeof(roomBitmap));
	aoThreadWorker_t workers[NUM_WORKERS];
	aoThreadWorker_t *w = workers;

	FOR_TILES(0, 0, LIGHTMAP_RES, LIGHTMAP_RES)
	{
		w->x = x;
		w->y = y;
		w->bitmap = &roomBitmap;
		w->test = &test;
		w->dsurf = dsurf;
		w->threadid = gi->AddThread(AO_Thead, w);
		w++;
	}

	FOR_LOOP(idx, NUM_WORKERS)
	{
		gi->RemoveThread(workers[idx].threadid);
	}

	return gi->renderer->AllocateTexture(LIGHTMAP_SIZE, LIGHTMAP_SIZE, (color_t const *)roomBitmap);
}
