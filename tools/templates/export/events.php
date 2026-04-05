<?php foreach ($events as $name => $event):?>
<?php if ($event->getParentEvent()) echo("#define {$name}MsgArgs {$event->getParentEvent()->name}MsgArgs\n") ?>
<?php endforeach ?>

<?php foreach ($events as $name => $event):?>
static luaL_Reg _<?= $name ?>MsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _<?= $name ?>MsgArgs[] = {
	<?php include_template("export/properties", ['properties' => $event->getAllFields(), 'name' => "{$name}MsgArgs"]) ?>
};
<?php endforeach ?>

<?php foreach ($events as $name => $event):?>
STRUCT(<?= $name ?>MsgArgs, <?= $name ?>MsgArgs);
<?php endforeach ?>

<?php
$module_name = $model->getModuleName();
$event_defs = [];
foreach ($events as $name => $event):
	$container = $event->container_name ?? $module_name;
	$effective = $event->hasAnyFields() ? $event->getEffectiveStructName() : null;
	$push_val = $effective ? "(void*)luaX_push" . $effective : "NULL";
?>
struct MessageType <?= $name ?>Message = {
	.name = "<?= $name ?>",
	.id = ID_<?= $container ?>_<?= $name ?>,
	.routing = kMessageRouting<?= $event->routing ?>,
	.size = sizeof(<?= $event->getEffectiveTypeDecl() ?>),
	.push = <?= $push_val ?>,
};
<?php
	$event_defs[] = [$name, "ID_{$container}_{$name}"];
endforeach;
?>

static lpcMessageType_t _<?= $module_name ?>_messages[] = {
<?php foreach ($event_defs as $pair):?>
	&<?= $pair[0] ?>Message,
<?php endforeach ?>
};
static uint32_t _<?= $module_name ?>_messages_count = <?= count($event_defs) ?>;
