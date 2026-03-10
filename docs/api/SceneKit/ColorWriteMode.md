# ColorWriteMode

**Enum** | `orca.SceneKit`

Control which color channels are written to the framebuffer during rendering

## Constants

| Name | Description |
|------|-------------|
| `None` | Disable all color channel writes (useful for depth-only or stencil-only passes) |
| `RGB` | Write to red, green, and blue channels only |
| `RGBA` | Write to all four color channels including alpha |
| `R` | Write to red channel only |
| `G` | Write to green channel only |
| `B` | Write to blue channel only |
| `GB` | Write to green and blue channels only |
| `A` | Write to alpha channel only |

