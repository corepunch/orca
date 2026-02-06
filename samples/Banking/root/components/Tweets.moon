ui = require "orca.ui"

users = {
	"StarWarsFan247"
	"DarthVibes"
	"JediMasterLuke"
	"SithLordOfficial"
	"ForceAwakens42"
	"GalacticBounty"
	"RebelScumHQ"
	"CloneWarsElite"
	"HyperspacePilot"
	"TheRealYoda"
}

actions = {
	"chat"
	"retweet"
	"like"
	"bookmark"
	"share"
}

class TweetAction extends ui.StackView
	apply: => "gap-1"
	body: =>
		img Image: "assets/icons/#{@action}.svg?width=18&type=mask"
		h7 "%d"\format @counter

class Tweets extends ui.StackView
	title: "Tweets"
	apply: => "flex-col px-2 gap-4 overflow-y-scroll h-full"
	body: =>
		for i = 1, 10
			stack '.flex-col.gap-1', ->
				stack '.items-center.gap-1', ->
					h5 ".text-primary.font-bold", "@#{users[i]}"
					h5 ".text-foreground", "• User • 1d"
				h6 "The Force is strong with this one. Whether you're Jedi, Sith, or just here for the droids, there's no denying Star Wars shaped generations of fans. What's your favorite moment from the galaxy far, far away?"
				stack ".w-full.items-center.mt-1.justify-between.text-muted-foreground", ->
					for action in *actions
						TweetAction action: action, counter: 28

