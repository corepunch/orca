ui = require "orca.ui"

import Users, Chats from require "model"
ContactCard = require "root.components.ContactCard"

class Search extends ui.Node2D
	new: => 
		@searchQuery = ""
		super!

	performSearch: (sender) => 
		@searchQuery = sender.Text
		@rebuild!

	body: =>
		contacts = Users\search @searchQuery
		
		if #@searchQuery == 0
			h5 ".text-muted-foreground.align-middle-center", "Found results will appear here"
			return
		elseif #contacts == 0
			h5 ".text-muted-foreground.align-middle-center", "No people found"
			return

		stack ".flex-col.gap-2.my-2.overflow-y-scroll.h-full", ->
			for item in *contacts
				ContactCard user: item, onClick: -> Chats\create item
