import StackView, TextBlock, ImageView from require "orca.UIKit"
import Weather, get_current_location from require "model"

class HomeScreen extends require "orca.core.widget"
	title: "Today"

	content: =>
		current = Weather\current!
		icon_src = Weather\icon current.weather_code
		desc = Weather\description current.weather_code
		location_name = get_current_location!.name

		StackView class: "bg-background p-5 gap-4", =>
			-- Location row
			StackView class: "flex-row items-center gap-2", =>
				ImageView
					class: "align-middle-center text-accent"
					Source: "assets/icons/location.svg?width=16&type=mask"
				TextBlock class: "text-sm text-foreground-muted", location_name

			-- Main weather card
			StackView class: "bg-surface rounded-3 p-5 gap-3", =>
				StackView class: "flex-row items-center gap-4", =>
					ImageView
						class: "align-middle-center text-accent"
						Source: "#{icon_src}?width=80&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-5xl font-bold text-foreground",
							Weather\format_temp current.temperature_2m
						TextBlock class: "text-base text-foreground-muted", desc

			-- Stat cards
			StackView class: "gap-3", =>
				-- Feels like
				StackView class: "bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center text-accent"
						Source: "assets/icons/thermometer.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-xs text-foreground-muted", "Feels like"
						TextBlock class: "text-base font-bold text-foreground",
							Weather\format_temp current.apparent_temperature

				-- Wind
				StackView class: "bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center text-accent"
						Source: "assets/icons/wind.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-xs text-foreground-muted", "Wind"
						TextBlock class: "text-base font-bold text-foreground",
							"#{math.floor current.wind_speed_10m} m/s #{Weather\wind_dir current.wind_direction_10m}"
