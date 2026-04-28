import StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core.widget"
import Weather from require "model"

-- Number of forecast days to display.
FORECAST_DAYS = 7

class ForecastScreen extends Widget
	title: "Forecast"

	content: =>
		forecast = Weather\forecast!
		StackView class: "bg-background p-6 gap-2", =>
			TextBlock class: "text-xl text-foreground", "#{FORECAST_DAYS}-Day Forecast"
			for i = 1, math.min(FORECAST_DAYS, #forecast.time)
				desc = Weather\description forecast.weather_code[i]
				hi   = Weather\format_temp forecast.temperature_2m_max[i]
				lo   = Weather\format_temp forecast.temperature_2m_min[i]
				TextBlock class: "text-base text-foreground",
					"#{forecast.time[i]}: #{hi} / #{lo}  #{desc}"
