import StackView, TextBlock, ImageView from require "orca.UIKit"
Application = require "orca.core.application"
import LOCATIONS, get_current_location, set_current_location, location_subtitle from require "model"

navigate = (route) ->
	app = Application.current false
	app\navigate route if app

select_location = (loc) ->
	set_current_location loc
	navigate "/"

class SavedScreen extends require "orca.core.widget"
	title: "Saved"

	content: =>
		current = get_current_location!
		StackView class: "bg-background p-5 gap-3", =>
			StackView class: "flex-row items-center gap-3", =>
				TextBlock class: "text-2xl font-bold text-foreground", "Saved Locations"
				StackView {
					class: "align-middle-right bg-accent rounded-full p-2"
					LeftButtonUp: -> navigate "/location-search"
				}, =>
					ImageView
						class: "align-middle-center text-accent-foreground"
						Source: "assets/icons/plus.svg?width=22&type=mask"
			for loc in *LOCATIONS
				selected = loc == current
				row_class = selected and
					"bg-accent rounded-3 px-4 py-3 flex-row items-center gap-3" or
					"bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3"
				icon_color = selected and "text-accent-foreground" or "text-accent"
				title_color = selected and "text-accent-foreground" or "text-foreground"
				subtitle_color = selected and "text-accent-subtle" or "text-foreground-muted"
				StackView {
					class: row_class
					LeftButtonUp: -> select_location loc
				}, =>
					ImageView
						class: "align-middle-center #{icon_color}"
						Source: "assets/icons/location.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-base font-bold #{title_color}", loc.name
						TextBlock class: "text-xs #{subtitle_color}", location_subtitle loc
