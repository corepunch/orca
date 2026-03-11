<?php foreach ($model->getInterfaces() as $name => $interface):?>
<?php foreach ($interface->getMethods() as $method_name => $method):?>
int f_<?= $name ?>_<?= $method_name ?>(lua_State *L) {
	<?php $index = 1 ?>
	<?php foreach ($method->getArgs() as $param_name => $param_type):?>
		<?php if ($param_name === "L") continue; ?>
	<?= $param_type ?> <?= $param_name ?> = <?= $param_type->get('check', $index) ?>;
		<?php $index++ ?>
	<?php endforeach ?>
	<?php if ($method->returns && $method->returns->kind === 'nresults'): ?>
	return <?= $interface->prefix.$method_name ?>(<?= implode(", ", array_keys($method->getArgs())) ?>);
	<?php elseif ($method->returns): ?>
	<?= $method->returns ?> result_ = <?= $interface->prefix.$method_name ?>(<?= implode(", ", array_keys($method->getArgs())) ?>);
	<?= $method->returns->get('push', 'result_') ?>;
	return 1;
	<?php else: ?>
	<?= $interface->prefix.$method_name ?>(<?= implode(", ", array_keys($method->getArgs())) ?> <?= $method->getReturnType() != "void" ? ", " : "" ?>);
	return 0;
	<?php endif ?>
}
<?php endforeach ?>
int luaopen_orca_<?= $name ?>(lua_State *L) {
	luaL_newmetatable(L, "<?= $interface->export ?>");
	luaL_setfuncs(L, ((luaL_Reg[]) {
	<?php foreach ($interface->getMethods() as $method_name => $method):?>
		{ "<?= $method->export ?>", f_<?= $name ?>_<?= $method_name ?> },
	<?php endforeach ?>
		{ NULL, NULL },
	}), 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	return 1;
}
<?php endforeach ?>
