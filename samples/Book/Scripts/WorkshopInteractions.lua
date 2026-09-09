-- Camera export spec for the workshop blockout. Presentation and focus authoring
-- live in WondertownScenes; this only tells the exporter which scene and source
-- resolution to project, and which camera is the default reference.
return {
    scene_path = "Book/Rooms/workshop-new.blks",
    source_width = 1920,
    source_height = 1440,
    camera = "workshop-floor",
}
