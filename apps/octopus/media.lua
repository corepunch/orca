-- Photo picking and the app's private image cache. ImageView loads files, so
-- downloaded post photos are written once per file ID and removed on exit.
local Media = {}
Media.__index = Media

local function failure(code, message)
    return nil, { code = code, message = message }
end

local function kind(bytes)
    if type(bytes) ~= "string" then return nil end
    if bytes:sub(1, 3) == "\255\216\255" then return ".jpg" end
    if bytes:sub(1, 8) == "\137PNG\r\n\26\n" then return ".png" end
end

function Media.new()
    return setmetatable({ files = {}, keys = {} }, Media)
end

function Media:pick()
    local ok, native = pcall(require, "orca.media")
    if not ok or type(native.pickPhoto) ~= "function" then
        return failure("unsupported", "Photo selection needs ORCA's native photo support (macOS or iOS).")
    end
    return native.pickPhoto()
end

function Media:path(key)
    return self.keys[tostring(key)]
end

function Media:discard(key)
    key = tostring(key)
    local path = self.keys[key]
    if not path then return end
    os.remove(path)
    self.files[path], self.keys[key] = nil, nil
end

function Media:store(bytes, key)
    local extension = kind(bytes)
    if not extension or #bytes > 12000000 then return failure("invalid_image", "This photo can't be shown here.") end
    key = tostring(key or "photo")
    if self.keys[key] then return self.keys[key] end
    -- os.tmpname creates the file with private permissions; keep that inode.
    local temporary = os.tmpname()
    local path = temporary .. extension
    if not os.rename(temporary, path) then
        os.remove(temporary)
        return failure("file", "Could not prepare the photo.")
    end
    local file = io.open(path, "wb")
    local written = file and file:write(bytes)
    local closed = file and file:close()
    if not written or not closed then
        os.remove(path)
        return failure("file", "Could not save the photo.")
    end
    self.files[path], self.keys[key] = true, path
    return path
end

function Media:clear()
    for path in pairs(self.files) do os.remove(path) end
    self.files, self.keys = {}, {}
end

Media.__gc = Media.clear
return Media
