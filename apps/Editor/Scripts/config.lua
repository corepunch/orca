local config = {
	max_undos = 256,
	undo_merge_timeout = 300,
	char_width = 8,
	char_height = 16,
	indent = ' ',
	colors = {
		default = '![7;0m',
		group = '![15;0m',
		prefab = '![11;0m',
		view = '![10;0m',
		binding = '![11;0m',
		input = '![9;1m',
		placeholder = '![11;1m',
		property = '![7;0m',
		disabled = '![8;0m',
		textfield = '![10;2m',
		dropdown = '![15;8m',
		link = '![14;6m',
		readonly = '![7;0m',
		button = '![15;0m',
		menu = '![1;15m',
	},
	icons = {
		expanded = '\x1c\x1d',
		collapsed = '\x1e\x1f',
		checked = '\x18\x19\x1a',
		unchecked = '\x18\x1b\x1a',
		-- dropdown = '\x16\x17',
		dropdown = '\x1c\x1d',
		file = '\x06\x07',
		folder = '\x08\x09',
	}
}

return config