import StackView, TextBlock, ImageView, Input from require "orca.UIKit"
Application = require "orca.core.application"
import add_location, location_subtitle, search_locations from require "model"

search_query = ""

navigate = (route) ->
	app = Application.current false
	app\navigate route if app

choose_location = (loc) ->
	add_location loc
	search_query = ""
	navigate "/saved"

render_results = (query) ->
	results = search_locations query

	if #query == 0
		StackView class: "bg-surface rounded-3 px-4 py-4 gap-2", =>
			TextBlock class: "text-base font-bold text-foreground", "Search for a city"
			TextBlock class: "text-sm text-foreground-muted",
				"Type a city name, then choose the matching region."
	elseif #results == 0
		StackView class: "bg-surface rounded-3 px-4 py-4 gap-2", =>
			TextBlock class: "text-base font-bold text-foreground", "No matches"
			TextBlock class: "text-sm text-foreground-muted",
				"Try a larger nearby city or check the spelling."
	else
		StackView class: "gap-2 overflow-y-scroll h-full", =>
			for loc in *results
				StackView {
					class: "bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3"
					LeftButtonUp: -> choose_location loc
				}, =>
					ImageView
						class: "align-middle-center text-accent"
						Source: "assets/icons/location.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-base font-bold text-foreground", loc.name
						TextBlock class: "text-xs text-foreground-muted", location_subtitle loc

class LocationSearchScreen extends require "orca.core.widget"
	title: "Add Location"

	footer: =>
		StackView class: "bg-slate-900"

	content: =>
		StackView class: "bg-background p-5 gap-4", =>
			results_view = nil
			update_query = (sender) ->
				search_query = sender.Text or ""
				results_view\rebuild -> render_results search_query

			StackView class: "flex-row items-center gap-3", =>
				TextBlock {
					class: "text-sm font-bold text-accent px-2 py-2"
					LeftButtonUp: -> navigate "/saved"
				}, "Back"
				search_input = Input {
					Text: search_query
					PlaceholderText: "Search city"
					class: "w-full bg-surface-alt text-foreground placeholder-foreground-muted rounded-3 px-4 py-3"
					TextInput: update_query
				}
				search_input\setFocus!

			results_view = StackView class: "h-full", =>
				render_results search_query
