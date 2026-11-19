-- The explorer's passport: which articles have earned a stamp. It is kept in
-- a small text file, one article id per line, so progress survives a restart.
local Passport = {}
Passport.__index = Passport

-- A folder the app may write to: the sandbox's Library on Apple systems,
-- the usual data folder elsewhere.
local function defaultPath()
    local override, home = os.getenv("ENCARTA_DATA_DIR"), os.getenv("HOME")
    if override and override ~= "" then return override .. "/passport.txt" end
    if not home or home == "" then return nil end
    local fs = require "orca.filesystem"
    local folder = home .. "/Library"
    if fs.fileExists(folder .. "/.") then
        folder = folder .. "/Application Support"
    else
        folder = home .. "/.local/share"
    end
    for _, part in ipairs({ "", "/Encarta" }) do
        folder = folder .. part
        if not fs.fileExists(folder .. "/.") then fs.makeDirectory(folder) end
    end
    return folder .. "/passport.txt"
end

-- `path` is where the stamps are kept; without one the app's own folder is
-- used. A passport that cannot be read or written still works until the app
-- closes.
function Passport.open(path)
    local self = setmetatable({ path = path or defaultPath(), stamps = {}, count = 0 }, Passport)
    -- Update mode asks for the real file; plain reads go to the app bundle.
    local file = self.path and io.open(self.path, "r+b")
    if file then
        for id in (file:read("a") or ""):gmatch("[^\r\n]+") do
            if not self.stamps[id] then
                self.stamps[id], self.count = true, self.count + 1
            end
        end
        file:close()
    end
    return self
end

function Passport:has(id)
    return self.stamps[id] == true
end

-- Returns true when the stamp is new.
function Passport:add(id)
    if self.stamps[id] then return false end
    self.stamps[id], self.count = true, self.count + 1
    local file = self.path and io.open(self.path, "ab")
    if file then
        file:write(id, "\n")
        file:close()
    end
    return true
end

return Passport
