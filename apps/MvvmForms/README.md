# Draft forms

Run `build/bin/orca apps/MvvmForms` after `make unite`.

The schema in `Models/Profile.schema` defines the view-model's string and bool
fields. `Models/Profile.xml` supplies initial values through a managed XML
datasource. `Views/Form.lisp` owns the complete view, bindings and command wiring.

Editors use `(bind "…" Mode=TwoWay)`; the preview and status use the default `OneWay`.
Keystrokes and checkbox clicks update the draft immediately. `App.lua` owns a
separate committed snapshot and validates before saving. The controller
only forwards Save and Discard commands; it never copies fields from controls.
Saving persists only for this app session. Production code would submit the
validated snapshot through a service.

Two-way paths must identify a writable property whose type matches the
control property. Use string draft fields for text editors; numeric parsing
and domain validation belong in the save/validation layer. Display expressions
can use `OneWay` or `Expression`; arithmetic and concatenation cannot be reversed.
