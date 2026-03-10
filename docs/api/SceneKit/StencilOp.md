# StencilOp

**Enum** | `orca.SceneKit`

Operations performed on stencil buffer values when stencil tests pass or fail

## Constants

| Name | Description |
|------|-------------|
| `Keep` | Keep the current stencil buffer value unchanged |
| `Zero` | Set the stencil buffer value to zero |
| `Replace` | Replace stencil buffer value with the reference value |
| `Increment` | Increment stencil buffer value, clamping to maximum |
| `IncrementWrap` | Increment stencil buffer value, wrapping to zero at maximum |
| `Decrement` | Decrement stencil buffer value, clamping to zero |
| `DecrementWrap` | Decrement stencil buffer value, wrapping to maximum at zero |
| `Invert` | Bitwise invert the stencil buffer value |

