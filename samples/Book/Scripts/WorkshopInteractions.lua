-- Camera export spec for the workshop blockout. Presentation and focus authoring
-- live in WondertownScenes; this only tells the exporter which scene and source
-- resolution to project, and which camera is the default reference.
return {
    scene_path = "Book/Rooms/workshop.blks",
    source_width = 1536,
    source_height = 1024,
    camera = "workshop-floor",
}
