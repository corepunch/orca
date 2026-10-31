-- Ordered photo albums share a compact square footprint. Each row fills the
-- width, so incomplete rows never leave an empty cell. A single photo keeps
-- the caller's aspect-ratio treatment.
local PhotoLayout = {}
local rows = { { 1 }, { 2 }, { 1, 2 }, { 2, 2 }, { 2, 3 },
    { 3, 3 }, { 2, 2, 3 }, { 2, 3, 3 }, { 3, 3, 3 } }

function PhotoLayout.arrange(count, width, gap, singleHeight)
    assert(type(count) == "number" and count % 1 == 0 and count >= 0 and count <= 9,
        "Photo albums contain at most nine images")
    local result = { height = 0, tiles = {} }
    if count == 0 then return result end
    gap = gap or 2
    local bands = rows[count]
    local height = count == 1 and (singleHeight or width) or width
    local bandHeight = (height - gap * (#bands - 1)) / #bands
    result.height = height
    for row, columns in ipairs(bands) do
        local tileWidth = (width - gap * (columns - 1)) / columns
        for column = 1, columns do
            result.tiles[#result.tiles + 1] = {
                x = (column - 1) * (tileWidth + gap), y = (row - 1) * (bandHeight + gap),
                width = tileWidth, height = bandHeight,
                topLeft = row == 1 and column == 1,
                topRight = row == 1 and column == columns,
                bottomLeft = row == #bands and column == 1,
                bottomRight = row == #bands and column == columns,
            }
        end
    end
    return result
end

return PhotoLayout
