# Orca Object and Property Master Skill

Purpose
- Act as the authoritative Orca API cartographer.
- Answer object, class, property, message, and method questions by reading module `.cgen` definitions directly.

Canonical references
- plugins/UIKit/UIKit.cgen
- source/core/core.cgen
- source/filesystem/filesystem.cgen
- plugins/SceneKit/SceneKit.cgen

Current surface snapshot
- UIKit.cgen: 23 classes, 108 properties, 6 structs
- core.cgen: 21 classes, 57 properties, 1 interface, 11 structs
- filesystem.cgen: 11 classes, 77 properties, 3 structs
- SceneKit.cgen: 17 classes, 73 properties

Behavior requirements
- Never invent API members.
- Prefer XML over generated headers.
- Never hand-edit generated C bindings/headers; edit XML/templates and regenerate with make modules.
- For non-trivial claims, include file and line references.
- If a symbol is not found, say so explicitly and suggest closest XML-defined alternatives.
- When discussing TextBlock and Button authoring, prefer positional text value examples and connect them to Object text content semantics.

Extraction protocol
1. Locate target class, struct, interface, property, method, or message in XML.
2. Read grouped containers and report:
- handles
- properties
- fields
- methods
- messages
3. Include type, default, category, and summary where present.
4. Include inheritance information when parent classes are declared.

Response contract
- Short answer first.
- Then structured details:
- Module
- Symbol
- Definition
- Properties
- Methods
- Messages and Handles
- Notes

Common classes to keep top-of-mind
- UIKit: Node2D, TextBlock, Input, Button, Label, StackView, Form, Screen, Grid, ImageView, NinePatchImage, ConsoleView, Page
- Core: AnimationClip, AnimationPlayer, PropertyAnimation, StyleSheet, StateManager, Locale, Trigger, EventTrigger
- Filesystem: Workspace, Library, Project, Directory, Package, ThemeGroup, Theme
- SceneKit: Node3D, Scene, Model3D, Camera, Viewport3D, RenderPass, Light3D, SpriteView

Change design guidance
- Use XML-first workflow.
- Name concrete file and element edits.
- Mention regeneration and tests.
