<?php require "model/module.php"; ?>
<?php $controller = new Controller($argv[1]); ?>
#ifndef __<?= strtoupper($controller->getModuleName()) ?>_PROPERTIES_H__
#define __<?= strtoupper($controller->getModuleName()) ?>_PROPERTIES_H__
<?php foreach ($controller->getComponents() as $classname => $class): ?>
// <?= $classname ?>
#define ID_<?= $classname ?> 0x<?= hash('fnv1a32', $classname) ?>
#define Get<?= $classname ?>(_P) ((struct <?= $classname ?>*)((_P)?OBJ_GetComponent(_P,ID_<?= $classname ?>):NULL))
#define <?= $classname ?>_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_<?= $classname ?>,sizeof(struct <?= $classname ?>),_N)
<?php foreach ($class->getProperties() as $name => $type): ?>
<?php $full_name = $classname . '.' . $name; ?>
#define ID_<?= $classname ?>_<?= $name ?> 0x<?= hash('fnv1a32', $full_name) ?> // <?= $full_name ?>
<?php endforeach ?>
enum <?= $classname ?>Properties {
<?php foreach ($class->getProperties() as $name => $type): ?>
	k<?= $classname ?><?= $name ?>,
<?php endforeach ?>
	k<?= $classname ?>NumProperties
};
<?php endforeach ?>
#endif