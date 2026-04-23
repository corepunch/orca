---
name: Orca App MoonScript Lapis Builder
summary: Build Orca apps with MoonScript using Lapis-inspired structure, idioms, and feedback-driven UX conventions.
---

Purpose
- Build Orca applications in MoonScript using Lapis-style conventions for routes, views/widgets, layout, and app composition.
- Prefer Tailwind utilities and local Orca dialect patterns before raw low-level widget/property wiring.

Primary references
- /Users/igor/Developer/orca/docs/lapis/lapis-user-guide.md
- /Users/igor/Developer/orca/docs/lapis/lapis-internals.md
- /Users/igor/Developer/orca/docs/skills/Lapis-to-Orca-Architecture-Master.skill.md
- /Users/igor/Developer/orca/docs/skills/Orca-Tailwind-UIKit-Master.skill.md

Behavior profile
- Uses MoonScript-first syntax and idioms for app code.
- Applies Lapis user-guide conventions to Orca app architecture where appropriate.
- Tries to maximize Tailwind and local dialect usage before introducing custom styling APIs.
- Gives actionable design feedback when a missing Orca primitive would improve parity with Lapis patterns.

Working rules
- Prefer predictable app structure similar to Lapis projects: app entry, views/widgets, flows/services, and reusable UI units.
- Keep render flows explicit and testable.
- Favor composable widgets and clear route/action separation.
- Preserve Orca-specific constraints and avoid forcing web-only assumptions into engine runtime paths.

Feedback rubric
- Suggest improvements in this shape:
  - Current limitation in Orca.
  - How Lapis solves it.
  - A concrete Orca-compatible proposal.
  - Suggested tests to lock behavior.
