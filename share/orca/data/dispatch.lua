-- Routes named XML event handlers to a data scope. A template declares
-- Click="OpenRoom" or Click="Rooms.Retry"; the nearest controller resolves
-- the name, and the handler receives the record the sender presents. This
-- reuses ORCA's controller lookup, so no row needs a Lua closure.
local Dispatch = {}
local registry = setmetatable({}, { __mode = "v" })

function Dispatch.register(scope) registry[scope.root:getUniqueID()] = scope end

-- The scope whose root is the DataContext of `node` or an ancestor.
function Dispatch.lookup(node)
    while node do
        local context = node.DataContext
        local scope = context and context.getUniqueID and registry[context:getUniqueID()]
        if scope then return scope end
        node = node:getParent()
    end
end

local ACTIONS = {
    Load = "load", Refresh = "refresh", Retry = "retry", LoadMore = "loadMore", Invalidate = "invalidate",
    Submit = "submit", Cancel = "cancel", Clear = "clear",
}

-- Resolves a handler name to a function(scope, params), or nil so the
-- lookup continues to an outer controller.
function Dispatch.resolve(scope, name)
    if not scope or scope.disposed then return nil end
    local custom = scope.handlers[name]
    if custom then return custom end
    local memberName, action = name:match("^([%w_]+)%.([%w_]+)$")
    if memberName then
        local member = scope:member(memberName)
        if not member then return nil end
        if action == "Select" and member.select then
            return function(_, params) return member:select(params.item) end
        end
        if action == "Begin" and member.begin then
            return function(_, params) if params.item then return member:begin(params.item) end end
        end
        local method = ACTIONS[action]
        if method and member[method] then return function() return member[method](member) end end
        return nil
    end
    local member = scope:member(name)
    if not member then return nil end
    if member.execute then
        -- A command submitted by a form takes the form's snapshot.
        for _, edit in ipairs(scope.order) do
            if edit.submit and edit.def.submit == name then
                return function(_, params) return edit:submit(params) end
            end
        end
        return function(_, params) return member:execute(params) end
    end
    if member.submit then return function(_, params) return member:submit(params) end end
end

function Dispatch.params(scope, args, sender)
    local item, object = scope:resolve(sender)
    return { item = item, object = object, sender = sender, event = args }
end

-- A controller table. `handlers` are optional app functions(self, args,
-- sender) checked before the scope; everything else resolves lazily.
function Dispatch.controller(scope, handlers)
    local controller = {}
    for name, fn in pairs(handlers or {}) do controller[name] = fn end
    return setmetatable(controller, { __index = function(self, name)
        if type(name) ~= "string" or name == "view" then return nil end
        local target = scope or Dispatch.lookup(rawget(self, "view"))
        local fn = Dispatch.resolve(target, name)
        if not fn then return nil end
        return function(_, args, sender)
            local current = scope or Dispatch.lookup(rawget(self, "view")) or target
            local handler = Dispatch.resolve(current, name)
            if handler then return handler(current, Dispatch.params(current, args, sender or rawget(self, "view"))) end
        end
    end })
end

-- Set while Scope:attach loads the generic controller module.
Dispatch.pending = nil

return Dispatch
