-- Loads media for records and publishes it as record state, so a template
-- binds an image path and a loading/error state instead of owning a loader:
--
--   <ImageView Src="{Binding DataContext/AvatarPath}" Visible="{Binding DataContext/HasAvatar}"/>
--
-- Targets map entity columns to the media identity and the local columns
-- that receive the result. One request serves every record sharing an id.
-- Results from a replaced scope or an outdated revision are discarded. The
-- presentation layer reports which records are visible (`want`).
local Media = {}
Media.__index = Media

-- options: name, targets = { { entity, id = column, path = local column,
-- state = local column (optional), revision = column (optional) } },
-- lookup(ids) -> id -> file id | false (optional, batched; false means none),
-- fetch(file, record, stage) -> bytes | nil, err; store(bytes, file, stage, id) -> path;
-- cached(file, id) -> path; without lookup the file is the record's media id. stages ({ "full" } by default, e.g. { "preview", "full" });
-- maxActive (2), validate(path) -> ok, width, height (decoding check),
-- missing(id) after a lookup reports no media for an id, preview(bytes, file)
-- -> path for progressive previews fetch reports through its fourth argument.
-- A target's `ratio` local column receives width / height once decoded.
function Media.new(scope, options)
    local self = setmetatable({ scope = scope, options = options, name = options.name or "Media",
        targets = options.targets, pending = {}, requests = {}, paths = {}, failed = {}, known = {},
        active = 0, maxActive = options.maxActive or 2, stages = options.stages or { "full" } }, Media)
    for _, target in ipairs(self.targets) do
        local entity = scope.store:entity(target.entity)
        for _, column in ipairs { target.path, target.state, target.ratio } do
            if column and not (entity.byName[column] and entity.byName[column].isLocal) then
                error(("Media %s: %s.%s must be a Local column"):format(self.name, target.entity, column), 0)
            end
        end
    end
    return self
end

local function targetFor(self, record)
    for _, target in ipairs(self.targets) do
        if target.entity == record.entity.name then return target end
    end
end

-- A row may present the media record through an embedded relation (a feed
-- entry and its post); look through it.
local function mediaRecord(self, record)
    if targetFor(self, record) then return record end
    for _, related in pairs(record.related or {}) do
        if targetFor(self, related) then return related end
    end
end

-- Whether a file decodes to a native image; returns ok, width, height. A
-- download that succeeded can still be corrupt.
function Media.decodes(path)
    local ui = require "orca.UIKit"
    local image = ui.ImageView { Src = path }
    image:send("Node.LoadView")
    local source = image.Source
    if not source then return false end
    return true, source.Width, source.Height
end

function Media:identity(record)
    record = mediaRecord(self, record)
    local target = record and targetFor(self, record)
    local id = target and record.values[target.id]
    if id == nil or id == "" then return nil end
    local revision = target.revision and record.values[target.revision]
    return id, revision and (id .. "@" .. tostring(revision)) or id, target
end

-- Writes the outcome to every record presenting this media id.
function Media:publish(id, key, values)
    local store = self.scope.store
    store:batch(function()
        for _, target in ipairs(self.targets) do
            for _, record in pairs(store.records[target.entity] or {}) do
                local recordId, recordKey = self:identity(record)
                if recordId == id and recordKey == key then
                    local locals = {}
                    if target.path and values.path ~= nil then locals[target.path] = values.path end
                    if target.state and values.state then locals[target.state] = values.state end
                    if target.ratio and values.ratio then locals[target.ratio] = values.ratio end
                    if next(locals) then store:update(record, locals) end
                end
            end
        end
    end)
end

-- Requests media for a record the presentation shows (or is about to).
function Media:want(record)
    record = mediaRecord(self, record)
    if not record then return end
    local id, key = self:identity(record)
    if not id or self.scope.disposed then return end
    if self.paths[key] then
        local target = targetFor(self, record)
        if target.path and record.values[target.path] ~= self.paths[key] then
            self:publish(id, key, { path = self.paths[key], state = "ready", ratio = self.ratios and self.ratios[key] })
        end
        return
    end
    if self.failed[key] or self.requests[key] or self.known[id] == false then return end
    local file = id
    if self.options.lookup then
        file = self.known[id]
        if type(file) ~= "string" then return end
    end
    local cached = self.options.cached and self.options.cached(file, id)
    if cached then
        self.paths[key] = cached
        return self:publish(id, key, { path = cached, state = "ready" })
    end
    local request = { id = id, file = file, key = key, record = record, stage = 1, token = self.scope:token() }
    self.requests[key] = request
    self.pending[#self.pending + 1] = request
    self:publish(id, key, { state = "loading" })
end

-- Visible ids missing from a batched lookup have no media; ask once.
-- `records` is what the presentation shows now: queued requests for records
-- that scrolled away are dropped before they start.
function Media:lookup(records)
    self.visible = {}
    for _, record in ipairs(records) do
        local _, key = self:identity(record)
        if key then self.visible[key] = true end
    end
    if not self.options.lookup then
        for _, record in ipairs(records) do self:want(record) end
        return self:pump()
    end
    local ids, seen = {}, {}
    for _, record in ipairs(records) do
        local id = self:identity(record)
        if id and self.known[id] == nil and not seen[id] then seen[id] = true; ids[#ids + 1] = id end
    end
    local function proceed()
        for _, record in ipairs(records) do
            local id = self:identity(record)
            if id and self.known[id] and self.known[id] ~= "pending" then self:want(record) end
        end
        self:pump()
    end
    if #ids == 0 then return proceed() end
    for _, id in ipairs(ids) do self.known[id] = "pending" end
    local token = self.scope:token()
    self.scope:run(function()
        local ok, found = pcall(self.options.lookup, ids)
        if not self.scope:valid(token) then return end
        for _, id in ipairs(ids) do
            local file = ok and found and found[id]
            if ok and found then
                self.known[id] = type(file) == "string" and file or (file == true and id) or false
                if self.known[id] == false and self.options.missing then self.options.missing(id) end
            else self.known[id] = nil end
        end
        if ok and found then proceed() end
    end)
end

function Media:pump()
    while self.active < self.maxActive and #self.pending > 0 do
        local request = table.remove(self.pending, 1)
        if self.visible and not self.visible[request.key] then
            self.requests[request.key] = nil
            self:publish(request.id, request.key, { state = "" })
            goto continue
        end
        self.active = self.active + 1
        self.scope:run(function()
            -- Work queued before a sign-out or a closed page never starts.
            if not self.scope:valid(request.token) or self.requests[request.key] ~= request then
                self.active = self.active - 1
                return
            end
            local stage = self.stages[request.stage]
            local path, err
            local ok = pcall(function()
                local bytes
                bytes, err = self.options.fetch(request.file, request.record, stage, function(preview)
                    if not self.scope:valid(request.token) or self.requests[request.key] ~= request
                        or (self.visible and not self.visible[request.key]) or not self.options.preview then return false end
                    local snapshot = self.options.preview(preview, request.file)
                    if snapshot then self:publish(request.id, request.key, { path = snapshot, state = "preview" }) end
                end)
                if not bytes or not self.scope:valid(request.token) then return end
                path, err = self.options.store(bytes, request.file, stage, request.id)
                if path and self.options.validate then
                    local ok, width, height = self.options.validate(path)
                    if not ok then
                        if self.options.discard then self.options.discard(request.file) end
                        path, err = nil, { code = "decode", message = "This photo could not be decoded." }
                    elseif width and height and width > 0 and height > 0 then
                        request.ratio = width / height
                    end
                end
            end)
            self.active = self.active - 1
            if not self.scope:valid(request.token) or self.requests[request.key] ~= request then return end
            local last = request.stage == #self.stages
            if ok and path then
                if last then
                    self.paths[request.key], self.requests[request.key] = path, nil
                    self.ratios = self.ratios or {}
                    self.ratios[request.key] = request.ratio
                    self:publish(request.id, request.key, { path = path, state = "ready", ratio = request.ratio })
                else
                    -- A preview shows at once; the full stage follows.
                    self:publish(request.id, request.key, { path = path, state = "preview" })
                    request.stage = request.stage + 1
                    self.pending[#self.pending + 1] = request
                end
            elseif last then
                self.requests[request.key], self.failed[request.key] = nil, err or { code = "unavailable" }
                self:publish(request.id, request.key, { state = "error" })
            else
                request.stage = request.stage + 1
                self.pending[#self.pending + 1] = request
            end
            if self.options.repaint then self.options.repaint() end
            self:pump()
        end)
        ::continue::
    end
end

-- Explicit retry after a failure (a Retry button bound to Photos.Retry).
function Media:retry(record)
    record = record and mediaRecord(self, record)
    if not record then return end
    local id, key = self:identity(record)
    if not id then return end
    self.failed[key] = nil
    self:want(record)
    self:pump()
end

-- Forgets every failure, so the next visible pass asks again (a list that
-- refreshed retries photos that failed before).
function Media:retryFailed()
    if next(self.failed) == nil then return false end
    self.failed = {}
    return true
end

-- Drops cached results for an id (a new profile photo).
function Media:invalidate(id)
    for key in pairs(self.paths) do
        if key == id or key:sub(1, #id + 1) == id .. "@" then self.paths[key], self.failed[key] = nil, nil end
    end
    self.known[id] = nil
    self:publish(id, id, { path = "", state = "" })
end

function Media:clear()
    self.pending, self.requests, self.paths, self.failed, self.known = {}, {}, {}, {}, {}
end

return Media
