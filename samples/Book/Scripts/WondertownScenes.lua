-- Host-side mapping from Wondertown's stable companion scene keys to art.
-- ZIL owns narrative state; this manifest owns presentation assets.
return {
    actions = {
        ["workshop-floor.examine-hook"] = {
            asset = "Book/libs/zilscript/books/wondertown/illustrations/workshop-floor-initial-action.png",
            function_ = "reaction",
        },
        ["workshop-floor.climb-workbench"] = {
            asset = "Book/libs/zilscript/books/wondertown/illustrations/action-climb-workbench.png",
            function_ = "action",
        },
        ["workshop-floor.take-oil-can"] = {
            asset = "Book/libs/zilscript/books/wondertown/illustrations/action-take-oil-can.png",
            function_ = "close-up-action",
        },
        ["workshop-floor.oil-ladder"] = {
            asset = "Book/libs/zilscript/books/wondertown/illustrations/action-oil-spool-stairs.png",
            function_ = "action-consequence",
        },
        ["workshop-floor.climb-loft"] = {
            asset = "Book/libs/zilscript/books/wondertown/illustrations/action-climb-spool-stairs.png",
            function_ = "travel",
        },
        ["workshop-floor.go-toolbench"] = {
            asset = "Book/Images/Generated image 1.png",
            function_ = "travel",
        },
        ["tool-bench.wind-bertrand"] = {
            asset = "Book/libs/zilscript/books/wondertown/illustrations/action-wind-bertrand.png",
            function_ = "action-reaction",
        },
        ["tool-bench.climb-countertop"] = {
            asset = "Book/libs/zilscript/books/wondertown/illustrations/action-climb-spool-stairs.png",
            function_ = "travel",
        },
        ["workbench-top.open-repair-book"] = {
            asset = "Book/libs/zilscript/books/wondertown/illustrations/action-open-repair-book.png",
            function_ = "reveal",
        },
        ["workbench-top.close-repair-book"] = {
            asset = "Book/libs/zilscript/books/wondertown/illustrations/workbench-top-closed.png",
            function_ = "action-consequence",
        },
    },

    default = {
        asset = "Book/Images/room-1.png",
        function_ = "establishing",
    },

    ["workshop.floor-initial"] = {
        asset = "Book/Images/room-1.png",
        function_ = "establishing",
        alt = "Grandfather Tolliver's workshop, with the workbench, key hook, clock, and pet door visible.",
    },
    ["workshop.floor-oiled"] = {
        asset = "Book/Images/Generated image 3.png",
        function_ = "establishing-consequence",
        alt = "The workshop opens upward around the newly raised folding loft ladder.",
    },
    ["workshop.floor-study-open"] = {
        asset = "Book/Images/Generated image 3.png",
        function_ = "reveal",
        alt = "The workshop opens upward around Pip as the hidden route beyond the clock is revealed.",
    },
    ["workshop.floor-endgame"] = {
        asset = "Book/Images/room-1.png",
        function_ = "consequence",
    },

    ["toolbench.bertrand-frozen"] = {
        asset = "Book/Images/Generated image 1.png",
        function_ = "establishing",
    },
    ["workbench.top-closed"] = {
        asset = "Book/libs/zilscript/books/wondertown/illustrations/workbench-top-closed.png",
        function_ = "establishing",
        alt = "The workbench top, with Tolliver's closed illustrated repair book among half-finished toys.",
    },
    ["workbench.top-open"] = {
        asset = "Book/libs/zilscript/books/wondertown/illustrations/action-open-repair-book.png",
        function_ = "establishing-consequence",
        alt = "Tolliver's open repair book, its paper workshop rising around Pip.",
    },
    ["toolbench.bertrand-wound"] = {
        asset = "Book/Images/Generated image 1.png",
        function_ = "establishing-consequence",
    },
    ["countertop.initial"] = {
        asset = "Book/libs/zilscript/books/wondertown/images/countertop.png",
        function_ = "establishing",
    },
    ["countertop.marzipan-happy"] = {
        asset = "Book/libs/zilscript/books/wondertown/images/countertop.png",
        function_ = "reaction",
    },
}
