-- Presentation + focus manifest for the Wondertown workshop chapter.
--
-- ZIL owns world state and result prose. This manifest maps stable ZIL names to
-- Scener cameras, to the local choices a focused subject offers, and to
-- the action art that owns a story beat. It never decides world logic: every
-- choice submits an ordinary parser command that ZIL is free to accept, refuse,
-- or reshape according to current state.
--
-- Images are direct Scener renders, one per camera, produced by
-- `make render ROOM=workshop`.

local render_base, render_ext = "Book/Rooms/render/workshop/", ".jpg"

local M = {}

function M.image(camera)
    return render_base .. camera .. render_ext
end

-- Room establishing views. `camera` frames the room; `subjects` are the ordered
-- ZIL objects offered as affordances. `node` names the scene anchor when the
-- blockout labels that geometry with a decoration name instead of the ZIL name;
-- subjects without a resolvable anchor are offered as text affordances instead of
-- projected hotspots. `exits` are room-level movements shown as page choices.
M.rooms = {
    ["WORKSHOP-FLOOR"] = {
        camera = "WorkshopEstablishing",
        subjects = {
            {name = "KEY-HOOK"},
            {name = "WORKBENCH", node = "main_bench"},
            {name = "OIL-CAN", art = "OilCanCloseup"},
            {name = "LOFT-LADDER"},
            {name = "CLOCK-FACE"},
            {name = "PET-DOOR"},
        },
        exits = {
            {label = "Cross to the tool bench", command = "east"},
        },
    },
    ["WORKBENCH-TOP"] = {
        camera = "WorkbenchTopEstablishing",
        subjects = {
            {name = "REPAIR-BOOK"},
            {name = "HALF-FINISHED-TOYS"},
        },
        exits = {
            {label = "Climb back down to the floor", command = "down"},
        },
    },
    ["TOOL-BENCH"] = {
        camera = "ToolBenchEstablishing",
        subjects = {
            {name = "BERTRAND", node = "counter_chair"},
            {name = "MAKESHIFT-STEPS"},
            {name = "TOOL-RACK"},
            {name = "VARNISH-POT"},
        },
        exits = {
            {label = "Return to the workshop floor", command = "west"},
        },
    },
    ["COUNTERTOP"] = {
        camera = "CountertopEstablishing",
        subjects = {
            {name = "MARZIPAN"},
            {name = "DISPLAY-CASE"},
            {name = "SHOP-WINDOW"},
        },
        exits = {
            {label = "Climb back down to the tool bench", command = "down"},
        },
    },
    -- Reachable but unmodelled; keep a camera so the page still renders.
    ["STORAGE-LOFT"] = {camera = "WorkbenchTopEstablishing", subjects = {}, exits = {
        {label = "Climb back down", command = "down"},
    }},
}

-- Focus definitions keyed by canonical ZIL object name. `choices(env)` returns an
-- ordered list of {label, command[, art]}; when a choice names an `art` camera the
-- host holds that screenshot full-frame with the result prose and a single
-- Continue, otherwise the result quietly refreshes the focused page. The host adds
-- the `exit` affordance automatically so leaving a subject always reads in-world.
M.focuses = {
    ["KEY-HOOK"] = {
        camera = "EmptyHookReveal",
        exit = "Step back into the workshop",
        choices = function()
            return {
                {label = "Examine the empty hook", command = "examine hook"},
                {label = "Examine the frayed string", command = "examine string"},
                {label = "Listen for the ticking", command = "listen to clock"},
            }
        end,
    },
    ["WORKBENCH"] = {
        camera = "ClimbWorkbenchAction",
        exit = "Step back from the bench",
        choices = function()
            return {
                {label = "Examine the towering bench", command = "examine workbench"},
                {label = "Look underneath the bench", command = "look under workbench"},
                {label = "Climb the workbench leg", command = "climb workbench",
                 art = "ClimbWorkbenchAction"},
            }
        end,
    },
    ["LOFT-LADDER"] = {
        camera = "LoftLadderCloseup",
        exit = "Step back from the ladder",
        choices = function(env)
            local choices = {
                {label = "Examine the folding ladder", command = "examine ladder"},
                {label = "Examine the lifting mechanism", command = "examine mechanism"},
            }
            if env.LADDER_OILED then
                choices[#choices + 1] = {label = "Climb to the storage loft", command = "up"}
            else
                choices[#choices + 1] = {label = "Lubricate the rusty mechanism",
                    command = "lubricate mechanism", art = "LoftLadderCloseup"}
            end
            return choices
        end,
    },
    ["REPAIR-BOOK"] = {
        camera = "RepairBookCloseup",
        exit = "Step back from the book",
        choices = function(env)
            if env.REPAIR_BOOK_OPEN then
                return {
                    {label = "Read Tolliver's clue", command = "read book"},
                    {label = "Close the heavy cover", command = "close book",
                     art = "WorkbenchTopEstablishing"},
                }
            end
            return {
                {label = "Examine the leather book", command = "examine book"},
                {label = "Heave the cover open", command = "open book",
                 art = "RepairBookCloseup"},
            }
        end,
    },
    ["BERTRAND"] = {
        camera = "WindBertrandAction",
        exit = "Step away from Bertrand",
        choices = function(env)
            local choices = {{label = "Examine Captain Bertrand", command = "examine nutcracker"}}
            if env.INQ(env.BERTRAND_KEY, env.TOOL_BENCH) then
                choices[#choices + 1] = {label = "Examine the winding key", command = "examine winding key"}
                choices[#choices + 1] = {label = "Take the winding key", command = "take winding key"}
            end
            if not env.BERTRAND_WOUND then
                choices[#choices + 1] = {label = "Wind Bertrand", command = "wind nutcracker",
                    art = "WindBertrandAction"}
            end
            return choices
        end,
    },
    ["MAKESHIFT-STEPS"] = {
        camera = "MakeshiftClimbAction",
        exit = "Step back",
        choices = function()
            return {
                {label = "Examine the climbing route", command = "examine steps"},
                {label = "Climb to the countertop", command = "climb steps",
                 art = "MakeshiftClimbAction"},
            }
        end,
    },
    ["DISPLAY-CASE"] = {
        camera = "CountertopEstablishing",
        exit = "Step back from the case",
        choices = function(env)
            if env.FSETQ(env.DISPLAY_CASE, env.OPENBIT) then
                local choices = {{label = "Examine the open case", command = "examine case"}}
                if env.INQ(env.TIN_SOLDIER, env.DISPLAY_CASE) then
                    choices[#choices + 1] = {label = "Take the tin soldier", command = "take soldier"}
                end
                if env.INQ(env.MUSIC_BOX, env.DISPLAY_CASE) then
                    choices[#choices + 1] = {label = "Take the music box", command = "take music box"}
                end
                return choices
            end
            return {
                {label = "Examine the dusty case", command = "examine case"},
                {label = "Open the glass case", command = "open case"},
            }
        end,
    },
    ["MARZIPAN"] = {
        camera = "CountertopEstablishing",
        exit = "Give her some space",
        choices = function()
            return {
                {label = "Examine the rag doll", command = "examine doll"},
                {label = "Ask Marzipan about the key", command = "ask doll about key"},
            }
        end,
    },
}

return M
