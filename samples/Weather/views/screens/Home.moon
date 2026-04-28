import StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core.widget"
import Weather from require "model"

class HomeScreen extends Widget
	title: "Today"

	content: =>
		-- network.fetch yields a coroutine; guard against the initial
		-- synchronous render that happens before the event loop starts.
		ok, current = pcall -> Weather\current!

		StackView class: "bg-background p-6 gap-3", =>
			if ok and current
				TextBlock class: "text-xl text-foreground",
					"#{Weather\description current.weather_code}, #{Weather\format_temp current.temperature_2m}"
				TextBlock class: "text-base text-foreground",
					"Feels like #{Weather\format_temp current.apparent_temperature}"
				TextBlock class: "text-sm text-muted-foreground",
					"Wind: #{math.floor current.wind_speed_10m} m/s #{Weather\wind_dir current.wind_direction_10m}"
			else
				TextBlock class: "text-xl text-foreground", "Today"
				TextBlock class: "text-base text-muted-foreground", "Loading weather data..."
