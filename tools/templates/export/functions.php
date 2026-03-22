<?php foreach ($functions as $func_name => $func):?>
int f_<?= $prefix.$func_name ?>(lua_State *L) {
	<?php $index = 1 ?>
	<?php foreach ($func->getArgs() as $param_name => $param_type):?>
		<?php if ($param_name === "L") continue; ?>
		<?php $output_type = $param_type->output_attr; ?>
		<?php $bufsize_ref = $param_type->bufsize_attr; ?>
		<?php if ($output_type): ?>
	<?= $output_type ?> <?= $param_name ?> = {0};
		<?php elseif ($bufsize_ref): ?>
	int32_t <?= $param_name ?> = sizeof(<?= $bufsize_ref ?>);
		<?php else: ?>
	<?= $param_type ?> <?= $param_name ?> = <?= $param_type->get('check', $index++) ?>;
		<?php endif ?>
	<?php endforeach ?>
	<?php if ($func->returns && $func->returns->kind === 'nresults'): ?>
	return <?= $prefix.$func_name ?>(<?= implode(", ", array_keys($func->getArgs())) ?>);
	<?php elseif ($func->returns): ?>
	<?= $func->returns ?> result_ = <?= $prefix.$func_name ?>(<?= implode(", ", array_keys($func->getArgs())) ?>);
	<?= $func->returns->get('push', 'result_') ?>;
	return 1;
	<?php else: ?>
	<?= $prefix.$func_name ?>(<?= implode(", ", array_keys($func->getArgs())) ?> <?= $func->getReturnType() != "void" ? ", " : "" ?>);
	return 0;
	<?php endif ?>
}
<?php endforeach ?>