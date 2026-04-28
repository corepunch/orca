import StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core.widget"
import Weather from require "model"

class ForecastScreen extends Widget
	title: "Forecast"

	content: =>
		ok, forecast = pcall -> Weather\forecast!

		StackView class: "bg-background p-6 gap-2", =>
			TextBlock class: "text-xl text-foreground", "7-Day Forecast"
			if ok and forecast
				for i = 1, math.min(7, #forecast.time)
					desc = Weather\description forecast.weather_code[i]
					hi   = Weather\format_temp forecast.temperature_2m_max[i]
					lo   = Weather\format_temp forecast.temperature_2m_min[i]
					TextBlock class: "text-base text-foreground",
						"#{forecast.time[i]}: #{hi} / #{lo}  #{desc}"
			else
				TextBlock class: "text-base text-muted-foreground", "Loading forecast..."
