<?php

require "model/config.php";

// --- FieldName ---

class FieldName {
	public $name;
	public $id;
	public $addr;

	function __construct($name) {
		$this->name = $name;
		$this->id = "0x" . hash("fnv1a32", $name);
		$this->addr = $name;
	}

	function __toString() {
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
		$this->id = "0x" . hash("fnv1a32", $this);
	}

	function __toString() {
		$name = implode('.', array_map('ucfirst', $this->path));
		foreach (config::$Axis as $pair) {
			$pat = $pair[0];
			$repl = $pair[1];
			$pattern = '/' . $pat . '/';
			if (preg_match($pattern, $name)) {
				return str_replace(['[',']','.'], '', preg_replace($pattern, $repl, $name, 1));
			}
		}
		return str_replace(['[',']','.'], '', $name);
	}

	function getPath() {
		return implode('.', $this->path);
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

	function __toString() {
		return $this->name;
	}
}

// --- Field ---

class Field {
	public $name;
	public $type;
	public $doc;
	public $noexport;

	function __construct($elem, $model) {
		$this->name = new FieldName($elem["name"]);
		$this->type = new Type($elem, $model);
		$text = trim((string)$elem);
		$this->doc = strlen($text) > 0 ? $text : null;
		$this->noexport = isset($elem["noexport"]) ? strval($elem["noexport"]) === "true" : false;
	}
}

// --- Property ---

class Property {
	public $name;
	public $type;
	public $doc;

	function __construct($elem, $model, $classname) {
		if ($elem === null) { return; }
		$this->name = new PropertyName($classname, [(string)$elem["name"]]);
		$this->type = new Type($elem, $model);
		$text = trim((string)$elem);
		$this->doc = strlen($text) > 0 ? $text : null;
	}
}

// --- Base ---

class Base {
	protected $_elem;
	protected $_model;
	public $name;
	public $doc;

	function __construct($elem, $model) {
		$this->_elem = $elem;
		$this->_model = $model;
		foreach ($elem->attributes() as $k => $v) {
			$this->$k = $v;
		}
		$summary = $elem->xpath("summary");
		$this->doc = count($summary) > 0 ? trim((string)$summary[0]) : null;
	}
}

// --- Type ---

class Type extends Base {
	public $type;
	public $kind;
	public $data;
	public $fixed_array;
	public $array;
	public $export;
	public $default;
	public $const = null;
	public $pointer = null;

	function __construct($elem, $model) {
		parent::__construct($elem, $model);
		$this->type = $elem["type"] ?? "void";
		$kind_data = $model->getKind($this->type);
		$this->kind = $kind_data[0];
		$this->data = $kind_data[1];
		$fa = $elem["fixed-array"];
		$this->fixed_array = $fa !== null ? intval($fa) : null;
		$this->array = $elem["array"] ?? null;
		$this->export = ($this->kind === "struct" && $this->data) ? $this->data->export : $this->type;
		$this->default = $elem["default"];
		$this->output_attr = $elem["output"] ? strval($elem["output"]) : "";
		$this->bufsize_attr = $elem["bufsize"] ? strval($elem["bufsize"]) : "";
	}

	function __toString() {
		$info = config::$TypeInfos[$this->kind] ?? [];
		$format = $info["decl"] ?? "%s_t";
		$base = sprintf($format, $this->type);
		if ($this->const) {
			$base .= " const";
		}
		if ($this->pointer || $this->array) {
			$base .= "*";
		}
		return $base;
	}

	function get($name, $arg = null, $addr = null) {
		if ($name === 'push' && $this->kind === 'struct' && $this->pointer) $arg = '*'.$arg;
		$template = config::$TypeInfos[$this->kind][$name];
		return str_replace(['{type}', '{arg}', '{addr}'], [$this->type, $arg, $addr], $template);
	}

	function getContainer() {
		$map = [
			"%s" => "fixedString_t",
			"%d" => "int",
			"%f" => "float",
			"%u" => "unsigned",
			"%ld" => "long",
		];
		$fmt = config::$TypeInfos[$this->kind]["format"] ?? null;
		return $map[$fmt] ?? null;
	}
}

// --- Method ---

class Method extends Base {
	public $args;
	public $static;
	public $returns;
	public $full_name;
	public $export;
	public $id;

	function __construct($elem, $model, $owner = null) {
		parent::__construct($elem, $model);
		$this->args = [];
		foreach ($elem->xpath("arg") as $arg) {
			$this->args[] = [$arg["name"], new Type($arg, $model)];
		}
		$this->export = $elem["export"] ?? lcfirst($elem["name"]);
		$this->static = $elem["static"] == 'true';
		$this->id = "0x" . hash("fnv1a32", (lcfirst($elem["name"])));
		if ($owner !== null && !$this->static) {
			$thisElem = simplexml_load_string("<arg/>");
			$thisElem->addAttribute("name", "this_");
			$thisElem->addAttribute("type", $owner["name"]);
			$thisElem->addAttribute("pointer", "true");
			$const = $elem["const"];
			if ($const) {
				$thisElem->addAttribute("const", $const);
			}
			array_unshift($this->args, ["this_", new Type($thisElem, $model)]);
		}
		if ($elem["lua"]== 'true') {
			$thisElem = simplexml_load_string("<arg/>");
			$thisElem->addAttribute("name", "L");
			$thisElem->addAttribute("type", "lua_State");
			$thisElem->addAttribute("pointer", "true");
			array_unshift($this->args, ["L", new Type($thisElem, $model)]);
		}
		$rets = $elem->xpath("returns");
		$this->returns = count($rets) > 0 ? new Type($rets[0], $model) : null;
		$prefix = $owner !== null ? ($owner["prefix"] ?? "") : "";
		$this->full_name = $prefix . $elem["name"];
	}

	function isMetaMethod() { return str_starts_with($this->export, "__"); }

	function getReturnType() { return $this->returns ?? "void"; }

	function getArgs() {
		foreach ($this->args as $pair) {
			yield $pair[0] => $pair[1];
		}
	}

	function getArgsTypes() {
		$returned = false;
		foreach ($this->args as $pair) {
			$returned = true;
			yield $pair[1];
		}
		if (!$returned) {
			yield "void";
		}
	}
}

// --- Interface ---

class Interface extends Base {
	function __construct($elem, $model) {
		parent::__construct($elem, $model);
		$this->prefix = $elem["prefix"] ?? $elem["name"];
	}

	function getMethods() {
		foreach ($this->_elem->xpath("./methods/method[@name]") as $m) {
			yield $m["name"] => new Method($m, $this->_model, $this->_elem);
		}
	}

	function getTopics() {
		$currentTitle = '';
		$currentDesc = '';
		$currentMethods = [];
		foreach ($this->_elem->xpath("methods/*") as $child) {
			$tag = $child->getName();
			if ($tag === "topic") {
				if (count($currentMethods) > 0) {
					yield $currentTitle => ["desc" => $currentDesc, "methods" => $currentMethods];
					$currentMethods = [];
				}
				$currentTitle = strval($child["title"]);
				$currentDesc = trim(strval($child));
			} elseif ($tag === "method" && isset($child["name"])) {
				$mname = strval($child["name"]);
				$currentMethods[$mname] = new Method($child, $this->_model, $this->_elem);
			}
		}
		if (count($currentMethods) > 0) {
			yield $currentTitle => ["desc" => $currentDesc, "methods" => $currentMethods];
		}
	}

	function hasTopics() {
		return count($this->_elem->xpath("methods/topic")) > 0;
	}

	function getMessages() {
		foreach ($this->_elem->xpath("./messages/message[@name]") as $f) {
			yield new Event($f, $this->_model);
		}
	}
}

// --- Struct ---

class Struct extends Interface {
	public $sealed;
	public $export;
	public $prefix;

	function __construct($elem, $model) {
		parent::__construct($elem, $model);
		$this->sealed = $elem["sealed"] === "true";
		$this->export = $elem["export"] ?? $elem["name"];
	}

	function getFields($recursive = false) {
		foreach ($this->_elem->xpath("./fields/field[@name]") as $f) {
			$type_ = new Type($f, $this->_model);
			$text = trim((string)$f);
			$doc = strlen($text) > 0 ? $text : null;
			if ($recursive) {
				yield from $this->_walkProperties($type_, [$this->name, (string)$f["name"]], $doc);
			} else {
				yield new Field($f, $this->_model);
			}
		}
	}
	
	function getParsers() {
		$result = dict();
		foreach ($this->getFields() as $field_obj) {
			$name = $field_obj->name;
			$field = $field_obj->type;
			if ($field_obj->noexport) continue;
			if ($field->fixed_array) {
				for ($i = 0; $i < $field->fixed_array; $i++) {
					if ($field->kind === "struct") {
						foreach ($field->data->getFields() as $sub_field) {
							$sub_name = $sub_field->name;
							$sub_type = $sub_field->type;
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
				$pt = new ParserType(strval($name), strval($name), $field);
				$result[$pt] = $field;
			}
		}
		return $result;
	}

	function hasFromString() {
		foreach ($this->getFields() as $field) {
			if ($field->noexport) continue;
			if ($field->type->kind == "struct" && $field->type->name != "color") {
				return false;
			}
		}
		return true;
	}

	function getConstructors() {
		$ctor = $this->_elem["constructor"];
		if ($ctor) {
			foreach (explode(',', $ctor) as $n) {
				yield intval(trim($n));
			}
		}
	}

	function _walkProperties($type_, $args, $doc = null, $unwrapped = false) {
		if ($type_->fixed_array && !$unwrapped) {
			for ($i = 0; $i < $type_->fixed_array; $i++) {
				$new_seg = strval($args[count($args) - 1]) . "[$i]";
				yield from $this->_walkProperties($type_, array_merge(array_slice($args, 0, -1), [$new_seg]), $doc, true);
			}
			return;
		}
		$path = array_slice($args, 1);
		$p = new Property(null, null, null);
		$p->name = new PropertyName($args[0], $path);
		if ($type_->kind === 'int' && $this->_model->_has_in((string)$p->name, "enums")) {
			$p->type = new Type(simplexml_load_string("<arg type='{$p->name}'/>"), $this->_model);
		} else {
			$p->type = $type_;
		}
		$p->doc = $doc;
		yield $p;
		if ($type_->kind === "struct" && !$type_->data->sealed) {
			foreach ($type_->data->getFields() as $f) {
				yield from $this->_walkProperties($f->type, array_merge($args, [strval($f->name)]));
			}
		}
	}
}

// --- Component ---

class Component extends Struct {
	function __construct($elem, $model) {
		parent::__construct($elem, $model);
		$this->extension = $elem["extension"] ?? null;
	}

	function getProperties($recursive = true) {
		foreach ($this->_elem->xpath("./properties/property[@name]") as $f) {
			$type_ = new Type($f, $this->_model);
			$text = trim((string)$f);
			$doc = strlen($text) > 0 ? $text : null;
			if ($recursive) {
				yield from $this->_walkProperties($type_, [$this->name, (string)$f["name"]], $doc);
			} else {
				yield new Property($f, $this->_model, $this->name);
			}
			if ($type_->array === "true") {
				$int = new Type(simplexml_load_string("<type type=\"int\"/>"), $this->_model);
				$p = new Property(null, null, null);
				$p->name = new PropertyName($this->name, ['Num' . $f["name"]]);
				$p->type = $int;
				$p->doc = null;
				yield $p;
			}
		}
	}

	function getEventHandlers() {
		foreach ($this->_elem->xpath("handles/handle") as $node) {
			yield $node["message"];
		}
	}

	function getParents() {
		if ($this->_elem["parent"]) {
			yield $this->_elem["parent"];
		}
		if ($this->_elem["concept"]) {
			yield $this->_elem["concept"];
		}
	}
}

// --- Enum ---

class Enum extends Base {
	function __construct($elem, $model) {
		parent::__construct($elem, $model);
	}

	function getValues() {
		foreach ($this->_elem->xpath("./value[@name]") as $e) {
			yield $e["name"] => (string)$e;
		}
	}

	function getValuesNames() {
		foreach ($this->_elem->xpath("./value[@name]") as $e) {
			yield $e["name"];
		}
	}
}

class Event extends Type {
	public $parent_name;

	function __construct($elem, $model) {
		parent::__construct($elem, $model);
		$same_as = $elem["same-as"];
		$parent = $elem["parent"];
		$p = $same_as ? $same_as : $parent;
		$routing = $elem["routing"] ?? "TunnelingBubbling";
		$this->parent_name = $p ? strval($p) : null;
		$this->routing = strval($routing);
	}

	function getParentEvent() {
		if (!$this->parent_name) return null;
		return $this->_model->resolveEvent($this->parent_name);
	}

	function hasFields() {
		return count($this->_elem->xpath("fields/field[@name]")) > 0;
	}

	// Returns true if this event has a parent but no own fields (pure type alias)
	function isPureAlias() {
		return $this->getParentEvent() && !$this->hasFields();
	}

	// Returns true if this event or any ancestor has inline fields
	function hasAnyFields() {
		if ($this->hasFields()) return true;
		$parent = $this->getParentEvent();
		return $parent ? $parent->hasAnyFields() : false;
	}

	function getFields() {
		foreach ($this->_elem->xpath("fields/field[@name]") as $f) {
			yield new Field($f, $this->_model);
		}
	}

	// Yields fields from the ancestor chain followed by own fields
	function getAllFields() {
		$parent = $this->getParentEvent();
		if ($parent) {
			yield from $parent->getAllFields();
		}
		yield from $this->getFields();
	}

	// Returns the C type declaration string (without *) for events without inline fields
	function getEffectiveTypeDecl() {
		if ($this->hasFields()) return "struct " . $this->name . "MsgArgs";
		$parent = $this->getParentEvent();
		if ($parent) return $parent->getEffectiveTypeDecl();
		if (strval($this) == "void") {
			return "struct " . $this->name . "MsgArgs";
		}
		return strval($this); // delegates to Type::__toString() for kind-based formatting
	}

	// Returns the MsgArgs struct name to alias when a child has no own fields
	// but the parent chain does have fields
	function getEffectiveStructName() {
		if ($this->hasFields()) return $this->name . "MsgArgs";
		$parent = $this->getParentEvent();
		return $parent ? $parent->getEffectiveStructName() : null;
	}
}

// --- Resource ---

class Resource extends Base {
	function __construct($elem, $model) {
		parent::__construct($elem, $model);
	}
}

class IncludeFile extends Base {
	public $file;
	function __construct($elem, $model) {
		parent::__construct($elem, $model);
		$this->file = $elem['file'];
	}
	function __toString() {
		return $this->file;
	}
}

class ExternalStruct extends Base {
	public $struct;
	function __construct($elem, $model) {
		parent::__construct($elem, $model);
		$this->struct = $elem['struct'];
	}
	function __toString() {
		return $this->struct;
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
	public $includes;
	public $events;
	public $interfaces;
	public $functions;
	public $external_structs;
	public $on_luaopen;

	function __construct($xml_file, $include_file = null) {
		$xml = simplexml_load_file($xml_file);
		$this->root = $xml;
		$this->source = $include_file ?? $xml_file;
		$this->requires = [];
		$this->prefix = $xml["prefix"] ?? ($xml["name"] . '_');
		foreach ($xml->xpath("require") as $r) {
			$path = dirname($xml_file).'/'.$r["file"];
			$this->requires[] = new Model($path, $r["file"]);
		}
		$rn = $xml->xpath(".//external[@struct]");
		$this->external_structs = array_combine(
		array_map(fn($r) => $r["struct"], $rn),
		array_map(fn($r) => new ExternalStruct($r, $this), $rn)
		);
		$sn = $xml->xpath("./structs/struct[@name]");
		$this->structs = array_combine(
		array_map(fn($s) => $s["name"], $sn),
		array_map(fn($s) => new Struct($s, $this), $sn)
		);
		$sn = $xml->xpath("./interfaces/interface[@name]");
		$this->interfaces = array_combine(
		array_map(fn($s) => $s["name"], $sn),
		array_map(fn($s) => new Interface($s, $this), $sn)
		);
		$en = $xml->xpath("./enums/enum[@name]");
		$this->enums = array_combine(
		array_map(fn($e) => $e["name"], $en),
		array_map(fn($e) => new Enum($e, $this), $en)
		);
		$cn = $xml->xpath("./classes/class[@name]");
		$this->components = array_combine(
		array_map(fn($c) => $c["name"], $cn),
		array_map(fn($c) => new Component($c, $this), $cn)
		);
		$rn = $xml->xpath(".//include[@file]");
		$this->includes = array_combine(
		array_map(fn($r) => $r["file"], $rn),
		array_map(fn($r) => new IncludeFile($r, $this), $rn)
		);
		$rn = $xml->xpath(".//message[@name]");
		$this->events = array_combine(
		array_map(fn($r) => $r["name"], $rn),
		array_map(fn($r) => new Event($r, $this), $rn)
		);
		$rn = $xml->xpath("./functions/function[@name]");
		$this->functions = array_combine(
		array_map(fn($r) => $r["name"], $rn),
		array_map(fn($r) => new Method($r, $this), $rn)
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
	function getInterface($name) { return $this->interfaces[$name] ?? null; }
	function getEnum($name) { return $this->enums[$name] ?? null; }
	function getComponent($name) { return $this->components[$name] ?? null; }
	function resolveEvent($name) { return $this->_has_in($name, "events"); }

	function getStructs() { return $this->structs; }
	function getInterfaces() { return $this->interfaces; }
	function getEnums() { return $this->enums; }
	function getComponents() { return $this->components; }
	function getIncludes() { return $this->includes; }
	function getEvents() { return $this->events; }
	function getFunctions() { return $this->functions; }
	function getExternalStructs() { return $this->external_structs; }

	function getKind($_type) {
		$r = $this->_has_in($_type, "enums");
		if ($r) {
			return ["enum", $r];
		}
		$r = $this->_has_in($_type, "structs");
		if ($r) {
			return ["struct", $r];
		}
		$r = $this->_has_in($_type, "interfaces");
		if ($r) {
			return ["interface", $r];
		}
		$r = $this->_has_in($_type, "components");
		if ($r) {
			return ["component", $r];
		}
		$r = $this->_has_in($_type, "external_structs");
		if ($r) {
			return ["external_struct", $r];
		}
		// Check if type is an event-generated args struct (e.g. "HandleMessageMsgArgs")
		if (str_ends_with($_type, "MsgArgs")) {
			$eventName = substr($_type, 0, -strlen("MsgArgs"));
			$ev = $this->_has_in($eventName, "events");
			if ($ev && $ev->hasFields()) {
				return ["struct", null];
			}
		}
		return [$_type, null];
	}

	function getRequires() {
		foreach ($this->requires as $r) {
			yield $r->getModuleName() => $r;
		}
	}
}

function include_template($template, $variables = []) {
	extract($variables);
	require "templates/$template.php";
}
