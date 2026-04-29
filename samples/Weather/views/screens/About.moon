import StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core.widget"

class AboutScreen extends Widget
	title: "About"

	content: =>
		StackView class: "bg-slate-900 p-5 gap-3", =>
			TextBlock class: "text-2xl font-bold text-slate-100", "About"
			StackView class: "bg-slate-800 rounded-3 px-4 py-3 gap-2", =>
				TextBlock class: "text-base font-bold text-sky-400", "Weather Demo"
				TextBlock class: "text-sm text-slate-400",
					"Route-driven app with bottom navigation, live weather data, and a 7-day forecast."
			StackView class: "bg-slate-800 rounded-3 px-4 py-3 gap-2", =>
				TextBlock class: "text-base font-bold text-sky-400", "Data Source"
				TextBlock class: "text-sm text-slate-400",
					"Powered by Open-Meteo — free weather API, no key required."
