import StackView, TextBlock, RadioGroup, RadioButton from require "orca.UIKit"
Widget      = require "orca.core.widget"
Application = require "orca.core.application"
import Settings from require "model"

class SettingsScreen extends Widget
	title: "Settings"

	content: =>
		unit = Settings\get_unit!
		app  = Application.current!

		StackView class: "bg-slate-900 p-5 gap-4", =>
			TextBlock class: "text-2xl font-bold text-slate-100", "Settings"

			-- Temperature unit card
			StackView class: "bg-slate-800 rounded-3 px-4 py-4 gap-3", =>
				TextBlock class: "text-xs font-bold text-slate-400", "TEMPERATURE UNIT"
				RadioGroup {
					SelectedValue: unit
					Direction:     "Horizontal"
					Spacing:       8
					SelectionChanged: (self) ->
						Settings\set_unit self.SelectedValue
						app\navigate "/settings"
				}, =>
					RadioButton {
						Value:     "celsius"
						IsChecked: unit == "celsius"
						class:     "px-4 py-2 rounded-full " .. (unit == "celsius" and "bg-sky-500 text-white" or "bg-slate-700 text-slate-200")
					}, "°C  Celsius"
					RadioButton {
						Value:     "fahrenheit"
						IsChecked: unit == "fahrenheit"
						class:     "px-4 py-2 rounded-full " .. (unit == "fahrenheit" and "bg-sky-500 text-white" or "bg-slate-700 text-slate-200")
					}, "°F  Fahrenheit"

