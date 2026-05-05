import StackView, TextBlock, Input from require "orca.UIKit"

import Users, Chats from require "model"
import navigate from require "Banking/views/helpers"

class Search extends require "orca.core.widget"
	title: "Search"

	content: =>
		results_view = nil
		query        = ""

		do_search = (sender) ->
			query = sender.Text
			contacts = Users\search query
			results_view\rebuild =>
				if #query == 0
					TextBlock class: "text-foreground-muted align-middle-center p-4",
						"Search results will appear here"
					return
				if #contacts == 0
					TextBlock class: "text-foreground-muted align-middle-center p-4",
						"No people found"
					return
				for item in *contacts
					u = item
					StackView {
						class: "bg-surface rounded-3 px-4 py-3 flex-row items-center gap-3"
						LeftButtonUp: ->
							pcall Chats.create, Chats, u
							navigate "/send-money"
					}, =>
						StackView class: "flex-col gap-1", =>
							TextBlock class: "text-base font-bold text-foreground",
								Users\getFullName u
							TextBlock class: "text-xs text-foreground-muted",
								"@" .. u["$id"]

		StackView class: "bg-background flex-col h-full", =>
			Input
				class: "bg-surface mx-4 my-3 px-4 py-3 rounded text-foreground"
				PlaceholderText: "Search people..."
				Name: "query"
				TextInput: do_search

			results_view = StackView
				class: "flex-col px-4 gap-2 overflow-y-scroll"
				=>
					TextBlock class: "text-foreground-muted align-middle-center p-4",
						"Search results will appear here"
