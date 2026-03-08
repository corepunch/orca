<?php
// Generates DTD schema from XML module files.
// Usage: php templates/dtd.php file1.xml file2.xml ...
// Output goes to stdout (redirect to docs/schemas/orca.dtd)
// Appends content of docs/schemas/append.dtd at the end.

require __DIR__ . "/../model.php";

if ($argc < 2) {
    fwrite(STDERR, "Usage: php dtd.php file1.xml [file2.xml ...]\n");
    exit(1);
}

// Load all models and build a combined workspace
$all_models = [];
$ws_structs    = [];
$ws_enums      = [];
$ws_components = [];

for ($i = 1; $i < $argc; $i++) {
    $m = new Model($argv[$i]);
    $all_models[] = $m;
    $ws_structs    = array_merge($ws_structs,    $m->structs);
    $ws_enums      = array_merge($ws_enums,      $m->enums);
    $ws_components = array_merge($ws_components, $m->components);
}

// ---------------------------------------------------------------------------
// Property collection helpers (mirrors dtd_writer.py)
// ---------------------------------------------------------------------------

function dtd_collect_struct_fields($struct_elem, $prefix, &$attribs) {
    global $ws_structs;
    if ((string)($struct_elem['sealed'] ?? '') === 'true') {
        return;
    }
    foreach ($struct_elem->field as $field) {
        $fname = (string)$field['name'];
        if ($field['fixed-array']) {
            for ($idx = 0; $idx < (int)$field['fixed-array']; $idx++) {
                dtd_collect_property($field, "{$prefix}{$fname}[{$idx}]", $attribs);
            }
        } else {
            dtd_collect_property($field, "{$prefix}{$fname}", $attribs);
        }
    }
}

function dtd_collect_property($prop, $path, &$attribs) {
    global $ws_structs, $ws_enums;
    $prop_type = (string)$prop['type'];
    $struct = $ws_structs[$prop_type] ?? null;
    if (!(string)($prop['exclude-self'] ?? '')) {
        $sname = propertyNameFormat($path);
        $enum = $ws_enums[$prop_type] ?? null;
        if ($enum === null && $struct === null) {
            $enum = $ws_enums[$sname] ?? null;
        }
        if ($enum !== null) {
            $ptype = (string)($enum->getElement()['name'] ?? $prop_type);
        } else {
            $ptype = $prop_type;
        }
        $attribs[] = [$sname, $ptype];
    }
    if ($struct !== null) {
        dtd_collect_struct_fields($struct->getElement(), "{$path}.", $attribs);
    }
}

function dtd_iter_component_attribs($component_elem) {
    $attribs = [];
    foreach ($component_elem->property as $prop) {
        $pname = (string)$prop['name'];
        if ($prop['fixed-array']) {
            for ($idx = 0; $idx < (int)$prop['fixed-array']; $idx++) {
                dtd_collect_property($prop, "{$pname}[{$idx}]", $attribs);
            }
        } else {
            dtd_collect_property($prop, $pname, $attribs);
        }
    }
    return $attribs;
}

function is_component_type($comp_elem, $suffix) {
    global $ws_components;
    if ($comp_elem === null) return false;
    $cname = (string)$comp_elem['name'];
    if ($cname === "Viewport3D") return false;
    if (strpos($cname, $suffix) !== false) return true;
    $parent = (string)($comp_elem['parent'] ?? '');
    if ($parent === '') return false;
    $parent_comp = $ws_components[$parent] ?? null;
    return $parent_comp !== null && is_component_type($parent_comp->getElement(), $suffix);
}

// ---------------------------------------------------------------------------
// DTD generation
// ---------------------------------------------------------------------------

// Emit enum entity references
foreach ($all_models as $m) {
    foreach ($m->enums as $ename => $enum) {
        $values = $enum->getValuesNames();
        echo "<!ENTITY % {$ename} \"" . implode('|', $values) . "\">\n";
    }
}

// Emit component element/attribute definitions
foreach ($all_models as $m) {
    foreach ($m->components as $cname => $component) {
        $comp_elem = $component->getElement();

        echo "<!ENTITY % {$cname}Attribs \"\n";

        $parent = (string)($comp_elem['parent'] ?? '');
        $concept = (string)($comp_elem['concept'] ?? '');
        if ($parent !== '') {
            echo "\t%{$parent}Attribs;\n";
            if ($concept !== '') {
                echo "\t%{$concept}Attribs;\n";
            }
        } else {
            echo "\tName CDATA #REQUIRED\n";
            echo "\tStyle CDATA #IMPLIED\n";
            echo "\tIsDisabled (true|false) #IMPLIED\n";
            echo "\tStateManager CDATA #IMPLIED\n";
        }

        $attribs = dtd_iter_component_attribs($comp_elem);
        foreach ($attribs as $item) {
            list($sname, $ptype) = $item;
            if (isset($ws_enums[$ptype])) {
                echo "\t{$sname} (%{$ptype};) #IMPLIED\n";
            } elseif ($ptype === 'bool') {
                echo "\t{$sname} (true|false) #IMPLIED\n";
            } else {
                echo "\t{$sname} CDATA #IMPLIED\n";
            }
        }
        echo "\">\n";

        foreach ($attribs as $item) {
            list($sname, $_ptype) = $item;
            echo "<!ELEMENT {$cname}.{$sname} (#PCDATA | Binding)*>\n";
            echo "<!ATTLIST {$cname}.{$sname} Mode (expression|oneway) #IMPLIED "
               . "Attribute CDATA #IMPLIED Enabled (true|false) #IMPLIED>\n";
        }

        $element_attribs = array_map(fn($a) => "{$cname}.{$a[0]}", $attribs);
        if ($parent !== '') {
            $element_attribs[] = "%{$parent}Elements;";
        }
        if ($concept !== '') {
            $element_attribs[] = "%{$concept}Elements;";
        }
        echo "<!ENTITY % {$cname}Elements \"" . implode('|', $element_attribs) . "\">\n";

        // Determine container type and child elements
        $all_2d = array_keys(array_filter($ws_components, fn($c) => is_component_type($c->getElement(), '2D')));
        $all_3d = array_keys(array_filter($ws_components, fn($c) => is_component_type($c->getElement(), '3D')));

        echo "<!ELEMENT {$cname} ";
        $elm = "%{$cname}Elements;";
        if (is_component_type($comp_elem, '2D')) {
            $children = array_merge($all_2d, ["Viewport3D", "Resource", "StyleSheet", "LayerPrefabPlaceholder", $elm]);
            echo "(" . implode('|', $children) . ")*>\n";
        } elseif (is_component_type($comp_elem, '3D')) {
            $children = array_merge($all_3d, ["Resource", "StyleSheet", "ObjectPrefabPlaceholder", $elm]);
            echo "(" . implode('|', $children) . ")*>\n";
        } elseif ($cname === "Viewport3D") {
            $all_comps = array_keys($ws_components);
            $children = array_merge($all_comps, ["Resource", "StyleSheet", "ObjectPrefabPlaceholder", $elm]);
            echo "(" . implode('|', $children) . ")*>\n";
        } else {
            echo "EMPTY>\n";
        }
        echo "<!ATTLIST {$cname} %{$cname}Attribs;>\n";
    }
}

// Append content from append.dtd
$append_path = __DIR__ . "/../../docs/schemas/append.dtd";
if (file_exists($append_path)) {
    echo file_get_contents($append_path);
}
