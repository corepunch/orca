-- Narrative manifest for the Wondertown workshop chapter.
--
-- ZIL owns world state and response prose. This manifest owns the narrative
-- layer: which subjects appear on each room page, in what order, and what
-- actions a focused subject offers. It knows nothing about cameras or images;
-- the visual binding is handled by naming convention (see ILLUSTRATING_ADVENTURES.md):
-- room camera = ZIL room name lowercased, subject camera = ZIL object name
-- lowercased, action beat camera = {subject}-{verb} lowercased.
--
-- `beat = true` on a choice means the host holds the ZIL response in a
-- full-frame beat page. Without it the response quietly refreshes the focus.

local M = {}

-- Room pages. `subjects` lists ZIL object names in display order; the runtime
-- filters to those currently reachable. `exits` are room-level movements.
M.rooms = {
    ["WORKSHOP-FLOOR"] = {
        subjects = {"KEY-HOOK", "WORKBENCH", "OIL-CAN", "LOFT-LADDER", "CLOCK-FACE", "PET-DOOR"},
        exits = {
            {label = "Cross to the tool bench", command = "east"},
        },
    },
    ["WORKBENCH-TOP"] = {
        subjects = {"REPAIR-BOOK", "HALF-FINISHED-TOYS"},
        exits = {
            {label = "Climb back down to the floor", command = "down"},
        },
    },
    ["TOOL-BENCH"] = {
        subjects = {"BERTRAND", "MAKESHIFT-STEPS", "TOOL-RACK", "VARNISH-POT"},
        exits = {
            {label = "Return to the workshop floor", command = "west"},
        },
    },
    ["COUNTERTOP"] = {
        subjects = {"MARZIPAN", "DISPLAY-CASE", "SHOP-WINDOW"},
        exits = {
            {label = "Climb back down to the tool bench", command = "down"},
        },
    },
    ["STORAGE-LOFT"] = {
        subjects = {},
        exits = {{label = "Climb back down", command = "down"}},
    },
}

-- Focus definitions. `choices(env, session)` returns an ordered list of
-- {label, command[, beat]}. `exit` is the label for the leave-focus affordance.
M.focuses = {
    ["KEY-HOOK"] = {
        exit = "Step back into the workshop",
        choices = function()
            return {
                {label = "Examine the empty hook",    command = "examine hook"},
                {label = "Examine the frayed string", command = "examine string"},
                {label = "Listen for the ticking",    command = "listen to clock"},
            }
        end,
    },
    ["WORKBENCH"] = {
        exit = "Step back from the bench",
        choices = function()
            return {
                {label = "Examine the towering bench", command = "examine workbench"},
                {label = "Look underneath the bench",  command = "look under workbench"},
                {label = "Climb the workbench leg",    command = "climb workbench", beat = true},
            }
        end,
    },
    ["LOFT-LADDER"] = {
        exit = "Step back from the ladder",
        choices = function(env)
            local choices = {
                {label = "Examine the folding ladder",    command = "examine ladder"},
                {label = "Examine the lifting mechanism", command = "examine mechanism"},
            }
            if env.LADDER_OILED then
                choices[#choices + 1] = {label = "Climb to the storage loft", command = "up"}
            else
                choices[#choices + 1] = {label = "Lubricate the rusty mechanism",
                    command = "lubricate mechanism", beat = true}
            end
            return choices
        end,
    },
    ["REPAIR-BOOK"] = {
        exit = "Step back from the book",
        choices = function(env)
            if env.REPAIR_BOOK_OPEN then
                return {
                    {label = "Read Tolliver's clue",    command = "read book"},
                    {label = "Close the heavy cover",   command = "close book", beat = true},
                }
            end
            return {
                {label = "Examine the leather book", command = "examine book"},
                {label = "Heave the cover open",     command = "open book", beat = true},
            }
        end,
    },
    ["BERTRAND"] = {
        exit = "Step away from Bertrand",
        choices = function(env)
            local choices = {{label = "Examine Captain Bertrand", command = "examine nutcracker"}}
            if env.INQ(env.BERTRAND_KEY, env.TOOL_BENCH) then
                choices[#choices + 1] = {label = "Examine the winding key", command = "examine winding key"}
                choices[#choices + 1] = {label = "Take the winding key",    command = "take winding key"}
            end
            if not env.BERTRAND_WOUND then
                choices[#choices + 1] = {label = "Wind Bertrand", command = "wind nutcracker", beat = true}
            end
            return choices
        end,
    },
    ["MAKESHIFT-STEPS"] = {
        exit = "Step back",
        choices = function()
            return {
                {label = "Examine the climbing route", command = "examine steps"},
                {label = "Climb to the countertop",    command = "climb steps", beat = true},
            }
        end,
    },
    ["DISPLAY-CASE"] = {
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
                {label = "Open the glass case",    command = "open case"},
            }
        end,
    },
    ["MARZIPAN"] = {
        exit = "Give her some space",
        choices = function()
            return {
                {label = "Examine the rag doll",          command = "examine doll"},
                {label = "Ask Marzipan about the key",    command = "ask doll about key"},
            }
        end,
    },
}

return M
