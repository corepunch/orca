# orca.UIKit

API reference for the `UIKit` module.

## Enumerations

- [**Direction**](Direction.md) — Defines the primary axis for layout operations
- [**Box3Field**](Box3Field.md) — Fields of a 3D box, used to describe position and size components
- [**TextOverflow**](TextOverflow.md) — Controls how overflowing text is displayed, similar to the CSS text-overflow property
- [**TextWrapping**](TextWrapping.md) — Controls text wrapping behavior
- [**TextHorizontalAlignment**](TextHorizontalAlignment.md) — Horizontal text alignment options within text bounds
- [**TextVerticalAlignment**](TextVerticalAlignment.md) — Vertical text alignment options within text bounds
- [**HorizontalAlignment**](HorizontalAlignment.md) — Horizontal alignment of elements within their parent container
- [**VerticalAlignment**](VerticalAlignment.md) — Vertical alignment of elements within their parent container
- [**AlignItems**](AlignItems.md) — Cross-axis alignment in flexbox-style layout containers
- [**DepthAlignment**](DepthAlignment.md) — 3D depth alignment options for positioning along Z-axis
- [**Overflow**](Overflow.md) — Content overflow handling strategies
- [**ForegroundHint**](ForegroundHint.md)
- [**JustifyContent**](JustifyContent.md) — Main-axis content distribution in flexbox-style layouts
- [**FontWeight**](FontWeight.md) — Font weight options for typography
- [**FontStyle**](FontStyle.md) — Font style options for typography
- [**BorderStyle**](BorderStyle.md) — Visual styles for element borders
- [**PlaybackMode**](PlaybackMode.md) — Animation playback behavior modes
- [**ButtonType**](ButtonType.md) — Button behavior types for different interaction patterns
- [**InputType**](InputType.md) — Input control behavior types for different data entry needs
- [**Sizing**](Sizing.md) — Size calculation modes for layout operations
- [**ResizeMode**](ResizeMode.md) — Defines how the window can be resized by the user or system.
- [**Stretch**](Stretch.md) — Specifies how the content of an element is resized to fill its layout area
- [**TransitionType**](TransitionType.md) — Specifies the type of transition animation to use when navigating between pages.
- [**StyleType**](StyleType.md) — Specifies the classification of a style and how it is applied to components.

## Structures

- [**BorderRadiusShorthand**](BorderRadiusShorthand.md) — Corner rounding configuration for rectangular elements
- [**EdgeShorthand**](EdgeShorthand.md) — Spacing values for opposite edges of an axis
- [**AlignmentShorthand**](AlignmentShorthand.md) — Axis-specific alignment parameters.
- [**FontShorthand**](FontShorthand.md) — Complete font specification including weight, style, size, and family
- [**BrushShorthand**](BrushShorthand.md) — Represents a generic brush used for filling UI elements.
- [**ShadowShorthand**](ShadowShorthand.md) — Drop shadow effect configuration
- [**RingShorthand**](RingShorthand.md) — Decorative ring/outline effect around elements
- [**OverflowShorthand**](OverflowShorthand.md) — Per-axis overflow behavior configuration
- [**UnderlineShorthand**](UnderlineShorthand.md) — Defines shorthand underline properties (offset, width, and color).
- [**MarginShorthand**](MarginShorthand.md) — External spacing configuration for rectangular elements
- [**BorderShorthand**](BorderShorthand.md) — Complete border definition for rectangular element
- [**SizeAxisShorthand**](SizeAxisShorthand.md) — Complete sizing definition for one axis
- [**SizeShorthand**](SizeShorthand.md) — Sizing configuration of elements (e.g., Width, Height, Depth)
- [**PushPropertyEventArgs**](PushPropertyEventArgs.md)
- [**ForegroundContentEventArgs**](ForegroundContentEventArgs.md)
- [**HandleMessageEventArgs**](HandleMessageEventArgs.md)
- [**MakeTextEventArgs**](MakeTextEventArgs.md) — Event data structure for text rendering requests
- [**DrawBrushEventArgs**](DrawBrushEventArgs.md) — Event data structure for brush drawing operations
- [**LoadViewEventArgs**](LoadViewEventArgs.md)
- [**TriggeredEventArgs**](TriggeredEventArgs.md)
- [**NavigateToPageArguments**](NavigateToPageArguments.md) — Event triggered to navigate to a different page within a PageHost.
- [**NavigateBackArguments**](NavigateBackArguments.md) — Event triggered to navigate back to the previous page in the navigation history.

## Classes

- [**DataObject**](DataObject.md) — Base class for data-holding objects within the framework
- [**AnimationPlayer**](AnimationPlayer.md) — Controls playback of animations within the UI engine.
- [**Trigger**](Trigger.md) — Base class for event-driven actions within the UI system
- [**OnPropertyChangedTrigger**](OnPropertyChangedTrigger.md) — Responds to property value changes on specified nodes
- [**OnAttachedTrigger**](OnAttachedTrigger.md) — Triggers when an element is attached to the visual tree
- [**EventTrigger**](EventTrigger.md) — Responds to routed UI events (mouse, keyboard, etc.)
- [**Setter**](Setter.md) — Applies property values when triggered by specified conditions
- [**Handler**](Handler.md) — Executes custom functions when triggered by specified conditions
- [**Brush**](Brush.md) — Base class for all brush types used in rendering.
- [**ColorBrush**](ColorBrush.md) — Brush that fills an area with a solid color.
- [**Node**](Node.md) — Base class for all UI engine nodes.
- [**TextRun**](TextRun.md) — Shared concept for text rendering in 2D and 3D nodes.
- [**TextBlockConcept**](TextBlockConcept.md) — Shared concept for text rendering in 2D and 3D nodes.
- [**Node2D**](Node2D.md) — Primary class for 2D UI elements with comprehensive rendering capabilities
- [**PrefabView2D**](PrefabView2D.md) — Container for loading and displaying prefabricated UI components
- [**TextBlock**](TextBlock.md) — Concrete implementation of text display combining Node2D and TextBlockConcept
- [**Input**](Input.md) — Interactive text input control with multiple input modes
- [**Button**](Button.md) — Interactive button control for user actions
- [**Label**](Label.md) — Text label with optional association to form controls
- [**StackView**](StackView.md) — Linear layout container arranging children in a single direction
- [**Form**](Form.md) — Specialized stack view for form input collection and submission
- [**Control**](Control.md) — Base class for interactive UI controls with state management
- [**Screen**](Screen.md) — Top-level container representing a display surface or window
- [**Cinematic**](Cinematic.md) — Video or animation playback view for media content
- [**Grid**](Grid.md) — Grid-based layout container with configurable rows and columns
- [**ImageView**](ImageView.md) — Displays image content with advanced rendering and scaling options
- [**NinePatchImage**](NinePatchImage.md) — Specialized image view for nine-patch (stretchable) images
- [**TerminalView**](TerminalView.md) — Displays a fixed-size text grid using a monospace layout
- [**Page**](Page.md) — Represents a single page within a document or UI container.
- [**PageHost**](PageHost.md) — Container that manages multiple pages and navigation between them.
- [**PageViewport**](PageViewport.md) — Viewport for rendering the active page within a PageHost.
- [**Style**](Style.md) — Defines a reusable set of property values and visual configurations for a target component type.

