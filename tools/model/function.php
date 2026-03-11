<?php

require "model/config_new.php";

class Workspace {
	private $workspace;
	
	public function __construct($filename) {
		$this->workspace = [];
		$this->require($filename);
	}

	function require($filename) {
		$tmp = simplexml_load_file($filename);
		$this->workspace[] = $tmp;
		foreach ($tmp->require as $require) {
			$this->require(dirname($filename).'/'.$require['file']);
		}
	}

	function findType($name, $class) {
		foreach ($this->workspace as $ws) {
			if ($ws->xpath("//{$class}[@name='{$name}']")) {
				return true;
			}
		}
		return false;
	}

	function decorate($arg) {
		foreach (config_new::$TypeInfos as $type => $config) {
			if ($arg['type'] == $type || $this->findType($arg['type'], $type)) {
				foreach ($config as $key => $value) {
					$arg->addAttribute($key, $value);
				}
			}
		}
	}
}

class Function {
	private $locals;
	private $workspace;
	
	public function __construct($filename) {
		$this->workspace = new Workspace($filename);
		$this->locals = simplexml_load_file($filename);
	}

	function getFunctions() {
		foreach ($this->locals->xpath('//function') as $function) {
			yield $function['name'] => $function;
		}
		foreach ($this->locals->xpath('//struct') as $struct) {
			foreach ($struct->method as $method) {
				// $thisElem = $method->addChild('arg');
				// $thisElem->addAttribute('name', 'this_');
				// $thisElem->addAttribute('type', $struct['name']);
				// $thisElem->addAttribute('pointer', 'true');
				// if ($method['const']) $thisElem->addAttribute('const', $method['const']);
				// $method->addChild($thisElem);
				foreach ($method->arg as $arg) $this->workspace->decorate($arg);
				foreach ($method->returns as $arg) $this->workspace->decorate($arg);
				yield "{$struct['prefix'] ?? $struct['name']}_{$method['name']}" => $method;
			}
		}
	}
}