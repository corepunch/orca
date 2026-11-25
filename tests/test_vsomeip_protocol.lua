local test = require "orca.test"
local plugin = require "orca.vsomeip"

local ok, err = pcall(plugin.SpeedTest.new)
test.expect(not ok and tostring(err):find("string expected", 1, true),
    "Validate the protocol argument before starting transport")

local path = os.tmpname()
local file = assert(io.open(path .. ".xml", "w"))
file:write("<DataObject/>")
file:close()
ok, err = pcall(plugin.SpeedTest.new, path)
os.remove(path .. ".xml")
os.remove(path)
test.expect(not ok and tostring(err):find("<protocol>", 1, true),
    "Reject non-protocol XML before opening a client or contacting a router")
print("PASS: vsomeip offline protocol validation")
