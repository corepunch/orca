local appwrite = require("orca.plugins.appwrite.functions")
local Model
do
  local _class_0
  local _base_0 = {
    list = function(self, ...)
      return appwrite.listCollections(...)
    end,
    create = function(self, ...)
      return appwrite.createDocument(...)
    end,
    createWithId = function(self, ...)
      return appwrite.createDocumentWithId(...)
    end,
    signin = function(self, params)
      return appwrite.signInAccount(params)
    end,
    signup = function(self, params)
      return appwrite.createUserAccount(params)
    end,
    signout = function(self)
      return appwrite.signOutAccount()
    end,
    getaccount = function(self)
      return appwrite.getAccount()
    end
  }
  _base_0.__index = _base_0
  _class_0 = setmetatable({
    __init = function() end,
    __base = _base_0,
    __name = "Model"
  }, {
    __index = _base_0,
    __call = function(cls, ...)
      local _self_0 = setmetatable({}, _base_0)
      cls.__init(_self_0, ...)
      return _self_0
    end
  })
  _base_0.__class = _class_0
  Model = _class_0
  return _class_0
end
