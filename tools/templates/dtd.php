<?php require "model/module.php"; ?>
<?php
// Load all models from all provided XML files
$all_models = [];
$all_components = [];
$all_enums = [];
$all_structs = [];

for ($i = 1; $i < $argc; $i++) {
    if (!file_exists($argv[$i])) continue;
    $m = new Model($argv[$i]);
    $all_models[] = $m;
    foreach ($m->getComponents() as $name => $cmp) {
        $all_components[$name] = $cmp;
    }
    foreach ($m->getEnums() as $name => $enum) {
        $all_enums[$name] = $enum;
    }
    foreach ($m->getStructs() as $name => $struct) {
        $all_structs[$name] = $struct;
    }
}

function is_component_type($comp_name, $suffix) {
    global $all_components;
    if ($comp_name === "Viewport3D") return false;
    if ($comp_name === null) return false;
    if (strpos($comp_name, $suffix) !== false) return true;
    $comp = $all_components[$comp_name] ?? null;
    if (!$comp) return false;
    $parent = (string)($comp->_elem["parent"] ?? "");
    if (!$parent) return false;
    return is_component_type($parent, $suffix);
}

function collect_struct_fields($struct_name, $prefix, &$attribs) {
    global $all_structs, $all_enums;
    $struct = $all_structs[$struct_name] ?? null;
    if (!$struct || $struct->sealed) return;
    foreach ($struct->getFields() as $fname => $field) {
        $fname_str = (string)$fname;
        if ($field->fixed_array) {
            for ($idx = 0; $idx < $field->fixed_array; $idx++) {
                collect_property($field->type, "{$prefix}{$fname_str}[{$idx}]", $attribs);
            }
        } else {
            collect_property($field->type, "{$prefix}{$fname_str}", $attribs);
        }
    }
}

function collect_property($prop_type, $path, &$attribs) {
    global $all_structs, $all_enums;
    $struct = $all_structs[$prop_type] ?? null;
    $prop_name = property_name($path);
    $enum = $all_enums[$prop_type] ?? ($struct === null ? ($all_enums[$prop_name] ?? null) : null);
    $ptype = $enum ? (string)$enum->name : $prop_type;
    $attribs[] = [$prop_name, $ptype];
    if ($struct !== null) {
        collect_struct_fields($prop_type, "{$path}.", $attribs);
    }
}

function property_name($path) {
    static $axis_rules = null;
    if ($axis_rules === null) {
        $axis_rules = [
            ['/(.+)\.Axis\[0\]\.Left(.*)/', '\1Left\2'],
            ['/(.+)\.Axis\[0\]\.Right(.*)/', '\1Right\2'],
            ['/(.+)\.Axis\[1\]\.Left(.*)/', '\1Top\2'],
            ['/(.+)\.Axis\[1\]\.Right(.*)/', '\1Bottom\2'],
            ['/(.+)\.Axis\[2\]\.Left(.*)/', '\1Front\2'],
            ['/(.+)\.Axis\[2\]\.Right(.*)/', '\1Back\2'],
            ['/Size\.Axis\[0\]\.(?:Requested)?(.*)/', '\1Width'],
            ['/Size\.Axis\[1\]\.(?:Requested)?(.*)/', '\1Height'],
            ['/Size\.Axis\[2\]\.(?:Requested)?(.*)/', '\1Depth'],
            ['/(.+)\.Axis\[0\]/', 'Horizontal\1'],
            ['/(.+)\.Axis\[1\]/', 'Vertical\1'],
            ['/(.+)\.Axis\[2\]/', 'Depth\1'],
            ['/Border\.Radius\.(.+)Radius/', 'Border\1Radius'],
        ];
    }
    foreach ($axis_rules as [$pat, $repl]) {
        $result = preg_replace($pat, $repl, $path, 1, $count);
        if ($count > 0) {
            return str_replace('.', '', $result);
        }
    }
    return str_replace('.', '', $path);
}

function iter_component_attribs($component) {
    global $all_structs, $all_enums;
    $attribs = [];
    foreach ($component->_elem->xpath(".//property[@name]") as $prop) {
        $pname = (string)$prop["name"];
        $ptype = (string)$prop["type"];
        if ((int)($prop["fixed-array"] ?? 0)) {
            for ($idx = 0; $idx < (int)$prop["fixed-array"]; $idx++) {
                collect_property($ptype, "{$pname}[{$idx}]", $attribs);
            }
        } else {
            collect_property($ptype, $pname, $attribs);
        }
    }
    return $attribs;
}

// Process each model
foreach ($all_models as $model) {
    // Output enum entities
    foreach ($model->getEnums() as $ename => $enum) {
        $values = implode('|', $enum->getValuesNames());
        echo "<!ENTITY % {$ename} \"{$values}\">\n";
    }

    // Output component DTD entries
    foreach ($model->getComponents() as $cname => $component) {
        echo "<!ENTITY % {$cname}Attribs \"\n";
        $parent = (string)($component->_elem["parent"] ?? "");
        $concept = (string)($component->_elem["concept"] ?? "");
        if ($parent) {
            echo "\t%{$parent}Attribs;\n";
            if ($concept) {
                echo "\t%{$concept}Attribs;\n";
            }
        } else {
            echo "\tName CDATA #REQUIRED\n";
            echo "\tStyle CDATA #IMPLIED\n";
            echo "\tIsDisabled (true|false) #IMPLIED\n";
            echo "\tStateManager CDATA #IMPLIED\n";
        }

        $attribs = iter_component_attribs($component);
        foreach ($attribs as [$aname, $atype]) {
            if (isset($all_enums[$atype])) {
                echo "\t{$aname} (%{$atype};) #IMPLIED\n";
            } elseif ($atype === 'bool') {
                echo "\t{$aname} (true|false) #IMPLIED\n";
            } else {
                echo "\t{$aname} CDATA #IMPLIED\n";
            }
        }
        echo "\">\n";

        foreach ($attribs as [$aname, $_]) {
            echo "<!ELEMENT {$cname}.{$aname} (#PCDATA | Binding)*>\n";
            echo "<!ATTLIST {$cname}.{$aname} Mode (expression|oneway) #IMPLIED Attribute CDATA #IMPLIED Enabled (true|false) #IMPLIED>\n";
        }

        $element_attribs = array_map(fn($a) => "{$cname}.{$a[0]}", $attribs);
        if ($parent) $element_attribs[] = "%{$parent}Elements;";
        if ($concept) $element_attribs[] = "%{$concept}Elements;";
        echo "<!ENTITY % {$cname}Elements \"" . implode('|', $element_attribs) . "\">\n";

        echo "<!ELEMENT {$cname} ";
        $elm = "%{$cname}Elements;";
        if (is_component_type($cname, '2D')) {
            $arr = array_keys(array_filter($all_components, fn($c) => is_component_type((string)$c->name, '2D')));
            $arr = array_merge($arr, ['Viewport3D', 'Resource', 'StyleSheet', 'LayerPrefabPlaceholder', $elm]);
            echo '(' . implode('|', $arr) . ")*>\n";
        } elseif (is_component_type($cname, '3D')) {
            $arr = array_keys(array_filter($all_components, fn($c) => is_component_type((string)$c->name, '3D')));
            $arr = array_merge($arr, ['Resource', 'StyleSheet', 'ObjectPrefabPlaceholder', $elm]);
            echo '(' . implode('|', $arr) . ")*>\n";
        } elseif ($cname === 'Viewport3D') {
            $arr = array_keys($all_components);
            $arr = array_merge($arr, ['Resource', 'StyleSheet', 'ObjectPrefabPlaceholder', $elm]);
            echo '(' . implode('|', $arr) . ")*>\n";
        } else {
            echo "EMPTY>\n";
        }
        echo "<!ATTLIST {$cname} %{$cname}Attribs;>\n";
    }
}
?>
