import Screen, Grid, StackView, TextBlock, ImageView, Input from require "orca.UIKit"
Application = require "orca.core.application"

import Users, Chats, Messages from require "model"
import navigate from require "Banking/views/helpers"

render_bubble = (msg, me) ->
	sender_id = msg.sender["$id"]
	is_mine   = me and me["$id"] == sender_id
	align     = is_mine and "bg-primary ml-8 mr-2" or "bg-surface mr-8 ml-2"
	StackView class: "rounded-3 p-3 my-1 #{align}", =>
		TextBlock class: "text-foreground text-sm", msg.body

class Chat extends require "orca.core.widget"
	content: =>
		app     = Application.current false
		data    = app and app.nav_data
		chat    = data and data.chat and Chats\find data.chat
		me      = Users\auth!
		partner = chat and Chats\getPartner chat, me
		title   = partner and Users\getFullName partner or "Chat"

		-- Push the partner name as the title for the layout header.
		@content_for "title", title

		-- messages_view will be captured by send_message once assigned.
		messages_view = nil

		send_message = (sender) ->
			text = sender.Text
			return if text == "" or not chat
			sender.Text = ""
			pcall Messages.create, Messages, { chat: chat["$id"], body: text }
			msgs = Messages\findAll chat
			messages_view\rebuild =>
				for msg in *msgs
					render_bubble msg, me

		-- Override the layout footer with the message-compose bar.
		@content_for "footer", StackView class: "bg-header-bg flex-row px-4 py-2 gap-2 items-center", =>
			ImageView {
				class: "text-foreground-muted"
				Source: "assets/icons/back.svg?width=24&type=mask"
				LeftButtonUp: -> navigate "/send-money"
			}
			Input
				class: "bg-surface flex-1 px-4 py-2 rounded text-foreground"
				PlaceholderText: ". . ."
				Name: "message"
				Submit: send_message

		-- Messages list returned as the main content.
		messages_view = StackView class: "bg-background flex-col overflow-y-scroll h-full px-2", =>
			if chat
				for msg in *Messages\findAll chat
					render_bubble msg, me
			else
				TextBlock class: "text-foreground-muted align-middle-center p-4",
					"Select a conversation to start messaging"

