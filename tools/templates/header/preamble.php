#ifndef __<?= strtoupper($moduleName) ?>_H__
#define __<?= strtoupper($moduleName) ?>_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;

<?php foreach ($externalStructs as $name => $module): ?>
struct <?= $name ?>;
<?php endforeach ?>

<?php foreach ($includes as $include):?>
#include <<?= $include ?>>
<?php endforeach ?>

#include "<?= $moduleName ?>_properties.h"
<?php foreach ($requires as $name => $module): ?>
#include "<?= substr($module->source, 0, -4) ?>.h"
<?php endforeach ?>

<?php foreach ($events as $name => $event):?>
typedef <?= $event ?>* <?= $name ?>EventPtr;
<?php endforeach ?>

