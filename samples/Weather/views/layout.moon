import Screen, Grid, StackView, TextBlock, ImageView, Node2D from require "orca.UIKit"
Widget = require "orca.core.widget"

NAV_ITEMS = {
	{ route: "/", label: "Home", icon: "assets/icons/home.svg" }
	{ route: "/forecast", label: "Forecast", icon: "assets/icons/search.svg" }
	{ route: "/alerts", label: "Alerts", icon: "assets/icons/chat.svg" }
	{ route: "/saved", label: "Saved", icon: "assets/icons/bookmark.svg" }
}

make_header = (title) ->
	StackView class: "bg-violet-900 px-5 py-3 justify-center", =>
		TextBlock class: "text-2xl align-center", title

make_footer = (active_route, navigate) ->
	StackView class: "bg-violet-900 flex-row justify-evenly items-center p-2", =>
		for item in *NAV_ITEMS
			selected = active_route == item.route
			icon_color = selected and "text-foreground" or "text-muted-foreground"
			label_color = selected and "text-foreground" or "text-muted-foreground"
			StackView class: "w-12 flex-col items-center justify-center gap-1", =>
				ImageView
					class: "align-middle-center text-blue-200"--#{icon_color}"
					Source: "#{item.icon}?width=48&type=mask"
					LeftButtonUp: -> navigate item.route
				TextBlock class: "text-xs #{label_color}", item.label

make_placeholder = ->
	StackView class: "p-6 gap-2", =>
		TextBlock class: "text-base text-muted-foreground", "No content for this route"

class Default extends Widget
	content: =>
		inner = @content_for "inner"
		title = if inner and inner.title then inner.title else if @app_title then @app_title! else "Weather"
		route_value = @current_route
		active_route = if type(route_value) == "function" then route_value! else route_value or "/"
		navigate = @navigate

		Screen ->
			Grid Rows: "32px 48px 1fr 72px 24px", =>
				Node2D class: "bg-violet-900/70"
				make_header title
				@addChild (inner or make_placeholder!)
				make_footer active_route, navigate -- this causes following node to be darker
				Node2D class: "bg-violet-900/70"
		