-- Generic controller for views presenting a data scope. Scope:attach loads
-- it; a page can also name it in XML (Controller="orca.data.controller"),
-- in which case it finds the scope through the page's DataContext.
local Dispatch = require "orca.data.dispatch"
return Dispatch.controller(Dispatch.pending)
