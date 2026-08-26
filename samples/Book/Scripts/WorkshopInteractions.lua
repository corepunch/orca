-- Camera and rendered image states only. Scene object names identify ZIL objects.
local M = {
    scene_path = "Book/Images/prototype/workshop.blks",
    camera = "WorkshopEstablishing",
    source_width = 1536,
    source_height = 1024,
    present_asset = "Book/Images/prototype/workshop-present.png",
    taken_asset = "Book/Images/prototype/workshop-taken.png",
}

function M.asset(env)
    return env.INQ(env.OIL_CAN, env.WORKSHOP_FLOOR) and M.present_asset or M.taken_asset
end

return M
