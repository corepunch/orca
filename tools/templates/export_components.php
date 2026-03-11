<?php if ($hasComponents):?>
#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }
<?php endif ?>

<?php foreach ($events as $name => $event):?>
typedef <?= $event ?>* <?= $name ?>EventPtr;
<?php endforeach ?>
<?php foreach ($components as $name => $component):?>
	<?php foreach ($component->getEventHandlers() as $event): ?>
LRESULT <?= $name ?>_<?= $event ?>(struct Object*, struct <?= $name ?>*, wParam_t, <?= $event ?>EventPtr);
	<?php endforeach ?>

static struct PropertyType const <?= $name ?>Properties[k<?= $name ?>NumProperties] = {
	<?php foreach ($component->getProperties() as $property):?>
		<?php if ($property->type->kind === 'enum'): ?>
	DECL(<?= $property->name->id ?>, <?= $name ?>, <?= $property->name ?>, <?= $property->name->addr ?>, kDataType<?= ucfirst($property->type->kind) ?>, .TypeString = "<?= implode(',', $property->type->data->getValuesNames()) ?>"), // <?= $name ?>.<?= $property->name ?>
		<?php elseif ($property->type->array): ?>
	ARRAY_DECL(<?= $property->name->id ?>, <?= $name ?>, <?= $property->name ?>, <?= $property->name->addr ?>, kDataType<?= ucfirst($property->type->kind) ?>), // <?= $name ?>.<?= $property->name ?>
		<?php elseif ($property->type->kind === 'struct'): ?>
	DECL(<?= $property->name->id ?>, <?= $name ?>, <?= $property->name ?>, <?= $property->name->addr ?>, kDataType<?= ucfirst($property->type->kind) ?>, .TypeString = "<?= $property->type->export ?>"), // <?= $name ?>.<?= $property->name ?>
		<?php elseif ($property->type->kind === 'component'): ?>
	DECL(<?= $property->name->id ?>, <?= $name ?>, <?= $property->name ?>, <?= $property->name->addr ?>, kDataTypeObject, .TypeString = "<?= $property->type->export ?>"), // <?= $name ?>.<?= $property->name ?>
		<?php else: ?>
	DECL(<?= $property->name->id ?>, <?= $name ?>, <?= $property->name ?>, <?= $property->name->addr ?>, kDataType<?= ucfirst($property->type->kind) ?>), // <?= $name ?>.<?= $property->name ?>
		<?php endif ?>
	<?php endforeach ?>
};
static struct <?= $name ?> <?= $name ?>Defaults = {
	<?php foreach (array_filter($component->getProperties(), fn($prop) => $prop->type->default) as $property): ?>
		<?php if ($property->type->kind === 'struct'): ?>
  .<?= $property->name ?> = {<?= $property->type->default ?>},
		<?php elseif ($property->type->kind === 'enum'): ?>
  .<?= $property->name ?> = k<?= $property->type->type.$property->type->default ?>,
		<?php else: ?>
  .<?= $property->name ?> = <?= $property->type->default ?>,
		<?php endif ?>
	<?php endforeach ?>
};
LRESULT <?= $name ?>Proc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	<?php foreach ($component->getEventHandlers() as $event): ?>
		case kEvent<?= $event ?>: return <?= $name ?>_<?= $event ?>(object, cmp, wparm, lparm); // <?= $event ?>
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
<?php foreach ($component->getParents() as $parent) echo "extern struct ClassDesc _$parent;\n"; ?>
ORCA_API struct ClassDesc _<?= $name ?> = {
	.ClassName = "<?= $name ?>",
	.DefaultName = "<?= $name ?>",
	.ContentType = "<?= $name ?>",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { <?= implode(', ', array_merge(array_map(fn($p) => "&_$p", $component->getParents()), ['NULL'])) ?> },
	.ClassID = ID_<?= $name ?>,
	.ClassSize = sizeof(struct <?= $name ?>),
	.Properties = <?= $name ?>Properties,
	.ObjProc = <?= $name ?>Proc,
	.Defaults = &<?= $name ?>Defaults,
	.NumProperties = k<?= $name ?>NumProperties,
};

<?php endforeach ?>

