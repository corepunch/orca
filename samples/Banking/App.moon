require "html"

routing = require "routing"
ui = require "orca.ui"
loc = require "orca.localization"
Layout = require "root.RootLayout"
page = require "root.components"

loc.load "assets/localization/en"

import Account from require "model"
import Application from require "routing"
import SearchPage from require "root.pages"

class App extends Application
	@include "applications.users"
	@include "applications.chat"

	@stylesheet require "tailwind"
	@stylesheet "assets/globals.css"

	"/": => Layout page.HomePage
	"/send-money": => Layout page.SendMoney
	"/settings": => Layout page.Settings
	"/tweets": => Layout page.Tweets
	"/new-tweet": => Layout page.NewTweet
	"/user/:user": => Layout page.ContactDetails, @params
	"/transaction/:transaction": => Layout page.TransactionDetails, @params
	"/search": => SearchPage!

	onAwake2: => 
		import parse from require "orca.parsers.css"
		-- routing.navigate '/sign-out'
		@navigate '/sign-in' unless pcall Account\auth, Account
