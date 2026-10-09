---
name: orca-ui-design
description: >
  Design guidelines for the ORCA modern UI framework, adapted from Apple's
  Human Interface Guidelines. Provides rules for visual hierarchy, components,
  layout, accessibility, and consistency so agents produce polished, usable
  interfaces with declarative components, MoonScript/XML, and Tailwind-style
  utilities.
  Use when designing or reviewing ORCA apps, screens, or components.
---

# ORCA UI Design Skill

ORCA is a modern UI framework for Lua with declarative components, routing,
Tailwind-style utilities, and cross-platform rendering. Write less code with
MoonScript or XML. Good taste means applying established design principles
(clarity, consistency, hierarchy, accessibility) while leveraging ORCA's
component model and styling approach.

Source inspiration: https://github.com/justinwetch/HIGAgentSkills. Principles
are adapted to ORCA's declarative style, component library, and CSS-like
utilities rather than native AppKit/UIKit controls.

## When to use

- Designing a new screen, route, or reusable component.
- Reviewing XML, MoonScript, or Lua UI definitions for hierarchy and usability.
- Choosing layout primitives, typography, spacing, and color roles.
- Ensuring accessibility and responsive behavior.

## Core principles (HIG-inspired)

- **Clarity**: One primary purpose per screen. Visual hierarchy guides the eye to the most important content and actions.
- **Consistency**: Reuse the same component and utility patterns for the same purpose. Prefer framework components over one-off custom drawing.
- **Deference**: UI supports the content. Use spacing, alignment, and weight rather than heavy decoration.
- **Accessibility**: Support sufficient contrast, readable type, logical focus order, and alternative text or labels for non-text elements. Do not rely on color alone.
- **Feedback**: Interactive elements show clear hover, pressed, focused, and disabled states.
- **Density and breathing room**: Use consistent spacing scale (Tailwind-style utilities). Primary content should not feel cramped; related items group tightly, groups separate clearly.
- **Responsive / adaptive**: Layouts should adapt to window size using flexible containers and utilities rather than fixed pixel assumptions where possible.

## Layout and components

- Prefer declarative XML or MoonScript component trees.
- Use stack, grid, or flow layout primitives for arrangement.
- Typography: establish a clear type scale (headings, body, captions) via utilities or theme.
- Spacing: use a consistent spacing scale. Sibling spacing is explicit; avoid magic margins.
- Color: use theme or semantic roles (primary, secondary, error, success, surface) rather than raw hex when the system supports it.
- Icons: Lucide stroke-based SVGs with `currentColor` so they inherit tint.
- Interactive controls: buttons, inputs, lists, cards should have adequate hit areas and visible focus indicators.

## Typical patterns

| Pattern | ORCA approach |
|---------|---------------|
| Primary action | Prominent button with clear label |
| Navigation | Routing + nav components or sidebars |
| Data lists | List or table components with consistent row height |
| Forms | Labeled inputs in a grid or stacked layout with aligned fields |
| Empty / error states | Clear message + one primary next action |
| Cards / grouping | Card component with theme radius and padding |

## Related skills and docs

- See existing skills under `.opencode/skills/` (e.g. `orca-xml-authoring`, `orca-lua-authoring`).
- [AGENTS.md](../../../AGENTS.md)
- [docs/UI_SYSTEM.md](../../../docs/UI_SYSTEM.md) and generated API docs.

When exact measurements or platform-native behavior are required, consult the target platform's guidelines and express them with ORCA's layout and styling primitives.
