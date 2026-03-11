<?php
// --- Global registry: collect entities from all supplied XML files ---

$globalEnums = [];
$globalStructs = [];
$globalComponents = [];

function loadModule($model) {
    global $globalEnums, $globalStructs, $globalComponents;
    foreach ($model->getEnums() as $name => $enum) {
        $globalEnums[strval($name)] = $enum;
    }
    foreach ($model->getStructs() as $name => $struct) {
        $globalStructs[strval($name)] = $struct;
    }
    foreach ($model->getComponents() as $name => $comp) {
        $globalComponents[strval($name)] = $comp;
    }
}

foreach ($models as $model) {
    loadModule($model);
}

// --- Helper: check if a component (by name) is a 2D or 3D node via inheritance ---

function isNode($compName, $suffix) {
    global $globalComponents;
    if ($compName === "Viewport3D") return false;
    if (str_contains($compName, $suffix)) return true;
    $comp = isset($globalComponents[$compName]) ? $globalComponents[$compName] : null;
    if ($comp === null) return false;
    $parentAttr = $comp->_elem["parent"];
    $parent = $parentAttr !== null ? strval($parentAttr) : "";
    if (!$parent) return false;
    return isNode($parent, $suffix);
}

// --- Helper: convert a dotted property path to a DTD attribute name ---
// Mirrors Python utils.property_name() and PHP PropertyName::__toString()

function dtdPropertyName($path) {
    $name = $path;
    foreach (config::$Axis as $pair) {
        $pat = '/' . $pair[0] . '/';
        if (preg_match($pat, $name)) {
            $name = preg_replace($pat, $pair[1], $name, 1);
            break;
        }
    }
    return implode('', array_map('ucfirst', explode('.', $name)));
}

// --- Helper: recursively collect DTD attribute (name, type) pairs for a property ---
// Returns a flat array of [sname, ptype] pairs.

function collectDtdAttribs($type_, $path) {
    global $globalEnums;
    $ptype = strval($type_->type);
    $sname = dtdPropertyName($path);
    $enumByType = isset($globalEnums[$ptype]) ? $globalEnums[$ptype] : null;
    $enumByName = isset($globalEnums[$sname]) ? $globalEnums[$sname] : null;
    $enum = $enumByType !== null ? $enumByType : $enumByName;
    if ($enum !== null) {
        $ptype = strval($enum->name);
    }
    $result = [[$sname, $ptype]];
    if ($type_->kind === "struct" && $type_->data !== null && !$type_->data->sealed) {
        foreach ($type_->data->getFields() as $field) {
            $fname = strval($field->name);
            if ($field->type->fixed_array) {
                for ($i = 0; $i < $field->type->fixed_array; $i++) {
                    $result = array_merge($result, collectDtdAttribs($field->type, $path . "." . $fname . "[$i]"));
                }
            } else {
                $result = array_merge($result, collectDtdAttribs($field->type, $path . "." . $fname));
            }
        }
    }
    return $result;
}

// --- Collect the attribute list for a component ---

function iterComponentAttribs($comp) {
    $attribs = [];
    foreach ($comp->getProperties(false) as $prop) {
        $pname = strval($prop->name->path[0]);
        if ($prop->type->fixed_array) {
            for ($i = 0; $i < $prop->type->fixed_array; $i++) {
                $attribs = array_merge($attribs, collectDtdAttribs($prop->type, $pname . "[$i]"));
            }
        } else {
            $attribs = array_merge($attribs, collectDtdAttribs($prop->type, $pname));
        }
    }
    return $attribs;
}

// --- Generate DTD ---

// 1. Enum entity declarations
foreach ($globalEnums as $name => $enum) {
    $values = [];
    foreach ($enum->getValuesNames() as $vname) {
        $values[] = strval($vname);
    }
    echo "<!ENTITY % " . $name . " \"" . implode("|", $values) . "\">\n";
}

// 2. Component entity/element declarations
foreach ($globalComponents as $name => $comp) {
    $parentAttr = $comp->_elem["parent"];
    $parent = $parentAttr !== null ? strval($parentAttr) : "";
    $conceptAttr = $comp->_elem["concept"];
    $concept = $conceptAttr !== null ? strval($conceptAttr) : "";

    // --- Attribs entity ---
    echo "<!ENTITY % " . $name . "Attribs \"\n";
    if ($parent) {
        echo "\t%" . $parent . "Attribs;\n";
        if ($concept) {
            echo "\t%" . $concept . "Attribs;\n";
        }
    } else {
        echo "\tName CDATA #REQUIRED\n";
        echo "\tStyle CDATA #IMPLIED\n";
        echo "\tIsDisabled (true|false) #IMPLIED\n";
        echo "\tStateManager CDATA #IMPLIED\n";
    }

    $attribs = iterComponentAttribs($comp);
    foreach ($attribs as $pair) {
        $sname = $pair[0];
        $ptype = $pair[1];
        if (isset($globalEnums[$ptype])) {
            echo "\t" . $sname . " (%" . $ptype . ";) #IMPLIED\n";
        } elseif ($ptype === "bool") {
            echo "\t" . $sname . " (true|false) #IMPLIED\n";
        } else {
            echo "\t" . $sname . " CDATA #IMPLIED\n";
        }
    }
    echo "\">\n";

    // --- Binding sub-elements ---
    foreach ($attribs as $pair) {
        $sname = $pair[0];
        echo "<!ELEMENT " . $name . "." . $sname . " (#PCDATA | Binding)*>\n";
        echo "<!ATTLIST " . $name . "." . $sname . " Mode (expression|oneway) #IMPLIED Attribute CDATA #IMPLIED Enabled (true|false) #IMPLIED>\n";
    }

    // --- Elements entity ---
    $elemParts = [];
    foreach ($attribs as $p) {
        $elemParts[] = $name . "." . $p[0];
    }
    if ($parent) {
        $elemParts[] = "%" . $parent . "Elements;";
    }
    if ($concept) {
        $elemParts[] = "%" . $concept . "Elements;";
    }
    echo "<!ENTITY % " . $name . "Elements \"" . implode("|", $elemParts) . "\">\n";

    // --- Element content model ---
    $elm = "%" . $name . "Elements;";
    echo "<!ELEMENT " . $name . " ";
    if ($name === "Viewport3D") {
        $arr = [];
        foreach ($globalComponents as $k => $v) {
            $arr[] = $k;
        }
        $arr[] = "Resource";
        $arr[] = "StyleSheet";
        $arr[] = "ObjectPrefabPlaceholder";
        $arr[] = $elm;
        echo "(" . implode("|", $arr) . ")*>\n";
    } elseif (isNode($name, "2D")) {
        $arr = [];
        foreach ($globalComponents as $k => $v) {
            if (isNode($k, "2D")) {
                $arr[] = $k;
            }
        }
        $arr[] = "Viewport3D";
        $arr[] = "Resource";
        $arr[] = "StyleSheet";
        $arr[] = "LayerPrefabPlaceholder";
        $arr[] = $elm;
        echo "(" . implode("|", $arr) . ")*>\n";
    } elseif (isNode($name, "3D")) {
        $arr = [];
        foreach ($globalComponents as $k => $v) {
            if (isNode($k, "3D")) {
                $arr[] = $k;
            }
        }
        $arr[] = "Resource";
        $arr[] = "StyleSheet";
        $arr[] = "ObjectPrefabPlaceholder";
        $arr[] = $elm;
        echo "(" . implode("|", $arr) . ")*>\n";
    } elseif (isNode($name, "SKNode") || isNode($name, "SKScene")) {
        $arr = [];
        foreach ($globalComponents as $k => $v) {
            if (isNode($k, "SKNode")) {
                $arr[] = $k;
            }
        }
        $arr[] = "Resource";
        $arr[] = $elm;
        echo "(" . implode("|", $arr) . ")*>\n";
    } else {
        echo "EMPTY>\n";
    }
    echo "<!ATTLIST " . $name . " %" . $name . "Attribs;>\n";
}

// Append shared append.dtd content
echo file_get_contents(__DIR__ . "/../../docs/schemas/append.dtd");
?>
