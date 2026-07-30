Your separable two-pass blur approach is right, but I’d separate box shadows from content/text shadows and add caching only after the uncached path is correct.

For a `TextBlock` shadow:

1. Use the existing glyph texture as the source mask.
2. Horizontal Gaussian pass → temporary texture.
3. Vertical Gaussian pass → final shadow texture.
4. Draw the final alpha mask tinted with `BoxShadow.Color` and positioned by `Offset`.
5. Keep the current direct duplicate draw when blur is zero.

Blur alpha only and apply color during composition. That avoids dark/color fringes and lets color/opacity changes reuse the cached blur.

Spread needs separate treatment: Gaussian blur does not create CSS spread. For glyphs, spread is morphological dilation—a max filter, an SDF expansion, or an approximate alpha-threshold technique. For rounded boxes, spread is easy: expand the rectangle by `spread` and increase its corner radii accordingly before generating the mask.

I would not initially put two persistent textures on every `Node2D`. Instead:

- Keep one optional private `_shadowTexture` per node—the cached final result.
- Have the renderer own a reusable scratch render target for the horizontal pass.
- Allocate both lazily only when `BlurRadius > 0` and the shadow is visible.
- Release `_shadowTexture` when blur becomes zero or the node is destroyed.
- Eventually use an `R8`/alpha-only lightweight target; the current render-target implementation allocates RGBA plus depth/stencil, which is excessive for a shadow mask.

Caching should use a shadow-source revision/signature, not just `OF_DIRTY`. The current dirty flag propagates to ancestors and is cleared recursively, so it conflates content changes with transforms and layout traversal: [object_core.c](/Users/igor/Developer/orca/source/core/object/object_core.c:76). A useful cache key would include:

```text
content revision
source width/height
display scale
blur radius
spread radius
shape radius/border geometry
```

Changes that require rebuilding:

- Text, font, wrapping width, or glyph texture
- Node dimensions or corner radius
- Blur or spread
- Display/render scale
- Foreground/source image if it defines the mask

Changes that should only affect composition:

- Shadow offset
- Shadow color or opacity
- Node position, transform, or opacity

For the first implementation, I’d deliberately render the two passes every frame when blur is enabled. That keeps correctness simple and gives us something measurable. Then introduce `_shadowTexture` plus a monotonically increasing `_visualRevision`. Otherwise we risk building a cache-invalidation system before establishing exactly what constitutes the shadow source.

I’d structure it as:

```text
UIKit decides source mask and cache validity
    ↓
renderer R_BlurMask(source, destination, scratch, sigma)
    ↓
UIKit composites destination using color + offset
```

The renderer function should save and restore the framebuffer and pipeline state because `R_BindFramebuffer` currently changes global renderer state: [r_texture.c](/Users/igor/Developer/orca/source/renderer/texture/r_texture.c:414).

One semantic issue is worth deciding first: `BoxShadow` should probably describe the rounded node box, while glyph shadows should be a separate `TextShadow` property. The current TextBlock behavior effectively treats `BoxShadow` as a content shadow. If we want CSS-like semantics, rounded boxes can use an analytic shadow shader—no cached texture or invalidation needed—while only `TextShadow` and arbitrary-content shadows need the offscreen blur pipeline.