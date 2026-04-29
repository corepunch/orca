import StackView, TextBlock, ImageView from require "orca.UIKit"
Widget = require "orca.core.widget"
import Weather from require "model"

class HomeScreen extends Widget
	title: "Today"

	content: =>
		current = Weather\current!
		icon_src = Weather\icon current.weather_code
		StackView class: "bg-background p-6 gap-3", =>
			StackView class: "flex-row items-center gap-4", =>
				ImageView
					class: "align-middle-center text-foreground"
					Source: "#{icon_src}?width=64&type=mask"
				StackView class: "gap-1", =>
					TextBlock class: "text-xl text-foreground",
						"#{Weather\description current.weather_code}, #{Weather\format_temp current.temperature_2m}"
					TextBlock class: "text-base text-muted-foreground",
						"Feels like #{Weather\format_temp current.apparent_temperature}"
			TextBlock class: "text-sm text-muted-foreground",
				"Wind: #{math.floor current.wind_speed_10m} m/s #{Weather\wind_dir current.wind_direction_10m}"
