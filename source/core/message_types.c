#include "core_local.h"

#define MAX_MESSAGE_TABLES 32

static struct {
    lpcMessageType_t const* tables[MAX_MESSAGE_TABLES];
    uint32_t counts[MAX_MESSAGE_TABLES];
    uint32_t num_tables;
} g_msg;

ORCA_API void
OBJ_RegisterMessageTypes(lpcMessageType_t const* table, uint32_t count)
{
    if (g_msg.num_tables >= MAX_MESSAGE_TABLES) {
        Con_Warning("OBJ_RegisterMessageTypes: table limit reached");
        return;
    }
    g_msg.tables[g_msg.num_tables] = table;
    g_msg.counts[g_msg.num_tables] = count;
    g_msg.num_tables++;
}

ORCA_API lpcMessageType_t
OBJ_FindMessageType(uint32_t id)
{
    for (uint32_t t = 0; t < g_msg.num_tables; t++) {
        lpcMessageType_t const* table = g_msg.tables[t];
        uint32_t count = g_msg.counts[t];
        for (uint32_t i = 0; i < count; i++) {
            if (table[i]->id == id) {
                return table[i];
            }
        }
    }
    return NULL;
}
