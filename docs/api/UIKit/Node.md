# Node

**Class** | `orca.UIKit`

Base class for all UI engine nodes.

## Overview

Node is the fundamental class for all engine objects (e.g., Node2D, Node3D, RenderPass). It manages shared functionality such as size, layout, visibility, opacity, tags, and data context. You normally work with derived classes rather than creating instances of Node directly.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `Size` | `SizeShorthand` | Sizing configuration of this element |
| `Margin` | `MarginShorthand` | External spacing around the element |
| `Padding` | `MarginShorthand` | Internal spacing between element border and content |
| `Border` | `BorderShorthand` | Border definition along each axis |
| `Alignment` | `AlignmentShorthand` | Alignment definition for each axis. |
| `Visible` | `bool` | Whether this node is visible in the scene. |
| `QuickHide` | `bool` | If true, node is quickly hidden without triggering full visibility transitions. |
| `VisibleAmountInParent` | `float` | Specifies how much of the node is visible within its parent container. Value in range [0.0, 1.0]. |
| `Opacity` | `float` | Opacity of the node, range [0.0 = transparent, 1.0 = fully opaque]. |
| `Tags` | `objectTags` | Tag collection for categorizing or querying nodes. |
| `DataContext` | `DataObject` | Data context (used for data binding, similar to XAML's DataContext). |

