import Screen, Grid, StackView, TextBlock, ImageView, Node2D from require "orca.UIKit"
Widget = require "orca.core.widget"

NAV_ITEMS = {
	{ route: "/", label: "Home", icon: "assets/icons/home.svg" }
	{ route: "/forecast", label: "Forecast", icon: "assets/icons/search.svg" }
	{ route: "/alerts", label: "Alerts", icon: "assets/icons/chat.svg" }
	{ route: "/saved", label: "Saved", icon: "assets/icons/bookmark.svg" }
}

make_header = (title) ->
	StackView class: "bg-slate-800 px-5 py-3 justify-center", =>
		TextBlock class: "text-2xl font-bold align-center text-slate-100", title

make_footer = (active_route, navigate) ->
	StackView class: "bg-slate-800 flex-row justify-evenly items-center p-2", =>
		for item in *NAV_ITEMS
			selected = active_route == item.route
			color = selected and "text-sky-400" or "text-slate-500"
			weight = selected and "font-bold" or "font-normal"
			StackView class: "w-12 flex-col items-center justify-center gap-1", =>
				ImageView
					class: "align-middle-center #{color}"
					Source: "#{item.icon}?width=28&type=mask"
					LeftButtonUp: -> navigate item.route
				TextBlock class: "text-xs #{color} #{weight}", item.label

make_placeholder = ->
	StackView class: "bg-slate-900 p-6 gap-2", =>
		TextBlock class: "text-base text-slate-400", "No content for this route"

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
				Node2D class: "bg-slate-800"
				make_header title
				@addChild (inner or make_placeholder!)
				@addChild (footer or make_footer active_route, navigate)
				Node2D class: "bg-slate-800"
		