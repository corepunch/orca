import Screen, Grid, StackView, TextBlock, ImageView, Node2D from require "orca.UIKit"
Widget = require "orca.core.widget"

NAV_ITEMS = {
	{ route: "/", label: "Home", icon: "assets/icons/home.svg" }
	{ route: "/forecast", label: "Forecast", icon: "assets/icons/search.svg" }
	{ route: "/alerts", label: "Alerts", icon: "assets/icons/chat.svg" }
	{ route: "/saved", label: "Saved", icon: "assets/icons/bookmark.svg" }
}

make_header = (title) ->
	header = StackView {
		Height: 64
		class: "px-5 py-3 justify-center"
	}, =>
		TextBlock {
			Text: title
			class: "text-2xl"
		}
	return header

make_footer = (active_route, navigate) ->
	footer = StackView {
		Name: "FooterStack"
		Height: 72
		Direction: "Horizontal"
		JustifyContent: "SpaceEvenly"
		AlignItems: "Center"
		BackgroundColor: "#405060"
		class: "p-2"
	}, ->
		for item in *NAV_ITEMS
			selected = active_route == item.route
			icon_color = selected and "text-foreground" or "text-muted-foreground"
			label_color = selected and "text-foreground" or "text-muted-foreground"
			Node2D class: "items-center justify-center gap-1", ->
				ImageView {
					class: "align-middle-center #{icon_color}"
					Source: "#{item.icon}?width=72&mask=true"
					LeftButtonUp: -> navigate item.route
				}
			-- print('adding')
			-- StackView class: "items-center justify-center gap-1", =>
			-- 	ImageView {
			-- 		class: "align-middle-center #{icon_color}"
			-- 		Source: "#{item.icon}?width=26&mask=true"
			-- 		LeftButtonUp: -> navigate item.route
			-- 	}
			-- 	TextBlock {
			-- 		Text: item.label
			-- 		class: "text-xs #{label_color}"
			-- 	}

make_placeholder = ->
	placeholder = StackView class: "p-6 gap-2"
	placeholder + TextBlock class: "text-base text-muted-foreground", "No content for this route"
	return placeholder

class Default extends Widget
	content: =>
		inner = @content_for "inner"
		title = if inner and inner.title then inner.title else if @app_title then @app_title! else "Weather"
		route_value = @current_route
		active_route = if type(route_value) == "function" then route_value! else route_value or "/"
		navigate = @navigate

		screen = Screen!
		screen + Grid Rows: "64px 1fr 72px", =>
			make_header title
			@addChild (inner or make_placeholder!)
			make_footer active_route, navigate
		return screen
		