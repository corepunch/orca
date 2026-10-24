-- One app instance owns the lifetime of its private image preview files.
local Media = {}
Media.__index = Media

local function failure(code, message)
    return nil, { code = code, message = message }
end

function Media.new()
    return setmetatable({ files = {}, keys = {} }, Media)
end

function Media:pick()
    local ok, native = pcall(require, "orca.media")
    if not ok then return failure("unsupported", "Rebuild ORCA with native photo support to attach a photo.") end
    return native.pickPhoto()
end

function Media:storePreview(bytes, key)
    if type(bytes) ~= "string" or #bytes < 4 or #bytes > 1000000
        or bytes:sub(1, 2) ~= "\255\216" or bytes:sub(-2) ~= "\255\217" then
        return failure("invalid_image", "The photo preview is not a supported JPEG.")
    end
    key = tostring(key or "preview")
    -- os.tmpname creates the file with private permissions. Keep that inode
    -- instead of recreating a predictable suffix with the process umask.
    local path = self.keys[key]
    if not path then
        local temporary = os.tmpname()
        path = temporary .. ".jpg"
        if not os.rename(temporary, path) then
            os.remove(temporary)
            return failure("file", "Could not prepare the photo preview.")
        end
    end
    local file = io.open(path, "wb")
    if not file then
        os.remove(path); self.keys[key], self.files[path] = nil, nil
        return failure("file", "Could not save the photo preview.")
    end
    local written = file:write(bytes)
    local closed = file:close()
    if not written or not closed then
        os.remove(path)
        self.keys[key], self.files[path] = nil, nil
        return failure("file", "Could not save the photo preview.")
    end
    self.files[path] = true
    self.keys[key] = path
    return path
end

function Media:clear()
    for path in pairs(self.files) do os.remove(path) end
    self.files = {}
    self.keys = {}
end

Media.__gc = Media.clear
return Media
