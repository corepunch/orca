<?php
require "model/module.php";

// Routes: "controller/action" => "ControllerClass@method"
$routes = [
	"codegen/header"     => "CodegenController@header",
	"codegen/properties" => "CodegenController@properties",
	"codegen/export"     => "CodegenController@export",
	"dtd/generate"       => "DtdController@generate",
	"docs/generate"      => "DocsController@generate",
	"docs/index"         => "DocsController@index",
];

$route = $argv[1];
$xmlArgs = array_slice($argv, 2);

$handler = isset($routes[$route]) ? $routes[$route] : "";
if ($handler === "") {
	fwrite(STDERR, "Unknown route: " . $route . "\n");
	exit(1);
}

$parts = explode('/', $route, 2);
$controller = $parts[0];
$action = $parts[1];

if ($controller === 'codegen') {
	require "templates/controllers/CodegenController.php";
	(new CodegenController())->handle($action, $xmlArgs);
} elseif ($controller === 'dtd') {
	require "templates/controllers/DtdController.php";
	(new DtdController())->handle($action, $xmlArgs);
} elseif ($controller === 'docs') {
	require "templates/controllers/DocsController.php";
	(new DocsController())->handle($action, $xmlArgs);
}
