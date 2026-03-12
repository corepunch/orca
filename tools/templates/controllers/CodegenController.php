<?php
class CodegenController {
	function handle($action, $args) {
		if ($action === 'header') {
			$this->header($args);
		} elseif ($action === 'properties') {
			$this->properties($args);
		} elseif ($action === 'export') {
			$this->export($args);
		} else {
			exit(1);
		}
	}

	function header($args) {
		$model = new Model($args[0]);
		include_template('header/preamble', [
			'moduleName'      => $model->getModuleName(),
			'externalStructs' => $model->getExternalStructs(),
			'includes'        => $model->getIncludes(),
			'requires'        => $model->getRequires(),
			'events'          => $model->getEvents(),
		]);
		include_template('header/enums', [
			'enums' => $model->getEnums(),
		]);
		include_template('header/declarations', [
			'structs'     => $model->getStructs(),
			'functions'   => $model->getFunctions(),
			'prefix'      => $model->prefix,
			'interfaces'  => $model->getInterfaces(),
		]);
		include_template('header/structs', [
			'structs' => $model->getStructs(),
		]);
		include_template('header/components', [
			'components' => $model->getComponents(),
		]);
		include_template('header/footer', []);
	}

	function properties($args) {
		$model = new Model($args[0]);
		include_template('properties', [
			'moduleName' => $model->getModuleName(),
			'events'     => $model->getEvents(),
			'components' => $model->getComponents(),
		]);
	}

	function export($args) {
		$model = new Model($args[0]);
		include_template('export/preamble', [
			'moduleName'      => $model->getModuleName(),
			'externalStructs' => $model->getExternalStructs(),
		]);
		include_template('export/enums', [
			'enums' => $model->getEnums(),
		]);
		include_template('export/interfaces', [
			'interfaces' => $model->getInterfaces(),
		]);
		include_template('export/structs', [
			'structs' => $model->getStructs(),
		]);
		include_template('export/components', [
			'components'    => $model->getComponents(),
			'events'        => $model->getEvents(),
			'hasComponents' => $model->hasComponents(),
		]);
		include_template('export/functions', [
			'functions' => $model->getFunctions(),
			'prefix'    => $model->prefix,
		]);
		include_template('export/luaopen', [
			'moduleName' => $model->getModuleName(),
			'prefix'     => $model->prefix,
			'on_luaopen' => $model->on_luaopen,
			'functions'  => $model->getFunctions(),
			'structs'    => $model->getStructs(),
			'interfaces' => $model->getInterfaces(),
			'components' => $model->getComponents(),
		]);
	}
}
