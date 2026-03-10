<?php foreach ($model->getInterfaces() as $name => $interface):?>
void luaX_push<?= $name ?>(lua_State *L, struct <?= $name ?> const* data);
struct <?= $name ?>* luaX_check<?= $name ?>(lua_State *L, int idx);
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
int f_<?= $name ?>___index(lua_State *L) {
	struct <?= $name ?>* self = luaX_check<?= $name ?>(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	<?php foreach ($interface->getMethods() as $method_name => $method):?>
		<?php if ($method->static) continue; ?>
	case <?= $method->id ?>: lua_pushcfunction(L, f_<?= $name ?>_<?= $method_name ?>); return 1; // <?= lcfirst($method_name) ?>
	<?php endforeach ?>
	}
	return luaL_error(L, "Unknown field in <?= $name ?>(%p): %s", self, luaL_checkstring(L, 2));
}
int luaopen_orca_<?= $name ?>(lua_State *L) {
	luaL_newmetatable(L, "<?= $interface->export ?>");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "__index", f_<?= $name ?>___index },
	<?php foreach ($interface->getMethods() as $method_name => $method):?>
		{ "<?= $method->export ?>", f_<?= $name ?>_<?= $method_name ?> },
	<?php endforeach ?>
		{ NULL, NULL },
	}), 0);
	return 1;
}
<?php endforeach ?>
