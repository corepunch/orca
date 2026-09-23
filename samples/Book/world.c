#include "world.h"

const room_t rooms[ROOM_COUNT] = {
    [ROOM_FLOOR] = {
        "Чердак · пол",
        "Сквозняк сорвал три звёздочки с бумажного созвездия Миры. "
        "У гнезда лежит латунная звёздочка. К столу ведут ступени из книг.",
        "Латунная звёздочка уже у Миры. Над гнездом ждёт бумажное созвездие, "
        "а к столу ведут ступени из книг.",
        "floor-show-room",
        "floor-show-cleared-room",
        ITEM_GOLD_STAR,
    },
    [ROOM_TABLE] = {
        "Чердак · письменный стол",
        "Между листами и карандашом поблёскивает медная звёздочка. "
        "Сложенные открытки ведут выше, к подоконнику.",
        "Мира уже забрала медную звёздочку. Среди листов остался карандаш, "
        "а открытки всё ещё ведут к подоконнику.",
        "table-show-desk",
        "table-show-cleared-desk",
        ITEM_COPPER_STAR,
    },
    [ROOM_SILL] = {
        "Чердак · подоконник",
        "У оконной рамы Мира замечает светлую звёздочку. "
        "За окном темно, а внизу виден письменный стол.",
        "У оконной рамы больше нет звёздочки. За окном темно, "
        "а внизу виден письменный стол.",
        "sill-show-window",
        "sill-show-window",
        ITEM_PEARL_STAR,
    },
};

const item_t items[ITEM_COUNT] = {
    [ITEM_GOLD_STAR] = { "латунная звёздочка", ROOM_FLOOR },
    [ITEM_COPPER_STAR] = { "медная звёздочка", ROOM_TABLE },
    [ITEM_PEARL_STAR] = { "светлая звёздочка", ROOM_SILL },
};

const action_t actions[] = {
    { ROOM_FLOOR, ACTION_TAKE, ITEM_GOLD_STAR, "Поднять латунную звёздочку",
      "floor-take-gold-star", "Мира бережно поднимает латунную звёздочку у своего гнезда." },
    { ROOM_FLOOR, ACTION_MOVE, ROOM_TABLE, "Взобраться на стол по книгам",
      "floor-climb-table", "Мира перебирается по книжным ступеням на столешницу." },
    { ROOM_TABLE, ACTION_TAKE, ITEM_COPPER_STAR, "Поднять медную звёздочку",
      "table-take-copper-star", "Мира находит медную звёздочку между листами." },
    { ROOM_TABLE, ACTION_MOVE, ROOM_FLOOR, "Спуститься на пол по книгам",
      "table-go-floor", "Мира осторожно спускается по книгам на пол." },
    { ROOM_TABLE, ACTION_MOVE, ROOM_SILL, "Подняться к окну по открыткам",
      "table-climb-sill", "Мира взбирается по открыткам на подоконник." },
    { ROOM_SILL, ACTION_TAKE, ITEM_PEARL_STAR, "Поднять светлую звёздочку",
      "sill-take-pearl-star", "Мира поднимает последнюю звёздочку у оконной рамы." },
    { ROOM_SILL, ACTION_MOVE, ROOM_TABLE, "Спуститься на стол по открыткам",
      "sill-go-table", "Мира спускается по открыткам обратно на стол." },
};

const size_t action_count = sizeof(actions) / sizeof(actions[0]);
const char *ending_camera = "attic-return-stars";
const char *ending_text =
    "Мира возвращает все три звёздочки на бумажное созвездие. "
    "Чердак снова становится тихим, а над гнездом сияет маленькая карта неба.";
