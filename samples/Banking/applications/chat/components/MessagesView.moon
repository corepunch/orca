ui = require "orca.ui"

import Users, Chats, Messages from require "model"

class MessagesView extends ui.StackView
	new: (@params) => 
		super ".flex-col.p-4.overflow-y-scroll.h-full.w-full", ClipChildren: "true"
		@setTimer duration: 2000

	body: =>
		@user = Users\auth!
		@chat = Chats\find @params.chat
		@last = {}
		for msg in *Messages\findAll @chat
			@bubble msg

	onScrollHeightChanged: () => @setScrollTop @ScrollHeight

	bubbleClass: (msg) =>
		sender = msg.sender["$id"]
		margin = ".mt-2"
		margin = ".mt-1" if @last.sender and @last.sender["$id"] == sender
		color = ".align-left.bg-sky-700.mr-8"
		color = ".align-right.bg-orange-700.ml-8" if @user["$id"] == sender
		@last = msg
		return ".p-2.rounded#{margin}#{color}"

	bubble: (msg) => p (@bubbleClass msg), msg.body

	onTimer: => 
		return unless @last
		notimezone = (msg) -> msg["$createdAt"]\gsub "%+%d%d:%d%d$", "" if msg
		for msg in *Messages\findAll @chat, notimezone @last
			@addChild @bubble msg
