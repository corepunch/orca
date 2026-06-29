# C Style

Read this before editing C when no more specific area file already covers the
task.

- Minimize vertical space; prefer fewer, denser lines over many short ones.
- Single-statement functions go on one line: `int f(void) { return 0; }`
- Omit braces for single-statement `if`/`else`/`while` bodies.
- Chain closely related statements on one line with `;` when readable.
- Merge declarations that belong to the same logical step.
- Prefer aggregate assignments and shorthand initializers for structs, vectors,
  rectangles, boxes, and thickness values.
- Prefer helpers like `VEC2_*`, `VEC3_*`, `RECT_*`, and `BOX3_*` over manual
  member-by-member math.
- Use the comma operator inside ternary branches when it keeps conditional
  initialization clear and compact.
- Avoid helper variables that only name an already-readable expression.
- Do not split a declaration from its first assignment.
