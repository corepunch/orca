-- Written by Tools/build_rooms.py from the models: each exhibit's
-- centre and size, and each drawer front's left, top, right and
-- bottom, as fractions of the camera's front panel.
return {
    subjects = {
        space = { 0.1204, 0.0843, 0.1898, 0.1459 },
        animals = { 0.2750, 0.5303, 0.1967, 0.1809 },
        ["your-body"] = { 0.3484, 0.1931, 0.1192, 0.1636 },
        earth = { 0.0877, 0.5558, 0.1104, 0.3855 },
        plants = { 0.8546, 0.2460, 0.2583, 0.2917 },
        science = { 0.7423, 0.5692, 0.1042, 0.3609 },
        ["long-ago"] = { 0.5516, 0.3197, 0.2200, 0.1145 },
    },
    drawers = {
        OpenPassport = { 0.0414, 0.7845, 0.2666, 0.9475 },
        OpenHunt = { 0.2756, 0.7845, 0.4952, 0.9475 },
        DeskQuestions = { 0.5028, 0.7845, 0.7244, 0.9475 },
        DeskBookshelf = { 0.7334, 0.7845, 0.9579, 0.9475 },
    },
}
