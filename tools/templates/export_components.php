<?php if ($model->hasComponents()):?>
#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }
<?php endif ?>

<?php foreach ($model->getEvents() as $name => $event):?>
typedef <?= $event ?>* <?= $name ?>EventPtr;
<?php endforeach ?>
<?php foreach ($model->getComponents() as $name => $component):?>
	<?php foreach ($component->getEventHandlers() as $event): ?>
LRESULT <?= $name ?>_<?= $event ?>(struct Object*, struct <?= $name ?>*, wParam_t, <?= $event ?>EventPtr);
	<?php endforeach ?>

static struct PropertyType const <?= $name ?>Properties[k<?= $name ?>NumProperties] = {
<?php foreach ($component->getProperties() as $property => $type):?>
	<?php if ($type->kind === 'enum'): ?>
	DECL(<?= $property->id ?>, <?= $name ?>, <?= $property ?>, <?= $property->addr ?>, kDataType<?= ucfirst($type->kind) ?>, .TypeString = "<?= implode(',', $type->data->getValuesNames()) ?>"), // <?= $name ?>.<?= $property ?>
	<?php elseif ($type->kind === 'struct'): ?>
	DECL(<?= $property->id ?>, <?= $name ?>, <?= $property ?>, <?= $property->addr ?>, kDataType<?= ucfirst($type->kind) ?>, .TypeString = "<?= $type->export ?>"), // <?= $name ?>.<?= $property ?>
	<?php elseif ($type->kind === 'component'): ?>
	DECL(<?= $property->id ?>, <?= $name ?>, <?= $property ?>, <?= $property->addr ?>, kDataTypeObject, .TypeString = "<?= $type->export ?>"), // <?= $name ?>.<?= $property ?>
	<?php else: ?>
	DECL(<?= $property->id ?>, <?= $name ?>, <?= $property ?>, <?= $property->addr ?>, kDataType<?= ucfirst($type->kind) ?>), // <?= $name ?>.<?= $property ?>
	<?php endif ?>
<?php endforeach ?>
};
static struct <?= $name ?> <?= $name ?>Defaults = {
<?php foreach (array_filter($component->getProperties(), fn($type) => $type->default) as $property => $type): ?>
	<?php if ($type->kind === 'struct'): ?>
  .<?= $property ?> = {<?= $type->default ?>},
	<?php elseif ($type->kind === 'enum'): ?>
  .<?= $property ?> = k<?= $type->name.$type->default ?>,
	<?php else: ?>
  .<?= $property ?> = <?= $type->default ?>,
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
ORCA_API struct ClassDesc _<?= $name ?> = {
	.ClassName = "<?= $name ?>",
	.DefaultName = "<?= $name ?>",
	.ContentType = "<?= $name ?>",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = {NULL},
	.ClassID = ID_<?= $name ?>,
	.ClassSize = sizeof(struct <?= $name ?>),
	.Properties = <?= $name ?>Properties,
	.ObjProc = <?= $name ?>Proc,
	.Defaults = &<?= $name ?>Defaults,
	.NumProperties = k<?= $name ?>NumProperties,
};

<?php endforeach ?>

