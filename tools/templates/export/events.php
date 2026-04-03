<?php foreach ($events as $name => $event):?>
<?php if ($event->isPureAlias()) echo("#define {$name}MsgArgs {$event->getEffectiveStructName()}\n") ?>
//struct MessageType <?= $name ?>Message = {
//	.name = "<?= $name ?>",
//	.id = kMsg<?= $name ?>,
//	.routing = kMessageRouting<?= $event->routing ?>,
//	.size = sizeof(<?= $event->getEffectiveTypeDecl() ?>),
//};
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
