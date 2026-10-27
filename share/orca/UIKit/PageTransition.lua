-- Page navigation as on Windows Phone. The navigation owner plays the page
-- it leaves out and then the page it shows in, each with that page's own
-- NavigationTransition. `direction` is "Forward" going into a page and
-- "Backward" coming back out of one. A page is the screen's visible Pivot,
-- found depth-first so tab hosts resolve to their active page.
local PageTransition = {}

function PageTransition.page(node)
    if not node or node.Visible == false then return nil end
    if node.className == "Pivot" then return node end
    local child = node:getFirstChild()
    while child do
        local page = PageTransition.page(child)
        if page then return page end
        child = child:getNext()
    end
end

-- Plays the screen's page out, then calls `done`. Returns false, without
-- calling it, when there is no direction or no page to play.
function PageTransition.leave(screen, direction, done)
    local page = direction and PageTransition.page(screen)
    if not page then return false end
    page.TransitionCompleted = done
    -- Restart a page that is still moving or holds a finished exit.
    page.TransitionMode = "None"
    page.TransitionMode = direction .. "Out"
    return true
end

-- Plays the screen's page in. Without a direction, a page held away by an
-- earlier leave returns at once, as when a retained tab is shown again.
function PageTransition.enter(screen, direction)
    local page = PageTransition.page(screen)
    if page then page.TransitionMode = direction and direction .. "In" or "None" end
    return page
end

return PageTransition
