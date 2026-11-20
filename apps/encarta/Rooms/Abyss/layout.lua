-- Written by Tools/build_rooms.py from the models: each exhibit's
-- centre and size, and each drawer front's left, top, right and
-- bottom, as fractions of the camera's front panel.
return {
    subjects = {
        space = { 0.1970, 0.1525, 0.1897, 0.2714 },
        animals = { 0.5000, 0.2527, 0.3547, 0.4558 },
        ["your-body"] = { 0.0684, 0.3820, 0.0930, 0.1909 },
        earth = { 0.2165, 0.4579, 0.1311, 0.2292 },
        plants = { 0.3615, 0.5442, 0.1033, 0.1256 },
        science = { 0.7717, 0.4872, 0.1783, 0.2668 },
        ["long-ago"] = { 0.9226, 0.4646, 0.1190, 0.1594 },
    },
    drawers = {
        OpenPassport = { 0.0260, 0.6830, 0.4910, 0.8100 },
        OpenHunt = { 0.5140, 0.6830, 0.9730, 0.8100 },
        DeskQuestions = { 0.0260, 0.8320, 0.4910, 0.9650 },
        DeskBookshelf = { 0.5140, 0.8320, 0.9730, 0.9650 },
    },
}
