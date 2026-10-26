local function make(method, attribute, ...)
	return { method = method, attribute = attribute, values = { ... } }
end

local Query = {
	Equal = function(attribute, value, ...) return make("equal", attribute, value, ...) end,
	NotEqual = function(attribute, value, ...) return make("notEqual", attribute, value, ...) end,
	LessThan = function(attribute, value) return make("lessThan", attribute, value) end,
	LessThanEqual = function(attribute, value) return make("lessThanEqual", attribute, value) end,
	GreaterThan = function(attribute, value) return make("greaterThan", attribute, value) end,
	GreaterThanEqual = function(attribute, value) return make("greaterThanEqual", attribute, value) end,
	Between = function(attribute, a, b) return make("between", attribute, a, b) end,
	StartsWith = function(attribute, value) return make("startsWith", attribute, value) end,
	EndsWith = function(attribute, value) return make("endsWith", attribute, value) end,
	Contains = function(attribute, value) return make("contains", attribute, value) end,
	Search = function(attribute, value) return make("search", attribute, value) end,
	OrderAsc = function(attribute) return make("orderAsc", attribute) end,
	OrderDesc = function(attribute) return make("orderDesc", attribute) end,
	CursorBefore = function(value) return make("cursorBefore", nil, value) end,
	CursorAfter = function(value) return make("cursorAfter", nil, value) end,
	Limit = function(value) return make("limit", nil, value) end,
	Offset = function(value) return make("offset", nil, value) end,
	And = function(a, b) return make("and", nil, a, b) end,
	Or = function(a, b) return make("or", nil, a, b) end,
	Select = function(value, ...) return make("select", nil, value, ...) end,
}

return Query