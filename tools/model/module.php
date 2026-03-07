<?php

// ET (ElementTree) is available via PHP-in-Python bridge
// Usage assumes $et->parse(), Element objects with ->get(), ->findall(), ->attrib, etc.

class Base {
	protected $_element;
	protected $_model;

	function __construct($element, $model) {
		$this->_element = $element;
		$this->_model = $model;
		foreach ($element->attrib as $k => $v) {
			$this->$k = $v;
		}
	}

	function getName() { return $this->_element->get('name'); }
	function getAttribute($key) { return $this->_element->get($key); }
	function getAttributes() { return iterator_to_array($this->_element->attrib); }
}

class FieldName {
	public string $name;
	public string $id;

	function __construct(string $name) {
		$this->name = $name;
		$this->id = "0x" .hash("	", $name);
	}

	function __toString(): string { return $this->name; }
}

class PropertyName {
	public string $classname;
	public array $path;
	public string $addr;
	public string $id;

	function __construct(string $classname, string ...$args) {
		$this->classname = $classname;
		$this->path = $args;
		$this->addr = implode('.', $args);
		$this->id = "0x" . hash("	", implode('', $args));
	}

	function __toString(): string { return $this->format(); }

	function getPath(): string { return implode('.', $this->path); }

	function format(): string {
		global $config;
		$name = $this->getPath();
		foreach ($config->Axis as [$pat, $repl]) {
			$count = 0;
			$s2 = preg_replace('/' . $pat . '/', $repl, $name, 1, $count);
			if ($count > 0) {
				return str_replace('.', '', $s2);
			}
		}
		return str_replace('.', '', $name);
	}
}

class ParserType {
	public string $name;
	public string $addr;
	public string $pointer;

	function __construct(string $name, string $addr, Type $type_) {
		$this->name = $name;
		$this->addr = $addr;
		$this->pointer = ($type_->get("format") !== "%s") ? "&{$name}" : $name;
	}

	function __toString(): string { return $this->name; }
}

// --- Type ---

class Type extends Base {
	public string $kind;
	public $data;
	public ?int $fixed_array;
	public string $export;
	public ?string $default;
	protected string $type;

	function __construct($element, $model) {
		parent::__construct($element, $model);
		$this->type = $element->get('type');
		[$this->kind, $this->data] = $model->getKind($this->type);
		$fa = $element->get("fixed-array");
		$this->fixed_array = $fa !== null ? intval($fa) : null;
		$this->export = ($this->kind === "struct" && $this->data) ? $this->data->export : $this->type;
		$this->default = $element->get("default");
	}

	function __toString(): string {
		$map = [
			"enum"	  => "enum %s",
			"struct"	=> "struct %s",
			"component" => "struct %s",
			"resource"  => "struct %s",
			"fixed"	 => "%sString_t",
		];
		$format = $map[$this->kind] ?? "%s";
		$base = sprintf($format, $this->type);
		if (!empty($this->const))   { $base .= " const"; }
		if (!empty($this->pointer)) { $base .= "*"; }
		return $base;
	}

	function get(string $name, $arg = null, $addr = null): string {
		global $config;
		return str_replace(
			['{type}', '{arg}', '{addr}'],
			[$this->type, $arg, $addr],
			$config->TypeInfos[$this->kind][$name]
		);
	}
}

// --- Method ---

class Method extends Base {
	public array $args;
	public ?string $static;
	public ?Type $returns;
	public string $full_name;

	function __construct($element, $model, $owner = null) {
		parent::__construct($element, $model);
		$this->args = [];
		foreach ($element->findall('arg') as $arg) {
			$this->args[] = [$arg->get('name'), new Type($arg, $model)];
		}
		$this->static = $element->get('static');
		if ($owner !== null && !$this->static) {
			$thisElem = new SimpleXMLElement('<arg name="this_" type="' . $owner->get('name') . '" pointer="1"/>');
			if ($element->get('const')) {
				$thisElem->addAttribute('const', $element->get('const'));
			}
			array_unshift($this->args, ["this_", new Type($thisElem, $model)]);
		}
		$returns = $element->find('returns');
		$this->returns = $returns !== null ? new Type($returns, $model) : null;
		$prefix = $owner ? ($owner->get('prefix') ?? '') : '';
		$this->full_name = $owner !== null ? $prefix . $this->getName() : $this->getName();
	}

	function getReturnType() { return $this->returns ?? "void"; }

	function getArgs(): \Generator {
		foreach ($this->args as [$name, $type_]) {
			yield $name => $type_;
		}
	}

	function getArgsTypes(): \Generator {
		foreach ($this->args as [, $type_]) {
			yield $type_;
		}
	}
}

// --- Struct ---

class Struct extends Base {
	public bool $sealed;
	public string $export;
	public string $prefix;

	function __construct($element, $model) {
		parent::__construct($element, $model);
		$this->sealed = $element->get('sealed') === "true";
		$this->export = $element->get('export') ?? $this->getName();
		$this->prefix = $element->get('prefix') ?? $this->getName();
	}

	function getFields(): \Generator {
		foreach ($this->_element->findall(".//field[@name]") as $f) {
			yield new FieldName($f->get('name')) => new Type($f, $this->_model);
		}
	}

	function getMethods(): \Generator {
		foreach ($this->_element->findall(".//method[@name]") as $m) {
			yield $m->get('name') => new Method($m, $this->_model, $this->_element);
		}
	}

	function __iter__(): \Generator { return $this->getFields(); }

	function __get_item__(string $key): Type {
		foreach ($this->getFields() as $name => $type_) {
			if ((string)$name === $key) return $type_;
		}
		throw new \OutOfBoundsException($key);
	}

	function getParsers(): array {
		$result = [];
		foreach ($this->getFields() as $name => $field) {
			if ($field->fixed_array) {
				for ($i = 0; $i < $field->fixed_array; $i++) {
					if ($field->kind === "struct") {
						foreach ($field->data->getFields() as $sub_name => $sub_type) {
							$pt = new ParserType("{$this->getName()}_{$name}{$i}_{$sub_name}", "{$name}[{$i}].{$sub_name}", $sub_type);
							$result[(string)$pt] = [$pt, $sub_type];
						}
					} else {
						$pt = new ParserType("{$this->getName()}_{$name}{$i}", "{$name}[{$i}]", $field);
						$result[(string)$pt] = [$pt, $field];
					}
				}
			} else {
				$pt = new ParserType((string)$name, (string)$name, $field);
				$result[(string)$pt] = [$pt, $field];
			}
		}
		return $result;
	}

	function getConstructors(): \Generator {
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

	function getProperties(): \Generator {
		$walk = function(Type $type_, string ...$args) use (&$walk): \Generator {
			yield new PropertyName(...$args) => $type_;
			if ($type_->kind === "struct" && !$type_->data->sealed) {
				foreach ($type_->data->getFields() as $k => $v) {
					if ($v->fixed_array) {
						for ($i = 0; $i < $v->fixed_array; $i++) {
							yield from $walk($v, ...$args, "{$k}[{$i}]");
						}
					} else {
						yield from $walk($v, ...$args, (string)$k);
					}
				}
			}
		};

		foreach ($this->_element->findall(".//property[@name]") as $f) {
			$type_ = new Type($f, $this->_model);
			yield from $walk($type_, $this->getName(), $f->get('name'));
		}
	}

	function getEventHandlers(): \Generator {
		foreach ($this->_element->findall("handles") as $node) {
			yield $node->get("event");
		}
	}
}

// --- Enum ---

class Enum extends Base {
	function __construct($element, $model) {
		parent::__construct($element, $model);
	}

	function getValues(): \Generator {
		foreach ($this->_element->findall(".//enum[@name]") as $e) {
			yield $e->get('name') => $e->text;
		}
	}

	function getValuesNames(): \Generator {
		foreach ($this->_element->findall(".//enum[@name]") as $e) {
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
	public string $source;
	public array $requires;
	public array $structs;
	public array $enums;
	public array $components;
	public array $resources;
	public ?string $on_luaopen;

	function __construct(string $xml_file, ?string $include_file = null) {
		global $ET;
		$tree = $ET->parse($xml_file);
		$this->root = $tree->getroot();
		$this->source = $include_file ?? $xml_file;
		$this->requires = [];
		foreach ($tree->getroot()->findall('require') as $req) {
			$this->requires[] = new Model(
				dirname($xml_file) . DIRECTORY_SEPARATOR . $req->get('file'),
				$req->get('file')
			);
		}
		$this->structs	= [];
		foreach ($this->root->findall(".//struct[@name]") as $s) {
			$this->structs[$s->get('name')] = new Struct($s, $this);
		}
		$this->enums = [];
		foreach ($this->root->findall(".//enums[@name]") as $e) {
			$this->enums[$e->get('name')] = new Enum($e, $this);
		}
		$this->components = [];
		foreach ($this->root->findall(".//class[@name]") as $c) {
			$this->components[$c->get('name')] = new Component($c, $this);
		}
		$this->resources = [];
		foreach ($this->root->findall(".//resource[@type]") as $c) {
			$this->resources[$c->get('type')] = new Resource($c, $this);
		}
		$this->on_luaopen = $this->root->get('on-luaopen');
	}

	private function _has_in(string $key, string $attr_name) {
		$map = $this->$attr_name;
		if (isset($map[$key])) return $map[$key];
		foreach ($this->requires as $req) {
			$result = $req->_has_in($key, $attr_name);
			if ($result) return $result;
		}
		return null;
	}

	function getModuleName(): ?string { return $this->root->get('name'); }
	function getStruct(string $name): ?Struct { return $this->structs[$name] ?? null; }
	function getEnum(string $name): ?Enum { return $this->enums[$name] ?? null; }
	function getComponent(string $name): ?Component { return $this->components[$name] ?? null; }
	function getResource(string $resource_type): ?Resource { return $this->resources[$resource_type] ?? null; }
	function getStructs(): array { return $this->structs; }
	function getEnums(): array { return $this->enums; }
	function getComponents(): array { return $this->components; }
	function getResources(): array { return $this->resources; }

	function getKind(string $_type): array {
		if ($r = $this->_has_in($_type, "enums"))	  return ["enum",	  $r];
		if ($r = $this->_has_in($_type, "structs"))	return ["struct",	$r];
		if ($r = $this->_has_in($_type, "components")) return ["component", $r];
		if ($r = $this->_has_in($_type, "resources"))  return ["resource",  $r];
		return [$_type, null];
	}

	function getRequires(): \Generator {
		foreach ($this->requires as $r) {
			yield $r->getModuleName() => $r;
		}
	}
}