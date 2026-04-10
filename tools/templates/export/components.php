#define REGISTER_CLASS(NAME, ...) \
ORCA_API struct ClassDesc _##NAME = { \
	.ClassName = #NAME, \
	.DefaultName = #NAME, \
	.ContentType = #NAME, \
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation", \
	.ParentClasses = { __VA_ARGS__ }, \
	.ClassID = ID_##NAME, \
	.ClassSize = sizeof(struct NAME), \
	.Properties = NAME##Properties, \
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
};
#define REGISTER_ATTACH_ONLY_CLASS(NAME, ...) \
ORCA_API struct ClassDesc _##NAME = { \
	.ClassName = #NAME, \
	.DefaultName = #NAME, \
	.ContentType = #NAME, \
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation", \
	.ParentClasses = { __VA_ARGS__ }, \
	.ClassID = ID_##NAME, \
	.ClassSize = sizeof(struct NAME), \
	.Properties = NAME##Properties, \
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
	.IsAttachOnly = TRUE, \
};
<?php foreach ($components as $name => $component):?>
	<?php foreach ($component->getEventHandlers() as $event): ?>
		<?php $pos = strrpos($event, '.');
					$after = ($pos !== false) ? substr($event, $pos + 1) : ''; 
					$ident = str_replace('.', ', ', $event); ?>
HANDLER(<?= $name ?>, <?= $ident ?>);
	<?php endforeach ?>
static struct PropertyType const <?= $name ?>Properties[k<?= $name ?>NumProperties] = {
	<?php include_template("export/properties", ['properties' => $component->getProperties(), 'name' => $name]) ?>
	<?php foreach ($component->getMessages() as $e) 
		$id = hash('fnv1a32',$e->name);
		echo "\tDECL(0x{$id}, {$name}, {$e->name}, {$e->name}, kDataTypeEvent, .TypeString = \"{$name}_{$e->name}EventArgs\"), // {$name}.{$e->name}\n"; ?>
};
static struct <?= $name ?> <?= $name ?>Defaults = {
	<?php foreach (array_filter($component->getProperties(), fn($prop) => $prop->type->default) as $property): ?>		
  .<?= $property->name ?> = <?= sprintf($property->type->get('default'), $property->type->default) ?>,
	<?php endforeach ?>
};
LRESULT <?= $name ?>Proc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	<?php foreach ($component->getEventHandlers() as $event): ?>
		<?php $pos = strrpos($event, '.');
					$after = ($pos !== false) ? substr($event, $pos + 1) : ''; 
					$ident = str_replace('.', '_', $event); ?>
		case ID_<?= $ident ?>: return <?= $name ?>_<?= $after ?>(object, cmp, wparm, lparm); // <?= $event ?>
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
<?php if ($component->attach_only): ?>
REGISTER_ATTACH_ONLY_CLASS(<?= $name ?>, <?= implode(', ', array_merge(array_map(fn($p) => "ID_$p", $component->getParents()), ['0'])) ?>);
<?php else: ?>
REGISTER_CLASS(<?= $name ?>, <?= implode(', ', array_merge(array_map(fn($p) => "ID_$p", $component->getParents()), ['0'])) ?>);
<?php endif ?>
<?php endforeach ?>
