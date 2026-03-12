<?php
class DtdController {
	function handle($action, $args) {
		if ($action === 'generate') {
			$this->generate($args);
		} else {
			exit(1);
		}
	}

	function generate($args) {
		$models = [];
		foreach ($args as $xmlFile) {
			$models[] = new Model($xmlFile);
		}
		include_template('dtd', [
			'models' => $models,
		]);
	}
}
