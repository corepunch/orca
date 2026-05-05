import StackView, TextBlock, ImageView from require "orca.UIKit"

import Users, Chats from require "model"
import navigate from require "Banking/views/helpers"

class SendMoney extends require "orca.core.widget"
	title: "Send Money"

	content: =>
		me    = Users\auth!
		chats = Chats\findAll me

		StackView class: "bg-background flex-col overflow-y-scroll h-full p-4 gap-3", =>
			TextBlock class: "text-2xl font-bold text-foreground", "Send Money"

			for chat in *chats
				partner = Chats\getPartner chat, me
				c = chat
				StackView {
					class: "bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3"
					LeftButtonUp: -> navigate "/chat", { chat: c["$id"] }
				}, =>
					StackView class: "gap-1", =>
						TextBlock class: "text-base font-bold text-foreground",
							Users\getFullName partner
						TextBlock class: "text-xs text-foreground-muted",
							"@" .. partner["$id"]
