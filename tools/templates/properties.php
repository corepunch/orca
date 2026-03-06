<?php require "model.py"; ?>
<?php $model = new Model($argv[1]); ?>
<?php foreach ($model->getComponents() as $name => $component): ?>
<?php $props = $component->getEnumProperties(); ?>
#define ID_<?= $name ?> 0x<?= hash('fnv1a32', $name) ?>
#define Get<?= $name ?>(_P)((struct <?= $name ?>*)((_P)?OBJ_GetComponent(_P,ID_<?= $name ?>):NULL))
#define <?= $name ?>_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_<?= $name ?>,sizeof(struct <?= $name ?>),_N)
enum <?= $name ?>Properties {
<?php foreach ($props as $prop): ?>
	k<?= $name ?><?= $prop ?>,
<?php endforeach ?>
	k<?= $name ?>NumProperties
};

<?php foreach ($props as $prop): ?>
#define ID_<?= $name ?>_<?= $prop ?> 0x<?= hash('fnv1a32', "$name.$prop") ?>
<?php endforeach ?>
<?php endforeach ?>
