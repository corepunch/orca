<?php

require "model/config_new.php";

class Function {
	private $locals;
	private $workspace;

	public function __construct($filename) {
		$this->workspace = [];
		$this->loadWs($filename);
		$this->locals = simplexml_load_file($filename);
	}

	function loadWs($filename) {
		$tmp = simplexml_load_file($filename);
		array_push($this->workspace, $tmp);
		foreach ($tmp->require as $req) $this->loadWs(dirname($filename).'/'.$req['file']);
	}

	function typeInfo($type) {
		foreach (config_new::$TypeInfos as $kind => $info)
			if ($type == $kind || array_filter($this->workspace, fn($w) => $w->xpath("//{$kind}[@name='{$type}']")))
				return $info;
		return [];
	}

	function makeArg($name, $type, $ptr, $const, $i) {
		$info = $this->typeInfo($type);
		$decl = sprintf($info['decl'] ?? '%s_t', $type) . ($const ? ' const' : '') . ($ptr ? '*' : '');
		$fmt = $ptr ? str_replace('= *', '= ', $info['pop'] ?? '') : ($info['pop'] ?? '');
		$n = substr_count($fmt, '%');
		$pop = !$n ? '' : ($n == 1 ? sprintf($fmt, $name) : ($n == 2 ? sprintf($fmt, $name, $i) : (strstr($fmt, 'strncpy') ? sprintf($fmt, $name, $i, $name) : sprintf($fmt, $name, $type, $i))));
		return ['name' => $name, 'decl' => $decl, 'pop' => $pop];
	}

	function buildArgs($elems, $struct = null, $const = false) {
		$args = [];
		if ($struct) array_push($args, $this->makeArg('this_', strval($struct['name']), true, $const, 1));
		foreach ($elems as $arg) array_push($args, $this->makeArg(strval($arg['name']), strval($arg['type']), $arg['pointer'] == 'true', $arg['const'] == 'true', count($args) + 1));
		return $args;
	}

	function buildReturns($elems) {
		$rets = [];
		foreach ($elems as $ret) {
			$type = strval($ret['type']); $info = $this->typeInfo($type);
			array_push($rets, ['type' => $type, 'decl' => sprintf($info['decl'] ?? '%s_t', $type), 'push' => sprintf($info['push'] ?? '', $type, '_result'), 'nresults' => $type == 'nresults']);
		}
		return $rets;
	}

	function getFunctions() {
		$locals = $this->locals;
		foreach ($locals->xpath('//function') as $func)
			yield strval($func['name']) => ['args' => $this->buildArgs($func->arg), 'returns' => $this->buildReturns($func->returns)];
		foreach ($locals->xpath('//struct') as $struct)
			foreach ($struct->method as $method)
				yield strval($struct['prefix'] ?? ($struct['name'] . '_')) . strval($method['name']) => ['args' => $this->buildArgs($method->arg, $struct, $method['const'] == 'true'), 'returns' => $this->buildReturns($method->returns)];
	}
}