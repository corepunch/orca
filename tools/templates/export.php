<?php require "model/module.php"; ?>
<?php $model = new Model($argv[1]); ?>
// Auto-generated from <?= basename($argv[1]) ?> by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "<?= $model->getModuleName() ?>.h"

<?php foreach ($model->getExternalStructs() as $name => $module): ?>
// <?= $name ?>
extern void luaX_push<?= $name ?>(lua_State *L, struct <?= $name ?> const* value);
extern struct <?= $name ?>* luaX_check<?= $name ?>(lua_State *L, int index);
<?php endforeach ?>

<?php
$local_enums = $model->getEnums();
$extern_enums = [];
foreach ($model->getComponents() as $cname => $comp) {
	foreach ($comp->getProperties() as $prop) {
		if ($prop->type->kind === 'enum') {
			$ename = strval($prop->type->type);
			$in_local = isset($local_enums[$ename]) ? $local_enums[$ename] : null;
			$in_extern = isset($extern_enums[$ename]) ? $extern_enums[$ename] : null;
			if ($in_local === null && $in_extern === null) {
				$extern_enums[$ename] = $prop->type->data;
			}
		}
	}
}
?>
<?php foreach ($extern_enums as $ename => $eobj): ?>
extern const char *_<?= $ename ?>[];
<?php endforeach ?>
<?php include_template("export/enums", ['enums' => $local_enums]) ?>
<?php include_template("export/interfaces", ['interfaces' => $model->getInterfaces()]) ?>
<?php include_template("export/structs", ['structs' => $model->getStructs()]) ?>
<?php include_template("export/events", ['events' => $model->getEvents()]) ?>
<?php include_template("export/components", ['components' => $model->getComponents(), 'events' => $model->getEvents()]) ?>
<?php include_template("export/functions", ['functions' => $model->getFunctions(), 'prefix' => $model->prefix]) ?>

ORCA_API int luaopen_orca_<?= $model->getModuleName() ?>(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
<?php foreach ($model->getFunctions() as $name => $func):?>
		{ "<?= lcfirst($name) ?>", f_<?= $model->prefix.$name ?> },
<?php endforeach ?>
		{ NULL, NULL } 
	}));
<?php if ($model->on_luaopen): ?>
	void <?= $model->on_luaopen ?>(lua_State *L);
	<?= $model->on_luaopen ?>(L);
<?php endif ?>
<?php foreach ($model->getStructs() as $name => $struct):?>
	lua_setfield(L, ((void)luaopen_orca_<?= $name ?>(L), -2), "<?= $struct->export ?>");
<?php endforeach ?>
<?php foreach ($model->getEvents() as $name => $event):?>
<?php if (!$event->hasFields()): continue; endif ?>
	lua_setfield(L, ((void)luaopen_orca_<?= $name ?>EventArgs(L), -2), "<?= $name ?>EventArgs");
<?php endforeach ?>
<?php foreach ($model->getInterfaces() as $name => $interface):?>
	lua_setfield(L, ((void)luaopen_orca_<?= $name ?>(L), -2), "<?= $interface->export ?>");
<?php endforeach ?>
<?php foreach ($model->getComponents() as $name => $component):?>
	lua_setfield(L, ((void)lua_pushclass(L, &_<?= $name ?>), -2), "<?= $component->export ?>");
<?php endforeach ?>
	return 1;
}
