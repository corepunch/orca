-- Written by Tools/build_rooms.py from the models: each exhibit's
-- centre and size, and each drawer front's left, top, right and
-- bottom, as fractions of the camera's front panel.
return {
    subjects = {
        space = { 0.2115, 0.1496, 0.2253, 0.1821 },
        animals = { 0.2043, 0.3696, 0.2842, 0.1582 },
        ["your-body"] = { 0.7703, 0.1596, 0.1232, 0.1641 },
        earth = { 0.5000, 0.3176, 0.2309, 0.3710 },
        plants = { 0.8723, 0.5518, 0.2257, 0.1516 },
        science = { 0.6887, 0.5065, 0.1790, 0.2287 },
        ["long-ago"] = { 0.2129, 0.5583, 0.3089, 0.1359 },
    },
    drawers = {
        OpenPassport = { 0.0159, 0.6915, 0.4883, 0.8177 },
        OpenHunt = { 0.5138, 0.6915, 0.9841, 0.8177 },
        DeskQuestions = { 0.0166, 0.8343, 0.4876, 0.9650 },
        DeskBookshelf = { 0.5138, 0.8343, 0.9841, 0.9650 },
    },
}
