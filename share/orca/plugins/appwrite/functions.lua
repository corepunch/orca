local network = require "orca.network"
local json = require "orca.parsers.json"
local projectId = "6793939300106a97df4f"
local databaseId = "679bb7320003ab334979"
local endpoint = "https://cloud.appwrite.io/v1"
local headers = {
	-- ["X-Appwrite-Key"] = apiKey,
	["X-Appwrite-Project"] = projectId,
	["Content-Type"] = "application/json",
}
local collections = {
	friends = "679bb73e002649c14d37",
	users = "679bd1410007f06de0fb",
	transactions = "679df0510029b54b28be",
	messages = "67a261a400165972fc59",
	chat_members = "chat-members",
}

local url = function(tbl)
	return table.concat(tbl, '/')
end

local compile_query = function(...)
	local tmp = {}
	for i, v in ipairs {...} do
		table.insert(tmp, string.format("queries[%d]=%s", i-1, network.url(json.encode(v))))
	end
	return '?'..(table.concat(tmp, "&"))
end

local appwrite = {}

function appwrite.signInAccount(user)
	return network.fetch(url{endpoint,"account","sessions","email"}, {
		method = "POST",
		body = json.encode(user),
		headers = headers,
		nocookies = true
	})
end

function appwrite.signOutAccount()
	return network.fetch(url{endpoint,"account","sessions", "current"}, {
		method = "DELETE",
		headers = headers
	})
end

function appwrite.createUserAccount(user)
	return network.fetch(url{endpoint,"account"}, {
		method = "POST",
		body = json.encode(user),
		headers = headers
	})
end

function appwrite.getAccount()
	return network.fetch(url{endpoint,"account"}, {
		method = "GET",
		headers = headers
	})
end

function appwrite.listCollections(collection, ...)
	local q = compile_query(...)
	local col = collections[collection]
	local db = databaseId
	return network.fetch(url{endpoint,"databases",db,"collections",col,"documents"}..q, {
		method = "GET",
		headers = headers
	})
end

function appwrite.createDocumentWithId(collection, documentId, document)
	local col = collections[collection]
	local db = databaseId
	print(documentId, document)
	return network.fetch(url{endpoint,"databases",db,"collections",col,"documents"}, {
		method = "POST",
		headers = headers,
		body = json.encode {
			documentId = documentId,
			data = document
		}
	})
end

function appwrite.createDocument(collection, document)
	local col = collections[collection]
	local db = databaseId
	return network.fetch(url{endpoint,"databases",db,"collections",col,"documents"}, {
		method = "POST",
		headers = headers,
		body = json.encode {
			documentId = "unique()",
			data = document
		}
	})
end

return appwrite
