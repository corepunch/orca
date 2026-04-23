<?php foreach ($interfaces as $name => $interface):?>

<?php include_template("export/functions", ['functions' => $interface->getMethods(), 'prefix' => $interface->prefix]) ?>

int luaopen_orca_<?= $name ?>(lua_State *L) {
	luaL_newmetatable(L, "<?= $interface->export ?>");
	luaL_setfuncs(L, ((luaL_Reg[]) {
	<?php foreach ($interface->getMethods() as $method_name => $method):?>
		{ "<?= $method->export ?>", f_<?= $interface->prefix.$method_name ?> },
		<?php if (isset($method->alias) && strval($method->alias) !== ""): ?>
		{ "<?= $method->alias ?>", f_<?= $interface->prefix.$method_name ?> },
		<?php endif ?>
	<?php endforeach ?>
		{ NULL, NULL },
	}), 0);
	lua_getfield(L, -1, "__index");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	} else {
		lua_pop(L, 1);
	}
	return 1;
}
<?php endforeach ?>
