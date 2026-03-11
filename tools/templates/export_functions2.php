<?php require "model/function.php"; $function = new Function($argv[1]); ?>
<?php foreach ($function->getFunctions() as $name => $func):?>
int f_<?= $name ?>(lua_State *L) {
<?php foreach ($func['args'] as $arg): ?>
	<?= $arg['decl'] ?> <?= $arg['name'] ?>;
<?php endforeach ?>
<?php foreach ($func['args'] as $arg): ?>
	<?= $arg['pop'] ?>;
<?php endforeach ?>
<?php if ($func['returns']): $ret = $func['returns'][0]; ?>
<?php if ($ret['nresults']): ?>
	return <?= $name ?>(<?= implode(", ", array_map(fn($a) => $a['name'], $func['args'])) ?>);
<?php else: ?>
	<?= $ret['decl'] ?> _result = <?= $name ?>(<?= implode(", ", array_map(fn($a) => $a['name'], $func['args'])) ?>);
	<?= $ret['push'] ?>;
	return 1;
<?php endif ?>
<?php else: ?>
	<?= $name ?>(<?= implode(", ", array_map(fn($a) => $a['name'], $func['args'])) ?>);
	return 0;
<?php endif ?>
}
<?php endforeach ?>