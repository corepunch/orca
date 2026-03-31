	<?php foreach ($properties as $property):?>
		<?php $datatype = 'kDataType' . ucfirst($property->type->kind) ?>
		<?php if ($property->type->kind === 'component') $datatype = 'kDataTypeObject'; ?>
		<?php if ($property->type->kind === 'struct' && $property->type->type == 'color') $datatype = 'kDataTypeColor'; ?>
		<?php if ($property->type->type == 'uint') $datatype = 'kDataTypeInt'; ?>
		<?php if ($property->type->kind == 'external_struct') $datatype = 'kDataTypeStruct'; ?>
		<?php echo($property->type->array ? "\tARRAY_DECL(" : "\tDECL(") ?>
		<?php echo("{$property->name->id}, $name, {$property->name}, {$property->name->addr}, $datatype") ?>
		<?php if ($property->type->kind === 'enum') {
			echo (", .EnumValues = _" . $property->type->type . "), // $name.{$property->name}\n");
		} elseif ($property->type->kind === 'external_struct') {
			echo (", .TypeString = \"{$property->type->export}\"), // $name.{$property->name}\n");
		} elseif ($property->type->kind === 'struct' && $property->type->type != 'color') {
			echo (", .TypeString = \"{$property->type->export}\"), // $name.{$property->name}\n");
		} elseif ($property->type->kind === 'component') {
			echo (", .TypeString = \"{$property->type->export}\"), // $name.{$property->name}\n");
 		} else {
			echo ("), // $name.{$property->name}\n");
 		} ?>
	<?php endforeach ?>
