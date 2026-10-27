# Example: XML and Lisp scenes

Example keeps both XML and Lisp screens and prefabs. `package.lua` uses the
extensionless startup path `Example/Screens/Application`, so the compiled scene
reader selects the corresponding file. XML is the default; define
`ORCA_USE_LISP` (`-DORCA_USE_LISP`) to build the Lisp reader instead.
See [build options](../../docs/BUILD.md).

The Lisp variants use the same parenthesized syntax for objects, binding
expressions, and message actions. CSS owns presentation in both formats;
`class` and theme references work the same way.

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
- Lisp scene and template paths include `.lisp` explicitly. XML paths are
  extensionless and resolve through the XML reader. The shared startup path is
  extensionless so either build can launch the app.

`Screens/Application2`, `ApplicationTW`, and `some` are retained alternate
scene examples in both formats; `Application` is the active application.
The XML metric prefab retains its existing `Mertic.xml` filename; its Lisp
counterpart is `Metric.lisp`. Each screen references the appropriate asset.
`Data/ApplicationData.xml` and `ApplicationData.schema` remain inputs to the
existing `XmlDataSource`. The Lua package, CSS, SVG icons, and data-source format
are independent of the scene syntax and are not converted by this migration.

After building either mode, run its executable with `apps/Example`, for example
`build/bin/orca apps/Example` for the default build. Check that cards show their
data, resizing switches the responsive columns, and **Get Started** opens a
popup whose **Close** action works. The scene reader is chosen at compile time;
there is no runtime fallback to the other format.
XML data/schema parsing and serialization are independent of this selection.

This is a declarative scene and binding language, not a general-purpose Lisp
interpreter. Binding expressions use the existing reactive property evaluator;
Lua continues to handle application code.
