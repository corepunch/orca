#include "types.h"

typedef int neighbor_t[3];

static drawIndex_t neighbors[NUM_BLOCKS * 6 * 8];
static const int verts[3][2] = {{0, 1}, {1, 2}, {2, 0}};

static int GetEdge(int index) { return index % 3; }
static int GetTriangle(int index) { return (index / 3) * 3; }

static float GetRatio(vec3_t const v, vec3_t const p0, vec3_t const p1)
{
	return Vector3Distance(v, p0) / Vector3Distance(p1, p0);
}

static int IsVertexBetween(vec3_t const v, vec3_t const p0, vec3_t const p1)
{
	vec3_t zero = {0, 0, 0};
	vec3_t vp0, vp1, p0p1, vc;

	Vector3Subtract(v, p0, vp0);
	Vector3Subtract(v, p1, vp1);
	Vector3Subtract(p0, p1, p0p1);
	Vector3CrossProduct(vp0, vp1, vc);

	if (!Vector3Compare(vc, zero))
		return 0;

	if (Vector3DotProduct(vp1, p0p1) <= 0)
		return 0;

	if (Vector3DotProduct(vp0, p0p1) >= 0)
		return 0;

	return 1;
}

static int EdgeHasNoNeighbor(dsurface_t const *dsurf, int index)
{
	return dsurf->neighbors[index] == (drawIndex_t)-1;
}

static drawIndex_t GetOppositeIndexToEdge(dsurface_t const *dsurf, int index)
{
	int edge = GetEdge(index);
	int triangle = GetTriangle(index);
	return dsurf->indices[triangle + 3 - verts[edge][0] - verts[edge][1]];
}

static int GetIndexAddressAtEdge(int index, int vertex)
{
	int edge = GetEdge(index);
	int triangle = GetTriangle(index);
	return triangle + verts[edge][vertex];
}

static drawIndex_t GetIndexAtEdge(dsurface_t const *dsurf, int index, int vertex)
{
	return dsurf->indices[GetIndexAddressAtEdge(index, vertex)];
}

drawVert_t const *GetVertexAtEdge(dsurface_t const *dsurf, int index, int vertex)
{
	return &dsurf->vertices[GetIndexAtEdge(dsurf, index, vertex)];
}

int AddIndex(dsurface_t *dsurf, drawIndex_t index)
{
	int pos = dsurf->numIndices++;
	((drawIndex_t *)dsurf->indices)[pos] = index;
	return pos;
}

static void Vector2Lerp(vec2_t const a, vec2_t const b, vec_t k, vec2_t out)
{
	out[0] = a[0] * k + b[0] * (1 - k);
	out[1] = a[1] * k + b[1] * (1 - k);
}

static void FindNeighbor(int t, dsurface_t *dsurf)
{
	drawVert_t const *a = GetVertexAtEdge(dsurf, t, 0);
	drawVert_t const *b = GetVertexAtEdge(dsurf, t, 1);
	for (int i = t + 1; i < dsurf->numIndices; i++)
	{
		drawVert_t const *c = GetVertexAtEdge(dsurf, i, 0);
		drawVert_t const *d = GetVertexAtEdge(dsurf, i, 1);
		if (Vector3Compare(a->xyz, d->xyz) && Vector3Compare(b->xyz, c->xyz))
		{
			((drawIndex_t *)dsurf->neighbors)[t] = GetTriangle(i);
			((drawIndex_t *)dsurf->neighbors)[i] = GetTriangle(t);
			return;
		}
	}
}

static void FindPotentialNeighbor(int t, dsurface_t *dsurf)
{
	drawVert_t const *a = GetVertexAtEdge(dsurf, t, 0);
	drawVert_t const *b = GetVertexAtEdge(dsurf, t, 1);

	for (int i = 0; i < dsurf->numIndices; i++)
	{
		if (!EdgeHasNoNeighbor(dsurf, i))
			continue;
		drawVert_t const *c = GetVertexAtEdge(dsurf, i, 0);
		drawVert_t const *d = GetVertexAtEdge(dsurf, i, 1);
		if (Vector3Compare(c->xyz, b->xyz) && IsVertexBetween(d->xyz, a->xyz, b->xyz))
		{
			// vec_t k = GetRatio(d->xyz, a->xyz, b->xyz);
			// drawIndex_t *indices = (drawIndex_t *)dsurf->indices;
			// drawVert_t *vert = AddVertex(dsurf);
			// memcpy(vert, a, sizeof(drawVert_t));
			// Vector3Copy(d->xyz, vert->xyz);
			// Vector2Lerp(a->texcoord0, b->texcoord0, k, vert->texcoord0);
			// AddIndex(dsurf, vert - dsurf->vertices);
			// AddIndex(dsurf, b - dsurf->vertices);
			// AddIndex(dsurf, GetOppositeIndexToEdge(dsurf, t));
			// indices[GetIndexAddressAtEdge(t, 1)] = vert - dsurf->vertices;
		}
		else if (Vector3Compare(d->xyz, a->xyz) && IsVertexBetween(c->xyz, a->xyz, b->xyz))
		{
			vec_t k = GetRatio(c->xyz, a->xyz, b->xyz);
			drawIndex_t *indices = (drawIndex_t *)dsurf->indices;
			// drawVert_t *vert = AddVertex(dsurf);
			drawVert_t *vert = (drawVert_t *)&dsurf->vertices[dsurf->numVertices++];
			memcpy(vert, a, sizeof(drawVert_t));
			Vector3Copy(c->xyz, vert->xyz);
			Vector2Lerp(a->texcoord0, b->texcoord0, k, vert->texcoord0);
			AddIndex(dsurf, a - dsurf->vertices);
			AddIndex(dsurf, vert - dsurf->vertices);
			AddIndex(dsurf, GetOppositeIndexToEdge(dsurf, t));
			indices[GetIndexAddressAtEdge(t, 0)] = vert - dsurf->vertices;
		}
	}
}

void Room_ClearInvisibleFaces(dsurface_t *dsurf)
{
	for (int t = 0; t < dsurf->numIndices;)
	{
		drawVert_t *v0 = &dsurf->vertices[dsurf->indices[t++]];
		drawVert_t *v1 = &dsurf->vertices[dsurf->indices[t++]];
		drawVert_t *v2 = &dsurf->vertices[dsurf->indices[t++]];
		if (//Vector3Compare(v0->xyz, v1->xyz) ||
				//Vector3Compare(v1->xyz, v2->xyz) ||
				//Vector3Compare(v2->xyz, v0->xyz) ||
				v0->color.a == 0)
		{
			// for (int t0 = t - 3, t1 = t; t1 < dsurf->numIndices; t0++, t1++)
			// {
			// 	((drawIndex_t *)dsurf->indices)[t0] = ((drawIndex_t *)dsurf->indices)[t1];
			// }
			// dsurf->numIndices -= 3;
			// t -= 3;
			Vector3Set(v0->xyz, 0, 0, 0);
			Vector3Set(v1->xyz, 0, 0, 0);
			Vector3Set(v2->xyz, 0, 0, 0);
		}
	}
}

void Room_Triangulate(room_t const *room, dsurface_t *dsurf, int splitQuads)
{
	memset(neighbors, -1, sizeof(neighbors));

	dsurf->neighbors = neighbors;

	FOR_LOOP(t, dsurf->numIndices)
	{
		if (EdgeHasNoNeighbor(dsurf, t))
		{
			FindNeighbor(t, dsurf);
		}
	}

	Room_FillWalls(room, dsurf);

#if 0
	if (!splitQuads)
		return;
	memset(neighbors, -1, sizeof(neighbors));

	FOR_LOOP(t, dsurf->numIndices)
	{
		if (EdgeHasNoNeighbor(dsurf, t))
		{
			FindNeighbor(t, dsurf);
		}
	}
#endif

	Room_ClearInvisibleFaces(dsurf);

#if 0
	for (int numVertices = 0; numVertices < dsurf->numVertices;)
	{
		numVertices = dsurf->numVertices;
		FOR_LOOP(t, dsurf->numIndices)
		{
			if (EdgeHasNoNeighbor(dsurf, t))
			{
				FindPotentialNeighbor(t, dsurf);
			}
			// ((drawVert_t *)GetVertexAtEdge(dsurf, t, 0))->color.a = 64;
			// ((drawVert_t *)GetVertexAtEdge(dsurf, t, 1))->color.a = 64;
		}
		memset(neighbors, -1, sizeof(neighbors)); // rebuild neighbors with holes filled
		FOR_LOOP(t, dsurf->numIndices)
		{
			if (EdgeHasNoNeighbor(dsurf, t))
			{
				FindNeighbor(t, dsurf);
			}
		}
	}
	#endif
	// int error = 0;
	// FOR_LOOP(t, dsurf->numIndices)
	// {
	// 	if (EdgeHasNoNeighbor(dsurf, t))
	// 	{
	// 		error++;
	// 	}
	// }
	// if (error > 0)
	// {
	// 	printf("Found %d edges with no neighbors\n", error);
	// }
}
