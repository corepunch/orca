import StackView, TextBlock, ImageView from require "orca.UIKit"
import Weather, get_current_location, format_time from require "model"

-- Number of hourly slots to display in the strip.
HOURLY_COUNT = 12

class HomeScreen extends require "orca.core.widget"
	title: "Today"

	content: =>
		current  = Weather\current!
		forecast = Weather\forecast!
		hourly   = Weather\hourly!
		icon_src = Weather\icon current.weather_code
		desc     = Weather\description current.weather_code
		location_name = get_current_location!.name

		-- Round a number to the nearest integer.
		round = (n) -> math.floor n + 0.5

		-- Hourly strip: start at the current hour; forecast_days=2 (48 entries) ensures
		-- the window is always available even when crossing midnight.
		start_i = (tonumber(os.date "%H") or 0) + 1
		end_i   = math.min start_i + HOURLY_COUNT - 1, #hourly.time

		sunrise_str = format_time (forecast.sunrise or {})[1]
		sunset_str  = format_time (forecast.sunset  or {})[1]

		StackView class: "bg-background overflow-y-scroll h-full p-5 gap-4", =>
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

			-- Hourly forecast strip
			StackView class: "flex-row overflow-x-scroll gap-3", =>
				for i = start_i, end_i
					h_icon = Weather\icon hourly.weather_code[i]
					h_time = format_time hourly.time[i]
					h_temp = Weather\format_temp hourly.temperature_2m[i]
					StackView class: "bg-surface rounded-3 px-3 py-2 flex-col items-center gap-1", =>
						TextBlock class: "text-xs text-foreground-muted", h_time
						ImageView
							class: "align-middle-center text-accent"
							Source: "#{h_icon}?width=24&type=mask"
						TextBlock class: "text-sm font-bold text-foreground", h_temp

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

				-- Humidity
				StackView class: "bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center text-accent"
						Source: "assets/icons/humidity.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-xs text-foreground-muted", "Humidity"
						TextBlock class: "text-base font-bold text-foreground",
							"#{round current.relative_humidity_2m or 0}%"

				-- UV Index
				StackView class: "bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center text-accent"
						Source: "assets/icons/uv.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-xs text-foreground-muted", "UV Index"
						TextBlock class: "text-base font-bold text-foreground",
							"#{round current.uv_index or 0}"

				-- Pressure
				StackView class: "bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center text-accent"
						Source: "assets/icons/pressure.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-xs text-foreground-muted", "Pressure"
						TextBlock class: "text-base font-bold text-foreground",
							"#{round current.surface_pressure or 0} hPa"

				-- Visibility
				StackView class: "bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center text-accent"
						Source: "assets/icons/visibility.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-xs text-foreground-muted", "Visibility"
						TextBlock class: "text-base font-bold text-foreground",
							"#{round (current.visibility or 0) / 1000} km"

			-- Sunrise / Sunset row
			StackView class: "flex-row gap-3", =>
				StackView class: "bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center text-amber-400"
						Source: "assets/icons/sunrise.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-xs text-foreground-muted", "Sunrise"
						TextBlock class: "text-base font-bold text-foreground", sunrise_str

				StackView class: "bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center text-orange-400"
						Source: "assets/icons/sunset.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-xs text-foreground-muted", "Sunset"
						TextBlock class: "text-base font-bold text-foreground", sunset_str
