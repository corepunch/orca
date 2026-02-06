ui = require "orca.ui"
routing = require "routing"

import MessagesView from require "applications.chat.components"
import header from require "assets.constants"
import Page from require "routing"
import Users, Chats, Messages from require "model"

getPartner = (user, chat) -> 
	for other in *chat.users
		if other["$id"] != user["$id"]
			return other

class Header extends ui.Node2D
	new: (@params, ...) => super...

	body: =>
		chat = Chats\find @params.chat
		partner = Chats\getPartner chat, Users\auth!
		title = Users\getFullName partner
		grid ".bg-muted.px-2", Columns: "auto auto", ->
			stack '.w-full.h-full.gap-2.items-center', ->
				img ".align-middle-left.text-muted-foreground"
					Image: "assets/icons/back.svg?width=#{header.iconSize}&type=mask"
					onLeftMouseUp: -> routing.navigate "/send-money"
				h0 "#title-name.text-muted-foreground", title
			stack ".align-middle-right.gap-2", ->
				for item in *header.links
					img "#control-button.align-middle-center.text-muted-foreground" 
						Image: "#{item.imgURL}?width=#{header.iconSize}&type=mask"
						onLeftMouseUp: -> routing.navigate item.route

class ChatLayout extends Page
	new: (@params, ...) =>
		super MessagesView, @params, ...

	title: =>
		chat = Chats\find @params.chat
		partner = Chats\getPartner chat, Users\auth!
		title = Users\getFullName partner

	body: =>
		grid Rows: "64px auto 56px", ->
			@header!
			@content!
			@footer!
	
	header: =>
		Header @params

	footer: =>
		sendMessage = (params) ->
			Input = @findChild 'Input', true
			text = Input.Text
			Input.Text = ""
			Messages\create chat: @params.chat, body: text

		div ".bg-muted", ->
			ui.Input "#msg-input.m-8.h-full.input"
				name: "message"
				placeholderText: ". . ."
				onChange: sendMessage
