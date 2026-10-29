-- Keeps a scrolling list's place across membership changes of the
-- collection it presents. A reader at the end follows new rows; history
-- added before the rows does not move what is on screen; otherwise the first
-- visible row stays where it was. Rows are laid out with their bound content
-- before the position is chosen, so measured heights are final.
local core = require "orca.core"
local Scroll = {}

-- A node's top within the content of `ancestor`.
local function top(node, ancestor)
    local y = 0
    while node and node ~= ancestor do
        y = y + (node.ActualY or 0)
        node = node:getParent()
    end
    return y
end

local function last(list) return math.min(0, (list.ActualHeight or 0) - (list.ScrollHeight or 0)) end

local function root(node)
    while node:getParent() do node = node:getParent() end
    return node
end

-- list: the scrolling node; rows: the items control whose children are the
-- rows; collection: the query's items. options.tolerance (points from the
-- end that still count as at the end, 8 by default). Returns a function
-- that stops keeping the place.
function Scroll.keep(list, rows, collection, options)
    options = options or {}
    local tolerance = options.tolerance or 8
    local captured
    return collection:watch {
        before = function()
            captured = nil
            if (list.ActualHeight or 0) <= 0 then return end
            local y = list.ContentOffset.Y
            captured = { y = y, atEnd = math.abs(y - last(list)) <= tolerance }
            -- Rows scroll under the header: the first visible one is below its inset.
            local visibleTop = list.AdjustedContentInsetTop or 0
            local child = rows:getFirstChild()
            while child do
                local at = top(child, list)
                if at + (child.ActualHeight or 0) + y > visibleTop then
                    captured.anchor, captured.anchorY = child, at + y
                    break
                end
                child = child:getNext()
            end
        end,
        after = function(_, change)
            local state = captured
            captured = nil
            -- A list that left the screen has no place to keep.
            local screen = root(list)
            if screen:getClassName() ~= "Screen" then return end
            -- Bindings run once per frame; changed fields need a new one.
            core.advanceFrame()
            core.runAllPrograms(list)
            screen:UpdateLayout(screen.Width, screen.Height)
            local y
            if not state or (state.atEnd and not change.prepended) then y = last(list)
            elseif state.anchor and state.anchor:getParent() == rows then y = state.anchorY - top(state.anchor, list)
            else y = state.y end
            list.ContentOffset = { X = list.ContentOffset.X, Y = math.max(last(list), math.min(0, y)) }
        end,
    }
end

return Scroll
