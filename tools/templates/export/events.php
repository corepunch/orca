<?php foreach ($events as $name => $event):?>
<?php if ($event->getParentEvent()) echo("#define " . $event->msgns . "_" . $name . "EventArgs " . $event->getParentEvent()->msgns . "_" . $event->getParentEvent()->name . "EventArgs\n") ?>
<?php endforeach ?>

<?php foreach ($events as $name => $event):?>
<?php $struct_name = $event->msgns . "_" . $name . "EventArgs"; ?>
static luaL_Reg _<?= $struct_name ?>_Methods[] = { { NULL, NULL } };
static struct PropertyType _<?= $struct_name ?>[] = {
	<?php include_template("export/properties", ['properties' => $event->getAllFields(), 'name' => $struct_name]) ?>
};
<?php endforeach ?>

<?php foreach ($events as $name => $event):?>
<?php $struct_name = $event->msgns . "_" . $name . "EventArgs"; ?>
STRUCT(<?= $struct_name ?>, <?= $struct_name ?>);
<?php endforeach ?>
