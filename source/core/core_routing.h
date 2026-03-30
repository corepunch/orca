// Manually-maintained routing API declarations — included from core.h.
// These supplement the auto-generated content with the message routing registry
// and the explicit-routing variant of DispatchEvent.

/// @brief Register message routing strategies for a module's events.
/// Called automatically from each module's luaopen function for events that
/// declare a routing= attribute in their XML definition.
ORCA_API void
OBJ_RegisterMessageRoutings(const char* const names[], enum MessageRouting const routings[], int count);

/// @brief Look up the registered routing strategy for a message by its FNV1a hash.
/// Returns kMessageRoutingBubbling when the message has no registered routing.
ORCA_API enum MessageRouting
OBJ_GetMessageRouting(uint32_t hash);

/// @brief Dispatch an event with an explicit routing strategy, overriding the event definition.
/// Use OBJ_DispatchEvent for the definition-time routing (recommended for Lua code).
ORCA_API struct Object*
OBJ_DispatchEventWithRouting(struct lua_State*, struct Object*, const char*, enum MessageRouting);
