import StackView, TextBlock from require "orca.UIKit"
RadioGroup  = require "orca.UIKit.RadioGroup"
Widget      = require "orca.core.widget"
Application = require "orca.core.application"
import Settings from require "model"

UNIT_OPTIONS = {
  { value: "celsius",    label: "°C  Celsius"    }
  { value: "fahrenheit", label: "°F  Fahrenheit" }
}

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
				RadioGroup
					value: unit
					options: UNIT_OPTIONS
					on_change: (v) ->
						Settings\set_unit v
						app\navigate "/settings"
