<?php foreach ($components as $name => $component):?>
	<?php foreach ($component->getEventHandlers() as $event): ?>
LRESULT <?= $name ?>_<?= $event ?>(struct Object*, struct <?= $name ?>*, wParam_t, <?= $event ?>MsgPtr);
	<?php endforeach ?>
static struct MessageType <?= $name ?>MessageTypes[k<?= $name ?>NumMessageTypes] = {	
	<?php foreach ($component->getMessages() as $event): ?>
		{ "<?= $name ?>.<?= $event->name ?>", kMsg<?= $event->name ?>, kMessageRouting<?= $event->routing ?>, sizeof(<?= $event->getEffectiveTypeDecl() ?>) },
	<?php endforeach ?>
};
static struct PropertyType const <?= $name ?>Properties[k<?= $name ?>NumProperties] = {
	<?php include_template("export/properties", ['properties' => $component->getProperties(), 'name' => $name]) ?>
};
static struct <?= $name ?> <?= $name ?>Defaults = {
	<?php foreach (array_filter($component->getProperties(), fn($prop) => $prop->type->default) as $property): ?>		
  .<?= $property->name ?> = <?= sprintf($property->type->get('default'), $property->type->default) ?>,
	<?php endforeach ?>
};
LRESULT <?= $name ?>Proc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	<?php foreach ($component->getEventHandlers() as $event): ?>
		case kMsg<?= $event ?>: return <?= $name ?>_<?= $event ?>(object, cmp, wparm, lparm); // <?= $event ?>
	<?php endforeach ?>
	}
	return FALSE;
}
void luaX_push<?= $name ?>(lua_State *L, struct <?= $name ?> const* <?= $name ?>) {
	luaX_pushObject(L, CMP_GetObject(<?= $name ?>));
}
struct <?= $name ?>* luaX_check<?= $name ?>(lua_State *L, int idx) {
	return Get<?= $name ?>(luaX_checkObject(L, idx));
}
<?php foreach ($component->getParents() as $parent) echo "#define ID_$parent 0x" . hash('fnv1a32', $parent) . "\n"; ?>
ORCA_API struct ClassDesc _<?= $name ?> = {
	.ClassName = "<?= $name ?>",
	.DefaultName = "<?= $name ?>",
	.ContentType = "<?= $name ?>",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { <?= implode(', ', array_merge(array_map(fn($p) => "ID_$p", $component->getParents()), ['0'])) ?> },
	.ClassID = ID_<?= $name ?>,
	.ClassSize = sizeof(struct <?= $name ?>),
	.Properties = <?= $name ?>Properties,
	.MessageTypes = <?= $name ?>MessageTypes,
	.ObjProc = <?= $name ?>Proc,
	.Defaults = &<?= $name ?>Defaults,
	.NumProperties = k<?= $name ?>NumProperties,
	.NumMessageTypes = k<?= $name ?>NumMessageTypes,
<?php if ($component->extension) {
	echo "\t.Extension = \"{$component->extension}\",\n";
} ?>
};

<?php endforeach ?>

