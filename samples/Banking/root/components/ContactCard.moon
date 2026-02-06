import StackView from require "orca.ui"

class ContactCard extends StackView
	apply: => "contact-card"
	body: =>
		-- img ".contact-card-image", Src: @user.userpic or "https://picsum.photos/64"
		img Src: "https://picsum.photos/64"
		stack ".flex-col", ->
			p ".contact-card-title", @user.name
			p ".contact-card-description", "@"..@user["$id"]
