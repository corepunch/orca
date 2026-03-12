<?php foreach ($list as $field) {
	echo("\t{$field->type} {$field->name}"); 
	if ($field->type->fixed_array) {
		echo("[{$field->type->fixed_array}];"); 
	} else {
		echo(";");
	}
	if ($field->doc) echo(" ///< {$field->doc}");
	echo("\n");
}
?>