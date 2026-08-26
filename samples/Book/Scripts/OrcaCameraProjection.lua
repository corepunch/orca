-- Adapter for the direct children of the exported, identity WorkshopCamera scene.
-- Reads the native SceneKit objects; the metadata copy is only an export fixture.
-- Arbitrarily parented/animated cameras require their world Matrix instead.
local Adapter = {}

local function vector(value)
    assert(value, "missing native transform vector")
    return {value.X, value.Y, value.Z}
end

function Adapter.anchor(node)
    assert(node, "missing native anchor node")
    return vector(node.RenderTransformTranslation)
end

function Adapter.camera(node)
    assert(node, "missing native camera node")
    assert(node.FovType == "Yfov" or node.FovType == 1, "expected vertical native FOV")
    assert(node.ProjectionType == "Perspective" or node.ProjectionType == 0, "expected perspective native camera")
    assert(not node.ZPositive or node.ZPositive == 0, "expected native -Z forward camera")
    local pos = Adapter.anchor(node)
    local rotation = vector(node.RenderTransformRotation)
    assert(math.abs(rotation[3]) < 1e-6, "rolled native cameras need a full basis projection")
    local pitch, yaw = rotation[1]*math.pi/180, rotation[2]*math.pi/180
    return {
        pos = pos,
        look = {
            pos[1] - math.sin(yaw)*math.cos(pitch),
            pos[2] + math.sin(pitch),
            pos[3] - math.cos(yaw)*math.cos(pitch),
        },
        fov = node.Fov,
        near = node.ZNear,
        far = node.ZFar,
    }
end

return Adapter
