#include "types.h"

extern struct game_import *gi;

static drawVert_t vertices[NUM_BLOCKS * 4 * 8];
static drawIndex_t indices[NUM_BLOCKS * 6 * 8];

static color_t MakeColor(uint8_t tilex, uint8_t tiley, uint8_t faceid, uint8_t texture)
{
	color_t color = {tilex, tiley, faceid, texture};
	return color;
}

void AddLightmapQuad(drawVert_t *a, drawVert_t *b, drawVert_t *c, drawVert_t *d, int index)
{
	const int u = index * LIGHTMAP_RES;
	const int width = LIGHTMAP_SIZE - LIGHTMAP_RES;
	const int lmu0 = u % width;
	const int lmv0 = (u / width) * LIGHTMAP_RES;
	const int lmu1 = lmu0 + LIGHTMAP_RES;
	const int lmv1 = lmv0 + LIGHTMAP_RES;

	a->texcoord1.u = lmu0;
	a->texcoord1.v = lmv0;
	b->texcoord1.u = lmu1;
	b->texcoord1.v = lmv0;
	c->texcoord1.u = lmu1;
	c->texcoord1.v = lmv1;
	d->texcoord1.u = lmu0;
	d->texcoord1.v = lmv1;
}

static void AddQuad(long s, drawIndex_t **tt, drawVert_t *v, color_t color, color_t normal, color_t tileinfo)
{
	drawIndex_t *t = *tt;
	int start = (int)s - NUM_CORNERS;
	*(t++) = start + 1;
	*(t++) = start + 0;
	*(t++) = start + 2;
	*(t++) = start + 2;
	*(t++) = start + 0;
	*(t++) = start + 3;
	for (int i = 1; i < 5; i++)
	{
		int g = (v - i)->userdata.g;
		(v - i)->color = color;
		(v - i)->normal = normal;
		(v - i)->userdata = tileinfo;
		(v - i)->userdata.g += g;
	}
	AddLightmapQuad(v - 4, v - 3, v - 2, v - 1, start / NUM_CORNERS);
	*tt = t;
}

uint8_t CheckTexturing(uvquad_t const quad)
{
	FOR_TILES(0, 0, 4, 2)
		if (quad[x][y] >= 250)
			return 0;
	return 1;
}

static void AddVertex(drawVert_t *vert, int x, int y, int z, uvquad_t const quad, int corner)
{
	uv_t uv = {
		quad[corner][0] * CLICK_SCALE_U,
		1 - quad[corner][1] * CLICK_SCALE_V
	};
	Vector3Set(vert->xyz, x * BLOCK_SIZE, y * CLICK_SIZE, z * BLOCK_SIZE);
	Vector2Set(vert->texcoord0, uv.u, uv.v);
	ColorSet(vert->color, 0xff, 0xff, 0xff, 0xff);
	vert->texcoord1.u = ((corner & 1) ^ (corner >> 1)) ? 0 : ~0;
	vert->texcoord1.v = (corner & 2) ? ~0 : 0;
	vert->userdata.g = corner;
}

static int GetFloor(block_t const *t, int index)
{
	return t->height[kBlockLayerFloor][index];
}

static int GetCeiling(block_t const *t, int index)
{
	return t->height[kBlockLayerCeiling][index];
}

static int IsFloor(block_t const *block)
{
	return block->type == kBlockTypeFloor;
}

dsurface_t Room_AllocateModel(room_t const *room)
{
	dsurface_t dsurf = {vertices, indices, NULL, 0, 0, 0};
	drawVert_t *v = vertices;
	drawIndex_t *t = indices;
	memset(vertices, 0, sizeof(vertices));
	memset(indices, 0, sizeof(indices));

	for (int i0 = 0, i1 = 1; i0 < ROOM_SIZE - 1; i0++, i1++)
	{
		for (int j0 = 0, j1 = 1; j0 < ROOM_SIZE - 1; j0++, j1++)
		{
			block_t const *tp = Room_NavigateToCBlockAt(room, i0, j0);
			int const pos = 127, neg = 128, zero = 0;
			int const tp_floor = IsFloor(&room->blocks[i0][j0]);

			if (tp->type == kBlockTypeNone && tp == &room->blocks[i0][j0])
				continue;

			if (tp_floor)
			{
				iuv_t const *uv = room->blocks[i0][j0].uv[kBlockFaceFloor];
				portal_t const *portal = Room_GetPortalOfTypeAt(room, kPortalDirectionFloor, i0, j0);
				color_t const normal = MakeColor(zero, neg, zero, 0);
				color_t const tileinfo = MakeColor(kBlockFaceFloor, CheckTexturing(uv) ? 0 : 64, i0, j0);
				color_t const color = { ~0, ~0, ~0, portal ? 0 : ~0 };
				AddVertex(v++, i0, GetFloor(tp, 0), j0, uv, 0);
				AddVertex(v++, i1, GetFloor(tp, 1), j0, uv, 1);
				AddVertex(v++, i1, GetFloor(tp, 2), j1, uv, 2);
				AddVertex(v++, i0, GetFloor(tp, 3), j1, uv, 3);
				AddQuad(v - vertices, &t, v, color, normal, tileinfo);
			}

			if (tp_floor)
			{
				iuv_t const *uv = room->blocks[i0][j0].uv[kBlockFaceCeiling];
				portal_t const *portal = Room_GetPortalOfTypeAt(room, kPortalDirectionCeiling, i0, j0);
				color_t const normal = MakeColor(zero, pos, zero, 0);
				color_t const tileinfo = MakeColor(kBlockFaceCeiling, CheckTexturing(uv) ? 0 : 64, i0, j0);
				color_t const color = { ~0, ~0, ~0, portal ? 0 : ~0 };
				AddVertex(v++, i1, GetCeiling(tp, 1), j0, uv, 1);
				AddVertex(v++, i0, GetCeiling(tp, 0), j0, uv, 0);
				AddVertex(v++, i0, GetCeiling(tp, 3), j1, uv, 3);
				AddVertex(v++, i1, GetCeiling(tp, 2), j1, uv, 2);
				AddQuad(v - vertices, &t, v, color, normal, tileinfo);
			}
		}
	}

	dsurf.numIndices = (int)(t - indices);
	dsurf.numVertices = (int)(v - vertices);

	Room_Triangulate(room, &dsurf, 1);

	gi->renderer->GenerateModelTangents(&dsurf);

	return dsurf;
}

color_t Room_GetClickedBlock(room_t const *room, vec3_t const a, vec3_t const b)
{
	line_t line;
	triangle_t tri;
	vec3_t new_out;
	vec3_t out;
	vec3_t p[4];
	dsurface_t dsurf = Room_AllocateModel(room);
	color_t value = MakeColor(0, 0, 0, 0);

	Vector3Copy(a, line.a);
	Vector3Copy(b, line.b);
	Vector3Copy(b, out);

	for (int t = 0; t < dsurf.numIndices;)
	{
		drawVert_t const *v = &dsurf.vertices[dsurf.indices[t]];

		Vector3Copy(dsurf.vertices[dsurf.indices[t]].xyz, p[0]);
		t++;

		Vector3Copy(dsurf.vertices[dsurf.indices[t]].xyz, p[1]);
		t++;

		Vector3Copy(dsurf.vertices[dsurf.indices[t]].xyz, p[2]);
		t++;

		Vector3Copy(p[0], tri.a);
		Vector3Copy(p[1], tri.b);
		Vector3Copy(p[2], tri.c);

		if (GetIntersection(&line, &tri, new_out) &&
				Vector3Distance(new_out, a) < Vector3Distance(out, a))
		{
			value = v->userdata;
			Vector3Copy(new_out, out);
		}
	}
	return value;
}

// void Debug_AddLine(vec3_t const a, vec3_t const b);

// void TraceAgainstSurface(dsurface_t const *dsurf, vec3_t const a, vec3_t const b, vec3_t out)
// {
// 	line_t line;
// 	triangle_t tri;
// 	vec3_t new_out;
// 	vec3_t p[4];

// 	Vector3Copy(a, line.a);
// 	Vector3Copy(b, line.b);
// 	Vector3Copy(b, out);

// 	for (int t = 0; t < dsurf->numIndices;)
// 	{
// 		Vector3Copy(dsurf->vertices[dsurf->indices[t]].xyz, p[0]);
// 		t++;

// 		Vector3Copy(dsurf->vertices[dsurf->indices[t]].xyz, p[1]);
// 		t++;

// 		Vector3Copy(dsurf->vertices[dsurf->indices[t]].xyz, p[2]);
// 		t++;

// 		Vector3Copy(p[0], tri.a);
// 		Vector3Copy(p[1], tri.b);
// 		Vector3Copy(p[2], tri.c);

// 		if (GetIntersection(&line, &tri, new_out) &&
// 				Vector3Distance(new_out, a) < Vector3Distance(out, a))
// 		{
// 			Vector3Copy(new_out, out);
// 		}
// 	}
// }
