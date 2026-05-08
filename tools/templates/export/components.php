<?php foreach ($components as $name => $component):?>
	<?php foreach ($component->getEventHandlers() as $event): ?>
		<?php $pos = strrpos($event, '.');
					$after = ($pos !== false) ? substr($event, $pos + 1) : ''; 
					$ident = str_replace('.', ', ', $event); ?>
HANDLER(<?= $name ?>, <?= $ident ?>);
	<?php endforeach ?>
COMPONENT_PROPERTIES(<?= $name ?>) = {
	<?php include_template("export/properties", ['properties' => $component->getProperties(), 'name' => $name]) ?>
	<?php foreach ($component->getMessages() as $e) {
		$id = hash('fnv1a32', $e->name);
		echo "\tDECL_EVENT(0x{$id}, {$name}, {$e->name}), // {$name}.{$e->name}\n";
	} ?>
};
COMPONENT_DEFAULTS(<?= $name ?>) = {
	<?php foreach (array_filter($component->getProperties(), fn($prop) => $prop->type->default) as $property): ?>		
  .<?= $property->name ?> = <?= sprintf($property->type->get('default'), $property->type->default) ?>,
	<?php endforeach ?>
};
COMPONENT_PROC_BEGIN(<?= $name ?>)
	<?php foreach ($component->getEventHandlers() as $event): ?>
		<?php $pos = strrpos($event, '.');
					$ns    = ($pos !== false) ? substr($event, 0, $pos) : $event;
					$after = ($pos !== false) ? substr($event, $pos + 1) : $event; ?>
		<?php echo("\t\tDISPATCH($name, $ns, $after) // $event\n") ?>
	<?php endforeach ?>
COMPONENT_PROC_END()
void luaX_push<?= $name ?>(lua_State *L, struct <?= $name ?> const* <?= $name ?>) {
	luaX_pushObject(L, CMP_GetObject(<?= $name ?>));
}
struct <?= $name ?>* luaX_check<?= $name ?>(lua_State *L, int idx) {
	return Get<?= $name ?>(luaX_checkObject(L, idx));
}
<?php foreach ($component->getParents() as $parent) echo "#define ID_$parent 0x" . hash('fnv1a32', $parent) . "\n"; ?>
<?php if ($component->attach_only): ?>
REGISTER_ATTACH_ONLY_CLASS(<?= $name ?>, <?= implode(', ', array_merge(array_map(fn($p) => "ID_$p", $component->getParents()), ['0'])) ?>);
<?php else: ?>
REGISTER_CLASS(<?= $name ?>, <?= implode(', ', array_merge(array_map(fn($p) => "ID_$p", $component->getParents()), ['0'])) ?>);
<?php endif ?>
<?php endforeach ?>
