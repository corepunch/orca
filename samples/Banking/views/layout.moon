import Screen, Grid, StackView, TextBlock, ImageView, Node2D from require "orca.UIKit"
Widget = require "orca.core.widget"

NAV_ITEMS = {
	{ route: "/",           icon: "assets/icons/home.svg",   label: "Home"   }
	{ route: "/send-money", icon: "assets/icons/people.svg", label: "Send"   }
	{ route: "/new-tweet",  icon: "assets/icons/edit.svg",   label: "Write"  }
	{ route: "/tweets",     icon: "assets/icons/chat.svg",   label: "Tweets" }
	{ route: "/search",     icon: "assets/icons/search.svg", label: "Search" }
}

make_header = (title) ->
	StackView class: "bg-header-background px-5 py-3 justify-center", =>
		TextBlock class: "text-2xl font-bold align-center text-accent-foreground", title

make_footer = (active_route, navigate) ->
	StackView class: "bg-header-background flex-row justify-evenly items-center p-2", =>
		for item in *NAV_ITEMS
			selected = active_route == item.route
			color  = selected and "text-accent" or "text-foreground-muted"
			weight = selected and "font-bold"   or "font-normal"
			StackView class: "w-12 flex-col items-center justify-center gap-1", =>
				ImageView
					class: "align-middle-center #{color}"
					Source: "#{item.icon}?width=28&type=mask"
					LeftButtonUp: -> navigate item.route
				TextBlock class: "text-xs #{color} #{weight}", item.label

make_placeholder = ->
	StackView class: "bg-background p-6 gap-2", =>
		TextBlock class: "text-base text-foreground-muted", "No content for this route"

class Default extends Widget
	content: =>
		inner      = @content_for "inner"
		title_slot = @content_for "title"
		no_chrome  = @content_for "no_chrome"
		navigate   = @navigate
		active_route = @path or "/"
		footer     = @content_for("footer") or make_footer active_route, navigate

		-- Compute title: prefer the slot set by the screen, then the app helper, then the default.
		title = title_slot
		unless title
			title = if @app_title then @app_title! else "Banking"

		-- Auth screens opt out of the navigation chrome.
		if no_chrome
			return Screen =>
				@addChild (inner or make_placeholder!)

		Screen ->
			Grid Rows: "56px 1fr 72px", =>
				make_header title
				@addChild (inner or make_placeholder!)
				@addChild footer
