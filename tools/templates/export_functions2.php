<?php 
// this is a placeholder, later on we'll use proper controller class to render views
require "model/function.php"; 
$function = new Function(argv[1]);
?>

<?php foreach ($function->getFunctions() as $name => $func):?>
int f_<?= $name ?>(lua_State *L) {
	<?php foreach ($func->arg as $i => $arg) echo('\t', sprintf($arg['decl'], $arg['type']), ' ', $arg['name'], ';\n') ?>
	<?php foreach ($func->arg as $i => $arg) echo('\t', sprintf($arg['pop'], $arg['name'], $arg['type'], $i+1), ';\n') ?>
	<?php foreach ($func->returns as $arg): ?>
	<?= sprintf($arg['decl'], $arg['type']) ?> _result = <?= $name ?>(<?= implode(", ", array_map(fn($arg) => $arg['name'], $func->arg)) ?>);
	<?= sprintf($arg['push'], $arg['type'], '_result') ?>;
	<?php endforeach ?>
	return 1;
}
<?php endforeach ?>