#include "world.h"

const room_t rooms[ROOM_COUNT] = {
    [ROOM_FLOOR] = {
        .name = "Чердак · пол",
        .description = "Сквозняк сорвал три звёздочки с бумажного созвездия Миры. "
                       "У гнезда лежит латунная звёздочка. К столу ведут ступени из книг.",
        .after_take_description = "Латунная звёздочка уже у Миры. Над гнездом ждёт "
                                  "бумажное созвездие, а к столу ведут ступени из книг.",
        .camera = "floor-show-room",
        .after_take_camera = "floor-show-cleared-room",
        .item = ITEM_GOLD_STAR,
    },
    [ROOM_TABLE] = {
        .name = "Чердак · письменный стол",
        .description = "Между листами и карандашом поблёскивает медная звёздочка. "
                       "Сложенные открытки ведут выше, к подоконнику.",
        .after_take_description = "Мира уже забрала медную звёздочку. Среди листов "
                                  "остался карандаш, а открытки всё ещё ведут к подоконнику.",
        .camera = "table-show-desk",
        .after_take_camera = "table-show-cleared-desk",
        .item = ITEM_COPPER_STAR,
    },
    [ROOM_SILL] = {
        .name = "Чердак · подоконник",
        .description = "У оконной рамы Мира замечает светлую звёздочку. "
                       "За окном темно, а внизу виден письменный стол.",
        .after_take_description = "У оконной рамы больше нет звёздочки. За окном "
                                  "темно, а внизу виден письменный стол.",
        .camera = "sill-show-window",
        .after_take_camera = "sill-show-window",
        .item = ITEM_PEARL_STAR,
    },
};

const item_t items[ITEM_COUNT] = {
    [ITEM_GOLD_STAR] = { .name = "латунная звёздочка", .room = ROOM_FLOOR },
    [ITEM_COPPER_STAR] = { .name = "медная звёздочка", .room = ROOM_TABLE },
    [ITEM_PEARL_STAR] = { .name = "светлая звёздочка", .room = ROOM_SILL },
};

const action_t actions[] = {
    {
        .room = ROOM_FLOOR, .kind = ACTION_TAKE, .target = ITEM_GOLD_STAR,
        .label = "Поднять латунную звёздочку",
        .camera = "floor-take-gold-star",
        .text = "Мира бережно поднимает латунную звёздочку у своего гнезда.",
    },
    {
        .room = ROOM_FLOOR, .kind = ACTION_MOVE, .target = ROOM_TABLE,
        .label = "Взобраться на стол по книгам",
        .camera = "floor-climb-table",
        .text = "Мира перебирается по книжным ступеням на столешницу.",
    },
    {
        .room = ROOM_TABLE, .kind = ACTION_TAKE, .target = ITEM_COPPER_STAR,
        .label = "Поднять медную звёздочку",
        .camera = "table-take-copper-star",
        .text = "Мира находит медную звёздочку между листами.",
    },
    {
        .room = ROOM_TABLE, .kind = ACTION_MOVE, .target = ROOM_FLOOR,
        .label = "Спуститься на пол по книгам",
        .camera = "table-go-floor",
        .text = "Мира осторожно спускается по книгам на пол.",
    },
    {
        .room = ROOM_TABLE, .kind = ACTION_MOVE, .target = ROOM_SILL,
        .label = "Подняться к окну по открыткам",
        .camera = "table-climb-sill",
        .text = "Мира взбирается по открыткам на подоконник.",
    },
    {
        .room = ROOM_SILL, .kind = ACTION_TAKE, .target = ITEM_PEARL_STAR,
        .label = "Поднять светлую звёздочку",
        .camera = "sill-take-pearl-star",
        .text = "Мира поднимает последнюю звёздочку у оконной рамы.",
    },
    {
        .room = ROOM_SILL, .kind = ACTION_MOVE, .target = ROOM_TABLE,
        .label = "Спуститься на стол по открыткам",
        .camera = "sill-go-table",
        .text = "Мира спускается по открыткам обратно на стол.",
    },
};

const size_t action_count = sizeof(actions) / sizeof(actions[0]);
const char *ending_camera = "attic-return-stars";
const char *ending_text =
    "Мира возвращает все три звёздочки на бумажное созвездие. "
    "Чердак снова становится тихим, а над гнездом сияет маленькая карта неба.";
