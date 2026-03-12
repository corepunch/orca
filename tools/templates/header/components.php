<?php foreach ($components as $name => $component): ?>
<?php if ($component->doc): ?>
/// @brief <?= $component->doc ?>
<?php endif ?>
/** <?= $name ?> component */
typedef struct <?= $name ?> <?= $name ?>_t, *<?= $name ?>Ptr, *lp<?= $name ?>_t;
typedef struct <?= $name ?> const *<?= $name ?>CPtr, *lpc<?= $name ?>_t;
struct <?= $name ?> {
<?php printContents($component->getProperties(false)) ?>
<?php printContents($component->getFields()) ?>
};
ORCA_API void luaX_push<?= $name ?>(lua_State *L, struct <?= $name ?> const* <?= $name ?>);
ORCA_API struct <?= $name ?>* luaX_check<?= $name ?>(lua_State *L, int idx);
<?php foreach ($component->getMethods() as $method_name => $method): ?>
<?php if ($method->doc): ?>
/// @brief <?= $method->doc ?>
<?php endif ?>
ORCA_API <?= $method->getReturnType() ?>
<?= $component->name ?>_<?= $method->name ?>(<?= implode(', ', $method->getArgsTypes()) ?>);
<?php endforeach ?>
<?php endforeach ?>

