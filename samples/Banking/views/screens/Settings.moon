import StackView, TextBlock, ImageView from require "orca.UIKit"

import navigate from require "Banking/views/helpers"

class Settings extends require "orca.core.widget"
	title: "Settings"

	content: =>
		StackView class: "bg-background flex-col p-4 gap-4 h-full", =>
			TextBlock class: "text-2xl font-bold text-foreground", "Settings"

			StackView class: "bg-surface rounded-3 px-4 py-3 gap-2", =>
				StackView class: "flex-row items-center gap-2", =>
					ImageView
						class: "align-middle-center text-foreground-muted"
						Source: "assets/icons/profile-placeholder.svg?width=20&type=mask"
					TextBlock class: "text-base font-bold text-foreground", "Account"
				StackView {
					class: "bg-destructive rounded px-4 py-3 flex-row items-center gap-2"
					LeftButtonUp: -> navigate "/sign-out"
				}, =>
					ImageView
						class: "align-middle-center text-destructive-foreground"
						Source: "assets/icons/logout.svg?width=18&type=mask"
					TextBlock class: "text-destructive-foreground text-base font-bold", "Sign Out"
