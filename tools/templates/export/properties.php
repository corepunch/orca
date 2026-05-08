	<?php foreach ($properties as $property):?>
		<?php
		$short   = $property->name->id;
		$cls     = $name;
		$pname   = $property->name;
		$field   = $property->name->addr;
		$kind    = $property->type->kind;
		$is_arr  = $property->type->array === "true";
		$pfx     = $is_arr ? "ARRAY_DECL" : "DECL";
		$comment = "// $cls.{$property->name}";

		// Kind overrides — mirror the logic that was previously in the datatype variable
		if ($kind === 'component') $kind = 'object';
		if ($kind === 'struct' && $property->type->type == 'color') $kind = 'color';
		if ($property->type->type == 'uint') $kind = 'int';
		if ($kind === 'external_struct') $kind = 'struct';
		if ($kind === 'interface') $kind = 'struct';

		switch ($kind) {
			case 'float':
				echo("\t{$pfx}_FLOAT($short, $cls, $pname, $field), $comment\n");
				break;
			case 'int':
				echo("\t{$pfx}_INT($short, $cls, $pname, $field), $comment\n");
				break;
			case 'bool':
				echo("\t{$pfx}_BOOL($short, $cls, $pname, $field), $comment\n");
				break;
			case 'string':
				echo("\t{$pfx}_STRING($short, $cls, $pname, $field), $comment\n");
				break;
			case 'color':
				echo("\t{$pfx}_COLOR($short, $cls, $pname, $field), $comment\n");
				break;
			case 'enum':
				echo("\t{$pfx}_ENUM($short, $cls, $pname, $field, {$property->type->type}), $comment\n");
				break;
			case 'struct':
				echo("\t{$pfx}_STRUCT_TYPE($short, $cls, $pname, $field, {$property->type->export}), $comment\n");
				break;
			case 'object':
				echo("\t{$pfx}_OBJECT($short, $cls, $pname, $field, {$property->type->export}), $comment\n");
				break;
			default:
				// Generic fallback — should not be reached for well-formed XML
				$dt = 'kDataType' . ucfirst($kind);
				echo("\t{$pfx}($short, $cls, $pname, $field, $dt), $comment\n");
				break;
		}
		?>
	<?php endforeach ?>

