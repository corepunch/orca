<?php foreach ($structs as $name => $struct): ?>
<?php if ($struct->doc): ?>
/// @brief <?= $struct->doc ?>
<?php endif ?>
/** <?= $name ?> struct */
struct <?= $name ?> {
<?php printContents($struct->getFields()) ?>
};
ORCA_API void luaX_push<?= $name ?>(lua_State *L, struct <?= $name ?> const* <?= $name ?>);
ORCA_API struct <?= $name ?>* luaX_check<?= $name ?>(lua_State *L, int idx);
<?php foreach ($struct->getMethods() as $method_name => $method): ?>
<?php if ($method->doc): ?>
/// @brief <?= $method->doc ?>
<?php endif ?>
ORCA_API <?= $method->getReturnType() ?>
<?= $method->full_name ?>(<?= implode(', ', $method->getArgsTypes()) ?>);
<?php endforeach ?>
<?php endforeach ?>

