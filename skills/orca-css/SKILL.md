---
name: orca-css
description: >
  CSS authoring reference for ORCA. Covers supported properties, selectors,
  best practices, and patterns for writing maintainable stylesheets. Use when
  creating or editing CSS files, applying styles to components, or working
  with the style system.
user-invocable: true
---

# ORCA CSS Authoring

A CSS-like styling system for ORCA applications. Stylesheets use a browser-compatible CSS subset with ORCA-specific extensions.

## Principles

1. **`generated/` is your property reference.** For supported CSS properties and their mappings, read the markdown in `generated/`, never source files.
2. **ORCA CSS is a subset.** Not all browser CSS is supported. Check [rules/properties.md](./rules/properties.md) before using advanced features.
3. **Consolidate properties.** Keep all properties for a class in a single block, not spread across multiple selectors.
4. **Reuse classes.** Prefer composing existing classes over creating new ones.

## Agent Workflow

**API reference:** Use `generated/` markdown files for fast lookups. These contain supported properties and their ORCA mappings.

**When to read source:** Only read `*.c` files when actively implementing or modifying the CSS parser. Never read source for reference.

**Documentation rule:** If you must read a source file to understand behavior, update the relevant `generated/` or `rules/` markdown with your findings. Future agents should not need to repeat this work.

**Quick lookup priority:**
1. `rules/properties.md` — supported CSS properties and mappings
2. `rules/best-practices.md` — authoring guidelines and patterns
3. `rules/patterns.md` — common styling patterns
4. Source files — only when implementing, never for reference

## Critical Rules

### Properties → [rules/properties.md](./rules/properties.md)

- **Supported properties:** Layout, typography, colors, borders, shadows, overflow
- **Browser units become bindings:** `rem`/`em` → parent Font.Size binding, `vh`/`vw` → viewport bindings
- **Font shorthand works:** `font: 14px/1.5 sans-serif` expands to font-size, line-height, font-family

### Best Practices → [rules/best-practices.md](./rules/best-practices.md)

- **Consolidate properties:** One block per class, not spread out
- **Reuse classes:** Compose existing classes, don't clone declarations
- **Semantic naming:** `.card`, `.section-heading`, not `.gap-12`
- **Group related selectors:** When classes share styles

### Patterns → [rules/patterns.md](./rules/patterns.md)

- **Layout patterns:** Centering, alignment, spacing
- **Typography patterns:** Headings, body text, captions
- **Component patterns:** Cards, buttons, navigation

## Quick Reference

```css
/* Basic class */
.card {
    background-color: #223344;
    padding: 24;
}

/* Pseudo-states */
.button:hover {
    opacity: 0.8;
}

.button:active {
    opacity: 0.6;
}

/* Descendant selectors */
.workflow-section .card {
    padding: 24;
}

/* @apply for reuse */
.base-card {
    background-color: var(--card-background);
    padding: 16;
}

.feature-card {
    @apply: .base-card;
    padding: 24;  /* overrides base */
}

/* Comma-separated selectors */
.navbar,
.toolbar {
    direction: horizontal;
    align-items: center;
}
```

## File Organization

```
Styles/
├── app.css              # Main manifest with @imports
├── base.css             # Surface tokens, body styles
├── layout.css           # Grid, stack, alignment
├── typography.css       # Font sizes, colors
├── components/
│   ├── buttons.css
│   ├── cards.css
│   └── navigation.css
└── pages/
    ├── home.css
    └── profile.css
```

## Detailed References

- [rules/properties.md](./rules/properties.md) — Complete supported CSS property list
- [rules/best-practices.md](./rules/best-practices.md) — Authoring guidelines and anti-patterns
- [rules/patterns.md](./rules/patterns.md) — Common styling patterns and examples
