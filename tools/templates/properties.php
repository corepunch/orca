#ifndef __<?= strtoupper($model->getModuleName()) ?>_PROPERTIES_H__
#define __<?= strtoupper($model->getModuleName()) ?>_PROPERTIES_H__
<?php foreach ($model->getEvents() as $name => $event):?>
#define kEvent<?= $name ?> 0x<?= hash('fnv1a32', $name) ?>
<?php endforeach ?>
<?php foreach ($model->getComponents() as $classname => $class):?>
// <?= $classname ?>
#define ID_<?= $classname ?> 0x<?= hash('fnv1a32', $classname) ?>
#define Get<?= $classname ?>(_P) ((struct <?= $classname ?>*)((_P)?OBJ_GetComponent(_P,ID_<?= $classname ?>):NULL))
#define <?= $classname ?>_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_<?= $classname ?>,sizeof(struct <?= $classname ?>),_N)
<?php foreach ($class->getProperties() as $prop) {
	$name = $prop->name;
	$hashed_property = hash('fnv1a32', "$classname.$name");
	echo("#define ID_{$classname}_{$name} 0x$hashed_property // {$classname}.{$name}\n");
}?>
enum <?= $classname ?>Properties {
	<?php foreach ($class->getProperties() as $prop) {
		$name = $prop->name;
		echo("\tk{$classname}{$name},\n")
	}
	echo("\tk{$classname}NumProperties")?>	
};
<?php endforeach ?>
#endif