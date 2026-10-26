#include "types.h"

#define CLAMP(x, min, max) MAX(min, MIN(max, x))

static const int hack_map[] = { kBlockLayerEd, kBlockLayerFloor, kBlockLayerCeiling, kBlockLayerRf };
static const int hack_vertex_map[] = { 2, 3, 1, 0 };
static const int wall_indices[] = {1, 0, 3, 3, 0, 2 };

static int Is_EqualHorizontally(vec3_t const a, vec3_t const b)
{
	if (fabs(a[0] - b[0]) > EPS)
		return 0;
	if (fabs(a[2] - b[2]) > EPS)
		return 0;
	return 1;
}

static drawVert_t *AddVertex(dsurface_t *dsurf)
{
	return (drawVert_t *)&dsurf->vertices[dsurf->numVertices++];
}

// static color_t GetColorFromVector3(vec3_t const n)
// {
// 	int8_t normal[4] = {
// 			((n[0] + 1.0f) * 0.5f) * 255 - 128,
// 			((n[1] + 1.0f) * 0.5f) * 255 - 128,
// 			((n[2] + 1.0f) * 0.5f) * 255 - 128,
// 			0};
// 	return *((color_t *)&normal);
// }

// static color_t GetNormal(vec3_t const a, vec3_t const b, vec3_t const c)
// {
// 	vec3_t side[2], n;
// 	Vector3Subtract(b, a, side[0]);
// 	Vector3Subtract(c, b, side[1]);
// 	Vector3CrossProduct(side[0], side[1], n);
// 	Vector3Normalize(n);
// 	return GetColorFromVector3(n);
// }

// static color_t GetWallNormal2(drawVert_t const *in[4])
// {
// 	if (Vector3Compare(in[0]->xyz, in[3]->xyz))
// 	{
// 		return GetNormal(in[0]->xyz, in[1]->xyz, in[2]->xyz);
// 	}
// 	else
// 	{
// 		return GetNormal(in[0]->xyz, in[1]->xyz, in[3]->xyz);
// 	}
// }

static color_t GetWallNormal(drawVert_t const *in[4])
{
	color_t normal = {0, 0, 0, 0};
	if (fabs(in[0]->xyz[0] - in[1]->xyz[0]) < EPS)
	{
		normal.r = in[0]->xyz[2] > in[1]->xyz[2] ? 127 : 128;
	}
	else
	{
		normal.b = in[0]->xyz[0] > in[1]->xyz[0] ? 128 : 127;
	}
	return normal;
}

static blockFace_t GetFaceFromNormal(color_t normal)
{
	return kBlockFaceNegativeX;
	if (normal.r > 0)
	{
		return normal.r < 128 ? kBlockFacePositiveX : kBlockFaceNegativeX;
	}
	else
	{
		return normal.b < 128 ? kBlockFacePositiveY : kBlockFaceNegativeY;
	}
}

static color_t GetUserData(color_t normal, drawVert_t const *in[4], int idx)
{
	color_t userdata = in[2]->userdata;

	userdata.r = GetFaceFromNormal(normal);
	userdata.g = in[idx]->userdata.g & 63;

	if (in[0]->userdata.r == kBlockFaceFloor && in[2]->userdata.r == kBlockFaceCeiling)
	{
		int x_indices[] = { 1, 0, 3, 2 };
		int y_indices[] = { 3, 2, 1, 0 };
		if (normal.r > 127)
		{
			userdata.b--;
			userdata.g = x_indices[userdata.g & 63];
		}
		else if (normal.r > 0)
		{
			userdata.b++;
			userdata.g = x_indices[userdata.g & 63];
		}
		else if (normal.b > 127)
		{
			userdata.a--;
			userdata.g = y_indices[userdata.g & 63];
		}
		else
		{
			userdata.a++;
			userdata.g = y_indices[userdata.g & 63];
		}
	}

	return userdata;
}

static block_t
Room_GetVertexBlock(room_t const *room, drawVert_t const *vertex)
{
	const int x = vertex->userdata.b;
	const int y = vertex->userdata.a;
	block_t const *txtr = Room_GetCBlockAt(room, x, y);
	block_t block = *Room_NavigateToCBlockAt(room, x, y);
	memcpy(block.uv, txtr->uv, sizeof(uvquad_t) * kBlockFaceCount);
	return block;
}

static int
Room_GetVertexValue(room_t const *room, drawVert_t const *vertex, int layer)
{
	const int c = vertex->userdata.g & 63;
	block_t const block = Room_GetVertexBlock(room, vertex);
	return block.height[layer][c] * CLICK_SIZE;
}

static int
Room_IsVertexInPortal(room_t const *room, drawVert_t const *vertex)
{
	const int x = vertex->userdata.b;
	const int y = vertex->userdata.a;
	return Room_NavigateToCBlockAt(room, x, y) != &room->blocks[x][y];
}

void AddLightmapQuad(drawVert_t *a, drawVert_t *b, drawVert_t *c, drawVert_t *d, int index);

static void
AddFaceQuad(room_t const *room, dsurface_t *dsurf, drawVert_t **face, int idx)
{
	const int layer = hack_map[idx];
	FOR_LOOP(v, 4)
	{
		int const facenum = GetFaceFromNormal(face[v]->normal) + idx;
		int const vertnum = hack_vertex_map[v];
		block_t const block = Room_GetVertexBlock(room, face[v]);
		face[v]->userdata.r = facenum;
		face[v]->texcoord0[0] = block.uv[facenum][vertnum][0] * CLICK_SCALE_U;
		face[v]->texcoord0[1] = 1 - block.uv[facenum][vertnum][1] * CLICK_SCALE_V;
		if (!CheckTexturing(block.uv[facenum]))
		{
			face[v]->userdata.g |= 64;
		}
		if (layer == kBlockLayerCeiling && Room_GetVertexBlock(room, face[v]).type == kBlockTypeFloor)
		{
			face[v]->color.a = Room_IsVertexInPortal(room, face[v]) ? 0 : ~0;
		}
	}
	AddLightmapQuad(face[0], face[1], face[3], face[2], dsurf->numIndices / 6);
	FOR_LOOP(idx, 6)
	{
		AddIndex(dsurf, face[wall_indices[idx]] - dsurf->vertices);
	}
}

void AddWallSegments(room_t const *room, dsurface_t *dsurf, drawVert_t **out)
{
	FOR_LOOP(idx, 4)
	{
		const int layer = hack_map[idx];
		const int p[] = {
				Room_GetVertexValue(room, out[0], layer),
				Room_GetVertexValue(room, out[1], layer),
		};
		if (p[0] > out[0]->xyz[1] || p[1] > out[1]->xyz[1])
		{
			drawVert_t *vert[4];

			FOR_LOOP(v, 4)
			{
				const vec_t low = out[v & 1]->xyz[1];
				const vec_t high = out[2 + (v & 1)]->xyz[1];
				vert[v] = AddVertex(dsurf);
				memcpy(vert[v], out[v & 1], sizeof(drawVert_t));
				vert[v]->xyz[1] = CLAMP(p[v & 1], low, high);
				vert[v]->color.a = ~0;
			}

			if (out[0]->xyz[1] == vert[0]->xyz[1] &&
					out[1]->xyz[1] == vert[1]->xyz[1])
			{
				continue;
			}

			drawVert_t *face[4] = { out[0], out[1], vert[0], vert[1] };

			AddFaceQuad(room, dsurf, face, idx);

			out[0] = vert[2];
			out[1] = vert[3];
		}
	}

	if (out[0]->xyz[1] < out[2]->xyz[1] || out[1]->xyz[1] < out[3]->xyz[1])
	{
		AddFaceQuad(room, dsurf, out, 4);
	}
}

void Room_FillWalls(room_t const *room, dsurface_t *dsurf)
{
	const int numIndices = dsurf->numIndices;

	for (int t0 = 0; t0 < numIndices; t0++)
	{
		if (dsurf->neighbors[t0] != -1)
			continue;

		const drawVert_t *in[4] =
		{
				GetVertexAtEdge(dsurf, t0, 0),
				GetVertexAtEdge(dsurf, t0, 1),
				NULL,
				NULL,
		};

		drawIndex_t edge = -1;

		for (int t1 = t0 + 1; t1 < numIndices; t1++)
		{
			if (dsurf->neighbors[t1] != -1)
				continue;

			const drawVert_t *other[2] =
			{
				GetVertexAtEdge(dsurf, t1, 1),
				GetVertexAtEdge(dsurf, t1, 0),
			};

			if (Is_EqualHorizontally(in[0]->xyz, other[0]->xyz) &&
					Is_EqualHorizontally(in[1]->xyz, other[1]->xyz) &&
					(edge == -1 || in[2]->userdata.r != in[0]->userdata.r))
			{
				in[2] = other[0];
				in[3] = other[1];
				edge = t1;
			}
		}

		if (edge != -1)
		{
			const int positive = in[0]->xyz[1] > in[2]->xyz[1] || in[1]->xyz[1] > in[3]->xyz[1];
			const drawVert_t *reverse[4] = {in[3], in[2], in[1], in[0]};
			const drawVert_t **ptr = positive ? reverse : in;
			drawVert_t *out[4];

			FOR_LOOP(idx, 4)
			{
				color_t const normal = GetWallNormal(ptr);
				color_t const userdata = GetUserData(normal, ptr, idx);
				out[idx] = AddVertex(dsurf);
				memcpy(out[idx], ptr[idx], sizeof(drawVert_t));
				out[idx]->color.a = ~0;
				out[idx]->normal = normal;
				out[idx]->userdata = userdata;
			}

			AddWallSegments(room, dsurf, out);

			((drawIndex_t *)dsurf->neighbors)[t0] = 0;
			((drawIndex_t *)dsurf->neighbors)[edge] = 0;
		}
	}
}
