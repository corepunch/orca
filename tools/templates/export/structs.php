<?php foreach ($structs as $name => $struct):?>
void luaX_push<?= $name ?>(lua_State *L, struct <?= $name ?> const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct <?= $name ?>* self = lua_newuserdata(L, sizeof(struct <?= $name ?>));
	luaL_setmetatable(L, "<?= $struct->export ?>");
	memcpy(self, data, sizeof(struct <?= $name ?>));
}
struct <?= $name ?>* luaX_check<?= $name ?>(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "<?= $struct->export ?>");
}
static int f_new_<?= $name ?>(lua_State *L) {
	struct <?= $name ?>* self = lua_newuserdata(L, sizeof(struct <?= $name ?>));
	luaL_setmetatable(L, "<?= $struct->export ?>");
	memset(self, 0, sizeof(struct <?= $name ?>));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		<?php foreach ($struct->getFields() as $field):?>
			<?php if ($field->type->fixed_array) continue; ?>
			<?php if ($field->noexport) continue; ?>
		lua_pop(L, (lua_getfield(L, 1, "<?= $field->name ?>"), <?= $field->type->get("pop", -1, "self->" . $field->name) ?>, 1));
		<?php endforeach ?>
	} else {
		<?php $index = 1 ?>
		<?php foreach ($struct->getFields() as $field):?>
			<?php if ($field->type->fixed_array) continue; ?>
			<?php if ($field->noexport) continue; ?>
		<?= $field->type->get("pop", $index, "self->" . $field->name) ?>;
		<?php $index = $index + 1 ?>
		<?php endforeach ?>
	}
	return 1;
}

<?php include_template("export/functions", ['functions' => $struct->getMethods(), 'prefix' => $struct->prefix]) ?>

int f_<?= $name ?>___index(lua_State *L) {
	struct <?= $name ?>* self = luaX_check<?= $name ?>(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	<?php foreach ($struct->getFields() as $field):?>
		<?php if ($field->type->fixed_array) continue; ?>
		<?php if ($field->noexport) continue; ?>
	case <?= $field->name->id ?>: <?= $field->type->get('push', "self->" . $field->name) ?>; return 1; // <?= $field->name ?>
	<?php endforeach ?>
	<?php foreach ($struct->getMethods() as $method_name => $method):?>
		<?php if ($method->static) continue; ?>
	case <?= $method->id ?>: lua_pushcfunction(L, f_<?= $struct->prefix.$method_name ?>); return 1; // <?= lcfirst($method_name) ?>
	<?php endforeach ?>
	}
	return luaL_error(L, "Unknown field in <?= $name ?>(%p): %s", self, luaL_checkstring(L, 2));
}
int f_<?= $name ?>___newindex(lua_State *L) {
	struct <?= $name ?>* self = luaX_check<?= $name ?>(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	<?php foreach ($struct->getFields() as $field):?>
		<?php if ($field->type->fixed_array) continue; ?>
		<?php if ($field->noexport) continue; ?>
	case <?= $field->name->id ?>: <?= $field->type->get("pop", 3, "self->" . $field->name) ?>; return 0; // <?= $field->name ?>
	<?php endforeach ?>
	}
	return luaL_error(L, "Unknown field in <?= $name ?>(%p): %s", self, luaL_checkstring(L, 2));
}
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
static int f_<?= $name ?>___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_<?= $name ?>(L));  // remove <?= $name ?> from stack and call constructor
}
int luaopen_orca_<?= $name ?>(lua_State *L) {
	luaL_newmetatable(L, "<?= $struct->export ?>");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_<?= $name ?> },
<?php if ($struct->hasFromString() || count($struct->getConstructors()) > 0):?>
		{ "fromstring", f_<?= $name ?>___fromstring },
<?php endif ?>
		{ "__newindex", f_<?= $name ?>___newindex },
		{ "__index", f_<?= $name ?>___index },
	<?php foreach ($struct->getMethods() as $method_name => $method):?>
		{ "<?= $method->export ?>", f_<?= $struct->prefix.$method_name ?> },
	<?php endforeach ?>
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
