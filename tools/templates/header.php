<?php require "model/module.php"; ?>
<?php $model = new Model($argv[1]); ?>

<?php function printContents($list) {
	foreach ($list as $name => $type) {
		echo("\t$type $name;\n"); 
	}
} ?>

#ifndef __<?= strtoupper($model->getModuleName()) ?>_H__
#define __<?= strtoupper($model->getModuleName()) ?>_H__

#include <stdbool.h>
#include <stdint.h>
<?php foreach ($model->getRequires() as $name => $module): ?>
// including <?= $name ?>
#include "<?= substr($module->source, 0, -4) ?>.h"
<?php endforeach ?>

<?php foreach ($model->getEnums() as $name => $enum): ?>
/** <?= $name ?> enum */
enum <?= $name ?> {
<?php foreach ($enum->getValues() as $enum_name => $enum_doc): ?>
	k<?= $name ?><?= $enum_name ?>, // <?= $enum_doc ?>
<?php endforeach ?>
};

<?php endforeach ?>
<?php foreach ($model->getStructs() as $name => $struct): ?>
/** <?= $name ?> struct */
typedef struct <?= $name ?> <?= $name ?>_t, *<?= $name ?>Ptr_t;
struct <?= $name ?> {
<?php printContents($struct->getFields()) ?>
};
<?php foreach ($struct->getMethods() as $method_name => $method): ?>
<?php $i = 0; $args = $method->getArgs(); ?>
<?= $method->getReturnType() . "\n" ?><?= $method->full_name ?>(<?= implode(', ', $method->getArgsTypes()) ?>);
<?php endforeach ?>
<?php endforeach ?>

<?php foreach ($model->getComponents() as $name => $component): ?>
/** <?= $name ?> component */
typedef struct <?= $name ?> <?= $name ?>_t, *<?= $name ?>Ptr_t;
struct <?= $name ?> {
	<?php printContents($component->getProperties()) ?>
	<?php printContents($component->getFields()) ?>
};
<?php endforeach ?>

#endif