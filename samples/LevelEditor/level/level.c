#include "types.h"
#include <code/common/files.h>

static const int IDLEVELHEADER = MAKE_ID('L', 'E', 'V', 'L');

extern struct game_import *gi;

typedef struct
{
	int ident; // == IDLEVELHEADER
	int num_rooms;
} dlevelheader_t;

typedef struct
{
	int num_portals;
	vec2int8_t location;
} droomheader_t;

typedef struct
{
    int target_room;
    uint8_t direction;
    region2int8_t region;
} dportalheader_t;

void Level_Init(level_t *level)
{
	room_t *room = Level_AddRoom(level);
	Room_Init(room, 0, DEFAULT_CEILING);
}

void Level_Release(level_t *level)
{
	FOR_LOOP(idx, level->numRooms)
	{
		Room_Release(&level->rooms[idx]);
	}
}

room_t *Level_AddRoom(level_t *level)
{
	room_t *room = &level->rooms[level->numRooms++];

	return room;
}

void Level_RemoveRoom(level_t *level, room_t *room)
{
	level->numRooms--;
	for (long idx = room - level->rooms; idx < level->numRooms; idx++)
	{
		level->rooms[idx] = level->rooms[idx + 1];
	}
	Room_Release(room);
}

void Level_Load(level_t *level, char const *filename)
{
	if (level->numRooms != 0)
		return;
	FILE *fp = fopen(filename, "rb");
	if (!fp)
	{
		fprintf(stderr, "Level_Load(): Can't open file %s\n", filename);
		return;
	}
	dlevelheader_t lheader;
	fread(&lheader, sizeof(dlevelheader_t), 1, fp);
	if (lheader.ident != IDLEVELHEADER)
	{
		fprintf(stderr, "Level_Load(): Wrong header in file %s\n", filename);
		return;
	}
	level->numRooms = lheader.num_rooms;
	FOR_LOOP(rid, lheader.num_rooms)
	{
		room_t *room = &level->rooms[rid];
		droomheader_t rhead;
		fread(&rhead, sizeof(droomheader_t), 1, fp);
		fread(room->blocks, sizeof(block_t), NUM_BLOCKS, fp);
		room->numPortals = rhead.num_portals;
		room->location[0] = rhead.location[0];
		room->location[1] = rhead.location[1];
		FOR_LOOP(pid, rhead.num_portals)
		{
			portal_t *portal = &room->portals[pid];
			dportalheader_t phead;
			fread(&phead, sizeof(dportalheader_t), 1, fp);
			portal->targetRoom = &level->rooms[phead.target_room];
			portal->direction = phead.direction;
			portal->region[0] = phead.region[0];
			portal->region[1] = phead.region[1];
			portal->region[2] = phead.region[2];
			portal->region[3] = phead.region[3];
		}
	}
	FOR_LOOP(rid, lheader.num_rooms)
	{
		room_t *room = &level->rooms[rid];
    dsurface_t dsurf = Room_AllocateModel(room);
		room->model = gi->renderer->AllocateModel(&dsurf);
		room->map = Room_AllocateTexture(room);
		room->lightmap = Room_AllocateLightmap(&dsurf);
	}
	fclose(fp);
}

void Level_Save(level_t const *level, const char *filename)
{
		FILE *fp = fopen(filename, "wb");
		if (!fp)
		{
			fprintf(stderr, "Level_Save(): Can't open file %s\n", filename);
			return;
		}
		dlevelheader_t lheader = { IDLEVELHEADER, level->numRooms };
		fwrite(&lheader, sizeof(dlevelheader_t), 1, fp);
		FOR_LOOP(rid, lheader.num_rooms)
		{
			room_t const *room = &level->rooms[rid];
			droomheader_t rhead;
			rhead.num_portals = room->numPortals;
			rhead.location[0] = room->location[0];
			rhead.location[1] = room->location[1];
			fwrite(&rhead, sizeof(droomheader_t), 1, fp);
			// FOR_LOOP(idx, NUM_BLOCKS) {
			// 	block_t const *b = ((block_t *)room->blocks) + idx;
			// 	fwrite(b, sizeof(block_t), 1, fp);
			// }
			fwrite(room->blocks, sizeof(block_t), NUM_BLOCKS, fp);
			FOR_LOOP(pid, rhead.num_portals)
			{
				portal_t const *portal = &room->portals[pid];
				dportalheader_t phead;
				phead.target_room = portal->targetRoom - level->rooms;
				phead.direction = portal->direction;
				phead.region[0] = portal->region[0];
				phead.region[1] = portal->region[1];
				phead.region[2] = portal->region[2];
				phead.region[3] = portal->region[3];
				fwrite(&phead, sizeof(dportalheader_t), 1, fp);
			}
		}
		fclose(fp);
}