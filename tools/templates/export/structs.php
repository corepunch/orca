extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(lua_State *L, const char* str, struct PropertyType const* prop, void* struct_ptr);
<?php foreach ($structs as $name => $struct):?>
<?php include_template("export/functions", ['functions' => $struct->getMethods(), 'prefix' => $struct->prefix]) ?>
static struct PropertyType _<?= $name ?>[] = {
	<?php include_template("export/properties", ['properties' => $struct->getFields(true), 'name' => $name]) ?>
};
static luaL_Reg _<?= $name ?>_Methods[] = {
<?php foreach ($struct->getMethods() as $method_name => $method):?>
	{ "<?= $method->export ?>", f_<?= $struct->prefix.$method_name ?> },
<?php endforeach ?>
	{ NULL, NULL }
};
<?php endforeach ?>

<?php foreach ($structs as $name => $struct):?>
STRUCT(<?= $name ?>, <?= $struct->export ?>);
<?php endforeach ?>
