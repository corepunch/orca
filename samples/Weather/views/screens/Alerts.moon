import StackView, TextBlock, ImageView from require "orca.UIKit"

class AlertsScreen extends require "orca.core.widget"
	title: "Alerts"

	content: =>
		StackView class: "bg-slate-900 p-5 gap-3", =>
			TextBlock class: "text-2xl font-bold text-slate-100", "Alerts"

			-- No active alerts card
			StackView class: "bg-slate-800 rounded-3 px-4 py-3 flex-row items-center gap-3", =>
				ImageView
					class: "align-middle-center text-green-400"
					Source: "assets/icons/alert.svg?width=22&type=mask"
				StackView class: "gap-1", =>
					TextBlock class: "text-base font-bold text-green-400", "All Clear"
					TextBlock class: "text-xs text-slate-400", "No active severe alerts"

			-- Advisory card
			StackView class: "bg-slate-800 rounded-3 px-4 py-3 flex-row items-center gap-3", =>
				ImageView
					class: "align-middle-center text-amber-400"
					Source: "assets/icons/alert.svg?width=22&type=mask"
				StackView class: "gap-1", =>
					TextBlock class: "text-base font-bold text-amber-400", "Advisory"
					TextBlock class: "text-xs text-slate-400",
						"Rain expected after 18:00 in coastal areas"
