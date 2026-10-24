-- Structural checks only; native preparation decodes pixels and strips source
-- metadata. ImageIO may add harmless generated EXIF while encoding its output.
local Jpeg = {}
local function privateMetadata(bytes, start, finish)
    if bytes:sub(start, start + 5) ~= "Exif\0\0" then
        local data = bytes:sub(start, finish):lower()
        return data:find("gpslatitude", 1, true) or data:find("gpslongitude", 1, true)
    end
    local base = start + 6
    local order = bytes:sub(base, base + 1)
    if order ~= "II" and order ~= "MM" then return true end
    local function number(offset, size)
        local at = base + offset
        if offset < 0 or at + size - 1 > finish then return nil end
        local value = 0
        for i = 0, size - 1 do
            value = value * 256 + bytes:byte(at + (order == "II" and size - 1 - i or i))
        end
        return value
    end
    if number(2, 2) ~= 42 then return true end
    local offset = number(4, 4)
    local count = offset and number(offset, 2)
    if not count or base + offset + 2 + count * 12 + 4 - 1 > finish then return true end
    for i = 0, count - 1 do
        if number(offset + 2 + i * 12, 2) == 0x8825 then return true end -- GPS IFD pointer.
    end
    return false
end
function Jpeg.dimensions(bytes)
    if type(bytes) ~= "string" or #bytes < 32 or #bytes > 1000000 or bytes:sub(1, 2) ~= "\255\216" then
        return nil, "The photo must be a complete JPEG image."
    end
    local at, width, height, scans = 3, nil, nil, 0
    while at <= #bytes do
        if bytes:byte(at) ~= 255 then return nil, "Invalid JPEG marker." end
        while bytes:byte(at) == 255 do at = at + 1 end
        local marker = bytes:byte(at); at = at + 1
        if marker == 217 then
            if at == #bytes + 1 and width and scans > 0 then return width, height end
            return nil, "The JPEG image is incomplete."
        end
        if not marker or marker == 0 or marker == 216 or (marker >= 208 and marker <= 215) then
            return nil, "Invalid JPEG marker."
        end
        local a, b = bytes:byte(at, at + 1)
        if not b then return nil, "Truncated JPEG image." end
        local length = a * 256 + b
        if length < 2 or at + length - 1 > #bytes then return nil, "Truncated JPEG segment." end
        if marker == 225 and privateMetadata(bytes, at + 2, at + length - 1) then
            return nil, "Remove location metadata before uploading."
        end
        if marker >= 192 and marker <= 207 and marker ~= 196 and marker ~= 200 and marker ~= 204 then
            if width or (marker ~= 192 and marker ~= 193 and marker ~= 194) or length < 11 then return nil, "Invalid JPEG frame." end
            local h1, h2, w1, w2, components = bytes:byte(at + 3, at + 7)
            width, height = w1 * 256 + w2, h1 * 256 + h2
            if bytes:byte(at + 2) ~= 8 or (components ~= 1 and components ~= 3) or length ~= 8 + components * 3
                or width < 1 or height < 1 then return nil, "Invalid JPEG dimensions." end
        end
        at = at + length
        if marker == 218 then
            if not width or length < 8 then return nil, "Invalid JPEG scan." end
            scans = scans + 1
            while at <= #bytes do
                if bytes:byte(at) ~= 255 then at = at + 1 else
                    local nextAt = at + 1
                    while bytes:byte(nextAt) == 255 do nextAt = nextAt + 1 end
                    local nextMarker = bytes:byte(nextAt)
                    if nextMarker == 0 or (nextMarker and nextMarker >= 208 and nextMarker <= 215) then at = nextAt + 1
                    else break end
                end
            end
        end
    end
    return nil, "The JPEG image is incomplete."
end
return Jpeg
