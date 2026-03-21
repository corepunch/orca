<?php foreach ($enums as $name => $enum):?>
ORCA_API const char *_<?= $name ?>[] = {"<?= implode('","', $enum->getValuesNames()) ?>",NULL};
const char *<?= $name ?>ToString(enum <?= $name ?> value) {
	assert(value >= 0 && value < <?= count($enum->getValuesNames()) ?>);
	return _<?= $name ?>[value];
}
enum <?= $name ?> luaX_check<?= $name ?>(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _<?= $name ?>);
}
void luaX_push<?= $name ?>(lua_State *L, enum <?= $name ?> value) {
	assert(value >= 0 && value < <?= count($enum->getValuesNames()) ?>);
	lua_pushstring(L, _<?= $name ?>[value]);
}
<?php endforeach ?>