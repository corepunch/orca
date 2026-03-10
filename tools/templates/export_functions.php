<?php foreach ($model->getFunctions() as $name => $func):?>
int f_<?= $model->prefix.$name ?>(lua_State *L) {
	<?php $index = 1 ?>
	<?php foreach ($func->getArgs() as $param_name => $param_type):?>
		<?php if ($param_name === "L") continue; ?>
	<?= $param_type ?> <?= $param_name ?> = <?= $param_type->get('check', $index) ?>;
		<?php $index++ ?>
	<?php endforeach ?>
	<?php if ($func->returns && $func->returns->kind === 'nresults'): ?>
	return <?= $model->prefix.$name ?>(<?= implode(", ", array_keys($func->getArgs())) ?>);
	<?php elseif ($func->returns): ?>
	<?= $func->returns ?> result_ = <?= $model->prefix.$name ?>(<?= implode(", ", array_keys($func->getArgs())) ?>);
	<?= $func->returns->get('push', 'result_') ?>;
	return 1;
	<?php else: ?>
	<?= $model->prefix.$name ?>(<?= implode(", ", array_keys($func->getArgs())) ?> <?= $func->getReturnType() != "void" ? ", " : "" ?>);
	return 0;
	<?php endif ?>
}
<?php endforeach ?>