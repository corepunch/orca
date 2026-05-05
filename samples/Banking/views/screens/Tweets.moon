import StackView, TextBlock, ImageView from require "orca.UIKit"

USERS = {
	"StarWarsFan247", "DarthVibes",      "JediMasterLuke",   "SithLordOfficial",
	"ForceAwakens42", "GalacticBounty",  "RebelScumHQ",      "CloneWarsElite",
	"HyperspacePilot","TheRealYoda",
}

ACTIONS = { "chat", "retweet", "like", "bookmark", "share" }

class Tweets extends require "orca.core.widget"
	title: "Tweets"

	content: =>
		StackView class: "bg-background flex-col px-4 gap-4 overflow-y-scroll h-full", =>
			for i = 1, 10
				StackView class: "bg-surface rounded-3 p-3 flex-col gap-1", =>
					StackView class: "flex-row items-center gap-2", =>
						TextBlock class: "text-primary font-bold", "@#{USERS[i]}"
						TextBlock class: "text-foreground-muted text-sm", "• 1d"
					TextBlock class: "text-foreground text-sm",
						"The Force is strong with this one. Whether you're Jedi, Sith, or just here for the droids, " ..
						"there's no denying Star Wars shaped generations of fans."
					-- Action bar
					StackView class: "flex-row justify-between mt-1 text-foreground-muted", =>
						for action in *ACTIONS
							StackView class: "flex-row gap-1 items-center", =>
								ImageView
									class: "align-middle-center text-foreground-muted"
									Source: "assets/icons/#{action}.svg?width=16&type=mask"
								TextBlock class: "text-xs text-foreground-muted", "28"
