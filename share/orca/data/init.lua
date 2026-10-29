-- Shared data runtime: schemas, an identity-preserving store, queries,
-- local views, commands, form drafts and media state, presented to XML
-- views through ordinary DataObject bindings. See docs/app/datasources.md.
local data = {
    Schema = require "orca.data.schema",
    Store = require "orca.data.store",
    Collection = require "orca.data.collection",
    Query = require "orca.data.query",
    View = require "orca.data.view",
    Command = require "orca.data.command",
    Edit = require "orca.data.edit",
    Scope = require "orca.data.scope",
    Declarations = require "orca.data.declarations",
    Dispatch = require "orca.data.dispatch",
    xml = require "orca.data.xml",
}

-- data.scope("App/Data/Inbox.xml", { provider = ..., store = ..., context = ... })
function data.scope(declaration, options)
    return data.Declarations.build(declaration, options)
end

function data.schema(path) return data.Schema.load(path) end

return data
