-- Photo picking and the app's private image cache. ImageView loads files, so
-- downloaded photos use a bounded persistent cache when one is supplied.
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

function Media.new(options)
    return setmetatable({ files = {}, keys = {}, cache = options and options.cache }, Media)
end

function Media:pick()
    local ok, native = pcall(require, "orca.media")
    if not ok or type(native.pickPhoto) ~= "function" then
        return failure("unsupported", "Photo selection needs ORCA's native photo support (macOS or iOS).")
    end
    return native.pickPhoto()
end

function Media:path(key)
    key = tostring(key)
    return self.keys[key] or (self.cache and self.cache:path(key))
end

function Media:discard(key)
    key = tostring(key)
    if self.cache then self.cache:discard(key) end
    local path = self.keys[key]
    if not path then return end
    os.remove(path)
    self.files[path], self.keys[key] = nil, nil
end

function Media:store(bytes, key)
    local extension = kind(bytes)
    if not extension or #bytes > 12000000 then return failure("invalid_image", "This photo can't be shown here.") end
    key = tostring(key or "photo")
    local existing = self:path(key)
    if existing then return existing end
    if self.cache then
        local path = self.cache:store(bytes, key)
        if path then return path end
    end
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
