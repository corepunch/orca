<?php

class Kind {
	const ATOMIC    = "atomic";
	const ENUM      = "enum";
	const STRUCT    = "struct";
	const COMPONENT = "component";
	const RESOURCE  = "resource";
	const FIXED     = "fixed";
	const UNKNOWN   = "unknown";
}

$Axis = [
	['/(.+)\.Axis\[0\]\.Left(.*)/',              '$1Left$2'],
	['/(.+)\.Axis\[0\]\.Right(.*)/',             '$1Right$2'],
	['/(.+)\.Axis\[1\]\.Left(.*)/',              '$1Top$2'],
	['/(.+)\.Axis\[1\]\.Right(.*)/',             '$1Bottom$2'],
	['/(.+)\.Axis\[2\]\.Left(.*)/',              '$1Front$2'],
	['/(.+)\.Axis\[2\]\.Right(.*)/',             '$1Back$2'],
	['/Size\.Axis\[0\]\.(?:Requested)?(.*)/',    '$1Width'],
	['/Size\.Axis\[1\]\.(?:Requested)?(.*)/',    '$1Height'],
	['/Size\.Axis\[2\]\.(?:Requested)?(.*)/',    '$1Depth'],
	['/(.+)\.Axis\[0\]/',                        'Horizontal$1'],
	['/(.+)\.Axis\[1\]/',                        'Vertical$1'],
	['/(.+)\.Axis\[2\]/',                        'Depth$1'],
	['/Border\.Radius\.(.+)Radius/',             'Border$1Radius'],
];

$atomic_types = [
	'float'  => ['luaL_checknumber',  'lua_pushnumber'],
	'int'    => ['luaL_checknumber',  'lua_pushnumber'],
	'uint'   => ['luaL_checknumber',  'lua_pushnumber'],
	'long'   => ['luaL_checkinteger', 'lua_pushinteger'],
	'bool'   => ['lua_toboolean',     'lua_pushboolean'],
	'string' => ['luaL_checkstring',  'lua_pushstring'],
	'fixed'  => ['luaL_checkstring',  'lua_pushstring'],
	'handle' => ['lua_touserdata',    'lua_pushlightuserdata'],
];

$printers = [
	Kind::ATOMIC    => '%s',
	Kind::ENUM      => 'enum %s',
	Kind::STRUCT    => 'struct %s',
	Kind::COMPONENT => 'struct %s',
	Kind::RESOURCE  => 'struct %s',
	Kind::FIXED     => '%sString_t',
	Kind::UNKNOWN   => '%s_t',
];

class Model {
	public $root;
	public $source;
	public $requires;
	public $structs;
	public $enums;
	public $components;
	public $resources;

	public function __construct($xml_file, $include_file = null) {
		$this->root = simplexml_load_file($xml_file);
		$this->source = $include_file;
		$dir = dirname($xml_file);
		$this->requires = [];
		foreach ($this->root->xpath('require') as $req) {
			$file = (string)$req['file'];
			$this->requires[] = new Model($dir . '/' . $file, $file);
		}
		$this->structs = [];
		foreach ($this->root->xpath('.//struct[@name]') as $s) {
			$this->structs[(string)$s['name']] = new Struct($s, $this);
		}
		$this->enums = [];
		foreach ($this->root->xpath('.//enums[@name]') as $e) {
			$this->enums[(string)$e['name']] = new XmlEnum($e, $this);
		}
		$this->components = [];
		foreach ($this->root->xpath('.//class[@name]') as $c) {
			$this->components[(string)$c['name']] = new Component($c, $this);
		}
		$this->resources = [];
		foreach ($this->root->xpath('.//resource[@type]') as $c) {
			$this->resources[(string)$c['type']] = new Resource($c, $this);
		}
	}

	private function _has_in($key, $attr_name, &$visited = null) {
		if ($visited === null) $visited = [];
		$id = spl_object_id($this);
		if (in_array($id, $visited, true)) return null;
		$visited[] = $id;
		$dict = $this->$attr_name;
		if (isset($dict[$key])) return $dict[$key];
		foreach ($this->requires as $req) {
			$result = $req->_has_in($key, $attr_name, $visited);
			if ($result !== null) return $result;
		}
		return null;
	}

	public function getModuleName() { return (string)$this->root['name']; }
	public function getStruct($name) { return $this->structs[$name] ?? null; }
	public function getEnum($name) { return $this->enums[$name] ?? null; }
	public function getComponent($name) { return $this->components[$name] ?? null; }
	public function getResource($resource_type) { return $this->resources[$resource_type] ?? null; }
	public function getStructs() { return $this->structs; }
	public function getEnums() { return $this->enums; }
	public function getComponents() { return $this->components; }
	public function getResources() { return $this->resources; }

	public function getKind($type) {
		global $atomic_types;
		if ($type === 'fixed') return [Kind::FIXED, null];
		if (isset($atomic_types[$type])) return [Kind::ATOMIC, null];
		$r = $this->_has_in($type, 'enums');
		if ($r !== null) return [Kind::ENUM, $r];
		$r = $this->_has_in($type, 'structs');
		if ($r !== null) return [Kind::STRUCT, $r];
		$r = $this->_has_in($type, 'components');
		if ($r !== null) return [Kind::COMPONENT, $r];
		$r = $this->_has_in($type, 'resources');
		if ($r !== null) return [Kind::RESOURCE, $r];
		return [Kind::UNKNOWN, null];
	}

	public function getRequires() {
		foreach ($this->requires as $r) {
			yield $r->getModuleName() => $r;
		}
	}
}

class Base {
	protected $_element;
	protected $_model;
	private $_attrs = [];

	public function __construct($element, $model) {
		$this->_element = $element;
		$this->_model = $model;
		foreach ($element->attributes() as $k => $v) {
			$this->_attrs[(string)$k] = (string)$v;
		}
	}

	public function __get($name) {
		return $this->_attrs[$name] ?? null;
	}

	public function __set($name, $value) {
		$this->_attrs[$name] = $value;
	}

	public function __isset($name) {
		return isset($this->_attrs[$name]);
	}

	public function getName() { return $this->_attrs['name'] ?? null; }
	public function getAttribute($key) { return $this->_attrs[$key] ?? null; }
	public function getAttributes() { return $this->_attrs; }
}

class Type extends Base {
	public $kind;
	public $data;
	public $fixed_array;

	public function __construct($element, $model) {
		parent::__construct($element, $model);
		[$this->kind, $this->data] = $model->getKind($this->type);
		$fa = isset($element['fixed-array']) ? (string)$element['fixed-array'] : null;
		$this->fixed_array = ($fa !== null && $fa !== '') ? (int)$fa : null;
	}

	public function __toString() {
		global $printers;
		$base = sprintf($printers[$this->kind] ?? '%s', $this->type);
		if ($this->getAttribute('const')) $base .= ' const';
		if ($this->getAttribute('pointer')) $base .= '*';
		return $base;
	}
}

class Method extends Base {
	public $args;
	public $return_type;
	public $full_name;

	public function __construct($element, $model, $owner = null) {
		parent::__construct($element, $model);
		$this->args = [];
		foreach ($element->xpath('arg') as $arg) {
			$this->args[] = [(string)$arg['name'], new Type($arg, $model)];
		}
		if ($owner !== null && !$this->getAttribute('static')) {
			$ownerName = (string)$owner['name'];
			$thisArg = new SimpleXMLElement("<arg name=\"this\" type=\"{$ownerName}\" pointer=\"true\"/>");
			array_unshift($this->args, ['this', new Type($thisArg, $model)]);
		}
		$returns = $element->xpath('returns');
		$this->return_type = !empty($returns) ? new Type($returns[0], $model) : 'void';
		$prefix = $owner !== null ? ((string)($owner['prefix'] ?? '')) : '';
		$this->full_name = $owner !== null ? $prefix . $this->getName() : $this->getName();
	}

	public function getArgs() {
		foreach ($this->args as [$name, $type]) {
			yield $name => $type;
		}
	}

	public function getArgsTypes() {
		$types = [];
		foreach ($this->args as [, $type]) {
			$types[] = $type;
		}
		return $types;
	}
}

class Struct extends Base {
	public $sealed;

	public function __construct($element, $model) {
		parent::__construct($element, $model);
		$this->sealed = ((string)($element['sealed'] ?? '')) === 'true';
	}

	public function getFields() {
		foreach ($this->_element->xpath('.//field[@name]') as $f) {
			yield (string)$f['name'] => new Type($f, $this->_model);
		}
	}

	public function getMethods() {
		foreach ($this->_element->xpath('.//method[@name]') as $m) {
			yield (string)$m['name'] => new Method($m, $this->_model, $this->_element);
		}
	}
}

class PropertyName {
	public $classname;
	public $path;

	public function __construct($classname, ...$args) {
		$this->classname = $classname;
		$this->path = $args;
	}

	public function __toString() { return $this->format(); }
	public function getPath() { return implode('.', $this->path); }

	public function format() {
		global $Axis;
		$name = $this->getPath();
		foreach ($Axis as [$pat, $repl]) {
			if (preg_match($pat, $name)) {
				return str_replace('.', '', preg_replace($pat, $repl, $name, 1));
			}
		}
		return str_replace('.', '', $name);
	}
}

class Component extends Struct {
	public function __construct($element, $model) {
		parent::__construct($element, $model);
	}

	public function getProperties() {
		$walk = function($type, ...$args) use (&$walk) {
			yield new PropertyName(...$args) => $type;
			if ($type->kind === Kind::STRUCT && !$type->data->sealed) {
				foreach ($type->data->getFields() as $k => $v) {
					if ($v->fixed_array !== null) {
						for ($i = 0; $i < $v->fixed_array; $i++) {
							$newArgs = array_merge($args, ["{$k}[{$i}]"]);
							yield from $walk($v, ...$newArgs);
						}
					} else {
						$newArgs = array_merge($args, [$k]);
						yield from $walk($v, ...$newArgs);
					}
				}
			}
		};

		foreach ($this->_element->xpath('.//property[@name]') as $f) {
			$type = new Type($f, $this->_model);
			yield from $walk($type, $this->getName(), (string)$f['name']);
		}
	}
}

class XmlEnum extends Base {
	public function __construct($element, $model) {
		parent::__construct($element, $model);
	}

	public function getValues() {
		foreach ($this->_element->xpath('.//enum[@name]') as $e) {
			yield (string)$e['name'] => (string)$e;
		}
	}
}

class Resource extends Base {
	public function __construct($element, $model) {
		parent::__construct($element, $model);
	}
}
