# 3D Rounded Box Model for UI Rendering

## Overview

The ORCA renderer includes a 3D rounded box model designed for rendering modern UI elements with rounded corners. Unlike capsules which are elongated along one axis, the rounded box provides uniform rounding on all 8 corners, making it ideal for creating buttons, panels, and other rectangular UI elements with smooth, rounded edges.

## Model: Rounded Box

### Function

```c
HRESULT Model_CreateRoundedBox(float width, float height, float depth, float radius, struct model** ppModel)
```

### Parameters

- **width**: Width of the box (X-axis dimension)
- **height**: Height of the box (Y-axis dimension)
- **depth**: Depth of the box (Z-axis dimension)
- **radius**: Corner rounding radius
  - `0.0` = No rounding (becomes a regular box)
  - `> 0.0` = Rounded corners (clamped to maximum of smallest dimension / 2)

### Technical Details

The rounded box model uses a **weight-based offset** approach for efficient shader-driven rendering:
- **8 corner spheres** - One quarter-sphere per box corner
- **8 segments** per corner arc
- **Weight field** stores the 3D offset direction for each vertex
- **Shader-driven radius** - The actual corner radius is applied in the vertex shader

The geometry consists of:
1. **8 corner spheres** - Quarter-spheres at each corner of the box
2. **12 edge cylinders** - Rounded edges connecting the corners
3. **6 flat faces** - Rectangular faces between the edges
4. **Weight-based displacement** - Vertices store their offset direction in the `weight` attribute
5. **Parametric radius** - Radius can be adjusted at render time via shader uniforms

### Key Difference from 2D Rounded Rectangle

While 2D rounded rectangles use the `texcoord` field (2 components) for corner offsets, the 3D rounded box uses the **weight field** (4 components) because:
- 3D geometry requires 3 offset components (x, y, z)
- `texcoord` only has 2 components (insufficient for 3D)
- `weight.xyz` provides the necessary 3D offset vector

### Model Enum

The rounded box model is registered as `MD_ROUNDED_BOX` in the renderer's model enumeration and is automatically created during renderer initialization with default parameters:
- Width: 1.0
- Height: 1.0
- Depth: 1.0
- Radius: 0.2

## Shader: Rounded Box Shader

### Description

The rounded box shader (`SHADER_ROUNDEDBOX`) is specifically designed to work with the weight-based rounded box model. It applies the corner radius in the vertex shader by displacing vertices based on their weight attribute.

### Features

1. **Shader-Driven Radius**: Radius is applied in the vertex shader, allowing dynamic adjustment
2. **Weight-Based Offset**: Uses `a_weight` attribute for 3D corner displacement
3. **Diffuse Lighting**: Standard lambertian diffuse lighting
4. **Specular Highlights**: Glossy reflections for polished appearance
5. **Fresnel Effect**: Edge-based reflections for realistic depth
6. **Refraction**: Simulated light refraction for glossy materials

### Vertex Shader Operation

```glsl
// Apply radius to weight-based offset
vec3 pos = a_position.xyz + a_weight.xyz * u_radius.x;
```

The vertex position represents the **center of the corner sphere**, and `a_weight.xyz` contains the **normalized direction** from that center to the surface of the sphere. Multiplying by `u_radius.x` gives the actual displacement.

### Uniforms

- `u_modelViewProjectionTransform` - MVP matrix
- `u_normalTransform` - Normal transformation matrix
- `u_opacity` - Global opacity (0.0 to 1.0)
- `u_color` - Tint color
- `u_radius` - Corner radius (vec4, only .x is used)
- `u_rect` - Rectangle dimensions and position
- `u_lightDir` - Light direction (normalized)
- `u_specularPower` - Controls glossiness
- `u_viewDir` - View direction for reflections

### Usage Example

```c
// In C code - the model and shader are already initialized
// Access them via:
struct model* roundedBoxModel = tr.models[MD_ROUNDED_BOX];
struct shader* roundedBoxShader = &tr.shaders[SHADER_ROUNDEDBOX].shader;

// Or use the entity type:
ViewEntity ent = {
  .type = ET_ROUNDED_BOX,
  .rect = {x, y, width, height},
  .color = {r, g, b, a},
  // radius can be set via uniform when rendering
};
```

## Visual Characteristics

The combination of the rounded box model and shader creates:
- **Uniform corner rounding** on all 8 corners
- **3D depth** from true geometric corners
- **Glossy appearance** from specular and fresnel effects
- **Parametric control** of rounding via shader uniforms

## Design Philosophy

The rounded box model provides flexibility through its parametric radius:
- **Geometry is radius-agnostic**: The mesh stores corner centers and directions
- **Shader applies radius**: Allows runtime adjustment without recreating geometry
- **Efficient for UI**: Single model can be reused with different radii
- **Similar to 2D approach**: Follows the same pattern as 2D rounded rectangles but in 3D

This design allows:
- Multiple buttons with different corner radii using the same model
- Animated corner rounding effects
- Consistent memory usage regardless of radius value

## Performance

The rounded box model is efficiently generated with:
- **Complete geometry**: 8 corners + 12 edges + 6 faces
- **Vertex count**: ~900-1000 vertices (varies with SEGS parameter)
  - 8 corners × (SEGS+1)² vertices = 648 vertices
  - 12 edges × (SEGS+1) × 2 vertices = 216 vertices  
  - 6 faces × 4 vertices = 24 vertices
- **Triangle count**: ~1400-1500 triangles
- Indexed triangle rendering
- Pre-calculated normals for lighting
- Single model instance can be reused with varying radii

## Comparison with Capsule

| Feature | Capsule | Rounded Box |
|---------|---------|-------------|
| **Shape** | Elongated along one axis | Uniform on all axes |
| **Rounding** | Hemispherical caps | Quarter-sphere corners |
| **Use Case** | Pill-shaped buttons | Rectangular buttons/panels |
| **Cylindriness** | Yes (0.0 to 1.0) | No (fixed shape, variable radius) |
| **Offset Method** | Baked into geometry | Weight-based shader offset |

A **capsule is NOT an edge case** of a rounded box - they represent fundamentally different geometries suitable for different UI design patterns.
