routing = require "routing"

import Node2D from require "orca.ui"
import footer from require "assets.constants"

class Footer extends Node2D
	body: =>
		stack ".bg-muted.w-full.h-full.justify-evenly", ->
			for item in *footer.links
				selected = routing.get_location! == item.route
				color = selected and ".text-foreground" or ".text-muted-foreground"
				img ".align-middle-center#{color}" 
					Image: "#{item.imgURL}?width=#{footer.iconSize}&type=mask"
					onLeftMouseUp: -> routing.navigate item.route
