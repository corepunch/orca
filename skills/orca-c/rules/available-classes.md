# Available Classes — Quick Index

Generated docs at `docs/generated/<module>/classes/ClassName.md` for full properties and messages.

## core (`source/core/core.cgen`)

| Class | Role |
|---|---|
| Node | Base class for all UI engine nodes |
| AnimationCurve | Single animated property curve with keyframes |
| AnimationClip | Reusable animation asset (standalone) |
| AnimationPlayer | Component managing animation playback |
| PropertyAnimation | One-shot property tween, self-destructs |
| StyleSheet | Container for StyleRule definitions |
| StyleRule | Maps a class name (+ pseudo-class) to property overrides |
| StyleController | Manages CSS-style classes and stylesheet rules |
| StateManager | Container for StateGroup definitions |
| StateGroup | Tracks one controller property → maps values to States |
| State | Property overrides when StateGroup controller matches Value |
| StateManagerController | Applies state-driven overrides from a StateManager |
| ResourceDictionary | Base class for alias and locale resource containers |
| Locale | Localization resource (text and asset strings) |
| DataObject | Base class for data-holding objects |
| Binding | Declarative property binding (attaches program to parent) |
| BindingExpression | Legacy explicit-expression binding alias |
| Action | Base class for executable trigger actions |
| SendMessageAction | Base for generated message actions (Target, Mode) |
| Trigger | Base class for event-driven actions |
| OnPropertyChangedTrigger | Responds to property value changes on specified nodes |
| OnAttachedTrigger | Triggers when element is attached to the visual tree |
| EventTrigger | Responds to routed UI events (mouse, keyboard, etc.) |
| Setter | Applies property values when triggered |
| HideAction | Hides the object at a given path when triggered |

## UIKit (`plugins/UIKit/UIKit.cgen`)

| Class | Role |
|---|---|
| Node2D | 2D layout node (position, size, margins, transforms) |
| TextBlock | Renders text with style, wrapping, alignment |
| Input | Text input field |
| Button | Clickable button |
| Label | Non-interactive text label |
| StackView | Linear stack layout (horizontal or vertical) |
| Form | Form container |
| Screen | Full-screen modal or page root |
| Grid | CSS-grid-style layout container |
| ImageView | Image display |
| NinePatchImage | Scalable nine-patch image |
| ConsoleView | Embedded terminal output view |
| Page | Navigation page |
| PageHost | Hosts a navigation stack |
| PageViewport | Viewport for a page within a PageHost |
| Popup | Floating overlay |
| ListBox | Scrollable item list (built on ItemsControl + StackView) |
| ItemsControl | Data-driven item container base |
| PrefabView2D | Inline prefab instance |
| Control | Base for interactive UI elements |
| Brush | Base class for fill brushes |
| ColorBrush | Solid color fill |
| Cinematic | Cinematic/animation playback control |

## filesystem (`source/filesystem/filesystem.cgen`)

| Class | Role |
|---|---|
| Workspace | Root project workspace |
| Library | Library asset container |
| Project | Loadable project |
| Directory | Directory node in the asset tree |
| Package | Asset package |
| ThemeGroup | Group of Theme objects |
| Theme | Visual theme resource |

## SceneKit (`plugins/SceneKit/SceneKit.cgen`)

| Class | Role |
|---|---|
| Node3D | 3D scene node |
| Scene | 3D scene root |
| Model3D | 3D mesh model |
| Camera | 3D camera |
| Viewport3D | 3D viewport rendering |
| RenderPass | Render pass descriptor |
| Light3D | 3D light source |
| SpriteView | 2D sprite within a 3D scene |
