<?php foreach ($events as $name => $event):?>
<?php if ($event->getParentEvent()) echo("#define " . $event->msgns . "_" . $event->name . "EventArgs " . $event->getParentEvent()->msgns . "_" . $event->getParentEvent()->name . "EventArgs\n") ?>
<?php endforeach ?>

<?php foreach ($events as $name => $event):?>
<?php $struct_name = $event->msgns . "_" . $event->name . "EventArgs"; ?>
static luaL_Reg _<?= $struct_name ?>_Methods[] = { { NULL, NULL } };
struct PropertyType _<?= $struct_name ?>_Properties[] = {
	<?php include_template("export/properties", ['properties' => $event->getAllFields(), 'name' => $struct_name]) ?>
};
#define _<?= $struct_name ?> _<?= $struct_name ?>_Properties
<?php endforeach ?>

<?php foreach ($events as $name => $event):?>
<?php $struct_name = $event->msgns . "_" . $event->name . "EventArgs"; ?>
STRUCT(<?= $struct_name ?>, <?= $struct_name ?>);
<?php endforeach ?>
