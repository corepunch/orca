<?php foreach ($model->getStructs() as $name => $struct):?>

void luaX_push<?= $name ?>(lua_State *L, lpc<?= $name ?>_t data) {
if (data == NULL) { lua_pushnil(L); return; }
lp<?= $name ?>_t self = lua_newuserdata(L, sizeof(struct <?= $name ?>));
luaL_setmetatable(L, "<?= $struct->export ?>");
memcpy(self, data, sizeof(struct <?= $name ?>));
}
lp<?= $name ?>_t luaX_check<?= $name ?>(lua_State *L, int idx) {
return luaL_checkudata(L, idx, "<?= $struct->export ?>");
}
static int f_new_<?= $name ?>(lua_State *L) {
lp<?= $name ?>_t self = lua_newuserdata(L, sizeof(struct <?= $name ?>));
luaL_setmetatable(L, "<?= $struct->export ?>");
memset(self, 0, sizeof(struct <?= $name ?>));
if (lua_gettop(L) == 1) return 1;
if (lua_istable(L, 1)) {
<?php foreach ($struct->getFields() as $field => $type):?>
lua_pop(L, (lua_getfield(L, 1, "<?= $field ?>"), self-><?= $field ?> = <?= $type->getImporter(-1) ?>, 1));
<?php endforeach ?>
} else {
<?php $index = 1 ?>
<?php foreach ($struct->getFields() as $field => $type):?>
self-><?= $field ?> = <?= $type->getImporter($index) ?>;
<?php $index = $index + 1 ?>
<?php endforeach ?>
}
return 1;
}
static int f_<?= $name ?>___call(lua_State *L) {
return (lua_remove(L, 1), f_new_<?= $name ?>(L));  // remove <?= $name ?> from stack
}
int f_<?= $name ?>___index(lua_State *L) {
lp<?= $name ?>_t self = luaX_check<?= $name ?>(L, 1);
switch(self?fnv1a32(luaL_checkstring(L, 2)):0) { // Check is not needed but to silence unused variable warning
<?php foreach ($struct->getFields() as $field => $type):?>
case <?= $field->id ?>: <?= $type->getExporter("self->$field") ?>; return 1; // <?= $field ?>
<?php endforeach ?>
}
return luaL_error(L, "Unknown field in <?= $name ?>: %s", luaL_checkstring(L, 2));
}
int f_<?= $name ?>___newindex(lua_State *L) {
lp<?= $name ?>_t self = luaX_check<?= $name ?>(L, 1);
switch(self?fnv1a32(luaL_checkstring(L, 2)):0) { // Check is not needed but to silence unused variable warning
<?php foreach ($struct->getFields() as $field => $type):?>
case <?= $field->id ?>: self-><?= $field ?> = <?= $type->getImporter(3) ?>; return 0; // <?= $field ?>
<?php endforeach ?>
}
return luaL_error(L, "Unknown field in <?= $name ?>: %s", luaL_checkstring(L, 2));
}
int luaopen_<?= $model->getNamespace() ?>_<?= $name ?>(lua_State *L) {
luaL_newmetatable(L, "<?= $struct->export ?>");
luaL_setfuncs(L, ((luaL_Reg[]) {
{ "new", f_new_<?= $name ?> },
{ "fromstring", f_fromstring_<?= $name ?> },
{ "__newindex", f_<?= $name ?>___newindex },
{ "__index", f_<?= $name ?>___index },
{ NULL, NULL },
}), 0);
// Make <?= $name ?> creatable via constructor-like syntax
lua_newtable(L);
lua_pushcfunction(L, f_<?= $name ?>___call);
lua_setfield(L, -2, "__call");
lua_setmetatable(L, -2);
return 1;
}
<?php endforeach ?>
