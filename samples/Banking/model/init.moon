appwrite = require "appwrite.functions"
query = require "appwrite.query"

class Model
	list: (...) => appwrite.listCollections ...
	create: (...) => appwrite.createDocument ...
	createWithId: (...) => appwrite.createDocumentWithId ...
	signin: (params) => appwrite.signInAccount params
	signup: (params) => appwrite.createUserAccount params
	signout: => appwrite.signOutAccount!
	getaccount: => appwrite.getAccount!

context = {}

class Account extends Model
	cached: nil
	auth: => 
		if context.account return context.account
		res = @getaccount!
		context.account = res\json!
		return context.account
	signin: (params) => 
		response = super params
		return response\json!
	signup: (params) => 
		response = super params
		return response\json!
	signout: () =>
		response = super!
		export context = {}
		return response\json!

class Users extends Model
	cached: nil
	getFullName: (user) => user.name
	auth: => 
		-- return {["$id"]: "679bd1b50008b3e1de9d" }
		if context.user return context.user
		account = Account\auth!
		response = @list "users", query.Equal("$id", account["$id"])
		data = response\json!
		context.user = data.documents[1]
		return context.user

	search: (search) =>
		response = @list "users", query.Or(
				query.Search("name", search), 
				query.Search("$id", search)
			),
			query.Limit(15)
		data = response\json!
		return data.documents

	find: (userId) =>
		response = @list "users", query.Contains("$id", userId)
		data = response\json!
		return data.documents[1]

	create: (...) => @createWithId "users", ...

class Chats extends Model
	findAll: (user) =>
		response = @list "friends", 
			query.Contains("users2", user["$id"]),
			query.Select("*", "users.*")
		data = response\json!
		return data.documents

	find: (chatId) =>
		response = @list "friends", query.Contains("$id", chatId)
		data = response\json!
		return data.documents[1]

	getPartner: (chat, user) => 
		for other in *chat.users
			if other["$id"] != user["$id"]
				return other

	create: (user) =>
		me = Users\auth!
		super "friends"
			users: { me["$id"], user["$id"] }
			users2: { me["$id"], user["$id"] }
			status: "pending"

class Messages extends Model
	findAll: (chat, after) =>
		response = @list "messages", 
			query.Equal("chatId", chat["$id"]),
			query.GreaterThan("$createdAt", after or "1970-01-01T00:00:00.000"),
			query.OrderAsc("$createdAt"),
			query.Select("body", "$createdAt", "sender.$id"),
			query.Limit(50)
		data = response\json!
		return data.documents

	create: (params) =>
		sender = Users\auth!
		super "messages"
			sender: sender["$id"]
			body: params.body
			chatId: params.chat

class Transactions extends Model
	find: (id) =>
		response = @list "transactions", query.Contains("$id", id)
		data = response\json!
		return data.documents[1]

	findAll: (user, limit) =>
		response = @list "transactions", 
			query.Or(query.Equal("sender", user["$id"]), query.Equal("beneficiary", user["$id"])),
			query.Select("*", "sender.*", "beneficiary.*"),
			if limit then query.Limit(limit) else nil
		data = response\json!
		return data.documents

	findTotal: (user) =>
		sum = 0
		for t in *Transactions\findAll user
			sum += t.amount
		return sum

	formatAmount: (transaction) => string.format('$%.02f', transaction.amount/100)

return {
	:Account
	:Users
	:Chats
	:Transactions
	:Messages
}