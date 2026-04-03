<?php require "model/module.php"; ?>
<?php $model = new Model($argv[1]); ?>
// Auto-generated from <?= basename($argv[1]) ?> by tools/templates/header.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __<?= strtoupper($model->getModuleName()) ?>_H__
#define __<?= strtoupper($model->getModuleName()) ?>_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;

<?php foreach ($model->getExternalStructs() as $name => $struct):?>
struct <?= $name ?>;
<?php endforeach ?>

<?php foreach ($model->getIncludes() as $include):?>
#include <<?= $include ?>>
<?php endforeach ?>

#include "<?= $model->getModuleName() ?>_properties.h"
<?php foreach ($model->getRequires() as $name => $module): ?>
#include "<?= substr($module->source, 0, -4) ?>.h"
<?php endforeach ?>

<?php foreach ($model->getEvents() as $name => $event):?>
ORCA_API extern struct MessageType <?= $name ?>Message;
<?php endforeach ?>

<?php foreach ($model->getEvents() as $name => $event):?>
<?php if ($event->hasAnyFields()):?>
typedef struct <?= $event->getEffectiveStructName() ?> <?= $name ?>Msg_t,* <?= $name ?>MsgPtr;
<?php else:?>
typedef <?= $event->getEffectiveTypeDecl() ?> <?= $name ?>Msg_t,* <?= $name ?>MsgPtr;
<?php endif ?>
<?php endforeach ?>

<?php foreach ($model->getEnums() as $name => $enum): ?>
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

<?php foreach ($model->getStructs() as $name => $struct): ?>
typedef struct <?= $name ?> <?= $name ?>_t, *lp<?= $name ?>_t;
typedef struct <?= $name ?> const c<?= $name ?>_t, *lpc<?= $name ?>_t;
<?php endforeach ?>

<?php foreach ($model->getFunctions() as $name => $func):?>
<?php if ($func->doc): ?>

/// @brief <?= $func->doc ?>
<?php endif ?>
ORCA_API <?= $func->getReturnType() ?>
<?= $model->prefix.$name ?>(<?= implode(', ', $func->getArgsTypes()) ?>);
<?php endforeach ?>

<?php foreach ($model->getInterfaces() as $name => $interface): ?>
<?php foreach ($interface->getTopics() as $topicTitle => $topicData): ?>
<?php $topicMethods = isset($topicData["methods"]) ? $topicData["methods"] : []; ?>
<?php $topicDesc = isset($topicData["desc"]) ? $topicData["desc"] : ''; ?>
<?php if ($topicTitle): ?>

/// @name <?= $topicTitle ?>
<?php if ($topicDesc): ?>
/// <?= $topicDesc ?>
<?php endif ?>
<?php endif ?>
<?php foreach ($topicMethods as $method_name => $method): ?>
<?php if ($method->doc): ?>

/// @brief <?= $method->doc ?>
<?php endif ?>
ORCA_API <?= $method->getReturnType() ?>
<?= $method->full_name ?>(<?= implode(', ', $method->getArgsTypes()) ?>);
<?php endforeach ?>
<?php endforeach ?>
<?php endforeach ?>

<?php foreach ($model->getStructs() as $name => $struct): ?>
<?php if ($struct->doc): ?>
/// @brief <?= $struct->doc ?>
<?php endif ?>
/** <?= $name ?> struct */
struct <?= $name ?> {
<?php include_template("struct_contents", ['list' => $struct->getFields()]) ?>
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

<?php foreach ($model->getEvents() as $name => $event):?>
<?php if ($event->isPureAlias() || !$event->hasAnyFields()) continue; ?>
/** <?= $name ?>MsgArgs struct */
struct <?= $name ?>MsgArgs {
<?php include_template("struct_contents", ['list' => $event->getAllFields()]) ?>
};
ORCA_API void luaX_push<?= $name ?>MsgArgs(lua_State *L, struct <?= $name ?>MsgArgs const* data);
ORCA_API struct <?= $name ?>MsgArgs* luaX_check<?= $name ?>MsgArgs(lua_State *L, int idx);
<?php endforeach ?>

<?php foreach ($model->getComponents() as $name => $component): ?>
<?php if ($component->doc): ?>

/// @brief <?= $component->doc ?>
<?php endif ?>
/** <?= $name ?> component */
typedef struct <?= $name ?> <?= $name ?>_t, *<?= $name ?>Ptr, *lp<?= $name ?>_t;
typedef struct <?= $name ?> const *<?= $name ?>CPtr, *lpc<?= $name ?>_t;
struct <?= $name ?> {
<?php include_template("struct_contents", ['list' => $component->getProperties(false)]) ?>
<?php include_template("struct_contents", ['list' => $component->getFields()]) ?>
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

#endif