-- Run from libs/zilscript: lua ../../Tests/test_workshop_prototype.lua
package.path = "../../../?.lua;" .. package.path
package.zilpath = "../../../?.zil;?.zil"
local Session = require "Book.Scripts.WorkshopSession"
local interactions = require "Book.Scripts.WorkshopInteractions"
local session = Session.new()
local env = session.env
assert(env.SUGGEST_ACTIONS == nil, "Companion authoring must not be loaded")
assert(env.HERE == env.WORKSHOP_FLOOR)
assert(session:description():find("Grandfather Tolliver", 1, true))
local moves = env.MOVES
session:description()
assert(env.MOVES == moves, "Reading scene text must not advance turns")
for _, id in ipairs({"hook", "door", "clock"}) do
    local beat = assert(session:activate(id), id)
    assert(#beat.output > 0)
    assert(session:activate("oil") == nil, "Action frame must block a second tap")
    session:continue()
end
assert(interactions.asset(env) == interactions.present_asset)
local beat = assert(session:activate("oil"))
assert(beat.action == "workshop-floor.take-oil-can")
assert(env.INQ(env.OIL_CAN, env.WINNER), beat.output)
assert(interactions.asset(env) == interactions.taken_asset)
moves = env.MOVES
session:continue()
assert(env.MOVES == moves, "Continue must not advance turns")
assert(session:activate("oil") == nil, "Collected items cannot be taken again")
assert(session:activate("not-an-object") == nil)
assert(session:activate("door"), "Permanent scenery remains interactive")
assert(env.HERE == env.WORKSHOP_FLOOR, "Prototype must stay inside covered room")
print("PASS: workshop prototype state, parser actions, Continue and no companion")
