<?php require "model.php"; ?>
<?php $model = new Model($argv[1]); ?>
#ifndef __<?= strtoupper($model->getModuleName()) ?>_PROPERTIES_H__
#define __<?= strtoupper($model->getModuleName()) ?>_PROPERTIES_H__
<?php foreach ($model->getComponents() as $name => $component)
	$hashed_class = hash('fnv1a32', $name)
	echo("\n// {$name}\n")
	echo("#define ID_{$name} 0x$hashed_class\n")
	foreach ($component->getProperties() as $property_name => $property_type) 
		$hashed_property = hash('fnv1a32', "$name.$property_name");
		echo("#define ID_{$name}_{$property_name} 0x$hashed_property // {$name}.{$property_name}\n");
 endforeach 
endforeach ?>
#endif