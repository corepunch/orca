<?php require "model/module.php"; ?>
<?php $model = new Model($argv[1]); ?>
// Auto-generated from <?= basename($argv[1]) ?> by tools/templates/properties.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __<?= strtoupper($model->getModuleName()) ?>_PROPERTIES_H__
#define __<?= strtoupper($model->getModuleName()) ?>_PROPERTIES_H__
<?php foreach ($model->getComponents() as $classname => $class):?>
// <?= $classname ?>
#define ID_<?= $classname ?> 0x<?= hash('fnv1a32', $classname) ?>
#define Get<?= $classname ?>(_P) ((struct <?= $classname ?>*)((_P)?OBJ_GetComponent(_P,ID_<?= $classname ?>):NULL))
#define <?= $classname ?>_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_<?= $classname ?>,sizeof(struct <?= $classname ?>),_N)
#define <?= $classname ?>_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_<?= $classname ?>,_N)
<?php foreach ($class->getProperties() as $prop) {
	$name = $prop->name;
	$hashed_property = hash('fnv1a32', "$classname.$name");
	echo("#define ID_{$classname}_{$name} 0x$hashed_property // {$classname}.{$name}\n");
}?>
#define k<?= $classname ?>NumMessageTypes <?= count($class->getMessages()) ?>
<?php if (count($class->getMessages()) > 0): ?>
enum <?= $classname ?>Messages {
	<?php foreach ($class->getMessages() as $event) {
		$name = $event->name;
		echo("\tk{$classname}{$name},\n");
	}?>};
<?php endif ?>
#define k<?= $classname ?>NumProperties <?= count($class->getProperties()) ?>
<?php if (count($class->getProperties()) > 0): ?>
enum <?= $classname ?>Properties {
	<?php foreach ($class->getProperties() as $prop) {
		$name = $prop->name;
		echo("\tk{$classname}{$name},\n");
	}?>};
<?php endif ?>
<?php endforeach ?>

<?php foreach ($model->getStructs() as $name => $struct):?>
#define ID_<?= $name ?> 0x<?= hash('fnv1a32', $name) ?>
<?php foreach ($struct->getFields(true) as $field) {
	$fieldName = $field->name;
	echo("#define ID_{$name}_{$fieldName} 0x" . hash('fnv1a32', "$name.$fieldName") . " // {$name}.{$fieldName}\n");
}?>
<?php endforeach ?>	

<?php foreach ($model->getEvents() as $name => $event):?>
#define ID_<?= $name ?> 0x<?= hash('fnv1a32', $name) ?>
<?php foreach ($event->getAllFields() as $field) {
	$fieldName = $field->name;
	echo("#define ID_{$name}MsgArgs_{$fieldName} 0x" . hash('fnv1a32', "{$name}MsgArgs.$fieldName") . " // {$name}MsgArgs.{$fieldName}\n");
}?>
<?php endforeach ?>	

#endif