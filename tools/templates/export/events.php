<?php foreach ($events as $name => $event):?>
<?php if ($event->getParentEvent()) echo("#define {$name}MsgArgs {$event->getParentEvent()->name}MsgArgs\n") ?>
struct MessageType <?= $name ?>Message = {
	.name = "<?= $name ?>",
	.id = kMsg<?= $name ?>,
	.routing = kMessageRouting<?= $event->routing ?>,
	.size = sizeof(<?= $event->getEffectiveTypeDecl() ?>),
};
<?php endforeach ?>

<?php foreach ($events as $name => $event):?>
static luaL_Reg _<?= $name ?>MsgArgs_Methods[] = {};
static struct PropertyType _<?= $name ?>MsgArgs[] = {
	<?php include_template("export/properties", ['properties' => $event->getAllFields(), 'name' => "{$name}MsgArgs"]) ?>
};
<?php endforeach ?>

<?php foreach ($events as $name => $event):?>
STRUCT(<?= $name ?>MsgArgs, <?= $name ?>MsgArgs);
<?php endforeach ?>
