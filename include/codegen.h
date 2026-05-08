#ifndef ORCA_CODEGEN_H
#define ORCA_CODEGEN_H

/* -----------------------------------------------------------------------
 * Low-level property table entry macros.
 * Prefer the type-specific DECL_* variants below over these.
 * ----------------------------------------------------------------------- */
#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) \
    { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, \
      .FullIdentifier=ID_##CLASS##_##NAME, \
      .Offset=offsetof(struct CLASS, FIELD), \
      .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), \
      .DataType=TYPE, ##__VA_ARGS__ }

#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) \
    { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, \
      .FullIdentifier=ID_##CLASS##_##NAME, \
      .Offset=offsetof(struct CLASS, FIELD), \
      .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), \
      .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }

/* -----------------------------------------------------------------------
 * Type-specific DECL helpers — encode the data type in the macro name so
 * property tables are self-documenting without a kDataType* lookup.
 *
 * Scalar types (no extra arguments):
 *   DECL_BOOL   / ARRAY_DECL_BOOL
 *   DECL_INT    / ARRAY_DECL_INT
 *   DECL_FLOAT  / ARRAY_DECL_FLOAT
 *   DECL_STRING / ARRAY_DECL_STRING
 *   DECL_COLOR  / ARRAY_DECL_COLOR
 *
 * Typed variants (require an extra type-name argument):
 *   DECL_ENUM(SHORT, CLASS, NAME, FIELD, EnumType)
 *   DECL_STRUCT_TYPE(SHORT, CLASS, NAME, FIELD, StructType)
 *   DECL_OBJECT(SHORT, CLASS, NAME, FIELD, ComponentType)
 *   ARRAY_DECL_STRUCT_TYPE(SHORT, CLASS, NAME, FIELD, StructType)
 *   ARRAY_DECL_OBJECT(SHORT, CLASS, NAME, FIELD, ComponentType)
 *
 * Event slot in a component property table:
 *   DECL_EVENT(SHORT, CLASS, EventName)
 * ----------------------------------------------------------------------- */

#define DECL_BOOL(SHORT, CLASS, NAME, FIELD) \
    DECL(SHORT, CLASS, NAME, FIELD, kDataTypeBool)
#define DECL_INT(SHORT, CLASS, NAME, FIELD) \
    DECL(SHORT, CLASS, NAME, FIELD, kDataTypeInt)
#define DECL_FLOAT(SHORT, CLASS, NAME, FIELD) \
    DECL(SHORT, CLASS, NAME, FIELD, kDataTypeFloat)
#define DECL_STRING(SHORT, CLASS, NAME, FIELD) \
    DECL(SHORT, CLASS, NAME, FIELD, kDataTypeString)
#define DECL_COLOR(SHORT, CLASS, NAME, FIELD) \
    DECL(SHORT, CLASS, NAME, FIELD, kDataTypeColor)
#define DECL_ENUM(SHORT, CLASS, NAME, FIELD, ENUMTYPE) \
    DECL(SHORT, CLASS, NAME, FIELD, kDataTypeEnum, .EnumValues = _##ENUMTYPE)
#define DECL_STRUCT_TYPE(SHORT, CLASS, NAME, FIELD, TYPENAME) \
    DECL(SHORT, CLASS, NAME, FIELD, kDataTypeStruct, .TypeString = #TYPENAME)
#define DECL_OBJECT(SHORT, CLASS, NAME, FIELD, TYPENAME) \
    DECL(SHORT, CLASS, NAME, FIELD, kDataTypeObject, .TypeString = #TYPENAME)
#define DECL_EVENT(SHORT, CLASS, NAME) \
    DECL(SHORT, CLASS, NAME, NAME, kDataTypeEvent, .TypeString = #CLASS "_" #NAME "EventArgs")

#define ARRAY_DECL_BOOL(SHORT, CLASS, NAME, FIELD) \
    ARRAY_DECL(SHORT, CLASS, NAME, FIELD, kDataTypeBool)
#define ARRAY_DECL_INT(SHORT, CLASS, NAME, FIELD) \
    ARRAY_DECL(SHORT, CLASS, NAME, FIELD, kDataTypeInt)
#define ARRAY_DECL_FLOAT(SHORT, CLASS, NAME, FIELD) \
    ARRAY_DECL(SHORT, CLASS, NAME, FIELD, kDataTypeFloat)
#define ARRAY_DECL_STRING(SHORT, CLASS, NAME, FIELD) \
    ARRAY_DECL(SHORT, CLASS, NAME, FIELD, kDataTypeString)
#define ARRAY_DECL_COLOR(SHORT, CLASS, NAME, FIELD) \
    ARRAY_DECL(SHORT, CLASS, NAME, FIELD, kDataTypeColor)
#define ARRAY_DECL_ENUM(SHORT, CLASS, NAME, FIELD, ENUMTYPE) \
    ARRAY_DECL(SHORT, CLASS, NAME, FIELD, kDataTypeEnum, .EnumValues = _##ENUMTYPE)
#define ARRAY_DECL_STRUCT_TYPE(SHORT, CLASS, NAME, FIELD, TYPENAME) \
    ARRAY_DECL(SHORT, CLASS, NAME, FIELD, kDataTypeStruct, .TypeString = #TYPENAME)
#define ARRAY_DECL_OBJECT(SHORT, CLASS, NAME, FIELD, TYPENAME) \
    ARRAY_DECL(SHORT, CLASS, NAME, FIELD, kDataTypeObject, .TypeString = #TYPENAME)

#define ENUM(NAME, ...) \
ORCA_API const char *_##NAME[] = {__VA_ARGS__, NULL}; \
const char *NAME##ToString(enum NAME value) { \
	return (assert(value >= 0 && value < sizeof(_##NAME) / sizeof(*_##NAME) - 1), _##NAME[value]); \
} \
enum NAME luaX_check##NAME(lua_State *L, int idx) { \
	return luaL_checkoption(L, idx, NULL, _##NAME); \
} \
void luaX_push##NAME(lua_State *L, enum NAME value) { \
	lua_pushstring(L, (assert(value >= 0 && value < sizeof(_##NAME) / sizeof(*_##NAME) - 1), _##NAME[value])); \
}

#define STRUCT(NAME, EXPORT) \
static struct StructDesc const _##NAME##_StructDesc = { \
	.StructName = #EXPORT, \
	.Properties = _##NAME, \
	.NumProperties = sizeof(_##NAME) / sizeof(*_##NAME), \
	.StructSize = sizeof(struct NAME), \
}; \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
	if (data == NULL) { lua_pushnil(L); return; } \
	memcpy(lua_newuserdata(L, sizeof(struct NAME)), data, sizeof(struct NAME)); \
	luaL_setmetatable(L, #EXPORT); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { return luaL_checkudata(L, idx, #EXPORT); } \
static int f_new_##NAME(lua_State *L) { \
	struct NAME self; \
	memset(&self, 0, sizeof(struct NAME)); \
	if (lua_islightuserdata(L, 1)) { \
		memcpy(&self, lua_touserdata(L, 1), sizeof(struct NAME)); \
	} else if (lua_istable(L, 1)) \
		for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); lua_pop(L, 1), i++) { \
			if (lua_getfield(L, 1, _##NAME[i].Name)) \
				read_property(L, -1, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); } \
	else for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		read_property(L, i + 1, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); \
	luaX_push##NAME(L, &self); \
	return 1; \
} \
static int f_##NAME##___index(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (write_property(L, &_##NAME[i], ((char*)luaX_check##NAME(L, 1))+_##NAME[i].Offset), 1); \
	for (uint32_t i = 0; i < sizeof(_##NAME##_Methods) / sizeof(*_##NAME##_Methods); i++) { \
		if (_##NAME##_Methods[i].name && strcmp(_##NAME##_Methods[i].name, luaL_checkstring(L, 2)) == 0) { \
			lua_pushcfunction(L, _##NAME##_Methods[i].func); \
			return 1; \
		} \
	} \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___newindex(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (read_property(L, 3, &_##NAME[i], ((char*)luaX_check##NAME(L, 1))+_##NAME[i].Offset), 0); \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___call(lua_State *L) { \
  lua_insert(L, (lua_getfield(L, 1, "new"), 2)); \
  lua_call(L, lua_gettop(L) - 2, 1); \
	return 1; \
} \
int luaopen_orca_##NAME(lua_State *L) { \
	OBJ_RegisterStructDesc(&_##NAME##_StructDesc); \
	luaL_newmetatable(L, #EXPORT); \
	luaL_setfuncs(L, ((luaL_Reg[]) { \
		{ "new", f_new_##NAME }, \
		{ "__newindex", f_##NAME##___newindex }, \
		{ "__index", f_##NAME##___index }, \
		{ NULL, NULL }, \
	}), 0); \
	luaL_setfuncs(L, _##NAME##_Methods, 0); \
	/* Make struct creatable via constructor-like syntax */ \
	lua_newtable(L); \
	lua_pushcfunction(L, f_##NAME##___call); \
	lua_setfield(L, -2, "__call"); \
	lua_setmetatable(L, -2); \
	return 1; \
}

/* -----------------------------------------------------------------------
 * Component property table and defaults — typed declarations whose sizes
 * are statically verified against the generated k##NAME##NumProperties.
 * ----------------------------------------------------------------------- */
#define COMPONENT_PROPERTIES(NAME) \
    static struct PropertyType const NAME##Properties[k##NAME##NumProperties]

#define COMPONENT_DEFAULTS(NAME) \
    static struct NAME NAME##Defaults

/* -----------------------------------------------------------------------
 * Component Proc dispatch — wrap the boilerplate message-switch function.
 * The DISPATCH macro wires a single message case to its HANDLER function.
 *
 * Example:
 *   COMPONENT_PROC_BEGIN(Button)
 *       DISPATCH(Button, Node,   LeftButtonUp)
 *       DISPATCH(Button, Object, Create)
 *   COMPONENT_PROC_END()
 * ----------------------------------------------------------------------- */
#define COMPONENT_PROC_BEGIN(NAME) \
    LRESULT NAME##Proc(struct Object *object, void *cmp, \
                       uint32_t message, wParam_t wparm, lParam_t lparm) { \
        switch (message) {

#define DISPATCH(CLASS, NS, EVENT) \
        case ID_##NS##_##EVENT: return CLASS##_##EVENT(object, cmp, wparm, lparm);

#define COMPONENT_PROC_END() \
        } \
        return FALSE; \
    }

/* -----------------------------------------------------------------------
 * Class descriptor — shared fields extracted to eliminate duplication
 * between REGISTER_CLASS and REGISTER_ATTACH_ONLY_CLASS.
 * ----------------------------------------------------------------------- */
#define CLASS_DESC_COMMON_FIELDS_(NAME, ...) \
    .ClassName     = #NAME,              \
    .DefaultName   = #NAME,              \
    .ContentType   = #NAME,              \
    .Xmlns         = "http://schemas.corepunch.com/orca/2006/xml/presentation", \
    .ParentClasses = { __VA_ARGS__ },    \
    .ClassID       = ID_##NAME,          \
    .ClassSize     = sizeof(struct NAME), \
    .Properties    = NAME##Properties,  \
    .ObjProc       = NAME##Proc,         \
    .Defaults      = &NAME##Defaults,   \
    .NumProperties = k##NAME##NumProperties

#define REGISTER_CLASS(NAME, ...) \
    ORCA_API struct ClassDesc _##NAME = { CLASS_DESC_COMMON_FIELDS_(NAME, __VA_ARGS__) }

#define REGISTER_ATTACH_ONLY_CLASS(NAME, ...) \
    ORCA_API struct ClassDesc _##NAME = { CLASS_DESC_COMMON_FIELDS_(NAME, __VA_ARGS__), .IsAttachOnly = TRUE }

#endif
