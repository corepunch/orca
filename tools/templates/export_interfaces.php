<?php foreach ($interfaces as $name => $interface):?>

<?php include_template("export_functions", ['functions' => $interface->getMethods(), 'prefix' => $interface->prefix]) ?>

int luaopen_orca_<?= $name ?>(lua_State *L) {
	luaL_newmetatable(L, "<?= $interface->export ?>");
	luaL_setfuncs(L, ((luaL_Reg[]) {
	<?php foreach ($interface->getMethods() as $method_name => $method):?>
		{ "<?= $method->export ?>", f_<?= $interface->prefix.$method_name ?> },
	<?php endforeach ?>
		{ NULL, NULL },
	}), 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	return 1;
}
<?php endforeach ?>
