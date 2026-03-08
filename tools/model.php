<?php
// PHP implementation of the data model for XML module files.
// Equivalent to tools/model.py.

$ATOMIC_TYPES = [
    'float'  => ['luaL_checknumber',  'lua_pushnumber'],
    'int'    => ['luaL_checknumber',  'lua_pushnumber'],
    'uint'   => ['luaL_checknumber',  'lua_pushnumber'],
    'long'   => ['luaL_checkinteger', 'lua_pushinteger'],
    'bool'   => ['lua_toboolean',     'lua_pushboolean'],
    'string' => ['luaL_checkstring',  'lua_pushstring'],
    'fixed'  => ['luaL_checkstring',  'lua_pushstring'],
    'handle' => ['lua_touserdata',    'lua_pushlightuserdata'],
];

// Axis substitution rules used by propertyNameFormat().
$AXIS_PATTERNS = [
    ['/^(.+)\.Axis\[0\]\.Left(.*)$/',            '$1Left$2'],
    ['/^(.+)\.Axis\[0\]\.Right(.*)$/',           '$1Right$2'],
    ['/^(.+)\.Axis\[1\]\.Left(.*)$/',            '$1Top$2'],
    ['/^(.+)\.Axis\[1\]\.Right(.*)$/',           '$1Bottom$2'],
    ['/^(.+)\.Axis\[2\]\.Left(.*)$/',            '$1Front$2'],
    ['/^(.+)\.Axis\[2\]\.Right(.*)$/',           '$1Back$2'],
    ['/^Size\.Axis\[0\]\.(?:Requested)?(.*)$/',  '${1}Width'],
    ['/^Size\.Axis\[1\]\.(?:Requested)?(.*)$/',  '${1}Height'],
    ['/^Size\.Axis\[2\]\.(?:Requested)?(.*)$/',  '${1}Depth'],
    ['/^(.+)\.Axis\[0\]$/',                      'Horizontal$1'],
    ['/^(.+)\.Axis\[1\]$/',                      'Vertical$1'],
    ['/^(.+)\.Axis\[2\]$/',                      'Depth$1'],
    ['/^Border\.Radius\.(.+)Radius$/',           'Border$1Radius'],
];

function type_printer($kind, $type) {
    switch ($kind) {
        case 'atomic':    return $type;
        case 'enum':      return "enum $type";
        case 'struct':    return "struct $type";
        case 'component': return "struct $type";
        case 'resource':  return "struct $type";
        case 'fixed':     return "{$type}String_t";
        default:          return "{$type}_t";
    }
}

function propertyNameFormat($path) {
    global $AXIS_PATTERNS;
    foreach ($AXIS_PATTERNS as $item) {
        list($pattern, $replacement) = $item;
        $result = preg_replace($pattern, $replacement, $path, 1, $count);
        if ($count > 0) {
            return str_replace('.', '', $result);
        }
    }
    return str_replace('.', '', $path);
}

class Model {
    public $root;       // SimpleXMLElement
    public $source;     // XML file path (used for include directives)
    public $requires = [];
    public $structs = [];
    public $enums = [];
    public $components = [];
    public $resources = [];

    public function __construct($xml_file, $include_file = null) {
        $this->source = $include_file ?? $xml_file;
        libxml_use_internal_errors(true);
        $xml = simplexml_load_file($xml_file);
        if ($xml === false) {
            $errors = libxml_get_errors();
            $msg = count($errors) ? $errors[0]->message : 'unknown error';
            throw new RuntimeException("Failed to parse $xml_file: $msg");
        }
        $this->root = $xml;

        foreach ($xml->require as $req) {
            $dir = dirname($xml_file);
            $req_file = $dir . '/' . (string)$req['file'];
            $this->requires[] = new Model($req_file, (string)$req['file']);
        }

        foreach ($xml->xpath('.//struct[@name]') as $s) {
            $name = (string)$s['name'];
            $this->structs[$name] = new Struct($s, $this);
        }
        foreach ($xml->xpath('.//enums[@name]') as $e) {
            $name = (string)$e['name'];
            $this->enums[$name] = new EnumDef($e, $this);
        }
        foreach ($xml->xpath('.//class[@name]') as $c) {
            $name = (string)$c['name'];
            $this->components[$name] = new Component($c, $this);
        }
        foreach ($xml->xpath('.//resource[@type]') as $r) {
            $type = (string)$r['type'];
            $this->resources[$type] = new Resource($r, $this);
        }
    }

    public function getModuleName() {
        return (string)$this->root['name'];
    }

    public function getStructs()    { return $this->structs; }
    public function getEnums()      { return $this->enums; }
    public function getComponents() { return $this->components; }
    public function getResources()  { return $this->resources; }

    public function getRequires() {
        $result = [];
        foreach ($this->requires as $req) {
            $result[$req->getModuleName()] = $req;
        }
        return $result;
    }

    public function getKind($type) {
        global $ATOMIC_TYPES;
        if ($type === 'fixed') return ['fixed', null];
        if (isset($ATOMIC_TYPES[$type])) return ['atomic', null];
        $found = $this->_hasIn($type, 'enums');
        if ($found) return ['enum', $found];
        $found = $this->_hasIn($type, 'structs');
        if ($found) return ['struct', $found];
        $found = $this->_hasIn($type, 'components');
        if ($found) return ['component', $found];
        $found = $this->_hasIn($type, 'resources');
        if ($found) return ['resource', $found];
        return ['unknown', null];
    }

    public function _hasIn($key, $attr) {
        if (isset($this->$attr[$key])) return $this->$attr[$key];
        foreach ($this->requires as $req) {
            $result = $req->_hasIn($key, $attr);
            if ($result !== null) return $result;
        }
        return null;
    }
}

class Base {
    protected $_element;
    protected $_model;

    public function __construct($element, $model) {
        $this->_element = $element;
        $this->_model = $model;
    }

    public function getName() {
        return (string)$this->_element['name'];
    }

    public function getElement() {
        return $this->_element;
    }

    public function getAttribute($key) {
        $val = $this->_element[$key];
        return $val !== null ? (string)$val : null;
    }
}

class TypeDef extends Base {
    public $type;
    public $kind;
    public $data;
    public $fixed_array;
    public $export;
    public $const;
    public $pointer;

    public function __construct($element, $model) {
        parent::__construct($element, $model);
        $this->type    = (string)($element['type'] ?? '');
        list($this->kind, $this->data) = $model->getKind($this->type);
        $fa = (string)($element['fixed-array'] ?? '');
        $this->fixed_array = $fa !== '' ? (int)$fa : null;
        $this->export  = ($this->kind === 'struct' && $this->data !== null)
            ? $this->data->export
            : $this->type;
        $this->const   = (string)($element['const'] ?? '');
        $this->pointer = (string)($element['pointer'] ?? '');
    }

    public function __toString() {
        $base = type_printer($this->kind, $this->type);
        if ($this->const)   $base .= ' const';
        if ($this->pointer) $base .= '*';
        return $base;
    }

    public function getImporter($index) {
        global $ATOMIC_TYPES;
        if ($this->kind === 'atomic' || $this->kind === 'fixed') {
            return "{$ATOMIC_TYPES[$this->type][0]}(L, $index)";
        }
        if ($this->kind === 'enum') {
            return "luaL_checkoption(L, $index, NULL, _{$this->type})";
        }
        if (in_array($this->kind, ['struct', 'component', 'resource'])) {
            return "luaX_check{$this->type}(L, $index)";
        }
        return null;
    }

    public function getExporter($variable) {
        global $ATOMIC_TYPES;
        if ($this->kind === 'atomic' || $this->kind === 'fixed') {
            return "{$ATOMIC_TYPES[$this->type][1]}(L, $variable)";
        }
        if ($this->kind === 'enum') {
            return "lua_pushstring(L, {$this->type}ToString($variable))";
        }
        if (in_array($this->kind, ['struct', 'component', 'resource'])) {
            return "luaX_push{$this->type}(L, $variable)";
        }
        return null;
    }
}

class Method extends Base {
    public $args = [];
    public $return_type;
    public $full_name;
    public $static;

    public function __construct($element, $model, $owner = null) {
        parent::__construct($element, $model);

        foreach ($element->arg as $arg) {
            $this->args[] = [(string)$arg['name'], new TypeDef($arg, $model)];
        }

        $this->static = (string)($element['static'] ?? '');

        if ($owner !== null && !$this->static) {
            $owner_elem = simplexml_load_string(
                '<arg name="this" type="' . htmlspecialchars((string)$owner['name']) . '" pointer="true"/>'
            );
            array_unshift($this->args, ['this', new TypeDef($owner_elem, $model)]);
        }

        $returns = $element->returns;
        $this->return_type = ($returns && count($returns))
            ? new TypeDef($returns, $model)
            : 'void';

        $prefix = ($owner !== null) ? (string)($owner['prefix'] ?? '') : '';
        $this->full_name = $prefix . $this->getName();
    }

    public function getArgs() {
        return $this->args;
    }

    public function getArgsTypes() {
        $types = [];
        foreach ($this->args as list($name, $type)) {
            $types[] = "$type $name";
        }
        return $types;
    }
}

class Struct extends Base {
    public $sealed;
    public $export;

    public function __construct($element, $model) {
        parent::__construct($element, $model);
        $this->sealed = (string)($element['sealed'] ?? '') === 'true';
        $exp = (string)($element['export'] ?? '');
        $this->export = ($exp !== '') ? $exp : $this->getName();
    }

    public function getFields() {
        $fields = [];
        foreach ($this->_element->xpath('.//field[@name]') as $f) {
            $name = (string)$f['name'];
            $fields[$name] = new TypeDef($f, $this->_model);
        }
        return $fields;
    }

    public function getMethods() {
        $methods = [];
        foreach ($this->_element->xpath('.//method[@name]') as $m) {
            $name = (string)$m['name'];
            $methods[$name] = new Method($m, $this->_model, $this->_element);
        }
        return $methods;
    }
}

class Component extends Struct {
    public function __construct($element, $model) {
        parent::__construct($element, $model);
    }

    public function getProperties() {
        $properties = [];
        foreach ($this->_element->xpath('.//property[@name]') as $prop) {
            $type = new TypeDef($prop, $this->_model);
            $propName = (string)$prop['name'];
            $this->_walkProperties($type, $properties, $this->getName(), $propName);
        }
        return $properties;
    }

    private function _walkProperties($type, &$properties, ...$args) {
        $path = implode('.', array_slice($args, 1));
        $name = propertyNameFormat($path);
        $properties[$name] = $type;

        if ($type->kind === 'struct' && $type->data !== null && !$type->data->sealed) {
            foreach ($type->data->getFields() as $k => $v) {
                if ($v->fixed_array !== null) {
                    for ($i = 0; $i < $v->fixed_array; $i++) {
                        $next_args = array_merge($args, ["{$k}[{$i}]"]);
                        $this->_walkProperties($v, $properties, ...$next_args);
                    }
                } else {
                    $next_args = array_merge($args, [$k]);
                    $this->_walkProperties($v, $properties, ...$next_args);
                }
            }
        }
    }
}

class EnumDef extends Base {
    public function getValues() {
        $values = [];
        foreach ($this->_element->xpath('.//enum[@name]') as $e) {
            $values[(string)$e['name']] = (string)$e;
        }
        return $values;
    }

    public function getValuesNames() {
        $names = [];
        foreach ($this->_element->xpath('.//enum[@name]') as $e) {
            $names[] = (string)$e['name'];
        }
        return $names;
    }
}

class Resource extends Base {
}
