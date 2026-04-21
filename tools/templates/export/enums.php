<?php foreach ($enums as $name => $enum):?>
ENUM(<?= $name ?>, <?= implode(', ', array_map(fn($v) => "\"$v\"", $enum->getValuesNames())) ?>)
<?php endforeach ?>
