import StackView, TextBlock from require "orca.UIKit"

=>
	view = StackView class: "p-6 gap-2", =>
		TextBlock class: "text-xl", "7-Day Forecast"
		TextBlock class: "text-base", "Mon: 23 C / 14 C, Clear"
		TextBlock class: "text-base", "Tue: 21 C / 13 C, Partly Cloudy"
		TextBlock class: "text-base", "Wed: 19 C / 11 C, Showers"
		TextBlock class: "text-base", "Thu: 20 C / 12 C, Windy"
	view.title = "Forecast"
	view
