<?php

require "model/config.php";

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
		$name = $this->getPath();
		foreach (config::Axis as $pair) {
			$pat = $pair[0];
			$repl = $pair[1];
			$pattern = '/' . $pat . '/';
			if (preg_match($pattern, $name)) {
				return str_replace('.', '', preg_replace($pattern, $repl, $name, 1));
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

// --- Base ---

class Base {
	protected $_elem;
	protected $_model;
	public $name;

	function __construct($elem, $model) {
		$this->_elem = $elem;
		$this->_model = $model;
		foreach ($elem->attributes() as $k => $v) {
			$this->$k = $v;
		}
	}
}

// --- Type ---

class Type extends Base {
	public $type;
	public $kind;
	public $data;
	public $fixed_array;
	public $export;
	public $default;
	public $const = null;
	public $pointer = null;

	function __construct($elem, $model) {
		parent::__construct($elem, $model);
		$this->type = $elem["type"];
		[$this->kind, $this->data] = $model->getKind($this->type);
		$fa = $elem["fixed-array"];
		$this->fixed_array = $fa !== null ? intval($fa) : null;
		$this->export = ($this->kind === "struct" && $this->data) ? $this->data->export : $this->type;
		$this->default = $elem["default"];
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
		if ($this->const) {
			$base .= " const";
		}
		if ($this->pointer) {
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
	public $args;
	public $static;
	public $returns;
	public $full_name;

	function __construct($elem, $model, $owner = null) {
		parent::__construct($elem, $model);
		$this->args = [];
		foreach ($elem->xpath("arg") as $arg) {
			$this->args[] = [$arg["name"], new Type($arg, $model)];
		}
		$this->static = $elem["static"];
		if ($owner !== null && !$this->static) {
			$wrapper = simplexml_load_string('<args/>');
			$thisElem = $wrapper->addChild("arg");
			$thisElem->addAttribute("name", "this_");
			$thisElem->addAttribute("type", $owner["name"]);
			$thisElem->addAttribute("pointer", "1");
			$const = $elem["const"];
			if ($const) {
				$thisElem->addAttribute("const", $const);
			}
			array_unshift($this->args, ["this_", new Type($thisElem, $model)]);
		}
		$rets = $elem->xpath("returns");
		$this->returns = count($rets) > 0 ? new Type($rets[0], $model) : null;
		$prefix = $owner !== null ? ($owner["prefix"] ?? "") : "";
		$this->full_name = $prefix . $elem["name"];
	}

	function getReturnType() { return $this->returns ?? "void"; }

	function getArgs() {
		foreach ($this->args as $pair) {
			yield $pair[0] => $pair[1];
		}
	}

	function getArgsTypes() {
		foreach ($this->args as $pair) {
			yield $pair[1];
		}
	}
}

// --- Struct ---

class Struct extends Base {
	public $sealed;
	public $export;
	public $prefix;

	function __construct($elem, $model) {
		parent::__construct($elem, $model);
		$this->sealed = $elem["sealed"] === "true";
		$this->export = $elem["export"] ?? $elem["name"];
		$this->prefix = $elem["prefix"] ?? $elem["name"];
	}

	function getFields() {
		foreach ($this->_elem->xpath(".//field[@name]") as $f) {
			yield new FieldName($f["name"]) => new Type($f, $this->_model);
		}
	}

	function getMethods() {
		foreach ($this->_elem->xpath(".//method[@name]") as $m) {
			yield $m["name"] => new Method($m, $this->_model, $this->_elem);
		}
	}

	function getParsers() {
		$result = dict();
		foreach ($this->getFields() as $name => $field) {
			if ($field->fixed_array) {
				for ($i = 0; $i < $field->fixed_array; $i++) {
					if ($field->kind === "struct") {
						foreach ($field->data->getFields() as $sub_name => $sub_type) {
							$pt_name = $this->name . "_{$name}{$i}_{$sub_name}";
							$pt_addr = "{$name}[$i].{$sub_name}";
							$pt = new ParserType($pt_name, $pt_addr, $sub_type);
							$result[$pt] = $sub_type;
						}
					} else {
						$pt_name = $this->name . "_{$name}{$i}";
						$pt_addr = "{$name}[$i]";
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
		$ctor = $this->_elem["constructor"];
		if ($ctor) {
			foreach (explode(',', $ctor) as $n) {
				yield intval(trim($n));
			}
		}
	}
}

// --- Component ---

class Component extends Struct {
	function __construct($elem, $model) {
		parent::__construct($elem, $model);
	}

	private function _walkProperties($type_, $args) {
		$path = array_slice($args, 1);
		yield new PropertyName($args[0], $path) => $type_;
		if ($type_->kind === "struct" && !$type_->data->sealed) {
			foreach ($type_->data->getFields() as $k => $v) {
				if ($v->fixed_array) {
					for ($i = 0; $i < $v->fixed_array; $i++) {
						$new_seg = "{$k}[$i]";
						yield from $this->_walkProperties($v, array_merge($args, [$new_seg]));
					}
				} else {
					yield from $this->_walkProperties($v, array_merge($args, [(string)$k]));
				}
			}
		}
	}

	function getProperties() {
		foreach ($this->_elem->xpath(".//property[@name]") as $f) {
			$type_ = new Type($f, $this->_model);
			yield from $this->_walkProperties($type_, [$this->name, $f["name"]]);
		}
	}

	function getEventHandlers() {
		foreach ($this->_elem->xpath("handles") as $node) {
			yield $node["event"];
		}
	}
}

// --- Enum ---

class Enum extends Base {
	function __construct($elem, $model) {
		parent::__construct($elem, $model);
	}

	function getValues() {
		foreach ($this->_elem->xpath(".//enum[@name]") as $e) {
			yield $e["name"] => (string)$e;
		}
	}

	function getValuesNames() {
		foreach ($this->_elem->xpath(".//enum[@name]") as $e) {
			yield $e["name"];
		}
	}
}

// --- Resource ---

class Resource extends Base {
	function __construct($elem, $model) {
		parent::__construct($elem, $model);
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
		$xml = simplexml_load_file($xml_file);
		$this->root = $xml;
		$this->source = $include_file ?? $xml_file;
		$this->requires = [];
		foreach ($xml->xpath("require") as $r) {
			$path = dirname($xml_file).'/'.$r["file"];
			$this->requires[] = new Model($path, $r["file"]);
		}
		$sn = $xml->xpath(".//struct[@name]");
		$this->structs = array_combine(
		array_map(fn($s) => $s["name"], $sn),
		array_map(fn($s) => new Struct($s, $this), $sn)
		);
		$en = $xml->xpath(".//enums[@name]");
		$this->enums = array_combine(
		array_map(fn($e) => $e["name"], $en),
		array_map(fn($e) => new Enum($e, $this), $en)
		);
		$cn = $xml->xpath(".//class[@name]");
		$this->components = array_combine(
		array_map(fn($c) => $c["name"], $cn),
		array_map(fn($c) => new Component($c, $this), $cn)
		);
		$rn = $xml->xpath(".//resource[@type]");
		$this->resources = array_combine(
		array_map(fn($r) => $r["type"], $rn),
		array_map(fn($r) => new Resource($r, $this), $rn)
		);
		$this->on_luaopen = $xml["on-luaopen"];
	}

	private function _has_in($key, $attr_name) {
		$map = $this->$attr_name;
		if (isset($map[$key])) {
			return $map[$key];
		}
		foreach ($this->requires as $req) {
			$result = $req->_has_in($key, $attr_name);
			if ($result) {
				return $result;
			}
		}
		return null;
	}

	function getModuleName() { return $this->root["name"]; }

	function getStruct($name) { return $this->structs[$name] ?? null; }

	function getEnum($name) { return $this->enums[$name] ?? null; }

	function getComponent($name) { return $this->components[$name] ?? null; }

	function getResource($resource_type) { return $this->resources[$resource_type] ?? null; }

	function getStructs() { return $this->structs; }

	function getEnums() { return $this->enums; }

	function getComponents() { return $this->components; }

	function getResources() { return $this->resources; }

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
		foreach ($this->requires as $r) {
			yield $r->getModuleName() => $r;
		}
	}
}

