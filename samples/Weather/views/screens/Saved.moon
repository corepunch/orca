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
		StackView class: "bg-slate-900 p-5 gap-3", =>
			StackView class: "flex-row items-center gap-3", =>
				TextBlock class: "text-2xl font-bold text-slate-100", "Saved Locations"
				StackView {
					class: "align-middle-right bg-sky-500 rounded-full p-2"
					LeftButtonUp: -> navigate "/location-search"
				}, =>
					ImageView
						class: "align-middle-center text-white"
						Source: "assets/icons/plus.svg?width=22&type=mask"
			for loc in *LOCATIONS
				selected = loc == current
				row_class = selected and
					"bg-sky-500 rounded-3 px-4 py-3 flex-row items-center gap-3" or
					"bg-slate-800 rounded-3 px-4 py-3 flex-row items-center gap-3"
				icon_color = selected and "text-white" or "text-sky-400"
				title_color = selected and "text-white" or "text-slate-100"
				subtitle_color = selected and "text-sky-100" or "text-slate-400"
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
