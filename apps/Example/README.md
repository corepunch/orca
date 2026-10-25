# Example: Lisp scenes and bindings

`package.lua` starts `Screens/Application.lisp`. Screens and prefabs use the
same parenthesized syntax for objects, binding expressions, and message actions.
CSS still owns presentation; `class` and theme references work as before.

```lisp
(StackView Name="FeatureCard" class="card stack-card feature-card"
  (TextBlock Name="FeatureCardTitle" class="card-title"
    Text=(bind "DataContext/Title"))
  (TextBlock Name="FeatureCardBody" class="card-body"
    Text=(bind "DataContext/Body")))
```

A property can contain an expression directly, with nested expressions using
the same syntax:

```lisp
(StackView Name="Body" class="app-body"
  Node.HorizontalPadding=(if (step 640 (bind "Screen.Width"))
                            (vector2 40 40)
                            (vector2 8 8))
  (TextBlock Text="Get Started"
    LeftButtonUp=(Screen.ShowModal Path="Example/Screens/GetStartedPopup.lisp")))
```

- `(ClassName Key=value ...)` constructs an object; child forms describe children.
- Strings use double quotes; numbers and booleans can be bare atoms.
- `(bind "path")` reads a property or data context value. Functions such as
  `if`, `step`, `bool`, and `vector2` compose binding expressions.
- Message attributes accept action nodes, for example
  `LeftButtonUp=(Popup.ClosePopup)`.
- Scene and template paths include `.lisp` explicitly.

`Screens/Application2.lisp`, `ApplicationTW.lisp`, and `some.lisp` are retained
alternate scene examples; `Application.lisp` is the active application.
`Data/ApplicationData.xml` and `ApplicationData.schema` remain inputs to the
existing `XmlDataSource`. The Lua package, CSS, SVG icons, and data-source format
are independent of the scene syntax and are not converted by this migration.

The scene loader accepts Lisp only: `.xml` scene registration and the
`loadObjectFromXml*` scene APIs have been removed. Other sample applications
have not been migrated. XML data/schema parsing and XML export remain separate
from scene loading.

This is a declarative scene and binding language, not a general-purpose Lisp
interpreter. Binding expressions use the existing reactive property evaluator;
Lua continues to handle application code.
