import Application from require "routing"
import Account from require "model"

routing = require "routing"
auth = require "applications.users.layouts.AuthLayout"
form = require "applications.users.forms"

class UsersApplication extends Application
	"/sign-in": => auth form.SignIn
	"/sign-up": => auth form.SignUp
	"/sign-out": => routing.navigate "/sign-in" if Account\signout!
