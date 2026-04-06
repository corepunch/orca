<?php foreach ($events as $name => $event):?>
<?php if ($event->getParentEvent()) echo("#define " . $event->msgns . "_" . $name . "MsgArgs " . $event->getParentEvent()->msgns . "_" . $event->getParentEvent()->name . "MsgArgs\n") ?>
//struct MessageType <?= $name ?>Message = {
//	.name = "<?= $name ?>",
//	.id = kMsg<?= $name ?>,
//	.routing = kMessageRouting<?= $event->routing ?>,
//	.size = sizeof(<?= $event->getEffectiveTypeDecl() ?>),
//};
<?php endforeach ?>

<?php foreach ($events as $name => $event):?>
<?php $struct_name = $event->msgns . "_" . $name . "MsgArgs"; ?>
static luaL_Reg _<?= $struct_name ?>_Methods[] = { { NULL, NULL } };
static struct PropertyType _<?= $struct_name ?>[] = {
	<?php include_template("export/properties", ['properties' => $event->getAllFields(), 'name' => $struct_name]) ?>
};
<?php endforeach ?>

<?php foreach ($events as $name => $event):?>
<?php $struct_name = $event->msgns . "_" . $name . "MsgArgs"; ?>
STRUCT(<?= $struct_name ?>, <?= $struct_name ?>);
<?php endforeach ?>
