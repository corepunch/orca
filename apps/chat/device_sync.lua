-- Publishes this device's end-to-end encryption keys for the signed-in account.
-- Runs in the background after sign-in and retries with backoff, so an
-- unreachable or older backend never blocks the app.
local Polling = require "Chat/polling"
local DeviceSync = {}
DeviceSync.__index = DeviceSync

local function timerFactory(self)
  local core = require "orca.core"
  self.owner = core.Timer { Name = "ChatDeviceKeys" }
  return function(callback, duration)
    local owner, id, canceled = self.owner
    local handle = {}
    function handle:cancel()
      if canceled then return end
      canceled = true
      owner:cancelTimer(id)
      owner.Tick = nil
    end
    owner.Tick = function(_, args)
      if canceled or args.timerId ~= id then return end
      handle:cancel()
      callback()
    end
    id = owner:setTimer(duration)
    assert(id and id > 0, "Could not schedule device key publishing")
    return handle
  end
end

-- Returns nil when this build has no crypto or secure storage.
function DeviceSync.new(app, config, options)
  options = options or {}
  local crypto, credentials = options.crypto, options.credentials
  if not crypto then
    local ok, module = pcall(require, "orca.crypto")
    if not ok then return nil end
    crypto = module
  end
  if not credentials then
    local ok, module = pcall(require, "orca.credentials")
    if not ok or not module.supported() then return nil end
    credentials = module
  end
  local scope = config.endpoint:gsub("/$", "") .. ":" .. config.projectId
  local store = require("api.device_keys").new { crypto = crypto, credentials = credentials, scope = scope }
  if not store then return nil end
  local self = setmetatable({ app = app, store = store }, DeviceSync)
  self.scheduler = Polling.new { createTimer = options.createTimer or timerFactory(self),
    maxDelay = options.maxDelay or 15 * 60 * 1000 }
  return self
end

function DeviceSync:tick()
  local client = self.app.client
  local session = client:getSession()
  local user = session and session.user.id
  if not user or self.app.logoutPending then return true end
  if self.published == user then return true end
  local device, err = self.store:ensure(client)
  local active = client:getSession()
  if not active or active.user.id ~= user then return true end
  if device then self.published = user; return true end
  self.lastError = err
  return false
end

-- Screen changes publish promptly after sign-in, but never cut short the
-- backoff after a failure.
function DeviceSync:activate()
  if self.app.logoutPending then return end
  if self.scheduler.job then
    local session = self.app.client:getSession()
    if session and self.published ~= session.user.id and self.scheduler.failures == 0 then self.scheduler:refresh() end
    return
  end
  self.scheduler:start({ interval = 60000, run = function() return self:tick() end }, true)
end

function DeviceSync:suspend()
  self.published = nil
  self.scheduler:stop()
end

return DeviceSync
