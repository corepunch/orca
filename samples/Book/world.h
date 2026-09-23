#ifndef THREE_STARS_WORLD_H
#define THREE_STARS_WORLD_H

#include <stddef.h>

typedef enum {
    ROOM_FLOOR,
    ROOM_TABLE,
    ROOM_SILL,
    ROOM_COUNT
} room_id_t;

typedef enum {
    ITEM_GOLD_STAR,
    ITEM_COPPER_STAR,
    ITEM_PEARL_STAR,
    ITEM_COUNT
} item_id_t;

typedef enum {
    ACTION_TAKE,
    ACTION_MOVE
} action_kind_t;

typedef struct {
    const char *name;
    const char *description;
    const char *after_take_description;
    const char *camera;
    const char *after_take_camera;
    item_id_t item;
} room_t;

typedef struct {
    const char *name;
    room_id_t room;
} item_t;

typedef struct {
    room_id_t room;
    action_kind_t kind;
    int target;
    const char *label;
    const char *camera;
    const char *text;
} action_t;

extern const room_t rooms[ROOM_COUNT];
extern const item_t items[ITEM_COUNT];
extern const action_t actions[];
extern const size_t action_count;
extern const char *ending_camera;
extern const char *ending_text;

#endif
