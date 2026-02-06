import Page from require "routing"
import Header, Footer from require "root.components"
import Users from require "model"
import header from require "assets.constants"

class RootLayout extends Page
	body: =>
		grid Rows: "64px auto 56px", ->
			@header!
			@content!
			@footer!

	title: => @view.title or "Page Title"

	-- header: => Header!
	footer: => Footer!

	header: =>
		routing = require "routing"
		name = Users\getFullName Users\auth!
		h0 ".px-2.w-full.h-full", @title!

		-- grid ".bg-muted.px-2", Columns: "auto 100px", ->
		-- 	stack ".align-middle-left.items-center", ->
		-- 		if routing.has_history!
		-- 			img ".align-middle-left.text-muted-foreground"
		-- 				Image: "assets/icons/back.svg?width=#{header.iconSize}&type=mask"
		-- 				onLeftMouseUp: -> routing.go_back!
		-- 		else
		-- 			h5 ".py-2.text-muted-foreground", name
		-- 	stack ".align-middle-right.gap-2", ->
		-- 		for item in *header.links
		-- 			img ".align-middle-center.text-muted-foreground" 
		-- 				Image: "#{item.imgURL}?width=#{header.iconSize}&type=mask"
		-- 				onLeftMouseUp: -> routing.navigate item.route

