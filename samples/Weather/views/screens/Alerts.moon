import StackView, TextBlock, ImageView from require "orca.UIKit"
import Weather, get_current_location from require "model"
Application = require "orca.core.application"

LEVEL_STYLE = {
	ok: {
		icon: "text-green-400"
		title: "text-green-400"
	}
	advisory: {
		icon: "text-amber-400"
		title: "text-amber-400"
	}
	warning: {
		icon: "text-red-400"
		title: "text-red-400"
	}
}

class AlertsScreen extends require "orca.core.widget"
	title: "Alerts"

	content: =>
		location = get_current_location!
		refresh_weather = ->
			Weather\refresh!
			app = Application.current false
			app\navigate "/alerts" if app

		ok, alerts = pcall -> Weather\alerts!
		unless ok and alerts
			return StackView class: "bg-background p-5 gap-3", =>
				TextBlock class: "text-2xl font-bold text-foreground", "Alerts"
				StackView class: "bg-surface rounded-3 px-4 py-4 gap-2", =>
					TextBlock class: "text-base font-bold text-foreground", "Unable to load alerts"
					TextBlock class: "text-sm text-foreground-muted",
						"Check your connection and tap refresh to retry."
					TextBlock {
						class: "text-sm font-bold text-accent"
						LeftButtonUp: refresh_weather
					}, "Refresh"

		StackView class: "bg-background p-5 gap-3", =>
			TextBlock class: "text-2xl font-bold text-foreground", "Alerts"
			StackView class: "flex-row items-center gap-2", =>
				ImageView
					class: "align-middle-center text-accent"
					Source: "assets/icons/location.svg?width=16&type=mask"
				TextBlock class: "text-sm text-foreground-muted", location.name
				TextBlock {
					class: "text-xs font-bold text-accent align-middle-right"
					LeftButtonUp: refresh_weather
				}, "Refresh"

			for item in *alerts
				style = LEVEL_STYLE[item.level] or LEVEL_STYLE.advisory
				StackView class: "bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3", =>
					ImageView
						class: "align-middle-center #{style.icon}"
						Source: "assets/icons/alert.svg?width=22&type=mask"
					StackView class: "gap-1", =>
						TextBlock class: "text-base font-bold #{style.title}", item.title
						TextBlock class: "text-xs text-foreground-muted", item.detail
