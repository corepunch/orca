-- ORCA Framework Main Entry Point
-- This file provides the main initialization and message loop for ORCA applications

-- Load all required ORCA modules
require 'orca'
require 'orca.core'
require 'orca.geometry'
require 'orca.filesystem'
require 'orca.renderer'
require 'orca.parsers.xml'
require 'orca.ui'
require 'orca.SceneKit'

local orca = require 'orca'
local sys = require 'orca.system'
local fs = require 'orca.filesystem'
local loc = require 'orca.localization'
local ref = require 'orca.renderer'
local s = require 'orca.backend'

local M = {}

-- Initialize and run an ORCA project
-- @param config table with project configuration
--   - dirname: Project directory path
--   - name: Project name
--   - startup_screen: Startup screen module path
--   - locales: Array of locale paths to load
--   - messages: Array of system message handlers
function M.run(config)
  config = config or {}
  
  -- Set project name global
  if config.name then
    PROJECTNAME = config.name
  end
  
  -- Initialize filesystem with project directory
  if config.dirname then
    fs.init(config.dirname)
  end
  
  -- Load localization files
  if config.locales then
    for _, locale_path in ipairs(config.locales) do
      loc.load(locale_path)
    end
  end
  
  -- Load and instantiate the startup screen
  local screen
  if config.startup_screen then
    local Screen = require(config.startup_screen)
    screen = Screen()
  else
    error("No startup_screen specified in config")
  end
  
  -- Initialize editor with the screen
  local editor = require 'orca.editor'
  editor.setScreen(screen)
  
  -- Main message loop
  while true do
    for msg in s.getMessage(screen) do
      fs.trackChangedFiles()
      
      -- Check for reload request
      if RELOAD then
        return RELOAD
      end
      
      -- Process custom message handlers
      local handled = false
      if config.messages then
        for _, handler in ipairs(config.messages) do
          if handler.match(msg) then
            handler.action(msg)
            handled = true
            break
          end
        end
      end
      
      -- Dispatch message and repaint if needed
      if s.dispatchMessage(msg) then
        screen:paint(ref.getSize())
      end
    end
  end
end

-- Helper function to create a message handler matcher
-- @param conditions table of conditions to match (e.g., {Message = "KeyDown", key = "q"})
-- @param action function or string to execute when matched
function M.messageHandler(conditions, action)
  return {
    match = function(msg)
      for key, value in pairs(conditions) do
        if key == "Message" then
          if not msg:is(value) then
            return false
          end
        else
          if msg[key] ~= value then
            return false
          end
        end
      end
      return true
    end,
    action = type(action) == "function" and action or function()
      loadstring(action)()
    end
  }
end

return M
