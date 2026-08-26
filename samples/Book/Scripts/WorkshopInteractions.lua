-- Local offsets select a visible part of each imported prefab, in scene units.
-- The floor-only slice deliberately inspects the door rather than leaving it.
local M = {
    scene_path = "Book/Images/prototype/workshop.blks",
    camera = "WorkshopEstablishing",
    source_width = 1536,
    source_height = 1024,
    present_asset = "Book/Images/prototype/workshop-present.png",
    taken_asset = "Book/Images/prototype/workshop-taken.png",
    targets = {
        { id = "hook", anchor = "fixtures/key_hook", offset = {0, 0, 0},
          command = "examine hook", action = "workshop-floor.examine-hook" },
        { id = "oil", anchor = "items/oil_can", offset = {0, 0.14, 0},
          command = "take oil can", action = "workshop-floor.take-oil-can", takeable = "OIL_CAN" },
        { id = "door", anchor = "architecture/workshop_door", offset = {0, 0.30, 0.09},
          command = "examine pet door" },
        { id = "clock", anchor = "fixtures/wall_clock", offset = {0, 0, 0.08},
          command = "examine clock" },
    },
}

function M.available(env, target)
    if env.HERE ~= env.WORKSHOP_FLOOR then return false end
    return not target.takeable or env.INQ(env[target.takeable], env.HERE)
end

function M.asset(env)
    return env.INQ(env.OIL_CAN, env.WORKSHOP_FLOOR) and M.present_asset or M.taken_asset
end

return M
