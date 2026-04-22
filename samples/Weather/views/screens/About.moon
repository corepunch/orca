import StackView, TextBlock from require "orca.UIKit"

class AboutScreen extends StackView
	title: "About"
	body: =>
		@class = "p-6 gap-3"
		TextBlock class: "text-xl", "About"
		TextBlock class: "text-base", "Weather demo with bottom navigation and route-driven content."

=> AboutScreen!
