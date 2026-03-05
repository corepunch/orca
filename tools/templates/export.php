<?php require "model.py"; ?>
<?php $model = new Model($argv[1]); ?>
<?php foreach ($model->getStructs() as $name => $struct) ?>
typedef struct <?= $name ?> <?= $name ?>_t, *lp<?= $name ?>_t;
struct <?= $name ?> {
<?php foreach ($struct->getFields() as $field_name => $field_type) ?>
	<?= $model->getDeclType($field_type) ?> <?= $field_name ?>;
<?php endforeach ?>
};
void print_<?= $name ?>(lp<?= $name ?> obj);
<?php endforeach ?>