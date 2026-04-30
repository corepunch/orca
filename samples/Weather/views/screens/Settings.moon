import StackView, TextBlock, RadioGroup, RadioButton from require "orca.UIKit"
import Settings from require "model"

temperature_units = {
	{ value: "celsius", label: "°C  Celsius" }
	{ value: "fahrenheit", label: "°F  Fahrenheit" }
}

class SettingsScreen extends require "orca.core.widget"
	title: "Settings"

	content: =>
		unit = Settings\get_unit!

		StackView class: "bg-slate-900 p-5 gap-4", =>
			TextBlock class: "text-2xl font-bold text-slate-100", "Settings"

			-- Temperature unit card
			StackView class: "bg-slate-800 rounded-3 px-4 py-4 gap-3", =>
				TextBlock class: "text-xs font-bold text-slate-400", "TEMPERATURE UNIT"
				RadioGroup {
					SelectedValue: unit
					Direction:     "Horizontal"
					Spacing:       8
					SelectionChanged: (self) -> Settings.set_unit self.SelectedValue
				}, =>
					for option in *temperature_units
						RadioButton {
							Value:     option.value
							IsChecked: unit == option.value
							class:     "px-4 py-2 rounded-full bg-slate-700 text-slate-200 active:bg-sky-500 active:text-white"
						}, option.label
