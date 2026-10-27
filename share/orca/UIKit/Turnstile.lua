-- Windows Phone page navigation. The navigation owner turns the page it shows
-- in on a hinge at the left edge of the screen: "ForwardIn" for a page it goes
-- into, "BackwardIn" for the page it returns to. The page is the screen's
-- visible Pivot, found depth-first so tab hosts resolve to their active page.
local Turnstile = {}

function Turnstile.page(node)
    if not node or node.Visible == false then return nil end
    if node.className == "Pivot" then return node end
    local child = node:getFirstChild()
    while child do
        local page = Turnstile.page(child)
        if page then return page end
        child = child:getNext()
    end
end

function Turnstile.play(screen, mode)
    local page = Turnstile.page(screen)
    if page then page.Turnstile = mode end
    return page
end

return Turnstile
