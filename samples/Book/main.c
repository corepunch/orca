#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "world.h"

typedef struct {
    room_id_t room;
    unsigned int inventory;
} game_t;

static bool has_item(const game_t *game, item_id_t item) {
    return (game->inventory & (1u << item)) != 0;
}

static bool action_available(const game_t *game, const action_t *action) {
    if (action->room != game->room) return false;
    if (action->kind == ACTION_TAKE && has_item(game, (item_id_t)action->target)) return false;
    return true;
}

static void print_camera(const char *camera) {
    printf("Кадр: illustrations/%s.jpg\n", camera);
}

static void print_inventory(const game_t *game) {
    printf("\nУ Миры: ");
    if (!game->inventory) {
        puts("пока нет звёздочек.");
        return;
    }

    bool first = true;
    for (int i = 0; i < ITEM_COUNT; i++) {
        if (!has_item(game, (item_id_t)i)) continue;
        printf("%s%s", first ? "" : ", ", items[i].name);
        first = false;
    }
    puts(".");
}

static size_t print_scene(const game_t *game, const action_t **choices) {
    const room_t *room = &rooms[game->room];
    bool cleared = has_item(game, room->item);
    printf("\n%s\n%s\n", room->name,
           cleared ? room->after_take_description : room->description);
    print_camera(cleared ? room->after_take_camera : room->camera);

    size_t count = 0;
    for (size_t i = 0; i < action_count; i++) {
        if (!action_available(game, &actions[i])) continue;
        choices[count++] = &actions[i];
        printf("%zu. %s\n", count, actions[i].label);
    }
    puts("i — инвентарь, l — осмотреться, q — выйти");
    return count;
}

static bool read_choice(size_t count, size_t *choice) {
    char input[64];
    if (!fgets(input, sizeof(input), stdin)) return false;
    if (strchr(input, '\n') == NULL && !feof(stdin)) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
    }

    if (input[0] == 'q' || input[0] == 'Q') return false;
    if (input[0] == 'i' || input[0] == 'I') { *choice = count; return true; }
    if (input[0] == 'l' || input[0] == 'L') { *choice = count + 1; return true; }

    char *end;
    long number = strtol(input, &end, 10);
    if (end == input || (*end != '\n' && *end != '\0') || number < 1 || number > (long)count) {
        *choice = count + 2;
        return true;
    }
    *choice = (size_t)number - 1;
    return true;
}

int main(void) {
    game_t game = { .room = ROOM_FLOOR };
    const action_t *choices[action_count];

    puts("Три звёздочки для карты");
    puts("Помоги мышке Мире найти три звёздочки в одной чердачной комнате.");

    for (;;) {
        size_t count = print_scene(&game, choices);
        printf("> ");
        fflush(stdout);

        size_t choice;
        if (!read_choice(count, &choice)) break;
        if (choice == count) { print_inventory(&game); continue; }
        if (choice == count + 1) continue;
        if (choice >= count) { puts("Выбери номер действия из списка."); continue; }

        const action_t *action = choices[choice];
        print_camera(action->camera);
        puts(action->text);
        if (action->kind == ACTION_TAKE) {
            game.inventory |= 1u << action->target;
        } else {
            game.room = (room_id_t)action->target;
        }

        if (game.inventory == (1u << ITEM_COUNT) - 1) {
            puts("\nТри звёздочки для карты · конец");
            print_camera(ending_camera);
            puts(ending_text);
            break;
        }
    }
    return 0;
}
