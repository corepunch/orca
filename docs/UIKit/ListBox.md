# ListBox

**Source:** `plugins/UIKit/ListBox.c`
**Definition:** `plugins/UIKit/UIKit.cgen:1019`
**Namespace:** `http://schemas.corepunch.com/orca/2006/xml/presentation`

## Summary

A list box that allows the user to select one or more items from a collection. Inherits from both `StackView` and `ItemsControl`, providing linear layout with data binding.

## Inheritance

```
Node2D → StackView → ListBox
                ↘ ItemsControl → ListBox
```

## Properties

### From ItemsControl

| Property | Type | Description |
|----------|------|-------------|
| `ItemsSource` | `DataObject*` | The collection of items to display |
| `ItemTemplate` | `Node2D*` | The template used to represent each item |

### From StackView

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `Reversed` | `bool` | — | Reverse the order of child elements |
| `Direction` | `Direction` | `Vertical` | Layout direction (horizontal or vertical) |
| `AlignItems` | `AlignItems` | `Stretch` | Cross-axis alignment of children |
| `JustifyContent` | `JustifyContent` | — | Main-axis distribution of children |
| `Spacing` | `float` | — | Space between adjacent children |

## Handled Messages

| Message | Handler | Description |
|---------|---------|-------------|
| `Object.Start` | `ListBox_Start` | Populates the list by instantiating item templates for each data item |

## Implementation Details

### Object.Start Handler

The `ListBox` handles the `Object.Start` message to populate its children:

1. Verifies that `ItemsSource` and `ItemTemplate` are set
2. Iterates over each `DataObject` in the `ItemsSource` collection
3. For each data item:
   - If `ItemTemplate` is provided, instantiates it via `OBJ_Instantiate`
   - Otherwise, creates a default `TextBlock` with text "Item"
4. Sets the `Node.DataContext` property on each item to the corresponding `DataObject`
5. Returns `FALSE` and logs an error if `ItemsSource` or `ItemTemplate` is not set

### Requirements

Both `ItemsSource` and `ItemTemplate` must be set before the ListBox is started, otherwise an error is printed:

```
ListBox needs ItemsSource and ItemTemplate to populate itself
```
