---
name: orca-xml-assets-scenes
description: Author or troubleshoot ORCA XML resource and scene declarations for package libraries, images, textures, shaders, font families, SceneKit 3D scenes, and SpriteKit 2D scenes and animations. Use when adding visual resources, scene graphs, or references from views and materials to packaged assets.
---

# ORCA XML Assets and Scenes

## Identify the owner

Select the owning module before writing XML:

- Read `.opencode/generated/xml/filesystem/` for `Project`, `Package`, and
  `Library` declarations.
- Read `.opencode/generated/xml/renderer/` for `Image`, `Texture`,
  `CubeMapTexture`, shader stages, `Shader`, `Material`, meshes, and
  `FontFamily`.
- Read `.opencode/generated/xml/SceneKit/` for 3D `Scene`, cameras, lights,
  nodes, meshes, materials, and view integration.
- Read `.opencode/generated/xml/SpriteKit/` for `SKScene`, `SKSpriteNode`,
  `SKView`, sprite frames, and animations.
- Read `.opencode/generated/xml/UIKit/` for consumers such as `ImageView`,
  `VectorView`, `Viewport3D`, and scene/prefab views.

Generate missing API pages with `make modules`. If generation is unavailable,
inspect only the relevant `.cgen`: `source/filesystem/filesystem.cgen`,
`source/renderer/api/renderer.cgen`, `plugins/SceneKit/SceneKit.cgen`,
`plugins/SpriteKit/SpriteKit.cgen`, or `plugins/UIKit/UIKit.cgen`.

## Declare resources

1. Inspect one nearby declaration of the same resource type.
2. Add or reference the owning package library. Use the exact library property
   defined on `Project`, such as `ImageLibrary`, `TextureLibrary`,
   `FontLibrary`, `SceneLibrary`, `SpriteLibrary`, or
   `SpriteAnimationLibrary`.
3. Keep paths consistent with the package's qualified asset naming. Resolve
   source files relative to the project conventions already in use.
4. Give reusable resources stable `Name` values and reference those qualified
   resource names from consumers.
5. Distinguish the declaration from its consumer: an `Image`/`Texture` loads
   data, while an `ImageView`, material, sprite, or scene property displays or
   samples it.

## Author each asset kind

- For an image, set `Source` and only the required loading properties, such as
  mask type, premultiplied alpha, mipmaps, scale, or target format.
- For a texture, set filtering, wrapping, format, mipmap, and anisotropy only
  when the consumer requires non-default sampling.
- For a shader, declare the generated vertex/fragment stage elements and
  program properties exactly as documented; connect it through the expected
  material or render-pass property.
- For a font family, point `Regular`, `Bold`, `Italic`, and `BoldItalic` to the
  available font files. Omit unavailable variants instead of inventing paths.
- For a SceneKit scene, use `Scene` as the 3D root, name cameras referenced by
  `Camera`, `PreviewCamera`, or `HitTestCamera`, and connect environment
  textures, materials, meshes, and render passes through their documented
  property types.
- For SpriteKit, distinguish `SKScene`/`SKSpriteNode` runtime scene content
  from `SpriteAnimation` resource data. Bind an animation to its spritesheet
  texture and declare `SpriteFrame` values under the generated array property.

## Validate

Load the smallest resource, scene, or consuming view that exercises the new
declaration. Check missing-resource diagnostics, qualified names, file-name
case, property types, camera names, texture sampling, and shader compilation.
Then run the narrowest SceneKit, SpriteKit, renderer, or XML test.

Use raw Lucide SVGs with `stroke="currentColor"` for new icons so CSS tinting
works with `color` and `-orca-tint-mode: template`.
