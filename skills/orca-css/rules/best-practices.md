# CSS Best Practices for ORCA

Guidelines for writing maintainable, efficient CSS in ORCA applications.

## Core Rules

### 1. Consolidate Properties

Keep all properties for a class in a single block. Never spread the same class across multiple rules.

**Good:**
```css
.card {
    background-color: var(--card-background);
    padding: 24;
    border-radius: 8;
}
```

**Bad:**
```css
.card { background-color: var(--card-background); }
.card { padding: 24; }
.card { border-radius: 8; }
```

### 2. Reuse Classes

Compose existing classes instead of cloning declarations. If you find yourself copying properties, create a base class.

**Good:**
```css
.card {
    background-color: var(--card-background);
    padding: 16;
}

.feature-card {
    @apply: .card;
    padding: 24;
}
```

**Bad:**
```css
.card {
    background-color: var(--card-background);
    padding: 16;
}

.feature-card {
    background-color: var(--card-background);
    padding: 24;
}
```

### 3. Semantic Naming

Use meaningful class names that describe purpose, not appearance.

**Good:**
```css
.card { }
.section-heading { }
.toolbar { }
.primary-button { }
```

**Bad:**
```css
.rounded-box { }
.big-text { }
.gray-bar { }
.blue-button { }
```

### 4. Avoid Utility Explosion

Don't create single-property utility classes in app CSS. Use semantic classes instead.

**Good:**
```css
.workflow-section .card { padding: 24; }
.workflow-section .stack-card { gap: 12; }
```

**Bad:**
```css
.p-24 { padding: 24; }
.gap-12 { gap: 12; }
.w-100 { width: 100; }
.text-14 { font-size: 14; }
```

## Selector Guidelines

### Prefer Class Selectors

```css
/* Good */
.card { }
.button { }
.primary-button { }

/* Avoid for styling */
#MyButton { }
Node2D { }
```

### Use Descendant Selectors for Context

```css
/* Good - contextual variation */
.workflow-section .card { padding: 24; }
.testimonials-section .card { padding: 28; }
```

### Group Related Selectors

When multiple classes share the same styles, group them:

```css
/* Good */
.navbar,
.toolbar,
.tab-panel-header {
    direction: horizontal;
    align-items: center;
    gap: 12;
}
```

### Avoid Deep Nesting

Keep selectors shallow. Deep nesting increases specificity and reduces reusability.

```css
/* Good */
.card { }

/* Avoid */
.app-body .section .card .title { }
```

## Property Organization

Order properties logically within a block:

1. **Layout** — `width`, `height`, `margin`, `padding`
2. **Visual** — `background`, `color`, `opacity`
3. **Border** — `border`, `border-radius`, `box-shadow`
4. **Typography** — `font-size`, `color`, `text-align`
5. **Other** — `direction`, `gap`, `overflow`

```css
.card {
    /* Layout */
    width: 300;
    padding: 24;
    
    /* Visual */
    background-color: var(--card-background);
    
    /* Border */
    border-radius: 8;
    box-shadow: 0 2 8 rgba(0,0,0,0.1);
    
    /* Typography */
    font-size: 14;
}
```

## File Organization

### Structure

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

### app.css Pattern

```css
@import "base.css";
@import "layout.css";
@import "typography.css";

@import "components/navigation.css";
@import "components/buttons.css";
@import "components/cards.css";

@import "pages/home.css";
@import "pages/profile.css";
```

### File Size Guidelines

- Keep files under 100 rules when possible
- Split by responsibility (components/, pages/)
- Use coarse files, not one file per class

## Anti-Patterns

### 1. Clone-and-Modify

Don't copy a class and change one property. Use `@apply` or create a variant.

### 2. Utility Proliferation

Don't create `.p-8`, `.p-12`, `.p-16`, `.p-24`. Use semantic spacing classes or inline values.

### 3. ID Selectors for Styling

Reserve `#Name` selectors for true one-off overrides tied to identity.

### 4. Deep Nesting

Keep selectors shallow. Deep nesting makes styles hard to override and maintain.

### 5. Repeating Declarations

If you write the same property in multiple places, consolidate into a base class.

## Checklist

Before finalizing CSS, verify:

- [ ] Each class has all properties in one block
- [ ] Classes are reused via `@apply` or composition
- [ ] Class names are semantic, not descriptive
- [ ] Selectors are shallow (max 2 levels)
- [ ] Related selectors are grouped
- [ ] File structure is organized by responsibility
- [ ] No utility classes in app CSS
