-- Run from libs/zilscript: lua ../../Tests/test_workshop_prototype.lua
--
-- Walks the whole workshop chapter through the host Session brain: establishing
-- pages, focus narrowing, action beats, and room transitions. ZIL stays
-- authoritative; the host only submits parser commands and frames the result.
package.path = "../../../?.lua;" .. package.path
package.zilpath = "../../../?.zil;?.zil"
local Session = require "Book.Scripts.WorkshopSession"
local session = Session.new()
local env = session.env

local function has(text, fragment) return text and text:find(fragment, 1, true) end
local function subject(view, name)
    for _, entry in ipairs(view.subjects or {}) do if entry.name == name then return entry end end
end
local function labelled(view, fragment)
    for index, choice in ipairs(view.choices or {}) do
        if choice.label:find(fragment, 1, true) then return index, choice end
    end
end

-- Companion authoring stays out of the Book prototype.
assert(env.SUGGEST_ACTIONS == nil, "Companion authoring must not be loaded")

-- === WORKSHOP FLOOR: establishing page ===
local view = session:view()
assert(view.kind == "room" and session:room() == "WORKSHOP-FLOOR")
assert(view.camera == "workshop-floor")
assert(view.image:find("Rooms/render/workshop/workshop-floor.jpg", 1, true))
assert(has(view.text, "Grandfather Tolliver"))
assert(subject(view, "KEY-HOOK").kind == "focus", "hook opens a focus")
assert(subject(view, "WORKBENCH").kind == "focus" and subject(view, "WORKBENCH").node == "WORKBENCH")
assert(subject(view, "OIL-CAN").kind == "take", "portable oil can defaults to take")
assert(subject(view, "LOFT-LADDER").kind == "focus")
assert(subject(view, "PET-DOOR").kind == "examine", "plain scenery examines")
assert(#view.exits == 1 and view.exits[1].command == "east")

-- === FOCUS: the empty key hook narrows the action space ===
view = session:tap("KEY-HOOK")
assert(view.kind == "focus" and view.subject == "KEY-HOOK")
assert(view.camera == "key-hook")
assert(has(view.text, "empty"), "focus opens on the hook's examine prose: " .. view.text)
assert(#view.choices >= 2 and view.exit:find("Step back", 1, true))
view = session:leave_focus()
assert(view.kind == "room", "leaving focus returns to the establishing page")

-- === TAKE beat: the oil can gets its own frame, then leaves the scene ===
view = session:tap("OIL-CAN")
assert(view.kind == "beat" and view.continue and view.camera == "oil-can")
assert(env.INQ(env.OIL_CAN, env.WINNER), "oil can is taken: " .. view.text)
assert(session:tap("KEY-HOOK") == nil, "a held beat blocks other interaction")
view = session:continue()
assert(view.kind == "room" and not subject(view, "OIL-CAN"), "collected oil can leaves the page")

-- === LOFT LADDER: oil the mechanism and climb to storage ===
view = session:tap("LOFT-LADDER")
assert(view.kind == "focus" and view.camera == "loft-ladder")
local lubricate = assert(labelled(view, "Lubricate the rusty mechanism"))
view = session:choose(lubricate)
assert(view.kind == "beat" and view.camera == "loft-ladder-lubricate")
assert(env.LADDER_OILED, "oil can frees the ladder mechanism: " .. view.text)
view = session:continue()
local loft = assert(labelled(view, "Climb to the storage loft"))
view = session:choose(loft)
assert(view.kind == "beat" and session:room() == "STORAGE-LOFT")
view = session:continue()
assert(view.kind == "room" and session:room() == "STORAGE-LOFT")
view = session:exit("down")
assert(view.kind == "room" and session:room() == "WORKSHOP-FLOOR")

-- === ACTION into a new scene: climb the bench ===
view = session:tap("WORKBENCH")
assert(view.kind == "focus")
local climb = assert(labelled(view, "Climb the workbench"))
view = session:choose(climb)
assert(view.kind == "beat" and view.camera == "workbench-climb")
assert(has(view.text, "tabletop") or has(view.text, "swings over"), "climb prose: " .. view.text)
view = session:continue()
assert(view.kind == "room" and session:room() == "WORKBENCH-TOP")
assert(view.camera == "workbench-top")

-- === Repair book: reveal, blocked descent, then close ===
view = session:tap("REPAIR-BOOK")
local open = assert(labelled(view, "Heave the cover open"))
view = session:choose(open)
assert(view.kind == "beat" and view.camera == "repair-book-open")
assert(env.REPAIR_BOOK_OPEN, "book state advances")
view = session:continue()
assert(view.kind == "focus" and labelled(view, "Read Tolliver"), "open book offers new choices")
view = session:leave_focus()
view = session:exit("down")
assert(view.kind == "beat" and not (session:room() == "WORKSHOP-FLOOR"), "open book blocks the climb down")
assert(has(view.text, "closed first") or has(view.text, "cannot leave"), view.text)
view = session:continue()
assert(session:room() == "WORKBENCH-TOP")
view = session:tap("REPAIR-BOOK")
local close = assert(labelled(view, "Close the heavy cover"))
view = session:choose(close)
view = session:continue()
assert(not env.REPAIR_BOOK_OPEN)
session:leave_focus()
view = session:exit("down")
assert(view.kind == "room" and session:room() == "WORKSHOP-FLOOR", "closed book allows the descent")

-- === Cross to the tool bench and wind Captain Bertrand ===
view = session:exit("east")
assert(view.kind == "room" and session:room() == "TOOL-BENCH")
assert(view.camera == "tool-bench" and subject(view, "BERTRAND").node == "BERTRAND")
view = session:tap("BERTRAND")
assert(labelled(view, "Wind Bertrand"), "winding offered before the key is taken")
local takekey = assert(labelled(view, "Take the winding key"))
view = session:choose(takekey)
assert(env.INQ(env.BERTRAND_KEY, env.WINNER), "the winding key is taken")
local wind = assert(labelled(view, "Wind Bertrand"))
view = session:choose(wind)
assert(view.kind == "beat" and view.camera == "bertrand-wind")
assert(env.BERTRAND_WOUND, "Bertrand awakens: " .. view.text)
view = session:continue()
assert(view.kind == "focus" and not labelled(view, "Wind Bertrand"), "a wound Bertrand drops the wind choice")
session:leave_focus()

-- === Makeshift climb up to the countertop ===
view = session:tap("MAKESHIFT-STEPS")
local up = assert(labelled(view, "Climb to the countertop"))
view = session:choose(up)
assert(view.kind == "beat" and view.camera == "makeshift-steps-climb")
view = session:continue()
assert(view.kind == "room" and session:room() == "COUNTERTOP")
assert(subject(view, "MARZIPAN").node == "MARZIPAN")
assert(subject(view, "DISPLAY-CASE").node == "DISPLAY-CASE")
assert(subject(view, "SHOP-WINDOW").node == "SHOP-WINDOW")

-- === Countertop projected subject: open the display case ===
view = session:tap("DISPLAY-CASE")
assert(view.kind == "focus" and view.subject == "DISPLAY-CASE")
local opencase = assert(labelled(view, "Open the glass case"))
view = session:choose(opencase)
assert(env.FSETQ(env.DISPLAY_CASE, env.OPENBIT), "the case opens")
assert(labelled(view, "Take the tin soldier"), "an open case reveals its treasures")

print("PASS: workshop chapter — establishing, focus, beats and room transitions")
