local network = require "orca.network"

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
}

local url = function(tbl)
	return table.concat(tbl, '/')
end

local function encodeJSON(data)
	local buffer, isArray = {}, #data > 0
	for k, v in (isArray and ipairs or pairs)(data) do
		if string.sub(k, 1, 2) ~= "__" then
			local value = type(v) == 'table' and encodeJSON(v)
				or type(v) == 'number' and string.format('%g', v)
				or string.format('"%s"', v)
			table.insert(buffer, isArray and value or string.format('"%s":%s', k, value))
		end
	end
	return (isArray and "[" or "{") .. table.concat(buffer, ",") .. (isArray and "]" or "}")
end

-- Function to URL-encode a string
local function url_encode(str)
	if not str then return "" end
	str = string.gsub(str, "([^%w%-%.%_%~])", function(c)
		return string.format("%%%02X", string.byte(c))
	end)
	return str
end

local compileQuery = function(...)
	local tmp = {}
	for i, v in ipairs {...} do
		table.insert(tmp, string.format("queries[%d]=%s", i-1, url_encode(encodeJSON(v))))
	end
	return '?'..(table.concat(tmp, "&"))
end

local appwrite = {}

function appwrite.signInAccount(user)
	return network.fetch(url{endpoint,"account","sessions","email"}, {
		method = "POST",
		body = encodeJSON(user),
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
		body = encodeJSON(user),
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
	local q = compileQuery(...)
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
		body = encodeJSON {
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
		body = encodeJSON {
			documentId = "unique()",
			data = document
		}
	})
end

return appwrite