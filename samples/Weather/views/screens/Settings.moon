import StackView, TextBlock, RadioGroup, RadioButton from require "orca.UIKit"
import Settings from require "model"

temperature_units = {
	{ value: "celsius", label: "Celsius (°C)" }
	{ value: "fahrenheit", label: "Fahrenheit (°F)" }
}

class SettingsScreen extends require "orca.core.widget"
	title: "Settings"

	content: =>
		unit = Settings\get_unit!

		StackView class: "bg-background p-5 gap-4", =>
			TextBlock class: "text-2xl font-bold text-foreground", "Settings"

			-- Temperature unit card
			StackView class: "bg-surface rounded-3 px-4 py-4 gap-3", =>
				TextBlock class: "text-xs font-bold text-foreground-muted", "TEMPERATURE UNIT"
				RadioGroup {
					SelectedValue: unit
					Direction:     "Vertical"
					Spacing:       10
					SelectionChanged: (self) -> Settings.set_unit self.SelectedValue
				}, =>
					for option in *temperature_units
						RadioButton {
							Value:  option.value
							class:  "w-full px-4 py-3 rounded-3 bg-surface-alt text-foreground active:bg-accent active:text-accent-foreground"
						}, option.label
