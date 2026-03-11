<?php
require "model/module.php";

// Route format: controller/action
// e.g. codegen/header, codegen/export, dtd/generate, docs/generate, docs/index
$route = $argv[1];
$xmlArgs = array_slice($argv, 2);

$routeParts = explode('/', $route, 2);
$controller = $routeParts[0];
$action = $routeParts[1];

if ($controller === 'codegen') {
	$model = new Model($xmlArgs[0]);
	$vars = [
		"moduleName"     => $model->getModuleName(),
		"prefix"         => $model->prefix,
		"on_luaopen"     => $model->on_luaopen,
		"enums"          => $model->getEnums(),
		"structs"        => $model->getStructs(),
		"interfaces"     => $model->getInterfaces(),
		"components"     => $model->getComponents(),
		"functions"      => $model->getFunctions(),
		"events"         => $model->getEvents(),
		"externalStructs" => $model->getExternalStructs(),
		"requires"       => $model->getRequires(),
		"includes"       => $model->getIncludes(),
		"hasComponents"  => $model->hasComponents(),
	];
	include_template($action, $vars);
} elseif ($controller === 'dtd') {
	$models = [];
	foreach ($xmlArgs as $xmlFile) {
		$models[] = new Model($xmlFile);
	}
	include_template('dtd', ["models" => $models]);
} elseif ($controller === 'docs') {
	if ($action === 'generate') {
		$model = new Model($xmlArgs[0]);
		$namespaceAttr = $model->root["namespace"];
		$namespace = $namespaceAttr !== null ? strval($namespaceAttr) : "orca";
		$vars = [
			"moduleName"  => $model->getModuleName(),
			"namespace"   => $namespace,
			"enums"       => $model->getEnums(),
			"structs"     => $model->getStructs(),
			"interfaces"  => $model->getInterfaces(),
			"components"  => $model->getComponents(),
		];
		include_template('docs', $vars);
	} elseif ($action === 'index') {
		$models = [];
		foreach ($xmlArgs as $xmlFile) {
			$models[] = new Model($xmlFile);
		}
		include_template('docs_index', ["models" => $models]);
	} else {
		fwrite(STDERR, "Unknown docs action: " . $action . "\n");
		exit(1);
	}
} else {
	fwrite(STDERR, "Unknown route: " . $route . "\n");
	exit(1);
}
