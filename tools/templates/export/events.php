<?php foreach ($events as $name => $event):?>
<?php if (!$event->hasFields()): continue; endif ?>
<?php $struct_name = $name . "MsgArgs" ?>
void luaX_push<?= $struct_name ?>(lua_State *L, struct <?= $struct_name ?> const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct <?= $struct_name ?>* self = lua_newuserdata(L, sizeof(struct <?= $struct_name ?>));
	luaL_setmetatable(L, "<?= $struct_name ?>");
	memcpy(self, data, sizeof(struct <?= $struct_name ?>));
}
struct <?= $struct_name ?>* luaX_check<?= $struct_name ?>(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "<?= $struct_name ?>");
}
static int f_new_<?= $struct_name ?>(lua_State *L) {
	struct <?= $struct_name ?>* self = lua_newuserdata(L, sizeof(struct <?= $struct_name ?>));
	luaL_setmetatable(L, "<?= $struct_name ?>");
	memset(self, 0, sizeof(struct <?= $struct_name ?>));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		<?php foreach ($event->getAllFields() as $field):?>
			<?php if ($field->type->fixed_array) continue; ?>
			<?php if ($field->noexport) continue; ?>
		lua_pop(L, (lua_getfield(L, 1, "<?= $field->name ?>"), <?= $field->type->get("pop", -1, "self->" . $field->name) ?>, 1));
		<?php endforeach ?>
	} else {
		<?php $index = 1 ?>
		<?php foreach ($event->getAllFields() as $field):?>
			<?php if ($field->type->fixed_array) continue; ?>
			<?php if ($field->noexport) continue; ?>
		<?= $field->type->get("pop", $index, "self->" . $field->name) ?>;
		<?php $index = $index + 1 ?>
		<?php endforeach ?>
	}
	return 1;
}
int f_<?= $struct_name ?>___index(lua_State *L) {
	struct <?= $struct_name ?>* self = luaX_check<?= $struct_name ?>(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	<?php foreach ($event->getAllFields() as $field):?>
		<?php if ($field->type->fixed_array) continue; ?>
		<?php if ($field->noexport) continue; ?>
	case <?= $field->name->id ?>: <?= $field->type->get('push', "self->" . $field->name) ?>; return 1; // <?= $field->name ?>
	<?php endforeach ?>
	}
	return luaL_error(L, "Unknown field in <?= $struct_name ?>(%p): %s", self, luaL_checkstring(L, 2));
}
int f_<?= $struct_name ?>___newindex(lua_State *L) {
	struct <?= $struct_name ?>* self = luaX_check<?= $struct_name ?>(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	<?php foreach ($event->getAllFields() as $field):?>
		<?php if ($field->type->fixed_array) continue; ?>
		<?php if ($field->noexport) continue; ?>
	case <?= $field->name->id ?>: <?= $field->type->get("pop", 3, "self->" . $field->name) ?>; return 0; // <?= $field->name ?>
	<?php endforeach ?>
	}
	return luaL_error(L, "Unknown field in <?= $struct_name ?>(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_<?= $struct_name ?>___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_<?= $struct_name ?>(L));  // remove <?= $struct_name ?> from stack and call constructor
}
int luaopen_orca_<?= $struct_name ?>(lua_State *L) {
	luaL_newmetatable(L, "<?= $struct_name ?>");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_<?= $struct_name ?> },
		{ "__newindex", f_<?= $struct_name ?>___newindex },
		{ "__index", f_<?= $struct_name ?>___index },
		{ NULL, NULL },
	}), 0);
	// Make <?= $struct_name ?> creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_<?= $struct_name ?>___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
<?php endforeach ?>
