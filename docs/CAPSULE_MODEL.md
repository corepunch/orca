# 3D Capsule Model for Button Rendering

## Overview

The ORCA renderer now includes a 3D capsule model specifically designed for rendering modern button interfaces. The capsule shape combines the clean aesthetics of rounded rectangles with true 3D depth, making it ideal for creating glossy, interactive button elements similar to those found in macOS Aqua interfaces.

## Model: Capsule

### Function

```c
HRESULT Model_CreateCapsule(float width, float height, float depth, float cylindriness, struct model** ppModel)
```

### Parameters

- **width**: Width of the capsule (X-axis scaling)
- **height**: Height of the capsule (Y-axis, determines overall length)
- **depth**: Depth of the capsule (Z-axis scaling)
- **cylindriness**: Controls the shape morphing (0.0 to 1.0)
  - `0.0` = Pure box shape
  - `1.0` = Full capsule (cylinder with hemispherical caps)
  - Values in between provide smooth interpolation

### Technical Details

The capsule model is generated procedurally using:
- **16 segments** around the circumference
- **8 rings** per hemisphere
- Proper UV mapping for texture application
- Smooth normals for realistic lighting

The geometry consists of three parts:
1. **Top hemisphere** - Rounded cap at the top
2. **Cylinder body** - Straight sides
3. **Bottom hemisphere** - Rounded cap at the bottom

### Model Enum

The capsule model is registered as `MD_CAPSULE` in the renderer's model enumeration and is automatically created during renderer initialization with default parameters:
- Width: 1.0
- Height: 1.0
- Depth: 1.0
- Cylindriness: 0.8

## Shader: Button Shader

### Description

The button shader (`SHADER_BUTTON`) is specifically designed for rendering 3D button elements with realistic lighting and depth. It provides a glossy, modern appearance suitable for interactive UI elements.

### Features

1. **Diffuse Lighting**: Standard lambertian diffuse with ambient light
2. **Specular Highlights**: Glossy reflections for a polished appearance
3. **Rim Lighting**: Subtle edge highlighting for depth perception
4. **Texture Support**: Full texture mapping with color modulation
5. **Opacity Control**: Adjustable transparency

### Uniforms

- `u_modelViewProjectionTransform` - MVP matrix
- `u_normalTransform` - Normal transformation matrix
- `u_opacity` - Global opacity (0.0 to 1.0)
- `u_texture` - Base color texture
- `u_color` - Tint color
- `u_lightDir` - Light direction (normalized)
- `u_specularPower` - Controls glossiness (higher = sharper highlights)

### Usage Example

```c
// In C code - the model and shader are already initialized
// Access them via:
struct model* capsuleModel = tr.models[MD_CAPSULE];
struct shader* buttonShader = &tr.shaders[SHADER_BUTTON].shader;

// Set shader parameters
vec3_t lightDir = {0.5f, 0.5f, 0.5f};
float specularPower = 32.0f; // Adjust for desired glossiness

// Render with the shader
// (Use appropriate rendering API calls)
```

## Visual Characteristics

The combination of the capsule model and button shader creates:
- **3D depth** from the capsule geometry
- **Glossy appearance** from specular highlights
- **Soft edges** from rim lighting
- **Professional look** similar to macOS Aqua buttons

## Design Philosophy

The capsule model with procedural cylindriness parameter provides flexibility:
- Use low cylindriness (0.2-0.4) for slightly rounded buttons
- Use medium cylindriness (0.5-0.7) for pill-shaped buttons
- Use high cylindriness (0.8-1.0) for fully rounded capsule buttons

This allows designers to match various UI styles while maintaining consistent lighting behavior through the button shader.

## Performance

The capsule model is efficiently generated with a fixed vertex count:
- Approximately 340 vertices per capsule
- Indexed triangle rendering
- Pre-calculated normals for lighting
- Suitable for real-time rendering of multiple button instances
