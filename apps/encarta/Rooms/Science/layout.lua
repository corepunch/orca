-- Written by Tools/build_rooms.py from the models: each exhibit's
-- centre and size, and each drawer front's left, top, right and
-- bottom, as fractions of the camera's front panel.
return {
    subjects = {
        space = { 0.2274, 0.1993, 0.1272, 0.2001 },
        animals = { 0.8203, 0.1297, 0.1339, 0.1540 },
        ["your-body"] = { 0.2430, 0.3813, 0.0904, 0.1688 },
        earth = { 0.5000, 0.3088, 0.2924, 0.3631 },
        plants = { 0.7504, 0.4127, 0.2082, 0.1609 },
        science = { 0.1045, 0.4966, 0.0930, 0.2327 },
        ["long-ago"] = { 0.8853, 0.4605, 0.1215, 0.1239 },
    },
    drawers = {
        OpenPassport = { 0.0152, 0.6878, 0.2583, 0.9079 },
        OpenHunt = { 0.2686, 0.6878, 0.4952, 0.9079 },
        DeskQuestions = { 0.5048, 0.6878, 0.7341, 0.9079 },
        DeskBookshelf = { 0.7465, 0.6878, 0.9869, 0.9079 },
    },
}
