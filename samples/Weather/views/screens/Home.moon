import StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core.widget"
import Weather from require "model"

class HomeScreen extends Widget
	title: "Today"

	content: =>
		current = Weather\current!
		StackView class: "bg-background p-6 gap-3", =>
			TextBlock class: "text-xl text-foreground",
				"#{Weather\description current.weather_code}, #{Weather\format_temp current.temperature_2m}"
			TextBlock class: "text-base text-foreground",
				"Feels like #{Weather\format_temp current.apparent_temperature}"
			TextBlock class: "text-sm text-muted-foreground",
				"Wind: #{math.floor current.wind_speed_10m} m/s #{Weather\wind_dir current.wind_direction_10m}"
