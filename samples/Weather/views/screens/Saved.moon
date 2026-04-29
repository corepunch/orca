import StackView, TextBlock, ImageView from require "orca.UIKit"
Widget = require "orca.core.widget"
import LOCATIONS from require "model"

class SavedScreen extends Widget
	title: "Saved"

	content: =>
		StackView class: "bg-slate-900 p-5 gap-3", =>
			TextBlock class: "text-2xl font-bold text-slate-100", "Saved Locations"
			for loc in *LOCATIONS
				StackView class: "bg-slate-800 rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center text-sky-400"
						Source: "assets/icons/location.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-base font-bold text-slate-100", loc.name
						TextBlock class: "text-xs text-slate-400",
							"#{loc.lat}, #{loc.lon}"
