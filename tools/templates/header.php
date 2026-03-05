<?php require "model.py"; ?>
<?php $model = new Model($argv[1]); ?>
#ifndef <?= strtoupper($model->getModuleName()) ?>_H
#define <?= strtoupper($model->getModuleName()) ?>_H

#include <stdbool.h>
#include <stdint.h>

<?php foreach ($model->getEnums() as $name => $enum) ?>
enum <?= $name ?> {
<?php foreach ($enum->getValues() as $enum_name => $enum_doc) ?>
	k<?= $name ?><?= $enum_name ?>, // <?= $enum_doc ?>
<?php endforeach ?>
};
<?php endforeach ?>
<?php foreach ($model->getStructs() as $name => $struct) ?>
/** <?= $name ?> struct */
typedef struct <?= $name ?> <?= $name ?>_t, *lp<?= $name ?>_t;
struct <?= $name ?> {
<?php foreach ($struct->getFields() as $field_name => $field_type) ?>
	<?= $field_type ?> <?= $field_name ?>;
<?php endforeach ?>
};
<?php foreach ($struct->getMethods() as $method_name => $method) ?>
<?php $i = 0; $args = $method->getArgs(); ?>
<?= $method->return_type ?>
<?= $method->full_name ?>(<?= implode(', ', $method->getArgsTypes()) ?>);
<?php endforeach ?>
<?php endforeach ?>
#endif