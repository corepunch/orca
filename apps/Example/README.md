# Example: XML scenes

Example demonstrates ORCA screens, reusable prefabs, responsive bindings and
data-driven cards. `package.lua` starts at `Example/Screens/Application`;
extensionless scene and template paths resolve to XML files.

XML owns the object tree and bindings, CSS owns presentation, and Lua handles
application behavior. For example, a card can bind its content to a record:

```xml
<StackView Name="FeatureCard" class="card stack-card feature-card">
  <TextBlock Name="FeatureCardTitle" class="card-title"
             Text="{Binding DataContext/Title}"/>
  <TextBlock Name="FeatureCardBody" class="card-body"
             Text="{Binding DataContext/Body}"/>
</StackView>
```

`Screens/Application2`, `ApplicationTW`, and `some` are retained alternate
scene examples; `Application` is the active application. The metric prefab
keeps its existing `Prefabs/Mertic.xml` filename.
`Data/ApplicationData.xml` and `ApplicationData.schema` supply the
application's `XmlDataSource`.

Build and run the example:

```bash
make unite
build/bin/orca apps/Example
```

Check that cards display their data, resizing switches the responsive columns,
and **Get Started** opens a popup whose **Close** action dismisses it. See
[Building ORCA applications](../../docs/app/index.md) for authoring conventions
and [Build](../../docs/BUILD.md) for platform setup.
