<?php if (count($components) > 0):?>
#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }
<?php endif ?>

<?php foreach ($events as $name => $event):?>
<?php if ($event->hasAnyFields()):?>
<?php if ($event->hasFields()):?>
struct <?= $name ?>EventArgs {
<?php include_template("struct_contents", ['list' => $event->getAllFields()]) ?>
};
typedef struct <?= $name ?>EventArgs* <?= $name ?>EventPtr;
<?php else:?>
typedef struct <?= $event->getEffectiveStructName() ?>* <?= $name ?>EventPtr;
<?php endif ?>
<?php else:?>
typedef <?= $event->getEffectiveTypeDecl() ?>* <?= $name ?>EventPtr;
<?php endif ?>
<?php endforeach ?>
<?php foreach ($components as $name => $component):?>
	<?php foreach ($component->getEventHandlers() as $event): ?>
LRESULT <?= $name ?>_<?= $event ?>(struct Object*, struct <?= $name ?>*, wParam_t, <?= $event ?>EventPtr);
	<?php endforeach ?>

static struct PropertyType const <?= $name ?>Properties[k<?= $name ?>NumProperties] = {
	<?php foreach ($component->getProperties() as $property):?>
		<?php $datatype = 'kDataType' . ucfirst($property->type->kind) ?>
		<?php if ($property->type->kind === 'component') $datatype = 'kDataTypeObject'; ?>
		<?php if ($property->type->kind === 'struct' && $property->type->type == 'color') $datatype = 'kDataTypeColor'; ?>
		<?php echo($property->type->array ? "\tARRAY_DECL(" : "\tDECL(") ?>
		<?php echo("{$property->name->id}, $name, {$property->name}, {$property->name->addr}, $datatype") ?>
		<?php if ($property->type->kind === 'enum') {
			echo (", .TypeString = \"" . implode(',', $property->type->data->getValuesNames()) . "\", .EnumValues = _" . $property->type->type . "), // $name.{$property->name}\n");
		} elseif ($property->type->kind === 'struct' && $property->type->type != 'color') {
			echo (", .TypeString = \"{$property->type->export}\"), // $name.{$property->name}\n");
		} elseif ($property->type->kind === 'component') {
			echo (", .TypeString = \"{$property->type->export}\"), // $name.{$property->name}\n");
 		} else {
			echo ("), // $name.{$property->name}\n");
 		} ?>
	<?php endforeach ?>
};
static struct <?= $name ?> <?= $name ?>Defaults = {
	<?php foreach (array_filter($component->getProperties(), fn($prop) => $prop->type->default) as $property): ?>		
  .<?= $property->name ?> = <?= sprintf($property->type->get('default'), $property->type->default) ?>,
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
	.ObjProc = <?= $name ?>Proc,
	.Defaults = &<?= $name ?>Defaults,
	.NumProperties = k<?= $name ?>NumProperties,
<?php if ($component->extension) {
	echo "\t.Extension = \"{$component->extension}\",\n";
} ?>
};

<?php endforeach ?>

