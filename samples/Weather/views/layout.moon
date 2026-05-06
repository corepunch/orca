import Screen, Grid, UniformGrid, StackView, TextBlock, ImageView, Node2D from require "orca.UIKit"
Widget = require "orca.core.widget"

NAV_ITEMS = {
	{ route: "/", label: "Home", icon: "assets/icons/home.svg" }
	{ route: "/forecast", label: "Forecast", icon: "assets/icons/calendar.svg" }
	{ route: "/alerts", label: "Alerts", icon: "assets/icons/alert.svg" }
	{ route: "/saved", label: "Saved", icon: "assets/icons/bookmark.svg" }
	{ route: "/settings", label: "Settings", icon: "assets/icons/settings.svg" }
}

make_header = (title) ->
	StackView class: "bg-header-bg px-5 py-3 justify-center", =>
		TextBlock class: "text-2xl font-bold align-center text-accent-foreground", title

make_footer = (active_route, navigate) ->
	UniformGrid class: "bg-header-bg p-2", =>
			for item in *NAV_ITEMS
				selected = active_route == item.route
				color = selected and "text-amber-300" or "text-sky-100"
				weight = selected and "font-bold" or "font-normal"
				cell = StackView class: "w-full h-full flex-col items-center justify-center gap-1", LeftButtonUp: -> navigate item.route
				cell\addChild ImageView
					class: "align-middle-center #{color}"
					Source: "#{item.icon}?width=36&type=mask"
				cell\addChild TextBlock class: "text-xs #{color} #{weight}", item.label

make_placeholder = ->
	StackView class: "bg-background p-6 gap-2", =>
		TextBlock class: "text-base text-foreground-muted", "No content for this route"

class Default extends Widget
	content: =>
		inner = @content_for "inner"
		title_slot = @content_for "title"
		title = if title_slot then title_slot else if @app_title then @app_title! else "Weather"
		route_value = @current_route
		active_route = if type(route_value) == "function" then route_value! else route_value or "/"
		navigate = @navigate
		footer = @content_for("footer") or make_footer active_route, navigate

		Screen ->
			Grid Rows: "32px 52px 1fr 72px 24px", =>
				Node2D class: "bg-header-bg"
				make_header title
				@addChild (inner or make_placeholder!)
				@addChild (footer or make_footer active_route, navigate)
				Node2D class: "bg-header-bg"
		
