import StackView, TextBlock, ImageView from require "orca.UIKit"
Widget = require "orca.core.widget"
import Weather from require "model"

-- Number of forecast days to display.
FORECAST_DAYS = 7

class ForecastScreen extends Widget
	title: "Forecast"

	content: =>
		forecast = Weather\forecast!
		StackView class: "bg-slate-900 p-5 gap-3", =>
			TextBlock class: "text-2xl font-bold text-slate-100", "#{FORECAST_DAYS}-Day Forecast"
			for i = 1, math.min(FORECAST_DAYS, #forecast.time)
				code = forecast.weather_code[i]
				desc = Weather\description code
				icon_src = Weather\icon code
				hi   = Weather\format_temp forecast.temperature_2m_max[i]
				lo   = Weather\format_temp forecast.temperature_2m_min[i]
				StackView class: "bg-slate-800 rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center text-sky-400"
						Source: "#{icon_src}?width=28&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-xs text-slate-400", forecast.time[i]
						TextBlock class: "text-sm text-slate-300", desc
					StackView class: "flex-row gap-2 items-center", =>
						TextBlock class: "text-base font-bold text-orange-400", hi
						TextBlock class: "text-sm text-blue-300", lo
