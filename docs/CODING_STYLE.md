# Coding Style

Use this file for the local conventions that matter most during edits.

## C style

- Keep code compact; avoid unnecessary vertical space.
- Single-statement functions may stay on one line.
- Omit braces for single-statement `if` / `else` / `while` bodies when readable.
- Keep declarations close to first assignment.
- Prefer existing helpers and macros over open-coded repetitive logic.
- Match the surrounding file before introducing a new pattern.

## Architectural style

- Prefer convention over configuration.
- Prefer component-based additions over new fields on `Object`.
- Keep core free of plugin-header dependencies.
- Do not edit generated files.

## Documentation style

- Prefer short operational pages over long prose for agent-facing docs.
- Put command names, file paths, and decision rules near the top.
- When documenting `.cgen`, make summaries dense and specific.

## Before copying patterns

Use the nearest file in the same subsystem as the style reference.
