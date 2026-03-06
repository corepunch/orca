<?php foreach ($model->getStructs() as $name => $struct):?>

void luaX_push<?= $name ?>(lua_State *L, <?= $name ?> const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct <?= $name ?>* self = lua_newuserdata(L, sizeof(struct <?= $name ?>));
	luaL_setmetatable(L, "<?= $name ?>");
	memcpy(self, data, sizeof(struct <?= $name ?>));
}
struct <?= $name ?>* luaX_check<?= $name ?>(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "<?= $name ?>");
}
static int f_new_<?= $name ?>(lua_State *L) {
	struct <?= $name ?>* self = lua_newuserdata(L, sizeof(struct <?= $name ?>));
	luaL_setmetatable(L, "<?= $name ?>");
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
static int f_fromstring_<?= $name ?>(lua_State *L) {
	<?php foreach ($struct->getParsers() as $field => $type):?>
	<?= $type ?> <?= $field ?>;
	<?php endforeach ?>
	<?php 
	$format = implode(" ", array_map(fn($v) => $v->getFormatPlaceholder(), $struct->getParsers())) 
	$targets = implode(", ", array_map(fn($k) => "&{$k}", array_keys($struct->getParsers())))
	?>
	switch (sscanf(luaL_checkstring(L, 1), "<?= $format ?>", <?= $targets ?>)) {
		default:
			return luaL_error(L, "Invalid format for <?= $name ?>: %s", luaL_checkstring(L, 1));
	}

	

	return 1;
}

int f_<?= $name ?>___index(lua_State *L) {
	stuct <?= $name ?>* self = luaX_check<?= $name ?>(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
		<?php foreach ($struct->getFields() as $field => $type):?>
		case <?= $field->id ?>: <?= $type->getExporter("self->$field") ?>; return 1; // <?= $field ?>
		<?php endforeach ?>
	}
	return luaL_error(L, "Unknown field in <?= $name ?>: %s", luaL_checkstring(L, 2));
}
int f_<?= $name ?>___newindex(lua_State *L) {
	stuct <?= $name ?>* self = luaX_check<?= $name ?>(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
		<?php foreach ($struct->getFields() as $field => $type):?>
		case <?= $field->id ?>: self-><?= $field ?> = <?= $type->getImporter(3) ?>; return 0; // <?= $field ?>
		<?php endforeach ?>
	}
	return luaL_error(L, "Unknown field in <?= $name ?>: %s", luaL_checkstring(L, 2));
}
static int f_<?= $name ?>___call(lua_State *L) {
	return (lua_remove(L, 1), f_new_<?= $name ?>(L));  // remove <?= $name ?> from stack and call constructor
}
int luaopen_orca_<?= $name ?>(lua_State *L) {
	luaL_newmetatable(L, "<?= $name ?>");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_<?= $name ?> },
		{ "fromstring", f_fromstring_<?= $name ?> },
		{ "__newindex", f_<?= $name ?>___newindex },
		{ "__index", f_<?= $name ?>___index },
		{ NULL, NULL },
	}), 0);
	// Make <?= $name ?> creatable like via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_<?= $name ?>___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
<?php endforeach ?>
