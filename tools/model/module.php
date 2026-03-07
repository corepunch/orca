<?php

// Python module imports (pyphp maps use A\B as C -> import A.B as C)
use xml\etree\ElementTree as ET;
use os;
use config;

// --- Base ---

class Base {
	public $_element;
	public $_model;

	function __construct($element, $model) {
		$this->_element = $element;
		$this->_model = $model;
		foreach ($element->attrib as $k => $v) {
			setattr($this, $k, $v);
		}
	}

	function getName() {
		return $this->_element->get('name');
	}

	function getAttribute($key) {
		return $this->_element->get($key);
	}

	function getAttributes() {
		return $this->_element->attrib;
	}
}

// --- FieldName ---

class FieldName {
	public $name;
	public $id;

	function __construct($name) {
		$this->name = $name;
		$this->id = "0x" . hash("fnv1a32", $name);
	}

	function __str__() {
		return $this->name;
	}
}

// --- PropertyName ---

class PropertyName {
	public $classname;
	public $path;
	public $addr;
	public $id;

	// $path is an array of property-path parts (not including classname)
	function __construct($classname, $path) {
		$this->classname = $classname;
		$this->path = $path;
		$this->addr = implode('.', $path);
		$this->id = "0x" . hash("fnv1a32", implode('', $path));
	}

	function __str__() {
		return $this->format();
	}

	function getPath() {
		return implode('.', $this->path);
	}

	function format() {
		$axis = config::Axis;
		$name = $this->getPath();
		foreach ($axis as $pair) {
			$pat = $pair[0];
			$repl = $pair[1];
			$pattern = '/' . $pat . '/';
			if (preg_match($pattern, $name)) {
				$s2 = preg_replace($pattern, $repl, $name, 1);
				return str_replace('.', '', $s2);
			}
		}
		return str_replace('.', '', $name);
	}
}

// --- ParserType ---

class ParserType {
	public $name;
	public $addr;
	public $pointer;

	function __construct($name, $addr, $type_) {
		$this->name = $name;
		$this->addr = $addr;
		if ($type_->get("format") !== "%s") {
			$this->pointer = "&" . $name;
		} else {
			$this->pointer = $name;
		}
	}

	function __str__() {
		return $this->name;
	}
}

// --- Type ---

class Type extends Base {
	function __construct($element, $model) {
		parent::__construct($element, $model);
		$this->type = $element->get('type');
		[$this->kind, $this->data] = $model->getKind($this->type);
		$fa = $element->get("fixed-array");
		$this->fixed_array = $fa !== null ? intval($fa) : null;
		$this->export = ($this->kind === "struct" && $this->data) ? $this->data->export : $this->type;
		$this->default = $element->get("default");
	}

	function __str__() {
		$map = [
		"enum"      => "enum %s",
		"struct"    => "struct %s",
		"component" => "struct %s",
		"resource"  => "struct %s",
		"fixed"     => "%sString_t",
		];
		$format = $map[$this->kind] ?? "%s";
		$base = sprintf($format, $this->type);
		if (getattr($this, 'const', null)) {
			$base .= " const";
		}
		if (getattr($this, 'pointer', null)) {
			$base .= "*";
		}
		return $base;
	}

	function get($name, $arg = null, $addr = null) {
		$template = config::TypeInfos[$this->kind][$name];
		return str_replace(['{type}', '{arg}', '{addr}'], [$this->type, $arg, $addr], $template);
	}
}

// --- Method ---

class Method extends Base {
	function __construct($element, $model, $owner = null) {
		parent::__construct($element, $model);
		$this->args = [];
		foreach ($element->findall('arg') as $arg) {
			array_push($this->args, [$arg->get('name'), new Type($arg, $model)]);
		}
		$this->static = $element->get('static');
		if ($owner !== null && !$this->static) {
			$attrs = ["name" => "this_", "type" => $owner->get('name'), "pointer" => "1"];
			if ($element->get('const')) {
				$attrs["const"] = $element->get('const');
			}
			$thisElem = ET::Element("arg", $attrs);
			array_unshift($this->args, ["this_", new Type($thisElem, $model)]);
		}
		$returns = $element->find('returns');
		$this->returns = $returns !== null ? new Type($returns, $model) : null;
		$prefix = '';
		if ($owner) {
			$p = $owner->get('prefix');
			$prefix = $p !== null ? $p : '';
		}
		$base_name = $this->getName();
		if ($owner !== null) {
			$this->full_name = $prefix . $base_name;
		} else {
			$this->full_name = $base_name;
		}
	}

	function getReturnType() {
		if ($this->returns !== null) {
			return $this->returns;
		}
		return "void";
	}

	function getArgs() {
		$args = $this->args;
		foreach ($args as $pair) {
			yield $pair[0] => $pair[1];
		}
	}

	function getArgsTypes() {
		$args = $this->args;
		foreach ($args as $pair) {
			yield $pair[1];
		}
	}
}

// --- Struct ---

class Struct extends Base {
	function __construct($element, $model) {
		parent::__construct($element, $model);
		$this->sealed = $element->get('sealed') === "true";
		$this->export = $element->get('export') ?? $this->getName();
		$this->prefix = $element->get('prefix') ?? $this->getName();
	}

	function getFields() {
		$elem = $this->_element;
		foreach ($elem->findall(".//field[@name]") as $f) {
			yield new FieldName($f->get('name')) => new Type($f, $this->_model);
		}
	}

	function getMethods() {
		$elem = $this->_element;
		foreach ($elem->findall(".//method[@name]") as $m) {
			yield $m->get('name') => new Method($m, $this->_model, $this->_element);
		}
	}

	function getParsers() {
		$result = dict();
		$fields = $this->getFields();
		foreach ($fields as $name => $field) {
			if ($field->fixed_array) {
				for ($i = 0; $i < $field->fixed_array; $i++) {
					if ($field->kind === "struct") {
						$sub_fields = $field->data->getFields();
						foreach ($sub_fields as $sub_name => $sub_type) {
							$pt_name = $this->getName() . "_" . $name . $i . "_" . $sub_name;
							$pt_addr = $name . "[" . $i . "]." . $sub_name;
							$pt = new ParserType($pt_name, $pt_addr, $sub_type);
							$result[$pt] = $sub_type;
						}
					} else {
						$pt_name = $this->getName() . "_" . $name . $i;
						$pt_addr = $name . "[" . $i . "]";
						$pt = new ParserType($pt_name, $pt_addr, $field);
						$result[$pt] = $field;
					}
				}
			} else {
				$pt = new ParserType((string)$name, (string)$name, $field);
				$result[$pt] = $field;
			}
		}
		return $result;
	}

	function getConstructors() {
		$ctor = $this->_element->get("constructor");
		if ($ctor) {
			foreach (explode(',', $ctor) as $n) {
				yield intval(trim($n));
			}
		}
	}
}

// --- Component ---

class Component extends Struct {
	function __construct($element, $model) {
		parent::__construct($element, $model);
	}

	// Recursive generator: $args[0] is classname, remaining elements are the property path
	private function _walkProperties($type_, $args) {
		$path = array_slice($args, 1);
		yield new PropertyName($args[0], $path) => $type_;
		if ($type_->kind === "struct" && !$type_->data->sealed) {
			$fields = $type_->data->getFields();
			foreach ($fields as $k => $v) {
				if ($v->fixed_array) {
					for ($i = 0; $i < $v->fixed_array; $i++) {
						$new_seg = (string)$k . "[" . $i . "]";
						yield from $this->_walkProperties($v, array_merge($args, [$new_seg]));
					}
				} else {
					yield from $this->_walkProperties($v, array_merge($args, [(string)$k]));
				}
			}
		}
	}

	function getProperties() {
		$elem = $this->_element;
		foreach ($elem->findall(".//property[@name]") as $f) {
			$type_ = new Type($f, $this->_model);
			yield from $this->_walkProperties($type_, [$this->getName(), $f->get('name')]);
		}
	}

	function getEventHandlers() {
		$elem = $this->_element;
		foreach ($elem->findall("handles") as $node) {
			yield $node->get("event");
		}
	}
}

// --- Enum ---

class Enum extends Base {
	function __construct($element, $model) {
		parent::__construct($element, $model);
	}

	function getValues() {
		$elem = $this->_element;
		foreach ($elem->findall(".//enum[@name]") as $e) {
			yield $e->get('name') => $e->text;
		}
	}

	function getValuesNames() {
		$elem = $this->_element;
		foreach ($elem->findall(".//enum[@name]") as $e) {
			yield $e->get('name');
		}
	}
}

// --- Resource ---

class Resource extends Base {
	function __construct($element, $model) {
		parent::__construct($element, $model);
	}
}

// --- Model ---

class Model {
	public $root;
	public $source;
	public $requires;
	public $structs;
	public $enums;
	public $components;
	public $resources;
	public $on_luaopen;

	function __construct($xml_file, $include_file = null) {
		$tree = ET::parse($xml_file);
		$root = $tree->getroot();
		$this->root = $root;
		$this->source = $include_file !== null ? $include_file : $xml_file;
		$this->requires = [];
		foreach ($root->findall('require') as $req) {
			array_push($this->requires, new Model(
			os::path->join(os::path->dirname($xml_file), $req->get('file')),
			$req->get('file')
			));
		}
		$this->structs = dict();
		$struct_nodes = $root->findall(".//struct[@name]");
		foreach ($struct_nodes as $s) {
			$this->structs[$s->get('name')] = new Struct($s, $this);
		}
		$this->enums = dict();
		$enum_nodes = $root->findall(".//enums[@name]");
		foreach ($enum_nodes as $e) {
			$this->enums[$e->get('name')] = new Enum($e, $this);
		}
		$this->components = dict();
		$class_nodes = $root->findall(".//class[@name]");
		foreach ($class_nodes as $c) {
			$this->components[$c->get('name')] = new Component($c, $this);
		}
		$this->resources = dict();
		$resource_nodes = $root->findall(".//resource[@type]");
		foreach ($resource_nodes as $c) {
			$this->resources[$c->get('type')] = new Resource($c, $this);
		}
		$this->on_luaopen = $root->get('on-luaopen');
	}

	private function _has_in($key, $attr_name) {
		$map = getattr($this, $attr_name);
		if (isset($map[$key])) {
			return $map[$key];
		}
		$reqs = $this->requires;
		foreach ($reqs as $req) {
			$result = $req->_has_in($key, $attr_name);
			if ($result) {
				return $result;
			}
		}
		return null;
	}

	function getModuleName() {
		return $this->root->get('name');
	}

	function getStruct($name) {
		if (isset($this->structs[$name])) {
			return $this->structs[$name];
		}
		return null;
	}

	function getEnum($name) {
		if (isset($this->enums[$name])) {
			return $this->enums[$name];
		}
		return null;
	}

	function getComponent($name) {
		if (isset($this->components[$name])) {
			return $this->components[$name];
		}
		return null;
	}

	function getResource($resource_type) {
		if (isset($this->resources[$resource_type])) {
			return $this->resources[$resource_type];
		}
		return null;
	}

	function getStructs() {
		return $this->structs;
	}

	function getEnums() {
		return $this->enums;
	}

	function getComponents() {
		return $this->components;
	}

	function getResources() {
		return $this->resources;
	}

	function getKind($_type) {
		$r = $this->_has_in($_type, "enums");
		if ($r) {
			return ["enum", $r];
		}
		$r = $this->_has_in($_type, "structs");
		if ($r) {
			return ["struct", $r];
		}
		$r = $this->_has_in($_type, "components");
		if ($r) {
			return ["component", $r];
		}
		$r = $this->_has_in($_type, "resources");
		if ($r) {
			return ["resource", $r];
		}
		return [$_type, null];
	}

	function getRequires() {
		$reqs = $this->requires;
		foreach ($reqs as $r) {
			yield $r->getModuleName() => $r;
		}
	}
}

