<?php require "model/module.php"; ?>
<?php
$model = new Model($argv[1]);
$moduleName = strval($model->getModuleName());
$namespaceAttr = $model->root["namespace"];
$namespace = $namespaceAttr !== null ? strval($namespaceAttr) : "orca";

$outDir = __DIR__ . "/../../docs/api/" . $moduleName;
if (!is_dir($outDir)) {
    mkdir($outDir, 511, true);
}

function mdEscape($text) {
    $text = str_replace('|', '\\|', strval($text));
    return str_replace(['<', '>'], ['&lt;', '&gt;'], $text);
}

function resolveSnippet($path) {
    // Snippet paths are relative to the tools/ working directory
    if (file_exists($path)) {
        return "\n```lua\n" . trim(file_get_contents($path)) . "\n```\n";
    }
    return "";
}

function mdDetails($elem) {
    $text = "";
    foreach ($elem->children() as $child) {
        $tag = $child->getName();
        if ($tag === "p") {
            $text .= "\n" . trim(strval($child)) . "\n";
        } elseif ($tag === "snippet") {
            $srcAttr = $child["src"];
            $src = $srcAttr !== null ? strval($srcAttr) : "";
            if ($src) {
                $text .= resolveSnippet($src);
            }
        }
    }
    if (!$text) {
        // Plain text — handle inline \snippet commands from Doxygen-style source
        $raw = trim(strval($elem));
        // Replace \snippet path with code block content
        $snippetMarker = "\\snippet ";
        $pos = strpos($raw, $snippetMarker);
        if ($pos >= 0 && $pos < strlen($raw)) {
            $before = trim(substr($raw, 0, $pos));
            $after = substr($raw, $pos + strlen($snippetMarker));
            $nl = strpos($after, "\n");
            $snippetPath = $nl >= 0 && $nl < strlen($after) ? trim(substr($after, 0, $nl)) : trim($after);
            $text = $before . "\n" . resolveSnippet($snippetPath);
        } else {
            $text = $raw;
        }
    }
    return $text;
}

function getDetails($elem) {
    $nodes = $elem->xpath("details");
    if (count($nodes) > 0) {
        return mdDetails($nodes[0]);
    }
    return "";
}

function methodSection($methods, $ownerName) {
    $md = "";
    foreach ($methods as $mname => $method) {
        if ($method->isMetaMethod()) continue;
        $export = strval($method->export);

        // Build signature: collect user-visible params (skip 'this_' and 'L')
        $sigParts = [];
        foreach ($method->getArgs() as $argName => $argType) {
            $argName = strval($argName);
            if ($argName === "this_" || $argName === "L") continue;
            $sigParts[] = strval($argType->type);
        }
        $sig = $export . "(" . implode(", ", $sigParts) . ")";
        if ($method->returns !== null) {
            $sig .= " → " . strval($method->returns->type);
        }

        $md .= "### `" . $sig . "`\n\n";
        if ($method->doc) {
            $md .= strval($method->doc) . "\n\n";
        }

        // Parameter table (only if any user-visible params have descriptions)
        $hasArgs = false;
        $paramTable = "| Parameter | Type | Description |\n|-----------|------|-------------|\n";
        foreach ($method->getArgs() as $argName => $argType) {
            $argName = strval($argName);
            if ($argName === "this_" || $argName === "L") continue;
            $hasArgs = true;
            $argDoc = trim(strval($argType->_elem));
            $paramTable .= "| `" . $argName . "` | `" . strval($argType->type) . "` | " . mdEscape($argDoc) . " |\n";
        }
        if ($hasArgs) {
            $md .= $paramTable . "\n";
        }

        if ($method->returns !== null) {
            $retDoc = trim(strval($method->returns->_elem));
            $retLine = "**Returns:** `" . strval($method->returns->type) . "`";
            if ($retDoc) {
                $retLine .= " — " . $retDoc;
            }
            $md .= $retLine . "\n\n";
        }
    }
    return $md;
}

// ---- Generate per-enum page ----
foreach ($model->getEnums() as $ename => $enum) {
    $ename = strval($ename);
    $summary = $enum->doc;
    $details = getDetails($enum->_elem);

    $md = "# " . $ename . "\n\n";
    $md .= "**Enum** | `" . $namespace . "." . $moduleName . "`\n\n";
    if ($summary) {
        $md .= strval($summary) . "\n\n";
    }
    if ($details) {
        $md .= "## Overview\n\n" . $details . "\n\n";
    }
    $md .= "## Constants\n\n";
    $md .= "| Name | Description |\n";
    $md .= "|------|-------------|\n";
    foreach ($enum->getValues() as $valName => $valDoc) {
        $md .= "| `" . strval($valName) . "` | " . mdEscape($valDoc) . " |\n";
    }
    $md .= "\n";
    file_put_contents($outDir . "/" . $ename . ".md", $md);
}

// ---- Generate per-struct page ----
foreach ($model->getStructs() as $sname => $struct) {
    $sname = strval($sname);
    $summary = $struct->doc;
    $details = getDetails($struct->_elem);
    $parentAttr = $struct->_elem["parent"];
    $parent = $parentAttr !== null ? strval($parentAttr) : "";

    $md = "# " . $sname . "\n\n";
    $headerLine = "**Struct** | `" . $namespace . "." . $moduleName . "`";
    if ($parent) {
        $headerLine .= " | extends [" . $parent . "](" . $parent . ".md)";
    }
    $md .= $headerLine . "\n\n";
    if ($summary) {
        $md .= strval($summary) . "\n\n";
    }
    if ($details) {
        $md .= "## Overview\n\n" . $details . "\n\n";
    }

    $hasFields = false;
    $fieldsMd = "## Fields\n\n| Field | Type | Description |\n|-------|------|-------------|\n";
    foreach ($struct->getFields() as $field) {
        $hasFields = true;
        $fname = strval($field->name);
        $ftype = strval($field->type->type);
        $fdoc = $field->doc;
        $farrSize = $field->type->fixed_array;
        if ($farrSize) {
            $ftype .= "[" . $farrSize . "]";
        }
        $fieldsMd .= "| `" . $fname . "` | `" . $ftype . "` | " . mdEscape($fdoc) . " |\n";
    }
    if ($hasFields) {
        $md .= $fieldsMd . "\n";
    }

    $methodsMd = methodSection($struct->getMethods(), $sname);
    if ($methodsMd) {
        $md .= "## Methods\n\n" . $methodsMd;
    }
    file_put_contents($outDir . "/" . $sname . ".md", $md);
}

// ---- Generate per-interface page ----
foreach ($model->getInterfaces() as $iname => $interface) {
    $iname = strval($iname);
    $summary = $interface->doc;
    $details = getDetails($interface->_elem);

    $md = "# " . $iname . "\n\n";
    $md .= "**Interface** | `" . $namespace . "." . $moduleName . "`\n\n";
    if ($summary) {
        $md .= strval($summary) . "\n\n";
    }
    if ($details) {
        $md .= "## Overview\n\n" . $details . "\n\n";
    }
    $hasTopics = $interface->hasTopics();
    if ($hasTopics) {
        foreach ($interface->getTopics() as $topicName => $methods) {
            if (!empty($methods)) {
                $md .= "## " . $topicName . "\n\n";
                $md .= methodSection($methods, $iname);
            }
        }
    } else {
        $methodsMd = methodSection($interface->getMethods(), $iname);
        if ($methodsMd) {
            $md .= "## Methods\n\n" . $methodsMd;
        }
    }
    file_put_contents($outDir . "/" . $iname . ".md", $md);
}

// ---- Generate per-class (component) page ----
foreach ($model->getComponents() as $cname => $comp) {
    $cname = strval($cname);
    $summary = $comp->doc;
    $details = getDetails($comp->_elem);
    $parentAttr = $comp->_elem["parent"];
    $parent = $parentAttr !== null ? strval($parentAttr) : "";

    $md = "# " . $cname . "\n\n";
    $headerLine = "**Class** | `" . $namespace . "." . $moduleName . "`";
    if ($parent) {
        $headerLine .= " | extends [" . $parent . "](" . $parent . ".md)";
    }
    $md .= $headerLine . "\n\n";
    if ($summary) {
        $md .= strval($summary) . "\n\n";
    }
    if ($details) {
        $md .= "## Overview\n\n" . $details . "\n\n";
    }

    $hasProps = false;
    $propsMd = "## Properties\n\n| Property | Type | Description |\n|----------|------|-------------|\n";
    foreach ($comp->getProperties(false) as $prop) {
        $hasProps = true;
        $pname = strval($prop->name);
        $ptype = strval($prop->type->type);
        $pdoc = $prop->doc;
        $propsMd .= "| `" . $pname . "` | `" . $ptype . "` | " . mdEscape($pdoc) . " |\n";
    }
    if ($hasProps) {
        $md .= $propsMd . "\n";
    }

    $methodsMd = methodSection($comp->getMethods(), $cname);
    if ($methodsMd) {
        $md .= "## Methods\n\n" . $methodsMd;
    }
    file_put_contents($outDir . "/" . $cname . ".md", $md);
}

// ---- Generate README.md (module index) ----
$readme = "# " . $namespace . "." . $moduleName . "\n\n";
$readme .= "API reference for the `" . $moduleName . "` module.\n\n";

$hasEnums = false;
$enumsMd = "## Enumerations\n\n";
foreach ($model->getEnums() as $ename => $enum) {
    $hasEnums = true;
    $ename = strval($ename);
    $summary = $enum->doc;
    $enumsMd .= "- [**" . $ename . "**](" . $ename . ".md)";
    if ($summary) {
        $enumsMd .= " — " . strval($summary);
    }
    $enumsMd .= "\n";
}
if ($hasEnums) {
    $readme .= $enumsMd . "\n";
}

$hasStructs = false;
$structsMd = "## Structures\n\n";
foreach ($model->getStructs() as $sname => $struct) {
    $hasStructs = true;
    $sname = strval($sname);
    $summary = $struct->doc;
    $structsMd .= "- [**" . $sname . "**](" . $sname . ".md)";
    if ($summary) {
        $structsMd .= " — " . strval($summary);
    }
    $structsMd .= "\n";
}
if ($hasStructs) {
    $readme .= $structsMd . "\n";
}

$hasIfaces = false;
$ifacesMd = "## Interfaces\n\n";
foreach ($model->getInterfaces() as $iname => $iface) {
    $hasIfaces = true;
    $iname = strval($iname);
    $summary = $iface->doc;
    $ifacesMd .= "- [**" . $iname . "**](" . $iname . ".md)";
    if ($summary) {
        $ifacesMd .= " — " . strval($summary);
    }
    $ifacesMd .= "\n";
}
if ($hasIfaces) {
    $readme .= $ifacesMd . "\n";
}

$hasComps = false;
$compsMd = "## Classes\n\n";
foreach ($model->getComponents() as $cname => $comp) {
    $hasComps = true;
    $cname = strval($cname);
    $summary = $comp->doc;
    $compsMd .= "- [**" . $cname . "**](" . $cname . ".md)";
    if ($summary) {
        $compsMd .= " — " . strval($summary);
    }
    $compsMd .= "\n";
}
if ($hasComps) {
    $readme .= $compsMd . "\n";
}

file_put_contents($outDir . "/README.md", $readme);
echo "Docs written to " . $outDir . "\n";
?>
