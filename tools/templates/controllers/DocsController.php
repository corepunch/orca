<?php
class DocsController {
	function handle($action, $args) {
		if ($action === 'generate') {
			$this->generate($args);
		} elseif ($action === 'index') {
			$this->index($args);
		} else {
			exit(1);
		}
	}

	function generate($args) {
		$model = new Model($args[0]);
		$namespaceAttr = $model->root["namespace"];
		$namespace = $namespaceAttr !== null ? strval($namespaceAttr) : "orca";
		include_template('docs', [
			'moduleName' => $model->getModuleName(),
			'namespace'  => $namespace,
			'enums'      => $model->getEnums(),
			'structs'    => $model->getStructs(),
			'interfaces' => $model->getInterfaces(),
			'components' => $model->getComponents(),
		]);
	}

	function index($args) {
		$models = [];
		foreach ($args as $xmlFile) {
			$models[] = new Model($xmlFile);
		}
		include_template('docs_index', [
			'models' => $models,
		]);
	}
}
