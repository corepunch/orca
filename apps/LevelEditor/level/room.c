#include "types.h"

static int Block_FloorEqualsTo(block_t const *block, int value)
{
	FOR_LOOP(idx, NUM_CORNERS)
	{
		if (block->height[kBlockLayerFloor][idx] != value)
			return 0;
	}
	return 1;
}

static int Block_CeilingEqualsTo(block_t const *block, int value)
{
	FOR_LOOP(idx, NUM_CORNERS)
	{
		if (block->height[kBlockLayerCeiling][idx] != value)
			return 0;
	}
	return 1;
}

static int Room_CheckTargetRoom(room_t const *room, portal_t const *portal, int x, int y)
{
	room_t const *target = portal->targetRoom;
	int const pos[2] = {
			room->location[0] - target->location[0],
			room->location[1] - target->location[1]};
	region2int8_t const region = {
			portal->region[0] + pos[0],
			portal->region[1] + pos[1],
			portal->region[2] + pos[0],
			portal->region[3] + pos[1],
	};
	vec2int8_t vbounds;
	block_t const *block = Room_NavigateToCBlockAt(target, pos[0] + x, pos[1] + y);
	if (!block || block->type != kBlockTypeFloor)
		return 0;
	Room_GetVerticalBounds(target, region, vbounds);
	switch (portal->direction)
	{
	case kPortalDirectionFloor:
		return Block_CeilingEqualsTo(block, vbounds[1]);
	case kPortalDirectionCeiling:
		return Block_FloorEqualsTo(block, vbounds[0]);
	default:
		return 1;
	}
}

void Room_Init(room_t *room, coord_t floor, coord_t ceiling)
{
	FOR_ALL_TILES()
	{
		int border = x == 0 || y == 0 || x == (ROOM_SIZE - 1) || y == (ROOM_SIZE - 1);
		block_t *block = &room->blocks[x][y];
		uvquad_t uvquad = {{250,250},{254,250},{254,254},{250,254}};
		block->type = border ? kBlockTypeBox : kBlockTypeFloor;
		FOR_LOOP(c, NUM_CORNERS)
		{
			block->height[kBlockLayerFloor][c] = floor;
			block->height[kBlockLayerEd][c] = floor;
			block->height[kBlockLayerCeiling][c] = ceiling;
			block->height[kBlockLayerRf][c] = ceiling;
		}
		FOR_LOOP(c, kBlockFaceCount)
		{
			memcpy(block->uv[c], uvquad, sizeof(uvquad_t));
		}
	}
	dsurface_t dsurf = Room_AllocateModel(room);
	memset(room->selection, 255, sizeof(room->selection));
	room->lightmap = Room_AllocateLightmap(&dsurf);
	room->model = gi->renderer->AllocateModel(&dsurf);
	room->map = Room_AllocateTexture(room);
}

void Room_Release(room_t *room)
{
	gi->renderer->ReleaseModel(room->model);
	gi->renderer->ReleaseTexture(room->map);
	gi->renderer->ReleaseTexture(room->lightmap);
}

portal_t const *Room_GetPortalAt(room_t const *room, int x, int y)
{
	FOR_LOOP(idx, room->numPortals)
	{
		if (Box2_Contains(room->portals[idx].region, x, y))
		{
			return &room->portals[idx];
		}
	}
	return NULL;
}

block_t *Room_GetPortalBlockAt(room_t *from, room_t *to, int x, int y)
{
	x += from->location[0] - to->location[0];
	y += from->location[1] - to->location[1];
	if (x < 0 || y < 0 || x >= ROOM_SIZE || y >= ROOM_SIZE)
		return NULL;
	return &to->blocks[x][y];
}

block_t *Room_GetBlockAt(room_t *room, int x, int y)
{
	if (x < 0 || y < 0 || x >= ROOM_SIZE || y >= ROOM_SIZE)
		return NULL;
	return &room->blocks[x][y];
}

block_t const *Room_GetCBlockAt(room_t const *room, int x, int y)
{
	return Room_GetBlockAt((room_t *)room, x, y);
}

block_t *Room_NavigateToBlockAt(room_t *room, int x, int y)
{
	if (x < 0 || y < 0 || x >= ROOM_SIZE || y >= ROOM_SIZE)
		return NULL;
	block_t *block = &room->blocks[x][y];
	if (block->type != kBlockTypeFloor)
	{
		portal_t const *portal = Room_GetPortalAt(room, x, y);
		if (!portal || portal->direction == kPortalDirectionFloor || portal->direction == kPortalDirectionCeiling)
		{
			return block;
		}
		else
		{
			return Room_GetPortalBlockAt(room, portal->targetRoom, x, y);
		}
	}
	else
	{
		return block;
	}
}

block_t const *Room_NavigateToCBlockAt(room_t const *room, int x, int y)
{
	return Room_NavigateToBlockAt((room_t *)room, x, y);
}

int Box2_Contains(region2int8_t const s, int x, int y)
{
	return x >= MIN(s[0], s[2]) && y >= MIN(s[1], s[3]) && x <= MAX(s[0], s[2]) && y <= MAX(s[1], s[3]);
}

int Box2_AlmostContains(region2int8_t const s, int x, int y)
{
	region2int8_t const e = {MIN(s[0], s[2]) - 1, MIN(s[1], s[3]) - 1, MAX(s[0], s[2]) + 1, MAX(s[1], s[3]) + 1};
	return !Box2_Contains(s, x, y) && x >= e[0] && y >= e[1] && x <= e[2] && y <= e[3];
}

static const int vertexmask[] = {12, 4, 6, 2, 3, 1, 9, 8};

static void changeBlockHeight(room_t *room, region2int8_t const region, int value, int smooth, int direction, int valueptr)
{
	int mask = direction > 0 ? vertexmask[(direction - 1) % 8] : ~0;
	FOR_ALL_TILES()
	{
		block_t *block = Room_NavigateToBlockAt(room, x, y);
		coord_t *out = ((void *)block) + valueptr;
		if (Box2_Contains(region, x, y))
		{
			if (mask & 1)
				out[0] += value;
			if (mask & 2)
				out[1] += value;
			if (mask & 4)
				out[2] += value;
			if (mask & 8)
				out[3] += value;
		}
		else if (!smooth)
		{
			continue;
		}
		else if (Box2_Contains(region, x + 1, y))
		{
			if (mask & 1)
				out[1] += value;
			if (mask & 8)
				out[2] += value;
		}
		else if (Box2_Contains(region, x - 1, y))
		{
			if (mask & 2)
				out[0] += value;
			if (mask & 4)
				out[3] += value;
		}
		else if (Box2_Contains(region, x, y + 1))
		{
			if (mask & 2)
				out[2] += value;
			if (mask & 1)
				out[3] += value;
		}
		else if (Box2_Contains(region, x, y - 1))
		{
			if (mask & 8)
				out[0] += value;
			if (mask & 4)
				out[1] += value;
		}
		else if (Box2_Contains(region, x - 1, y - 1))
		{
			if (mask & 4)
				out[0] += value;
		}
		else if (Box2_Contains(region, x + 1, y - 1))
		{
			if (mask & 8)
				out[1] += value;
		}
		else if (Box2_Contains(region, x + 1, y + 1))
		{
			if (mask & 1)
				out[2] += value;
		}
		else if (Box2_Contains(region, x - 1, y + 1))
		{
			if (mask & 2)
				out[3] += value;
		}
	}
}

void Room_ChangeHeight(room_t *room, region2int8_t const region, blockLayer_t vertical, int value, int smooth, int direction)
{
	changeBlockHeight(room, region, value, smooth, direction, (long)((block_t *)NULL)->height[vertical]);
}

void Room_SetBlockType(room_t *room, region2int8_t const region, blockType_t type)
{
	FOR_ALL_TILES()
	{
		if (Box2_Contains(region, x, y))
		{
			Room_NavigateToBlockAt(room, x, y)->type = type;
		}
	}
}

image_t *Room_AllocateTexture(room_t const *room)
{
	static color_t roomBitmap[ROOM_SIZE][ROOM_SIZE];
	memset(roomBitmap, 255, sizeof(roomBitmap));
	FOR_TILES(0, 0, ROOM_SIZE, ROOM_SIZE)
	{
		if (Box2_Contains(room->selection, x, y))
		{
			roomBitmap[y][x] = GetSelectedBlockColor();
		}
		else if (Room_GetPortalAt(room, x, y))
		{
			roomBitmap[y][x] = GetPortalBlockColor();
		}
		else
		{
			block_t const *block = Room_NavigateToCBlockAt(room, x, y);
			roomBitmap[y][x] = GetBlockTypeColor(block->type);
		}
	}
	return gi->renderer->AllocateTexture(ROOM_SIZE, ROOM_SIZE, (color_t const *)roomBitmap);
}

void Room_Crop(room_t *room, region2int8_t const region)
{
	FOR_ALL_TILES()
	{
		block_t *block = Room_NavigateToBlockAt(room, x, y);
		if (Box2_Contains(region, x, y))
		{
			block->type = kBlockTypeFloor;
		}
		else if (Box2_AlmostContains(region, x, y))
		{
			block->type = kBlockTypeBox;
		}
		else
		{
			block->type = kBlockTypeNone;
		}
	}
}

void Room_GetBounds(room_t const *room, region2int8_t out)
{
	out[0] = ROOM_SIZE;
	out[1] = ROOM_SIZE;
	out[2] = 0;
	out[3] = 0;
	FOR_ALL_TILES()
	{
		block_t const *block = Room_NavigateToCBlockAt(room, x, y);
		if (block->type == kBlockTypeNone)
			continue;
		out[0] = MIN(out[0], x);
		out[1] = MIN(out[1], y);
		out[2] = MAX(out[2], x);
		out[3] = MAX(out[3], y);
	}
}

void Room_AddPortal(room_t *room, portal_t const *portal)
{
	room->portals[room->numPortals++] = *portal;
}

void Room_GetVerticalBounds(room_t const *room, region2int8_t const region, vec2int8_t out)
{
	out[0] = 127;
	out[1] = -127;
	FOR_TILES(region[0], region[1], region[2] + 1, region[3] + 1)
	{
		block_t const *block = Room_GetCBlockAt(room, x, y);
		FOR_LOOP(corner, NUM_CORNERS)
		{
			out[0] = MIN(out[0], block->height[kBlockLayerFloor][corner]);
			out[1] = MAX(out[1], block->height[kBlockLayerCeiling][corner]);
		}
	}
}

portal_t const *Room_GetPortalOfTypeAt(room_t const *room, portalDirection_t dir, int x, int y)
{
	FOR_LOOP(idx, room->numPortals)
	{
		portal_t const *p = &room->portals[idx];
		coord_t const *a = p->region;
		if (p->direction == dir && a[0] <= x && a[1] <= y && a[2] >= x && a[3] >= y)
		{
			vec2int8_t vbounds;
			block_t const *block = Room_NavigateToCBlockAt(room, x, y);
			Room_GetVerticalBounds(room, a, vbounds);
			switch (dir)
			{
			case kPortalDirectionFloor:
				if (block->type == kBlockTypeFloor &&
						Block_FloorEqualsTo(block, vbounds[0]) &&
						Room_CheckTargetRoom(room, p, x, y))
				{
					return p;
				}
				else
				{
					return NULL;
				}
			case kPortalDirectionCeiling:
				if (block->type == kBlockTypeFloor &&
						Block_CeilingEqualsTo(block, vbounds[1]) &&
						Room_CheckTargetRoom(room, p, x, y))
				{
					return p;
				}
				else
				{
					return NULL;
				}
			default:
				return p;
			}
		}
	}
	return NULL;
}

object_t *Room_GetObjectAtIndex(room_t *room, int index)
{
	if (index < room->numObjects)
	{
		return &room->objects[index];
	}
	else
	{
		return NULL;
	}
}

object_t *Room_AddObject(room_t *room, object_t const *object)
{
	object_t *dest = &room->objects[room->numObjects++];
	memcpy(dest, object, sizeof(object_t));
	return dest;
}
