-- Camera/anchor import for SimpleSketch3D .blks scenes. No renderer dependency.
-- Supported anchors are scene/group children, including prefab instance origins.
-- Prefab contents, attach slots and pivot offsets require a full scene resolver;
-- requesting one of those anchors returns an error instead of a guessed position.
local Projection = {}
local radians = math.pi / 180

local function number(value)
    local n = tonumber(value)
    assert(n and n == n and math.abs(n) < math.huge, "invalid scene number")
    return n
end

local function vector(value, default)
    if not value then return default end
    local result = {}
    for part in value:gmatch("%S+") do result[#result + 1] = number(part) end
    assert(#result == 3, "expected a three-component vector")
    return result
end

local function dot(a, b)
    return a[1] * b[1] + a[2] * b[2] + a[3] * b[3]
end

local function cross(a, b)
    return {a[2]*b[3] - a[3]*b[2], a[3]*b[1] - a[1]*b[3], a[1]*b[2] - a[2]*b[1]}
end

local function normalize(v)
    local length = math.sqrt(dot(v, v))
    if length <= 1e-8 then return {0, 0, 1} end -- SimpleSketch3D vnorm fallback.
    return {v[1] / length, v[2] / length, v[3] / length}
end

local function transform(attrs)
    return {
        pos = vector(attrs.pos, {0, 0, 0}),
        rot = vector(attrs.rot, {0, 0, 0}),
        scale = vector(attrs.scale, {1, 1, 1}),
    }
end

local function apply(t, point)
    local x, y, z = point[1]*t.scale[1], point[2]*t.scale[2], point[3]*t.scale[3]
    local c, s = math.cos(t.rot[1]*radians), math.sin(t.rot[1]*radians)
    y, z = c*y - s*z, s*y + c*z
    c, s = math.cos(t.rot[2]*radians), math.sin(t.rot[2]*radians)
    x, z = c*x + s*z, -s*x + c*z
    c, s = math.cos(t.rot[3]*radians), math.sin(t.rot[3]*radians)
    x, y = c*x - s*y, s*x + c*y
    return {x + t.pos[1], y + t.pos[2], z + t.pos[3]}
end

local function attributes(raw)
    local attrs = {}
    local remainder = raw:gsub("([%w_:%-]+)%s*=%s*(['\"])(.-)%2", function(key, _, value)
        assert(not attrs[key], "duplicate attribute: " .. key)
        assert(not value:find("&"), "XML entities are not supported in scene metadata")
        attrs[key] = value
        return ""
    end)
    assert(remainder:match("^%s*$"), "unsupported scene XML attributes")
    return attrs
end

local objectTags = {
    prefab = true, group = true, box = true, sphere = true, cylinder = true,
    cone = true, wall = true, dummy = true, light = true, plane = true,
}

local function index(map, id, node)
    if not id then return end
    if map[id] then map[id] = {error = "ambiguous anchor: " .. id}
    else map[id] = node end
end

-- Parses the scene's metadata, not prefab geometry. Errors are returned, not thrown.
function Projection.parse(xml)
    local ok, result = pcall(function()
        assert(type(xml) == "string", "scene XML must be a string")
        xml = xml:gsub("<!%-%-.-%-%->", ""):gsub("<%?.-%?>", "")
        local scene = {cameras = {}, objects = {}, sources = {}}
        local stack, roots, cursor = {}, 0, 1
        for first, raw, last in xml:gmatch("()<([^<>]+)>()") do
            assert(xml:sub(cursor, first - 1):match("^%s*$"), "unexpected scene XML text")
            cursor = last
            local closing = raw:match("^/%s*([%w_:%-]+)%s*$")
            if closing then
                assert(stack[#stack] and stack[#stack].tag == closing, "mismatched scene XML tag")
                stack[#stack] = nil
            else
                local tag, rest = raw:match("^([%w_:%-]+)(.*)$")
                assert(tag, "unsupported scene XML tag")
                local empty = rest:match("/%s*$")
                rest = rest:gsub("/%s*$", "")
                local attrs = attributes(rest)
                local parent = stack[#stack]
                if not parent then
                    assert(tag == "scene" and roots == 0, "expected one scene root")
                    roots = roots + 1
                end
                local node = {tag = tag, transform = transform(attrs), parent = parent}
                if attrs.attach then node.error = "attach slots are not supported by the metadata importer" end
                if attrs.pivotOffset then node.error = "pivotOffset is not supported by the metadata importer" end
                if parent and parent.tag ~= "scene" and parent.tag ~= "group" then
                    node.error = "nested geometry is not an independently resolved anchor"
                end
                if parent and parent.error then node.error = parent.error end
                if tag == "camera" then
                    assert(parent and parent.tag == "scene", "cameras must be top-level scene children")
                    local name = attrs.name or ("Camera" .. tostring((scene.cameraCount or 0) + 1))
                    assert(not scene.cameras[name], "duplicate camera: " .. name)
                    local fov = number(attrs.fov or 60)
                    assert(fov > 0 and fov < 180, "camera FOV must be between 0 and 180")
                    scene.cameras[name] = {
                        name = name, pos = vector(attrs.pos, {0, 1.6, 5}),
                        look = vector(attrs.look, {0, 1.2, 0}), fov = fov,
                    }
                    scene.cameraCount = (scene.cameraCount or 0) + 1
                elseif objectTags[tag] then
                    index(scene.objects, attrs.name, node)
                    if tag == "prefab" then index(scene.sources, attrs.source, node) end
                end
                if not empty then stack[#stack + 1] = node end
            end
        end
        assert(roots == 1 and #stack == 0 and xml:sub(cursor):match("^%s*$"), "incomplete scene XML")
        return scene
    end)
    if ok then return result end
    return nil, tostring(result)
end

function Projection.load(path)
    local file, err = io.open(path, "r")
    if not file then return nil, err end
    local xml = file:read("*a")
    file:close()
    return Projection.parse(xml)
end

-- ID is an explicit instance name or an unambiguous prefab source path.
-- localOffset is in object-local units and inherits object/group rotation + scale.
function Projection.anchor(scene, id, localOffset)
    local node = scene.objects[id] or scene.sources[id]
    if not node then return nil, "unknown anchor: " .. tostring(id) end
    if node.error then return nil, node.error end
    local point = localOffset or {0, 0, 0}
    while node and node.tag ~= "scene" do
        point = apply(node.transform, point)
        node = node.parent
    end
    return point
end

-- Source dimensions must match the reference render used to paint the image.
-- The target uses centered UniformToFill cropping; x/y are viewport-local pixels.
function Projection.project(camera, point, sourceWidth, sourceHeight, viewWidth, viewHeight, near)
    if not camera or not point then return nil, "missing camera or anchor" end
    if sourceWidth <= 0 or sourceHeight <= 0 or viewWidth <= 0 or viewHeight <= 0 then
        return nil, "invalid image or viewport dimensions"
    end
    local forward = normalize({camera.look[1]-camera.pos[1], camera.look[2]-camera.pos[2], camera.look[3]-camera.pos[3]})
    local right = normalize(cross(forward, {0, 1, 0}))
    local up = cross(right, forward)
    local delta = {point[1]-camera.pos[1], point[2]-camera.pos[2], point[3]-camera.pos[3]}
    local depth = dot(delta, forward)
    if depth <= (near or 0.1) then return nil, "behind camera or near plane" end
    local focal = sourceHeight / (2 * math.tan(camera.fov * radians / 2))
    local sx = sourceWidth / 2 + dot(delta, right) * focal / depth
    local sy = sourceHeight / 2 - dot(delta, up) * focal / depth
    if sx < 0 or sx > sourceWidth or sy < 0 or sy > sourceHeight then return nil, "outside source image" end
    local scale = math.max(viewWidth / sourceWidth, viewHeight / sourceHeight)
    local x = sx * scale + (viewWidth - sourceWidth * scale) / 2
    local y = sy * scale + (viewHeight - sourceHeight * scale) / 2
    if x < 0 or x > viewWidth or y < 0 or y > viewHeight then return nil, "cropped by viewport" end
    return {x = x, y = y, sourceX = sx, sourceY = sy, depth = depth, scale = scale}
end

return Projection
