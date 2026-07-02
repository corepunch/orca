local network = require "orca.network"
local json = require "orca.parsers.json"

local openai = {}

local get_openai_key = function()
	local openai_key = os.getenv "OPENAI_API_KEY"
	assert(openai_key, "OPENAI_API_KEY environment variable is not set")
	return openai_key
end

local room_schema = {
  type = "object",
  properties = {
    id = {
      type = "string",
      description = "Unique identifier of the room"
    },
    title = {
      type = "string",
      description = "Short title of the room"
    },
    description = {
      type = "string",
      description = "Detailed description of what the player sees"
    },
    objects = {
      type = "array",
      items = {
        type = "object",
        properties = {
          name = {
            type = "string"
          },
          description = {
            type = "string"
          },
          state = {
            type = "string",
            enum = { "default", "opened", "taken", "locked" },
            description = "Current state of the object"
          }
        },
        required = { "name", "description" }
      }
    },
    exits = {
      type = "object",
      description = "Map of available exits to other room IDs",
      additionalProperties = {
        type = "string"
      }
    }
  },
  required = { "id", "title", "description", "objects", "exits" }
}

local schema = {
	name = "AdventureScene",
	description = "Scene description and choices for the text adventure",
	schema = {
		type = "object",
		properties = {
			scene = {
				type = "string",
				description = "The vivid description of the current scene"
			},
			choices = {
				type = "array",
				description = "List of available choices for the player",
				items = { type = "string" }
			}
		},
		required = { "scene", "choices" },
		additionalProperties = false
	},
	strict = true
}

function openai.test(system, user)
	local data = {
    model = "gpt-5-nano",
    input = "write a haiku about ai",
    store = true
		-- model= "gpt-4o-mini",
		-- messages = {
		-- 	{ role = "system", content = system },
		-- 	{ role = "user", content = user }
		-- },
		-- response_format = {
		-- 	type = "json_schema",
		-- 	json_schema = {
		-- 		name = "AdventureScene",
		-- 		description = "Scene description and choices for the text adventure",
		-- 		schema = schema,
		-- 		strict = true
		-- 	}
		-- }
	}
	return network.fetch(
	'https://api.openai.com/v1/responses', {
		 -- 'https://api.openai.com/v1/chat/completions', {
		method = "POST",
		body = json.encode(data),
		headers = {
			["Content-Type"] = "application/json; charset=utf-8",
			["Authorization"] = "Bearer " .. get_openai_key(),
		},
		nocookies = true
	})
end

function openai.simple(input)
	local data = {
    model = "gpt-5-nano",
		-- model= "gpt-4o-mini",
    input = input,
    store = true
	}
	return network.fetch('https://api.openai.com/v1/responses', {
		method = "POST",
		body = json.encode(data),
		headers = {
			["Content-Type"] = "application/json; charset=utf-8",
			["Authorization"] = "Bearer " .. get_openai_key(),
		},
		nocookies = true
	})
end

function openai.chat_completions(system, user)
	local data = {
		model= "gpt-4o-mini",
		messages = {
			{ role = "system", content = system },
			{ role = "user", content = user }
		},
		response_format = {
			type = "json_schema",
			json_schema = {
				name = "AdventureScene",
				description = "Scene description and choices for the text adventure",
				schema = schema,
				strict = true
			}
		}
	}
	return network.fetch( 'https://api.openai.com/v1/chat/completions', {
		method = "POST",
		body = json.encode(data),
		headers = {
			["Content-Type"] = "application/json; charset=utf-8",
			["Authorization"] = "Bearer " .. get_openai_key(),
		},
		nocookies = true
	})
end

return openai
