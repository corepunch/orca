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
	include_template($action);
} elseif ($controller === 'dtd') {
	$models = [];
	foreach ($xmlArgs as $xmlFile) {
		$models[] = new Model($xmlFile);
	}
	include_template('dtd');
} elseif ($controller === 'docs') {
	if ($action === 'generate') {
		$model = new Model($xmlArgs[0]);
		include_template('docs');
	} elseif ($action === 'index') {
		$models = [];
		foreach ($xmlArgs as $xmlFile) {
			$models[] = new Model($xmlFile);
		}
		include_template('docs_index');
	} else {
		fwrite(STDERR, "Unknown docs action: " . $action . "\n");
		exit(1);
	}
} else {
	fwrite(STDERR, "Unknown route: " . $route . "\n");
	exit(1);
}
