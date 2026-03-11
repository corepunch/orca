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
		array_push($this->workspace, $tmp);
		foreach ($tmp->require as $req) {
			$this->require(dirname($filename).'/'.$req['file']);
		}
	}

	function findType($name, $class) {
		$ws = $this->workspace;
		foreach ($ws as $w) {
			if ($w->xpath("//{$class}[@name='{$name}']")) {
				return true;
			}
		}
		return false;
	}

	function typeInfo($typeName) {
		foreach (config_new::$TypeInfos as $kind => $info) {
			if ($typeName == $kind || $this->findType($typeName, $kind)) {
				return $info;
			}
		}
		return [];
	}
}

class FuncInfo {
	public $args;
	public $returns;
	function __construct($args, $returns) {
		$this->args = $args;
		$this->returns = $returns;
	}
}

class Function {
	private $locals;
	private $workspace;

	public function __construct($filename) {
		$this->workspace = new Workspace($filename);
		$this->locals = simplexml_load_file($filename);
	}

	private function popExpr($fmt, $name, $type, $index) {
		$n = substr_count($fmt, '%');
		if ($n == 0) return '';
		if ($n == 1) return sprintf($fmt, $name);
		if ($n == 2) return sprintf($fmt, $name, $index);
		if (strstr($fmt, 'strncpy')) return sprintf($fmt, $name, $index, $name);
		return sprintf($fmt, $name, $type, $index);
	}

	private function makeArg($name, $type, $isPointer, $isConst, $index) {
		$ws = $this->workspace;
		$info = $ws->typeInfo($type);
		$decl = sprintf($info['decl'] ?? '%s_t', $type);
		if ($isConst) $decl .= ' const';
		if ($isPointer) $decl .= '*';
		$popFmt = strval($info['pop'] ?? '');
		if ($isPointer) $popFmt = str_replace('= *', '= ', $popFmt);
		$pop = $this->popExpr($popFmt, $name, $type, $index);
		return ['name' => $name, 'decl' => $decl, 'pop' => $pop];
	}

	private function buildArgs($argElems, $struct = null, $isConst = false) {
		$args = [];
		if ($struct !== null) {
			array_push($args, $this->makeArg('this_', strval($struct['name']), true, $isConst, 1));
		}
		foreach ($argElems as $arg) {
			array_push($args, $this->makeArg(
				strval($arg['name']),
				strval($arg['type']),
				$arg['pointer'] == 'true',
				$arg['const'] == 'true',
				count($args) + 1
			));
		}
		return $args;
	}

	private function buildReturns($retElems) {
		$returns = [];
		foreach ($retElems as $ret) {
			$type = strval($ret['type']);
			$ws = $this->workspace;
			$info = $ws->typeInfo($type);
			$decl = sprintf($info['decl'] ?? '%s_t', $type);
			$push = sprintf($info['push'] ?? '', $type, '_result');
			array_push($returns, ['type' => $type, 'decl' => $decl, 'push' => $push, 'nresults' => ($type == 'nresults')]);
		}
		return $returns;
	}

	function getFunctions() {
		$locals = $this->locals;
		foreach ($locals->xpath('//function') as $func) {
			$args = $this->buildArgs($func->arg);
			$returns = $this->buildReturns($func->returns);
			yield strval($func['name']) => new FuncInfo($args, $returns);
		}
		foreach ($locals->xpath('//struct') as $struct) {
			foreach ($struct->method as $method) {
				$args = $this->buildArgs($method->arg, $struct, $method['const'] == 'true');
				$returns = $this->buildReturns($method->returns);
				$prefix = strval($struct['prefix'] ?? ($struct['name'] . '_'));
				yield $prefix . strval($method['name']) => new FuncInfo($args, $returns);
			}
		}
	}
}