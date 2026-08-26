-- Run from libs/zilscript: lua ../../Tests/test_workshop_prototype.lua
package.path = "../../../?.lua;" .. package.path
package.zilpath = "../../../?.zil;?.zil"
local Session = require "Book.Scripts.WorkshopSession"
local interactions = require "Book.Scripts.WorkshopInteractions"
local session = Session.new()
local env = session.env
assert(env.SUGGEST_ACTIONS == nil, "Companion authoring must not be loaded")
assert(env.HERE == env.WORKSHOP_FLOOR)
assert(session:target("KEY-HOOK").verb == "examine")
assert(session:target("OIL_CAN").verb == "take", "Lua/ZIL symbol spellings should match")
assert(not session:target("MOVES"), "Numeric globals are not object declarations")
assert(not session:target("main_bench"), "Unmatched scene decorations have no circle")
assert(session:target("SWEEP-BROOM"), "Other declared objects need no target-list entry")
env.FSET(env.PET_DOOR, env.INVISIBLE)
assert(not session:target("PET-DOOR"), "Invisible objects must have no circle")
env.FCLEAR(env.PET_DOOR, env.INVISIBLE)
env.MOVE(env.OIL_CAN, env.WORKBENCH)
assert(session:target("OIL-CAN"), "Objects on a surface remain accessible")
env.FCLEAR(env.WORKBENCH, env.SURFACEBIT)
env.FCLEAR(env.WORKBENCH, env.OPENBIT)
assert(not session:target("OIL-CAN"), "Closed containers hide contained targets")
env.FSET(env.WORKBENCH, env.SURFACEBIT)
env.FSET(env.WORKBENCH, env.OPENBIT)
env.MOVE(env.OIL_CAN, env.WORKSHOP_FLOOR)
assert(session:description():find("Grandfather Tolliver", 1, true))
local moves = env.MOVES
session:description()
assert(env.MOVES == moves, "Reading scene text must not advance turns")
local descriptions = {
    ["KEY-HOOK"] = "The brass key hook on the wall is empty",
    ["PET-DOOR"] = "A small pet door is cut into the bottom",
    ["CLOCK-FACE"] = "An old cuckoo clock hangs on the workshop wall",
}
for _, id in ipairs({"KEY-HOOK", "PET-DOOR", "CLOCK-FACE"}) do
    local beat = assert(session:activate(id), id)
    assert(beat.output:find(descriptions[id], 1, true), id .. ": " .. beat.output)
    assert(session:activate("OIL-CAN") == nil, "Action frame must block a second tap")
    session:continue()
end
assert(interactions.asset(env) == interactions.present_asset)
local beat = assert(session:activate("OIL-CAN"))
assert(beat.action == "OIL-CAN.take")
assert(env.INQ(env.OIL_CAN, env.WINNER), beat.output)
assert(interactions.asset(env) == interactions.taken_asset)
moves = env.MOVES
session:continue()
assert(env.MOVES == moves, "Continue must not advance turns")
assert(session:activate("OIL-CAN") == nil, "Collected items cannot be taken again")
assert(session:activate("not-an-object") == nil)
assert(session:activate("PET-DOOR"), "Permanent scenery remains interactive")
assert(env.HERE == env.WORKSHOP_FLOOR, "Prototype must stay inside covered room")
print("PASS: workshop prototype state, parser actions, Continue and no companion")
