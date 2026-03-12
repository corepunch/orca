<?php foreach ($structs as $name => $struct): ?>
typedef struct <?= $name ?> <?= $name ?>_t, *lp<?= $name ?>_t;
typedef struct <?= $name ?> const c<?= $name ?>_t, *lpc<?= $name ?>_t;
<?php endforeach ?>

<?php foreach ($functions as $name => $func):?>
<?php if ($func->doc): ?>
/// @brief <?= $func->doc ?>
<?php endif ?>
ORCA_API <?= $func->getReturnType() ?>
<?= $prefix.$name ?>(<?= implode(', ', $func->getArgsTypes()) ?>);
<?php endforeach ?>

<?php foreach ($interfaces as $name => $interface): ?>
<?php foreach ($interface->getMethods() as $method_name => $method): ?>
<?php if ($method->doc): ?>
/// @brief <?= $method->doc ?>
<?php endif ?>
ORCA_API <?= $method->getReturnType() ?>
<?= $method->full_name ?>(<?= implode(', ', $method->getArgsTypes()) ?>);
<?php endforeach ?>
<?php endforeach ?>

