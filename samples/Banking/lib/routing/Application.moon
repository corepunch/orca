orca = require "orca"
ui = require "orca.ui"

each_route = (obj, callback) ->
	for path, handler in pairs obj
		-- confirm it's a route: either path is a string that starts with "/" or
		-- it's a table of { name: path }
		switch type path
			when "string"
				continue unless path\match "^/"
			when "table"
				k = next(path)
				continue unless type(k) == "string" or type(path[k]) == "string"
			else
				continue
		callback path, handler

	obj_mt = getmetatable obj
	if obj_mt and type(obj_mt.__index) == "table"
		each_route obj_mt.__index, callback

get_target_route_group = (obj) ->
	assert obj != Application, "Application is not able to be modified with routes. You must either subclass or instantiate it"
	if obj == obj.__class
		obj.__base
	else
		obj

class Application extends ui.Screen
	new: => 
		super!
		@history = {}
		@routes = {}
		@ResizeMode = 'canresize'
		orca.router = @
		for k, v in pairs getmetatable(@)
			if type(k) == 'table'
				for _k, _v in pairs k -- [name: "/url"] table
					if string.byte(_v, 1) == 47
						@routes[_v] = v
						@[_k] = v
			elseif type(k) == 'string'
				if string.byte(k, 1) == 47 -- / symbol
					@routes[k] = v
		for style in *@__styles
			for k, v in pairs style
				@addStyleSheet k, v
		@navigate "/"
		@ready!

	ready: =>

	route: (path, element) =>
		@routes[path] = element

	matchRoute: (url, route) =>
		split = (str, sep) -> [part for part in str\gmatch "[^/]+"]
		params = {}
		a, b = (split url), (split route)
		if #a != #b return nil
		for i = 1, #a
			if (b[i]\byte 1) == 58
				params[b[i]\sub 2] = a[i]
			elseif a[i] != b[i]
				return nil
		return params

	go_back: () =>
		table.remove @history 
		url = table.remove @history
		@navigate url, true
		
	navigate: (url, store) =>
		orca.angle = 90
		orca.location = url
		if not store
			@history = { url }
		else
			table.insert(@history, url)
		@params = {}
		for route, body in pairs @routes
			@params = @matchRoute url, route
			if @params
				@rebuild body
				return

	include: (other_app) =>
		if type(other_app) == 'string'
			other_app = require other_app
		source = get_target_route_group other_app
		into = get_target_route_group @
		each_route source, (path, action) ->
			into[path] = action

	__styles: {}

	stylesheet: (src) =>
		import parse from require "orca.parsers.css"
		import read_file from require "orca.filesystem"
		into = get_target_route_group @
		style = if (type src) ~= 'string' then src else parse read_file src
		table.insert(into.__styles, style)
