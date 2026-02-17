# Using the 3D Capsule Button Model

## Quick Start

The ORCA renderer now includes a procedurally generated 3D capsule model perfect for creating modern, glossy button interfaces.

## Basic Usage

The capsule model is automatically initialized during renderer startup and can be accessed via:

```c
// Access the pre-initialized capsule model
struct model* buttonModel = tr.models[MD_CAPSULE];

// Or create a custom capsule
struct model* myButton;
Model_CreateCapsule(
    1.0f,    // width
    0.5f,    // height
    1.0f,    // depth
    0.8f,    // cylindriness (0.0=box, 1.0=capsule)
    &myButton
);
```

## Using the Button Shader

The button shader provides realistic lighting for 3D buttons:

```c
// Access the button shader
struct shader* buttonShader = &tr.shaders[SHADER_BUTTON].shader;

// Set shader parameters before rendering
// u_lightDir: Direction of the light source (normalized vec3)
// u_specularPower: Controls glossiness (16.0-64.0 recommended)
// u_color: Button tint color
// u_opacity: Transparency
```

## Cylindriness Parameter Examples

The `cylindriness` parameter controls the shape morphing:

- **0.0** - Completely flat box (sharp edges)
- **0.3** - Slightly rounded button (subtle 3D effect)
- **0.5** - Moderately rounded button (balanced look)
- **0.7** - Pill-shaped button (modern iOS style)
- **1.0** - Full capsule (maximum roundness)

## Shader Parameters for Different Button Styles

### Glass Button (High Gloss)
- `u_specularPower`: 64.0
- `u_lightDir`: vec3(0.3, 0.5, 0.8) (upper-right light)
- `cylindriness`: 0.7

### Matte Button (Low Gloss)
- `u_specularPower`: 16.0
- `u_lightDir`: vec3(0.0, 1.0, 0.0) (top light)
- `cylindriness`: 0.4

### Aqua-Style Button
- `u_specularPower`: 48.0
- `u_lightDir`: vec3(0.4, 0.6, 0.7)
- `cylindriness`: 0.8

## Performance Notes

Each capsule model instance contains:
- ~340 vertices (17 segments × 20 rings)
- ~1920 triangles
- Pre-calculated normals
- Optimized for instanced rendering

For UI with many buttons, consider:
1. Using instanced rendering for repeated buttons
2. Sharing the same model instance across similar buttons
3. Adjusting CAPSULE_SEGMENTS/CAPSULE_RINGS for lower detail if needed

## Integration Example

```c
// Pseudo-code for rendering a button with the capsule model

void RenderButton(vec3_t position, vec3_t color) {
    // Setup shader
    glUseProgram(tr.shaders[SHADER_BUTTON].shader);
    
    // Set shader uniforms
    glUniform3f(u_lightDir_loc, 0.4f, 0.6f, 0.7f);
    glUniform1f(u_specularPower_loc, 48.0f);
    glUniform4f(u_color_loc, color.r, color.g, color.b, 1.0f);
    glUniform1f(u_opacity_loc, 1.0f);
    
    // Set transformation matrices
    mat4_t mvp;
    // ... calculate MVP matrix based on position
    glUniformMatrix4fv(u_mvp_loc, 1, GL_FALSE, &mvp);
    
    // Render the capsule model
    Model_Render(tr.models[MD_CAPSULE]);
}
```

## Customization

To create buttons with different proportions:

```c
// Wide button (for text labels)
Model_CreateCapsule(3.0f, 0.5f, 1.0f, 0.6f, &wideButton);

// Square button (for icons)
Model_CreateCapsule(1.0f, 1.0f, 1.0f, 0.7f, &squareButton);

// Tall button (for vertical layouts)
Model_CreateCapsule(1.0f, 2.0f, 1.0f, 0.8f, &tallButton);
```

## See Also

- [CAPSULE_MODEL.md](CAPSULE_MODEL.md) - Full technical documentation
- Aqua Button Design Reference: https://www.betalogue.com/images/uploads/AquaButton-Default.gif
