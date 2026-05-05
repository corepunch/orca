import StackView, TextBlock, ImageView from require "orca.UIKit"
import Weather, get_current_location from require "model"

-- Number of forecast days to display.
FORECAST_DAYS = 7

class ForecastScreen extends require "orca.core.widget"
	title: "Forecast"

	content: =>
		forecast = Weather\forecast!
		location = get_current_location!
		StackView class: "bg-background overflow-y-scroll h-full p-5 gap-3", =>
			TextBlock class: "text-2xl font-bold text-foreground", "#{FORECAST_DAYS}-Day Forecast"
			-- Location row (consistent with Home screen)
			StackView class: "flex-row items-center gap-2", =>
				ImageView
					class: "align-middle-center text-accent"
					Source: "assets/icons/location.svg?width=16&type=mask"
				TextBlock class: "text-sm text-foreground-muted", location.name
			for i = 1, math.min(FORECAST_DAYS, #forecast.time)
				code  = forecast.weather_code[i]
				desc  = Weather\description code
				icon_src = Weather\icon code
				hi    = Weather\format_temp forecast.temperature_2m_max[i]
				lo    = Weather\format_temp forecast.temperature_2m_min[i]
				prec  = (forecast.precipitation_probability_max or {})[i] or 0
				StackView class: "bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center"
						Source: "#{icon_src}?width=28"
					StackView class: "gap-1", =>
						TextBlock class: "text-xs text-foreground-muted", forecast.time[i]
						TextBlock class: "text-sm text-foreground", desc
					StackView class: "gap-1 items-end", =>
						StackView class: "flex-row gap-2 items-center", =>
							TextBlock class: "text-base font-bold text-orange-400", hi
							TextBlock class: "text-sm text-blue-300", lo
						StackView class: "flex-row items-center gap-1", =>
							ImageView
								class: "align-middle-center text-blue-300"
								Source: "assets/icons/weather/drizzle.svg?width=14&type=mask"
							TextBlock class: "text-xs text-blue-300", "#{prec}%"
