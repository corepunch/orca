<?php require "model/module.php"; ?>
<?php $model = new Model($argv[1]); ?>
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
<?php foreach ($struct->getFields() as $field => $type): ?>
	<?= $type ?> <?= $field ?>;
<?php endforeach ?>
};
<?php foreach ($struct->getMethods() as $method_name => $method): ?>
<?= $method->getReturnType() ?>
<?= $method->full_name ?>(<?= implode(', ', $method->getArgsTypes()) ?>);
<?php endforeach ?>
<?php endforeach ?>

<?php foreach ($model->getComponents() as $name => $component): ?>
/** <?= $name ?> component */
typedef struct <?= $name ?> <?= $name ?>_t, *<?= $name ?>Ptr_t;
struct <?= $name ?> {
<?php foreach ($component->getProperties() as $field => $type): ?>
	<?= $type ?> <?= $field ?>;
<?php endforeach ?>
<?php foreach ($component->getFields() as $field => $type): ?>
	<?= $type ?> <?= $field ?>;
<?php endforeach ?>
};
<?php endforeach ?>

#endif