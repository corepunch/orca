<?php

require "model/module.php";

// --- XmlConnection ---
// Analogous to DbConnection in MVC examples.
// Singleton that holds the loaded XML Model so templates do not need
// to pass a Model object around explicitly.

class XmlConnection {
	private static $model = null;

	public static function connect($file) {
		XmlConnection::$model = new Model($file);
	}

	public static function getModel() {
		return XmlConnection::$model;
	}
}

// --- XmlFunction ---
// Analogous to Book in MVC examples.
// Wraps a <function> XML element.  All XML attributes (name, lua, …)
// are copied onto the object by the Base constructor — no explicit
// property declarations are needed.  The public $node property exposes
// the underlying SimpleXMLElement so that child elements are accessible
// for free: foreach ($function->node->arg as $arg) works without any
// getter definitions.  In native PHP, __get also enables the shorthand
// $function->arg directly.

class XmlFunction extends Base {
	public $node;

	public function __construct($elem) {
		parent::__construct($elem, XmlConnection::getModel());
		$this->node = $elem;
	}

	public function __get($name) {
		return $this->_elem->xpath("./$name");
	}

	public function getArgType($arg_elem) {
		return new Type($arg_elem, XmlConnection::getModel());
	}

	public function getReturnType() {
		$rets = $this->_elem->xpath("returns");
		if (count($rets) === 0) return null;
		return new Type($rets[0], XmlConnection::getModel());
	}

	public static function findAll() {
		$model = XmlConnection::getModel();
		foreach ($model->root->xpath(".//function[@name]") as $elem) {
			yield new XmlFunction($elem);
		}
	}
}
