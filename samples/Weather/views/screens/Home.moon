import StackView, TextBlock, ImageView from require "orca.UIKit"
import Weather, LOCATIONS from require "model"

class HomeScreen extends require "orca.core.widget"
	title: "Today"

	content: =>
		current = Weather\current!
		icon_src = Weather\icon current.weather_code
		desc = Weather\description current.weather_code
		location_name = LOCATIONS[1].name

		StackView class: "bg-slate-900 p-5 gap-4", =>
			-- Location row
			StackView class: "flex-row items-center gap-2", =>
				ImageView
					class: "align-middle-center text-sky-400"
					Source: "assets/icons/location.svg?width=16&type=mask"
				TextBlock class: "text-sm text-slate-400", location_name

			-- Main weather card
			StackView class: "bg-slate-800 rounded-3 p-5 gap-3", =>
				StackView class: "flex-row items-center gap-4", =>
					ImageView
						class: "align-middle-center text-sky-400"
						Source: "#{icon_src}?width=80&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-5xl font-bold text-slate-100",
							Weather\format_temp current.temperature_2m
						TextBlock class: "text-base text-slate-400", desc

			-- Stat cards
			StackView class: "gap-3", =>
				-- Feels like
				StackView class: "bg-slate-800 rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center text-sky-400"
						Source: "assets/icons/thermometer.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-xs text-slate-400", "Feels like"
						TextBlock class: "text-base font-bold text-slate-100",
							Weather\format_temp current.apparent_temperature

				-- Wind
				StackView class: "bg-slate-800 rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center text-sky-400"
						Source: "assets/icons/wind.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-xs text-slate-400", "Wind"
						TextBlock class: "text-base font-bold text-slate-100",
							"#{math.floor current.wind_speed_10m} m/s #{Weather\wind_dir current.wind_direction_10m}"
