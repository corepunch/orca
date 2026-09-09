-- Camera/anchor import for Scener .blks scenes. No renderer dependency.
-- Supported anchors are scene/group children, including prefab instance origins.
-- Prefab contents, attach slots and pivot offsets require a full scene resolver;
-- requesting one of those anchors returns an error instead of a guessed position.
local Projection = {}
local radians = math.pi / 180
local centimeters = 0.01

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

local function position(value, default)
    local result = vector(value, default)
    if not value then return result end
    return {result[1] * centimeters, result[2] * centimeters, result[3] * centimeters}
end

local function dot(a, b)
    return a[1] * b[1] + a[2] * b[2] + a[3] * b[3]
end

local function cross(a, b)
    return {a[2]*b[3] - a[3]*b[2], a[3]*b[1] - a[1]*b[3], a[1]*b[2] - a[2]*b[1]}
end

local function normalize(v)
    local length = math.sqrt(dot(v, v))
    if length <= 1e-8 then return {0, 0, 1} end -- Scener vnorm fallback.
    return {v[1] / length, v[2] / length, v[3] / length}
end

local function transform(attrs)
    return {
        pos = position(attrs.pos, {0, 0, 0}),
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
                    scene.up = attrs.up or "y"
                    assert(scene.up == "y" or scene.up == "z", "unsupported scene up axis")
                    roots = roots + 1
                end
                local node = {tag = tag, attrs = attrs, transform = transform(attrs), parent = parent, children = {}}
                if parent then parent.children[#parent.children + 1] = node else scene.root = node end
                if tag == "box" then node.size = position(attrs.size, {0.01, 0.01, 0.01}) end
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
                        name = name, pos = position(attrs.pos, {0, 1.6, 5}),
                        look = position(attrs.look, {0, 1.2, 0}), fov = fov, up = scene.up,
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

local function worldPoint(node, point)
    while node and node.tag ~= "scene" do
        point = apply(node.transform, point)
        node = node.parent
    end
    return point
end

local function includePoint(bounds, point)
    if not bounds.min then
        bounds.min = {point[1], point[2], point[3]}
        bounds.max = {point[1], point[2], point[3]}
        return
    end
    for axis = 1, 3 do
        bounds.min[axis] = math.min(bounds.min[axis], point[axis])
        bounds.max[axis] = math.max(bounds.max[axis], point[axis])
    end
end

local function includeGeometry(owner, node, bounds)
    if node ~= owner and node.attrs.name then return end
    if node.tag == "box" and node.size then
        for x = -1, 1, 2 do
            for y = -1, 1, 2 do
                for z = -1, 1, 2 do
                    includePoint(bounds, worldPoint(node, {
                        x * node.size[1] / 2, y * node.size[2] / 2, z * node.size[3] / 2,
                    }))
                end
            end
        end
    end
    for _, child in ipairs(node.children) do includeGeometry(owner, child, bounds) end
end

-- Named groups define semantic object assemblies. Joined primitives inside one
-- assembly may overlap; positive-volume overlap between assemblies is invalid.
function Projection.semanticBounds(scene)
    local bounds = {}
    local function visit(node)
        if node.tag == "group" and node.attrs.name then
            local objectBounds = {name = node.attrs.name}
            includeGeometry(node, node, objectBounds)
            if objectBounds.min then bounds[#bounds + 1] = objectBounds end
        end
        for _, child in ipairs(node.children) do visit(child) end
    end
    visit(scene.root)
    table.sort(bounds, function(a, b) return a.name < b.name end)
    return bounds
end

function Projection.objectBounds(scene, name)
    for _, bounds in ipairs(Projection.semanticBounds(scene)) do
        if bounds.name == name then return bounds end
    end
    return nil, "semantic object has no box geometry: " .. tostring(name)
end

function Projection.distance(first, second)
    local gaps, squared = {}, 0
    for axis = 1, 3 do
        gaps[axis] = math.max(first.min[axis] - second.max[axis],
            second.min[axis] - first.max[axis], 0)
        squared = squared + gaps[axis] * gaps[axis]
    end
    return math.sqrt(squared), gaps
end

function Projection.roomBounds(scene)
    local result = {}
    for _, node in ipairs(scene.root.children) do
        if node.tag == "wall" then
            local length = number(node.attrs.length) * centimeters
            local height = number(node.attrs.height) * centimeters
            local thickness = number(node.attrs.thickness) * centimeters
            local bounds = {}
            for x = -1, 1, 2 do
                for y = -1, 1, 2 do
                    for z = 0, 1 do
                        includePoint(bounds, worldPoint(node,
                            {x * length / 2, y * thickness / 2, z * height}))
                    end
                end
            end
            local width, depth = bounds.max[1] - bounds.min[1], bounds.max[2] - bounds.min[2]
            if width < depth then
                if (bounds.min[1] + bounds.max[1]) / 2 < 0 then result.west = bounds.max[1]
                else result.east = bounds.min[1] end
            else
                if (bounds.min[2] + bounds.max[2]) / 2 < 5 then result.south = bounds.max[2]
                else result.north = bounds.min[2] end
            end
            result.floor = result.floor or bounds.min[3]
            result.ceiling = math.max(result.ceiling or 0, bounds.max[3])
        end
    end
    return result
end

function Projection.wallClearances(scene, bounds)
    local room = Projection.roomBounds(scene)
    return {
        west = bounds.min[1] - room.west,
        east = room.east - bounds.max[1],
        south = bounds.min[2] - room.south,
        north = room.north - bounds.max[2],
        floor = bounds.min[3] - room.floor,
        ceiling = room.ceiling - bounds.max[3],
    }
end

function Projection.intersections(scene, tolerance)
    tolerance = tolerance or 1e-6
    local bounds, hits = Projection.semanticBounds(scene), {}
    for first = 1, #bounds - 1 do
        for second = first + 1, #bounds do
            local overlap, intersects = {}, true
            for axis = 1, 3 do
                overlap[axis] = math.min(bounds[first].max[axis], bounds[second].max[axis])
                    - math.max(bounds[first].min[axis], bounds[second].min[axis])
                if overlap[axis] <= tolerance then intersects = false end
            end
            if intersects then
                hits[#hits + 1] = {first = bounds[first].name, second = bounds[second].name,
                    overlap = overlap}
            end
        end
    end
    return hits
end

-- Source dimensions must match the reference render used to paint the image.
-- The target uses centered UniformToFill cropping; x/y are viewport-local pixels.
function Projection.project(camera, point, sourceWidth, sourceHeight, viewWidth, viewHeight, near)
    if not camera or not point then return nil, "missing camera or anchor" end
    if sourceWidth <= 0 or sourceHeight <= 0 or viewWidth <= 0 or viewHeight <= 0 then
        return nil, "invalid image or viewport dimensions"
    end
    local forward = normalize({camera.look[1]-camera.pos[1], camera.look[2]-camera.pos[2], camera.look[3]-camera.pos[3]})
    local right = normalize(cross(forward, camera.up == "z" and {0, 0, 1} or {0, 1, 0}))
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
