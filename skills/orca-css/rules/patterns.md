# CSS Patterns for ORCA

Common styling patterns and examples for ORCA applications.

## Layout Patterns

### Centering

```css
/* Horizontal center */
.center-h {
    margin-left: auto;
    margin-right: auto;
}

/* Vertical center */
.center-v {
    margin-top: auto;
    margin-bottom: auto;
}

/* Both axes (requires fixed dimensions or content-based sizing) */
.center-both {
    margin-left: auto;
    margin-right: auto;
    margin-top: auto;
    margin-bottom: auto;
}
```

### Right/Bottom Alignment

```css
/* Align to right edge */
.align-right {
    margin-left: auto;
}

/* Align to bottom edge */
.align-bottom {
    margin-top: auto;
}
```

### Flex-like Layout

```css
/* Horizontal stack */
.h-stack {
    direction: horizontal;
    align-items: center;
    gap: 12;
}

/* Vertical stack */
.v-stack {
    direction: vertical;
    gap: 12;
}

/* Space between */
.space-between {
    direction: horizontal;
    justify-content: space-between;
    align-items: center;
}
```

## Typography Patterns

### Heading Hierarchy

```css
.hero-title {
    font-size: 48;
    color: var(--text-primary);
}

.section-heading {
    font-size: 28;
    color: var(--text-primary);
}

.subheading {
    font-size: 18;
    color: var(--text-secondary);
}
```

### Body Text

```css
.body-text {
    font-size: 14;
    color: var(--text-primary);
    word-wrap: true;
}

.caption {
    font-size: 12;
    color: var(--text-muted);
}

.muted {
    font-size: 12;
    color: var(--text-dim);
}
```

### Text Overflow

```css
/* Truncate with ellipsis */
.truncate {
    text-overflow: ellipsis;
    text-wrap: nowrap;
}

/* Allow wrapping */
.wrap {
    word-wrap: true;
}
```

## Component Patterns

### Card

```css
.card {
    background-color: var(--card-background);
    padding: 16;
    border-radius: 8;
}

.card-hover:hover {
    opacity: 0.95;
}
```

### Button

```css
.button {
    font-size: 14;
    padding: 12 24;
    text-align: center;
    text-vertical-align: center;
}

.primary-button {
    background-color: var(--accent);
    color: var(--white);
}

.secondary-button {
    background-color: var(--card-background);
    color: var(--text-secondary);
}

.button:active {
    opacity: 0.8;
}
```

### Navigation

```css
.navbar {
    direction: horizontal;
    align-items: center;
    padding: 12 24;
    background-color: var(--panel-background);
}

.nav-item {
    padding: 8 16;
    font-size: 14;
}

.nav-item:hover {
    background-color: var(--surface);
}
```

## Spacing Patterns

### Consistent Spacing Scale

```css
/* Use a consistent scale: 4, 8, 12, 16, 24, 32, 48, 64 */
.spacing-xs { gap: 4; }
.spacing-sm { gap: 8; }
.spacing-md { gap: 12; }
.spacing-lg { gap: 16; }
.spacing-xl { gap: 24; }
```

### Section Spacing

```css
.section {
    padding: 48 0;
}

.section-compact {
    padding: 24 0;
}

.section-tight {
    padding: 16 0;
}
```

## State Patterns

### Hover States

```css
.card {
    background-color: var(--card-background);
}

.card:hover {
    opacity: 0.95;
    box-shadow: 0 4 12 rgba(0,0,0,0.15);
}
```

### Dark Mode

```css
.card {
    background-color: white;
    color: var(--text-primary);
}

.card:dark {
    background-color: #1e1e2e;
    color: var(--text-primary);
}
```

### Active/Selected States

```css
.nav-item {
    padding: 8 16;
}

.nav-item:active {
    background-color: var(--surface);
}
```

## Theme Patterns

### Using Theme Variables

```css
.card {
    background-color: var(--card-background);
    color: var(--text-primary);
}

.muted {
    color: var(--text-muted);
}

.accent {
    color: var(--accent);
}
```

### Common Theme Variables

- `--text-primary` — Main text color
- `--text-secondary` — Secondary text
- `--text-muted` — Muted text
- `--text-dim` — Very dim text
- `--card-background` — Card surface
- `--panel-background` — Panel surface
- `--surface` — Generic surface
- `--accent` — Accent color
- `--white` — White

## Composition Patterns

### Composing Classes in XML

```xml
<!-- Good: compose semantic classes -->
<Button class="button primary-button" Text="Submit"/>

<!-- Avoid: inline all styles -->
<Button BackgroundColor="#4a90d9" Color="white" Padding="12" Text="Submit"/>
```

### Using @apply

```css
/* Define base styles */
.base-card {
    background-color: var(--card-background);
    padding: 16;
    border-radius: 8;
}

/* Extend with variants */
.feature-card {
    @apply: .base-card;
    padding: 24;
}

.compact-card {
    @apply: .base-card;
    padding: 8;
}
```

## Common Pitfalls

### 1. Spread Properties

Don't spread the same class across multiple rules:

```css
/* Bad */
.card { background: blue; }
.card { padding: 16; }
.card { border-radius: 8; }

/* Good */
.card {
    background: blue;
    padding: 16;
    border-radius: 8;
}
```

### 2. Clone Styles

Don't copy entire classes:

```css
/* Bad */
.card {
    background: blue;
    padding: 16;
    border-radius: 8;
}

.feature-card {
    background: blue;
    padding: 24;
    border-radius: 8;
}

/* Good */
.card {
    background: blue;
    padding: 16;
    border-radius: 8;
}

.feature-card {
    @apply: .card;
    padding: 24;
}
```

### 3. Utility Proliferation

Don't create single-property utilities:

```css
/* Bad */
.p-8 { padding: 8; }
.p-16 { padding: 16; }
.p-24 { padding: 24; }

/* Good - use semantic classes or inline */
.card { padding: 16; }
```
