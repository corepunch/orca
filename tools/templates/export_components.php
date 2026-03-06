<?php foreach ($model->getComponents() as $cname => $class):?>
<?php foreach ($class->getHandles() as $event):?>
LRESULT <?= $cname ?>_<?= $event ?>(lpObject_t, lp<?= $cname ?>_t, wParam_t, <?= $event ?>EventPtr);
<?php endforeach ?>
static struct PropertyType const <?= $cname ?>Properties[k<?= $cname ?>NumProperties] = {
<?php foreach ($class->getPropertyDecls() as $decl):?>
<?= $decl->getDecl() ?>
<?php endforeach ?>
};
<?php if ($class->hasDefaultProperties()):?>
static struct <?= $cname ?> <?= $cname ?>Defaults = {
<?php foreach ($class->getDefaultPropertyValues() as $dp):?>
.<?= $dp->name ?> = <?= formatDefault($dp->type_str, $dp->default, $model) ?>,
<?php endforeach ?>
};
<?php else:?>
static struct <?= $cname ?> <?= $cname ?>Defaults = {0};
<?php endif?>
LRESULT <?= $cname ?>Proc(lpObject_t object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
switch (message) {
<?php foreach ($class->getHandles() as $event):?>
case kEvent<?= $event ?>: return <?= $cname ?>_<?= $event ?>(object, cmp, wparm, lparm); // <?= $event ?>
<?php endforeach ?>
}
return FALSE;
}
void luaX_push<?= $cname ?>(lua_State *L, lpc<?= $cname ?>_t <?= $cname ?>) {
luaX_pushObject(L, CMP_GetObject(<?= $cname ?>));
}
lp<?= $cname ?>_t luaX_check<?= $cname ?>(lua_State *L, int idx) {
return Get<?= $cname ?>(luaX_checkObject(L, idx));
}
<?php foreach ($class->getParents() as $parent):?>
extern struct ClassDesc _<?= $parent ?>;
<?php endforeach ?>
ORCA_API struct ClassDesc _<?= $cname ?> = {
.ClassName = "<?= $cname ?>",
.DefaultName = "<?= $class->getDefaultName() ?>",
.ContentType = "<?= $class->getContentType() ?>",
.Xmlns = "<?= $class->getXmlns() ?>",
.ParentClasses = {<?php foreach ($class->getParents() as $parent):?>&_<?= $parent ?>, <?php endforeach ?>NULL},
.ClassID = ID_<?= $cname ?>,
.ClassSize = sizeof(struct <?= $cname ?>),
.Properties = <?= $cname ?>Properties,
.ObjProc = <?= $cname ?>Proc,
.Defaults = &<?= $cname ?>Defaults,
.NumProperties = k<?= $cname ?>NumProperties,
};
<?php endforeach ?>
