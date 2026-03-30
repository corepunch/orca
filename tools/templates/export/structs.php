#define FIELD(IDENT, STRUCT, NAME, TYPE, ...) { \
.Name = #NAME, \
.ShortIdentifier = IDENT, \
.DataType = TYPE, \
.Offset = offsetof(struct STRUCT, NAME), \
.DataSize = sizeof(((struct STRUCT*)NULL)->NAME), \
##__VA_ARGS__ \
}
#define STRUCT(NAME, EXPORT, ...) \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
if (data == NULL) { lua_pushnil(L); return; } \
struct NAME* self = lua_newuserdata(L, sizeof(struct NAME)); \
luaL_setmetatable(L, EXPORT); \
memcpy(self, data, sizeof(struct NAME)); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { \
return luaL_checkudata(L, idx, EXPORT); \
} \
static int f_new_##NAME(lua_State *L) { \
struct NAME* self = lua_newuserdata(L, sizeof(struct NAME)); \
luaL_setmetatable(L, EXPORT); \
memset(self, 0, sizeof(struct NAME)); \
if (lua_gettop(L) == 1) return 1; \
luaX_struct_new(L, self, _##NAME, sizeof(_##NAME)/sizeof(*_##NAME)); \
return 1; \
} \
int f_##NAME##___index(lua_State *L) { \
struct NAME* self = luaX_check##NAME(L, 1); \
if (luaX_struct_index(L, self, _##NAME, sizeof(_##NAME)/sizeof(*_##NAME))) return 1; \
lua_getmetatable(L, 1); \
lua_pushvalue(L, 2); \
lua_rawget(L, -2); \
return 1; \
} \
int f_##NAME##___newindex(lua_State *L) { \
return luaX_struct_newindex(L, luaX_check##NAME(L, 1), _##NAME, sizeof(_##NAME)/sizeof(*_##NAME)); \
} \
static int f_##NAME##___call(lua_State *L) { \
return ((void)lua_remove(L, 1), f_new_##NAME(L)); \
} \
int luaopen_orca_##NAME(lua_State *L) { \
luaL_newmetatable(L, EXPORT); \
luaL_setfuncs(L, ((luaL_Reg[]) { \
{ "new", f_new_##NAME }, \
{ "__newindex", f_##NAME##___newindex }, \
{ "__index", f_##NAME##___index }, \
__VA_ARGS__ \
{ NULL, NULL }, \
}), 0); \
lua_newtable(L); \
lua_pushcfunction(L, f_##NAME##___call); \
lua_setfield(L, -2, "__call"); \
lua_setmetatable(L, -2); \
return 1; \
}
<?php foreach ($structs as $name => $struct):?>
static struct PropertyType _<?= $name ?>[] = {
<?php foreach ($struct->getFields() as $field): ?>
<?php if ($field->type->fixed_array) continue; /* fixed arrays (e.g. int foo[3]) require specialised indexing beyond generic PropertyType metadata */ ?>
<?php if ($field->noexport) continue; ?>
<?php
$kind = $field->type->kind;
$extra = "";
if ($kind === "float") {
$datatype = "kDataTypeFloat";
} elseif ($kind === "int" || $kind === "uint" || $kind === "long") {
$datatype = "kDataTypeInt";
} elseif ($kind === "bool") {
$datatype = "kDataTypeBool";
} elseif ($kind === "string" || $kind === "fixed") {
$datatype = "kDataTypeString";
} elseif ($kind === "enum") {
$datatype = "kDataTypeEnum";
$extra = ", .EnumValues = _" . $field->type->type;
} elseif ($kind === "struct") {
if (strval($field->type->type) === "color") {
$datatype = "kDataTypeColor";
$extra = ", .TypeString = \"Color\"";
} else {
$datatype = "kDataTypeStruct";
$extra = ", .TypeString = \"" . $field->type->export . "\"";
}
} elseif ($kind === "component") {
$datatype = "kDataTypeObject";
$extra = ", .TypeString = \"" . $field->type->export . "\"";
} else {
continue;
}
?>
FIELD(0x<?= hash("fnv1a32", $field->name) ?>, <?= $name ?>, <?= $field->name ?>, <?= $datatype ?><?= $extra ?>),
<?php endforeach ?>
};
<?php endforeach ?>

<?php foreach ($structs as $name => $struct):?>
<?php include_template("export/functions", ['functions' => $struct->getMethods(), 'prefix' => $struct->prefix]) ?>
<?php if ($struct->hasFromString() || count($struct->getConstructors()) > 0):?>
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
<?php foreach ($struct->getConstructors() as $numargs):?>
void <?= $name ?>_Convert<?= $numargs ?>(struct <?= $name ?>*, <?= implode(", ", array_slice(array_values($struct->getParsers()), 0, $numargs)) ?>);
<?php endforeach ?>
static int f_<?= $name ?>___fromstring(lua_State *L) {
<?php foreach ($struct->getParsers() as $field => $type):?>
<?= $type->getContainer() ?> <?= $field ?>;
<?php endforeach ?>
<?php
$format = implode(" ", array_map(fn($v) => $v->get('format'), $struct->getParsers()));
$targets = implode(", ", array_map(fn($k) => $k->pointer, array_keys($struct->getParsers())));
?>
struct <?= $name ?> self = {0};
switch (sscanf(luaL_checkstring(L, 1), "<?= $format ?>", <?= $targets ?>)) {
<?php if ($struct->hasFromString()):?>
case <?= count($struct->getParsers()) ?>:
<?php foreach ($struct->getParsers() as $field => $type):?>
<?= $type->get('convert', $field, "self.".$field->addr) ?>;
<?php endforeach ?>
return (luaX_push<?= $name ?>(L, &self), 1);
<?php endif ?>
<?php foreach ($struct->getConstructors() as $numargs):?>
case <?= $numargs ?>:
<?= $name ?>_Convert<?= $numargs ?>(&self, <?= implode(", ", array_slice(array_keys($struct->getParsers()), 0, $numargs)) ?>);
return (luaX_push<?= $name ?>(L, &self), 1);
<?php endforeach ?>
default:
return luaL_error(L, "Invalid format for <?= $name ?>: %s", luaL_checkstring(L, 1));
}
}
<?php endif ?>
STRUCT(<?= $name ?>, "<?= $struct->export ?>",
<?php foreach ($struct->getMethods() as $method_name => $method):?>
{ "<?= $method->export ?>", f_<?= $struct->prefix.$method_name ?> },
<?php endforeach ?>
<?php if ($struct->hasFromString() || count($struct->getConstructors()) > 0):?>
{ "fromstring", f_<?= $name ?>___fromstring },
<?php endif ?>
)
<?php endforeach ?>
