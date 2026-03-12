<?php foreach ($enums as $name => $enum): ?>
<?php if ($enum->doc): ?>
/// @brief <?= $enum->doc ?>
<?php endif ?>
/** <?= $name ?> enum */
typedef enum <?= $name ?> {
<?php foreach ($enum->getValues() as $enum_name => $enum_doc): ?>
	k<?= $name ?><?= $enum_name ?>, ///< <?= $enum_doc ?>
<?php endforeach ?>
} e<?= $name ?>_t;
#define <?= $name ?>_Count <?= count($enum->getValues()) ?>
ORCA_API const char *<?= $name ?>ToString(enum <?= $name ?> value);
ORCA_API enum <?= $name ?> luaX_check<?= $name ?>(lua_State *L, int idx);
ORCA_API void luaX_push<?= $name ?>(lua_State *L, enum <?= $name ?> value);
<?php endforeach ?>

