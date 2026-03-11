<?php require "model/module.php"; ?>
<?php
$docsDir = __DIR__ . "/../../docs";

function countLabel($n, $singular, $plural) {
    if ($n > 1) {
        return $n . " " . $plural;
    }
    return $n . " " . $singular;
}

$md = "# Orca API Documentation\n\n";
$md .= "Welcome to the Orca API reference. Browse the modules below or use the search bar to find specific types and functions.\n\n";
$md .= "## API Modules\n\n";
$md .= "| Module | Contents |\n";
$md .= "|--------|----------|\n";

for ($i = 1; $i < count($argv); $i++) {
    $model = new Model($argv[$i]);
    $moduleName = strval($model->getModuleName());
    $namespaceAttr = $model->root["namespace"];
    $namespace = $namespaceAttr !== null ? strval($namespaceAttr) : "orca";

    $counts = [];
    $enumCount = count($model->getEnums());
    $structCount = count($model->getStructs());
    $compCount = count($model->getComponents());
    $ifaceCount = count($model->getInterfaces());

    if ($enumCount > 0) {
        $counts[] = countLabel($enumCount, "enum", "enums");
    }
    if ($structCount > 0) {
        $counts[] = countLabel($structCount, "struct", "structs");
    }
    if ($compCount > 0) {
        $counts[] = countLabel($compCount, "class", "classes");
    }
    if ($ifaceCount > 0) {
        $counts[] = countLabel($ifaceCount, "interface", "interfaces");
    }

    $summary = implode(", ", $counts);
    $md .= "| [**" . $namespace . "." . $moduleName . "**](api/" . $moduleName . "/README.md) | " . $summary . " |\n";
}

$md .= "\n";

$guides = [];
if (file_exists($docsDir . "/MODULE_XML_GUIDE.md")) {
    $guides[] = "- [Module XML Guide](MODULE_XML_GUIDE.md)";
}
if (file_exists($docsDir . "/MESH_POINTER_BOXING.md")) {
    $guides[] = "- [Mesh Pointer Boxing](MESH_POINTER_BOXING.md)";
}

if (count($guides) > 0) {
    $md .= "## Guides\n\n" . implode("\n", $guides) . "\n";
}

file_put_contents($docsDir . "/index.md", $md);
echo "Index written to " . $docsDir . "/index.md\n";
?>
