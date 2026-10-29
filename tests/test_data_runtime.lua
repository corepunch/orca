-- Shared data runtime: identity, partial/null fields, projections, queries
-- (stale responses, pagination, debounce, polling), views, selection,
-- commands, drafts and XML handler dispatch through real control events.
local core = require "orca.core"
local ui = require "orca.UIKit"
local data = require "orca.data"
local checks = 0
local function check(value, message) assert(value, message); checks = checks + 1 end

-- A host whose coroutines and timers the test advances explicitly.
local function host()
    local h = { pending = {}, timers = {}, reports = {} }
    function h.run(fn)
        local co = coroutine.create(fn)
        local ok, err = coroutine.resume(co)
        assert(ok, err)
        if coroutine.status(co) ~= "dead" then h.pending[#h.pending + 1] = co end
    end
    function h.resume(index)
        local co = table.remove(h.pending, index or 1)
        local ok, err = coroutine.resume(co)
        assert(ok, err)
    end
    function h.after(ms, fn)
        local timer = { ms = ms, fn = fn }
        function timer:cancel() self.canceled = true end
        h.timers[#h.timers + 1] = timer
        return timer
    end
    function h.tick()
        local due = h.timers
        h.timers = {}
        for _, timer in ipairs(due) do if not timer.canceled then timer.fn() end end
    end
    function h.report(err, stage) h.reports[#h.reports + 1] = { err = err, stage = stage } end
    return h
end

local schema = data.Schema.parse([[
<Schema>
  <Entity Name="TestAuthor" Key="Id">
    <Column Name="Id" Type="string" Source="id"/>
    <Column Name="DisplayName" Type="string" Source="name"/>
  </Entity>
  <Entity Name="TestTag" Key="Label">
    <Column Name="Label" Type="string" Source="value"/>
  </Entity>
  <Entity Name="TestPost" Key="Id">
    <Column Name="Id" Type="string" Source="id"/>
    <Column Name="Title" Type="string" Source="title"/>
    <Column Name="Likes" Type="int" Source="stats.likes" Nullable="true"/>
    <Column Name="AuthorId" Type="string" Source="author_id"/>
    <Column Name="AuthorName" Type="string" From="Author.DisplayName"/>
    <Column Name="Liked" Type="bool" Local="true"/>
    <Column Name="LikeLabel" Type="string" Computed="likeLabel" DependsOn="Likes Liked"/>
    <Relation Name="Author" Entity="TestAuthor" Key="AuthorId"/>
    <Relation Name="Tags" Entity="TestTag" Many="true" Source="tags"/>
  </Entity>
  <Entity Name="TestEntry" Key="Key">
    <Column Name="Key" Type="string" Source="key"/>
    <Column Name="PostId" Type="string" Source="post_id"/>
    <Column Name="RepostedBy" Type="string" Source="reposted_by" Nullable="true"/>
    <Column Name="Title" Type="string" From="Post.Title"/>
    <Column Name="LikeLabel" Type="string" From="Post.LikeLabel"/>
    <Relation Name="Post" Entity="TestPost" Key="PostId" Embed="true"/>
  </Entity>
  <Entity Name="TestDraft">
    <Column Name="Body" Type="string"/>
    <Column Name="Title" Type="string"/>
  </Entity>
</Schema>]], "test.Schema.xml")

-- Schema metadata for tools.
local described = schema:describe()
check(#described.entities == 5 and described.entities[3].columns[5].from == "Author.DisplayName"
    and described.entities[3].relations[2].cardinality == "many", "Schema reports columns, projections and relations")
check(not pcall(data.Schema.parse, [[<Schema><Entity Name="Bad" Key="Id"><Column Name="Title"/></Entity></Schema>]], "bad"),
    "A key that is not a column is rejected at load")
check(not pcall(data.Schema.parse, [[<Schema><Entity Name="Bad2"><Column Name="Name"/></Entity></Schema>]], "bad"),
    "A column cannot shadow the native object name")
local ok, err = pcall(data.Schema.parse, [[<Schema><Entity Name="BadRel"><Column Name="X" From="Nope.Y"/></Entity></Schema>]], "bad.xml")
check(not ok and err:find("bad.xml:1", 1, true) and err:find("Nope.Y", 1, true), "Unknown projections name resource, line and path")

local function scopeWith(h, provider, context)
    local store = data.Store.new { schema = schema, scope = "test", context = context or {},
        computed = { likeLabel = function(values) return values.Likes == nil and "…" or
            (values.Liked and "♥ " or "") .. tostring(values.Likes) end } }
    return data.Scope.new { name = "Test", store = store, host = h, provider = provider }
end

-- Store identity, partial updates, nulls, projections and relations.
do
    local h = host()
    local scope = scopeWith(h)
    local store = scope.store
    local post = store:put("TestPost", { id = "p1", title = "Hello", author_id = "a1", stats = { likes = 2 }, tags = { "x", "y" } })
    check(post.values.AuthorName == nil, "A not-yet-loaded related record leaves its projection empty")
    local mirror = store:mirror(post)
    check(mirror.object.AuthorName == "" and mirror.object.LikeLabel == "2", "Mirrors show defaults and computed values")
    store:put("TestAuthor", { id = "a1", name = "Ada" })
    check(mirror.object.AuthorName == "Ada", "A related record arriving later fills in projections")
    check(store:put("TestPost", { id = "p1", title = "Hello again", author_id = "a1", stats = { likes = 3 } }) == post,
        "The same key keeps one record")
    check(mirror.object.Title == "Hello again" and mirror.object.LikeLabel == "3", "Full updates reach existing mirrors")
    check(mirror.children.Tags:count() == 0, "A full update without tags clears the embedded relation")
    store:put("TestPost", { id = "p1", title = "Partial" }, { partial = true })
    check(post.values.Likes == 3 and post.values.AuthorId == "a1" and mirror.object.Title == "Partial",
        "Partial updates keep missing fields")
    store:put("TestPost", { id = "p1", stats = { likes = data.Schema.null } }, { partial = true })
    check(post.values.Likes == nil and mirror.object.LikeLabel == "…", "An explicit null clears a nullable field")
    store:update(post, { Liked = true })
    store:put("TestPost", { id = "p1", title = "Server", author_id = "a1", stats = { likes = 4 } })
    check(post.values.Liked == true and mirror.object.LikeLabel == "♥ 4", "Local columns survive service refreshes")
    check(not pcall(store.update, store, post, { Title = "x" }), "Only local columns accept client edits")
    local tagged = store:put("TestPost", { id = "p2", title = "Tags", author_id = "a1", tags = { "one", "two" } })
    local tagMirror = store:mirror(tagged)
    local tagsNode = tagMirror.object:findChild("Tags")
    check(tagsNode and tagsNode:getFirstChild().Name == "one", "Embedded relations are child collections of the mirror")
    local first = tagsNode:getFirstChild()
    store:put("TestPost", { id = "p2", title = "Tags", author_id = "a1", tags = { "zero", "one", "two" } })
    check(tagsNode:getFirstChild().Name == "zero" and tagsNode:getFirstChild():getNext() == first,
        "Nested collections keep row identity")
    check(store:recordFor(mirror.object) == post, "A mirror maps back to its record through a new Lua handle")
    store:remove("TestPost", "p2")
    check(tagged.deleted and store:get("TestPost", "p2") == nil, "Removal deletes the record")
    store:release(tagMirror); store:release(mirror)
    check(store:collect(0) >= 1 and store:get("TestPost", "p1") == nil, "Unreferenced records are collected")
end

-- Feed entries keep their own keys while showing one shared post.
do
    local h = host()
    local scope = scopeWith(h)
    local store = scope.store
    store:put("TestPost", { id = "p1", title = "Shared", author_id = "a1", stats = { likes = 1 } })
    local list = data.Collection.new(store, { name = "Feed" })
    local entries = store:putAll("TestEntry", {
        { key = "p1:post", post_id = "p1" }, { key = "p1:repost:bob", post_id = "p1", reposted_by = "bob" } })
    list:set(entries)
    local rows = {}
    local node = list.node:getFirstChild()
    while node do rows[#rows + 1] = node; node = node:getNext() end
    check(#rows == 2 and rows[1].Title == "Shared" and rows[2].Title == "Shared", "Two feed entries show one post")
    store:put("TestPost", { id = "p1", title = "Edited", author_id = "a1", stats = { likes = 5 } })
    check(rows[1].Title == "Edited" and rows[2].LikeLabel == "5", "A post update reaches every entry presenting it")
    local nested = rows[2]:findChild("Post")
    check(nested and nested.Title == "Edited" and rows[1]:findChild("Post") ~= nested,
        "Embedded relations give each row its own nested mirror of the shared record")
    check(store:recordFor(nested) == store:get("TestPost", "p1"), "A nested mirror resolves to the related record")
    check(list.node:getFirstChild() == rows[1], "Field updates never recreate rows")
    check(not pcall(list.set, list, { entries[1], entries[1] }), "Duplicate row keys are rejected")
    local same = {}
    for _, entry in ipairs(entries) do same[#same + 1] = entry end
    list:set { same[2], same[1] }
    check(list.node:getFirstChild() == rows[2] and list.node:getFirstChild():getNext() == rows[1],
        "Reordering keeps native row identity")
    store:remove("TestEntry", "p1:post")
    check(list:count() == 1 and list.node:getFirstChild() == rows[2], "Deleted records leave collections")
end

-- Queries: state, stale responses, session tokens and pagination.
do
    local h = host()
    local calls = {}
    local provider = {}
    local scope = scopeWith(h, provider)
    local query = scope:query { name = "Posts", entity = "TestPost", pageSize = 2,
        params = { { name = "Search", type = "string", debounce = 200 } },
        operation = function(params, page)
            local call = { params = params, page = page }
            calls[#calls + 1] = call
            coroutine.yield()
            if call.fail then return nil, { code = "network", message = "Offline" } end
            return call.result
        end }
    local state = query.state
    check(state.ShowLoading == false and state.HasLoaded == false, "A new query is idle")
    query:load()
    check(state.IsLoading and state.ShowLoading and not state.HasItems, "The first load shows loading")
    calls[1].result = { items = { { id = "p1", title = "One", author_id = "a" }, { id = "p2", title = "Two", author_id = "a" } }, cursor = "c2" }
    h.resume()
    check(not state.IsLoading and state.Count == 2 and state.HasMore and state.CanLoadMore, "Results publish state")
    local firstRow = state:findChild("Items"):getFirstChild()
    query:refresh()
    check(state.IsRefreshing and state.Count == 2 and not state.ShowLoading, "Refresh keeps successful rows")
    query:refresh()
    check(#calls == 2, "A second refresh while one is in flight coalesces")
    calls[2].result = { items = { { id = "p1", title = "One!", author_id = "a" }, { id = "p2", title = "Two", author_id = "a" } }, cursor = "c2" }
    h.resume()
    check(#calls == 3, "The coalesced refresh runs after the first")
    calls[3].result = calls[2].result
    h.resume()
    check(state:findChild("Items"):getFirstChild() == firstRow and firstRow.Title == "One!", "Refresh keeps row identity")
    query:loadMore()
    check(state.IsLoadingMore and calls[4].page.cursor == "c2", "Load more sends the cursor")
    calls[4].result = { items = { { id = "p2", title = "Two", author_id = "a" }, { id = "p3", title = "Three", author_id = "a" } } }
    h.resume()
    check(state.Count == 3 and not state.HasMore, "Pages merge without duplicates and the last page ends paging")
    -- Responses arriving in reverse order: only the newest parameters apply.
    query:set("Search", "a")
    query:set("Search", "ab")
    check(#h.timers == 2 and h.timers[1].canceled, "Typing restarts the debounce")
    h.tick()
    check(#calls == 5 and calls[5].params.Search == "ab" and calls[5].page.cursor == nil,
        "A parameter change resets the cursor and sends the latest value")
    query:set("Search", "abc")
    h.tick()
    calls[6].result = { items = { { id = "p9", title = "Newest", author_id = "a" } } }
    calls[5].result = { items = { { id = "p8", title = "Old", author_id = "a" } } }
    h.resume(2) -- newest first
    h.resume(1) -- then the stale response
    check(state.Count == 1 and state:findChild("Items"):getFirstChild().Title == "Newest",
        "A late response for older parameters is discarded")
    -- Errors keep loaded rows; a failed first load shows the error state.
    query:refresh()
    calls[7].fail = true
    h.resume()
    check(state.HasError and state.ErrorMessage == "Offline" and state.Count == 1 and not state.ShowError,
        "A refresh error keeps the rows and reports the error")
    query:retry()
    calls[8].result = { items = {} }
    h.resume()
    check(not state.HasError and state.IsEmpty and state.ShowEmpty, "Retry clears the error; empty results show empty")
    -- Session change: an in-flight response is discarded.
    query:refresh()
    scope:invalidateRequests()
    calls[9].result = { items = { { id = "late", title = "Late", author_id = "a" } } }
    h.resume()
    check(state.Count == 0, "A response from before a session change is discarded")
    -- Disposal discards results and releases rows.
    local other = scope:query { name = "Other", entity = "TestPost", operation = function()
        coroutine.yield(); return { { id = "o1", title = "x", author_id = "a" } } end }
    other:load()
    other:dispose()
    h.resume()
    check(other.items:count() == 0, "A disposed query ignores its response")
end

-- Polling follows the owner's activity and foreground.
do
    local h = host()
    local count = 0
    local scope = scopeWith(h)
    local query = scope:query { name = "Polled", entity = "TestPost", poll = 1000,
        operation = function() count = count + 1; return {} end }
    query:activate()
    check(count == 1 and #h.timers == 1, "Activation loads and schedules polling")
    h.tick()
    check(count == 2 and #h.timers == 1, "Polling refreshes quietly and reschedules")
    query:setForeground(false)
    h.tick()
    check(count == 2, "Background stops polling")
    query:setForeground(true)
    check(count == 3, "Returning to the foreground refreshes")
    query:deactivate()
    h.tick()
    check(count == 3, "An inactive query does not poll")
    query:invalidate()
    check(count == 3 and query.stale, "Invalidating an inactive query defers the reload")
    query:activate()
    check(count == 4, "Activation reloads a stale query")
end

-- Local views: filters, sort, selection by identity, master-detail params.
do
    local h = host()
    local scope = scopeWith(h)
    local query = scope:query { name = "All", entity = "TestPost", operation = function(params)
        return params.rows or {
            { id = "a", title = "Apple", author_id = "x", stats = { likes = 1 } },
            { id = "b", title = "Banana", author_id = "x", stats = { likes = 5 } },
            { id = "c", title = "Cherry", author_id = "x", stats = { likes = 0 } },
        } end }
    local view = scope:view { name = "Liked", source = "All",
        params = { { name = "Min", type = "int", default = "1" } },
        filter = function(record, params) return (record.values.Likes or 0) >= params.Min end,
        sort = function(a, b) return a.values.Title > b.values.Title end }
    check(view.state.ShowEmpty == false and view.state.ShowLoading == false, "An unloaded source shows nothing yet")
    query:load()
    check(view.state.Count == 2 and view.items:keys()[1] == "b", "Views filter and sort loaded records")
    view:select("a")
    check(view.state.HasSelection and view.state:findChild("Selected").Title == "Apple", "Selection exposes the record")
    scope.store:put("TestPost", { id = "a", title = "Apricot", author_id = "x", stats = { likes = 0 } })
    check(view.state.Count == 1 and not view.state.HasSelection, "A record leaving the view clears the selection")
    scope.store:put("TestPost", { id = "c", title = "Cherry", author_id = "x", stats = { likes = 9 } })
    check(view.items:keys()[1] == "c", "Field edits re-filter and re-sort")
    view:select("c")
    view:set("Min", 0)
    check(view.selected.key == "c" and view.state.Count == 3, "Selection survives refiltering by identity")
end

-- Commands: availability, busy, errors, re-entry and invalidation.
do
    local h = host()
    local scope = scopeWith(h)
    local loads = 0
    scope:query { name = "Feed", entity = "TestPost", operation = function() loads = loads + 1; return {} end }
    scope:member("Feed"):activate()
    local outcome
    local like = scope:command { name = "Like", invalidates = { "Feed" },
        canExecute = function(params) return params.item ~= false end,
        operation = function(params) coroutine.yield(); return outcome, outcome == nil and { code = "denied", message = "No" } or nil end }
    check(like.state.CanExecute and like.state.IsAvailable, "An available command can execute")
    check(like:execute {}, "Execute starts")
    check(like.state.IsExecuting and not like.state.CanExecute, "Blocking commands disable while running")
    check(not like:execute {}, "Re-entry is blocked")
    outcome = nil
    h.resume()
    check(not like.state.IsExecuting and like.state.HasError and like.state.ErrorMessage == "No", "Failures report and end busy")
    outcome = true
    like:execute {}
    h.resume()
    check(like.state.Succeeded and not like.state.HasError and loads == 2, "Success invalidates the named query")
    local missing = scope:command { name = "Missing" }
    check(not missing.state.IsAvailable and not missing.state.CanExecute, "Unsupported commands are visibly unavailable")
    missing:execute {}
    check(missing.state.HasError and missing.state.ErrorCode == "unsupported", "Unsupported commands never pretend to succeed")
    local order = {}
    local queued = scope:command { name = "Queued", concurrency = "queue",
        operation = function(params) coroutine.yield(); order[#order + 1] = params.n; return true end }
    queued:execute { n = 1 }; queued:execute { n = 2 }
    h.resume(); h.resume()
    check(order[1] == 1 and order[2] == 2, "Queued commands run in order")
    local seen = 0
    local latest = scope:command { name = "Latest", concurrency = "latest",
        operation = function() coroutine.yield(); return true end, succeeded = function() seen = seen + 1 end }
    latest:execute {}; latest:execute {}
    h.resume(1); h.resume(1)
    check(seen == 1, "Only the latest run applies its result")
    local crashed = scope:command { name = "Crash", sync = true, operation = function() error("boom") end }
    crashed:execute {}
    check(crashed.state.HasError and not crashed.state.IsExecuting and #h.reports == 1, "A thrown error is reported and ends busy")
end

-- Drafts: validation, failure keeps input, typing during submit survives.
do
    local h = host()
    local scope = scopeWith(h)
    local result
    scope:command { name = "Send", operation = function(params)
        coroutine.yield()
        if result then return { body = params.draft.Body } end
        return nil, { code = "network", message = "Couldn't send" }
    end }
    local edit = scope:edit { name = "Composer", entity = "TestDraft", submit = "Send",
        copy = { Body = "Title" },
        validate = function(values) if not values.Body:match("%S") then return { Body = "Write something" } end end }
    local draft = edit.draft
    check(not draft:findChild("Status").CanSubmit, "An empty draft cannot submit")
    check(not edit:submit(), "Validation blocks the submit")
    check(draft:findChild("Errors").Body == "Write something", "Field errors are bindable")
    draft.Body = "Hello"
    core.flushQueue()
    check(draft:findChild("Status").IsDirty and draft:findChild("Status").CanSubmit, "Editing marks the draft dirty")
    result = false
    edit:submit()
    check(draft:findChild("Status").IsSubmitting and not draft:findChild("Status").CanSubmit, "Submitting disables submit")
    h.resume()
    check(draft.Body == "Hello" and draft:findChild("Status").ErrorMessage == "Couldn't send", "Failure keeps the input")
    result = true
    edit:submit()
    draft.Body = "Hello and more"
    h.resume()
    check(draft.Body == "Hello and more", "Text typed during a pending submit is kept")
    edit:reset({ Body = "" })
    -- Per-key drafts and edit copies.
    edit:switch("room1"); draft.Body = "room one draft"
    edit:switch("room2")
    check(draft.Body == "", "Another key starts with its own draft")
    edit:switch("room1")
    check(draft.Body == "room one draft", "Switching back restores the draft")
    local post = scope.store:put("TestPost", { id = "p", title = "Original", author_id = "a" })
    edit:begin(post)
    check(draft.Body == "Original" and draft:findChild("Status").IsEditing, "An edit copy starts from the record")
    draft.Body = "Changed"
    edit:cancel()
    check(draft.Body == "room one draft" and post.values.Title == "Original", "Cancel restores the unsent draft")
end

-- XML handler names dispatch to scope members through real events.
do
    local h = host()
    local scope = scopeWith(h)
    local opened, loads = nil, 0
    scope:query { name = "Rows", entity = "TestPost", operation = function()
        loads = loads + 1
        return { { id = "r1", title = "First", author_id = "a" }, { id = "r2", title = "Second", author_id = "a" } } end }
    scope:command { name = "Open", sync = true, operation = function(params) opened = params.item; return true end }
    scope:member("Rows"):load()
    local screen = ui.Screen { Width = 320, Height = 240, ResizeMode = "NoResize" }
    local page = ui.StackView { Name = "Page" }
    screen:addChild(page)
    local list = ui.ItemsStack { Name = "List" }
    page:addChild(list)
    list:attachPropertyProgram("ItemsControl.ItemsSource", "{DataContext/Rows/Items}", "OneWay", true)
    local template = ui.Button { Name = "Row" }
    template.Click = "Open"
    list.ItemTemplate = template
    scope:attach(page)
    local retry = ui.Button { Name = "Retry" }
    retry.Click = "Rows.Retry"
    page:addChild(retry)
    core.flushQueue(); core.advanceFrame(); core.runAllPrograms(screen); core.flushQueue()
    local second = list:getFirstChild():getNext()
    check(second and second.DataContext.Title == "Second", "Rows bind to the query collection")
    second:send("Node.LeftButtonUp")
    core.flushQueue()
    check(opened and opened.key == "r2", "A row handler receives the record it presents")
    retry:send("Node.LeftButtonUp")
    core.flushQueue()
    check(loads == 2, "Member actions dispatch by Name.Action")
end

print(("PASS: data runtime (%d checks)"):format(checks))
