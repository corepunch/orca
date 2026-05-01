import StackView, TextBlock, ImageView from require "orca.UIKit"
import Application from require "orca.core.application"
import LOCATIONS, get_location, set_location from require "model"

class SavedScreen extends require "orca.core.widget"
	title: "Saved"

	content: =>
		current = get_location!
		StackView class: "bg-slate-900 p-5 gap-3", =>
			TextBlock class: "text-2xl font-bold text-slate-100", "Saved Locations"
			for loc in *LOCATIONS
				is_active = loc == current
				bg = is_active and "bg-sky-900" or "bg-slate-800"
				name_color = is_active and "text-sky-300" or "text-slate-100"
				StackView {
					class: "#{bg} rounded-3 px-4 py-3 flex-row items-center gap-3"
					LeftButtonUp: ->
						set_location loc
						Application\current!\navigate "/"
				}, =>
					ImageView
						class: "align-middle-center #{is_active and 'text-sky-400' or 'text-slate-400'}"
						Source: "assets/icons/location.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-base font-bold #{name_color}", loc.name
						TextBlock class: "text-xs text-slate-400",
							"#{loc.lat}, #{loc.lon}"
