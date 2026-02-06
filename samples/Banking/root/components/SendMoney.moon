ui = require "orca.ui"
routing = require "routing"

import Users, Chats from require "model"

ContactCard = require "root.components.ContactCard"

class SendMoney extends ui.StackView
	apply: => "flex-col gap-2 my-2 overflow-y-scroll h-full"
	title: "Send Money"
	body: =>
		for chat in *Chats\findAll Users\auth!
			ContactCard "#card",
				user: Chats\getPartner chat, Users\auth!
				onLeftMouseUp: -> routing.navigate "/chat/#{chat["$id"]}"
