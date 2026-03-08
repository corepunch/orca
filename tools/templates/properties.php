<?php require "model/module.php"; ?>
<?php $model = new Model($argv[1]); ?>
#ifndef __<?= strtoupper($model->getModuleName()) ?>_PROPERTIES_H__
#define __<?= strtoupper($model->getModuleName()) ?>_PROPERTIES_H__
<?php foreach ($model->getComponents() as $classname => $class):?>
// <?= $classname ?>
#define ID_<?= $classname ?> 0x<?= hash('fnv1a32', $classname) ?>

#define Get<?= $classname ?>(_P) ((struct <?= $classname ?>*)((_P)?OBJ_GetComponent(_P,ID_<?= $classname ?>):NULL))
#define <?= $classname ?>_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_<?= $classname ?>,sizeof(struct <?= $classname ?>),_N)
<?php foreach ($class->getProperties() as $name => $type): ?>
#define ID_<?= $classname ?>_<?= $name ?> 0x<?= hash('fnv1a32', "$classname.$name") ?> // <?= $classname ?>.<?= $name ?>

<?php endforeach; ?>
enum <?= $classname ?>Properties {
<?php foreach ($class->getProperties() as $name => $type): ?>
	k<?= $classname ?><?= $name ?>,
<?php endforeach; ?>
	k<?= $classname ?>NumProperties
};
<?php endforeach ?>
#endif