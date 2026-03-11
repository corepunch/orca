# Object

**Interface** | `orca.core`

Core engine host object.

## Overview


The `Object` type is the root of the runtime type system. Every element that can appear in a scene, be manipulated through scripts, or participate in rendering and logic derives its basic functionality from Object. It establishes the fundamental contract that all other types extend, providing common lifecycle methods, hierarchy management, and component composition.

An `Object` is not tied to a specific visual or logical role by itself. Instead, it serves as a container for components, and these components define the actual capabilities of the instance. Higher-level types such as `Node`, `Node2D`, or `ImageView` are constructed by layering specialized components on top of the base `Object`.


## Methods

### `clear()`

Clear all children of the object.

### `awake()`

Initializes the core component when it is loaded, an essential lifecycle method.

### `animate()`

Runs object animations.

### `loadPrefabs()`

Loads and instantiates prefabs.

### `emitPropertyChangedEvents()`

Emits onPropertyChanged events by comparing to previous values.

### `updateProperties()`

Updates object properties.

### `updateLayout(width: int, height: int)`

Updates object layout.

| Parameter | Type | Description |
|-----------|------|-------------|
| `width` | `int` | Available width for layout calculations |
| `height` | `int` | Available height for layout calculations |

### `addChild(child: Object, is_template: bool)`

Add a child object.

| Parameter | Type | Description |
|-----------|------|-------------|
| `child` | `Object` | The child object to add to this object's hierarchy |
| `is_template` | `bool` | If true, marks this object with the OF_TEMPLATE flag to indicate it contains template children |

### `removeFromParent(dropModal: bool)`

Destroys an object.

| Parameter | Type | Description |
|-----------|------|-------------|
| `dropModal` | `bool` | If set, would remove the entire chain of modal windows, like nested menus |

### `findChild(name: string, recursive: bool) → Object`

Find a child object by name.

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `string` | The exact name of the child object to locate |
| `recursive` | `bool` | If true, searches recursively through all descendants; if false, searches only immediate children |

**Returns:** `Object` — Pointer to the first child object found with the matching name, or null if no match is found

### `dispatchEvent(event: string) → Object`

Dispatch an event starting from this object and bubbling up parents.

| Parameter | Type | Description |
|-----------|------|-------------|
| `event` | `string` | The name of the event to dispatch through the hierarchy |

**Returns:** `Object` — Pointer to the object that successfully handled the event, or null if no object in the hierarchy handled it

### `postMessage(message: string)`

Posts a message to the global message queue.

| Parameter | Type | Description |
|-----------|------|-------------|
| `message` | `string` | The name of the message to post |

### `play(animation: string)`

Play an animation or resource on the object.

| Parameter | Type | Description |
|-----------|------|-------------|
| `animation` | `string` | Name or identifier of the animation to play, resolved from the object's animation library |

### `setFocus()`

Set focus on the object.

### `doTween()`

Tween an object property over time.

### `addStyleSheet(source: string)`

Add a stylesheet to the object.

| Parameter | Type | Description |
|-----------|------|-------------|
| `source` | `string` | Name or path of the stylesheet resource to load and apply to this object |

### `setTimer() → nresults`

Set a timer on the object.

**Returns:** `nresults` — Handle or identifier for the created timer, which can be used to control or cancel the timer before it expires

### `getName() → string`

Retrieves the object's name identifier

**Returns:** `string` — Current name of the object

### `setName(name: string)`

Sets the object's name identifier

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `string` | New name to assign to the object |

### `getClassName() → string`

Returns the object's class type name

**Returns:** `string` — Class name of the object type

### `checkName(name: string) → bool`

Checks if object has a specific name

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `string` | Name to check against |

**Returns:** `bool` — True if object name matches the provided string

### `findByPath(name: string) → Object`

Finds child object by hierarchical path

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `string` | Hierarchical path to the target object |

**Returns:** `Object` — Pointer to found object, or null if not found

### `getStyle() → uint`

Retrieves object style flags

**Returns:** `uint` — Current style flags as bitmask

### `setStyle(style: uint)`

Sets object style flags

| Parameter | Type | Description |
|-----------|------|-------------|
| `style` | `uint` | Style flags bitmask to apply |

### `findCallbackForID(event: uint) → string`

Retrieves callback function name for event ID

| Parameter | Type | Description |
|-----------|------|-------------|
| `event` | `uint` | Event ID to find callback for |

**Returns:** `string` — Name of callback function, or empty if none registered

### `applyStyles(recursive: bool)`

Applies style changes to object hierarchy

| Parameter | Type | Description |
|-----------|------|-------------|
| `recursive` | `bool` | Whether to apply styles to child objects recursively |

### `setDirty()`

Sets object dirty and queues it for recalculation

### `getParent() → Object`

Gets the parent object in the hierarchy

**Returns:** `Object` — Pointer to parent object, or null if no parent

### `getFirstChild() → Object`

Gets the first child object

**Returns:** `Object` — Pointer to first child object, or null if no children

### `getNext() → Object`

Gets the next sibling object

**Returns:** `Object` — Pointer to next sibling object, or null if no next sibling

### `getRoot() → Object`

Gets the root object of the hierarchy

**Returns:** `Object` — Pointer to root object

### `findChildByID(id: uint) → Object`

Finds a child object by its unique identifier

| Parameter | Type | Description |
|-----------|------|-------------|
| `id` | `uint` | Unique identifier of the child object to find |

**Returns:** `Object` — Pointer to child object with matching ID, or null if not found

### `findChildByAlias(id: uint) → Object`

Finds a child object by its alias identifier

| Parameter | Type | Description |
|-----------|------|-------------|
| `id` | `uint` | Alias identifier of the child object to find |

**Returns:** `Object` — Pointer to child object with matching alias, or null if not found

### `getSourceFile() → string`

Gets the source file path

**Returns:** `string` — Path to the source file

### `getTextContent() → string`

Gets the text content of the object

**Returns:** `string` — Text content of the object

### `getTimestamp() → long`

Gets the last modified timestamp

**Returns:** `long` — Timestamp of last dirty marking

### `getLuaObject() → uint`

Gets the Lua object reference

**Returns:** `uint` — Lua registry index for the object

### `getAlias() → uint`

Gets the alias identifier

**Returns:** `uint` — Alias identifier

### `getFlags() → uint`

Gets the object flags

**Returns:** `uint` — Bitfield of object flags

### `setFlags(flags: uint)`

Sets the object flags

| Parameter | Type | Description |
|-----------|------|-------------|
| `flags` | `uint` | New bitfield value for the object flags |

### `getIdentifier() → uint`

Gets the identifier of the object

**Returns:** `uint` — Hashed identifier of the object

### `getModal() → Object`

Gets the modal child object

**Returns:** `Object` — Pointer to the modal child object, or null if no modal is set

### `setModal(modal: Object)`

Sets or clears the modal child object

| Parameter | Type | Description |
|-----------|------|-------------|
| `modal` | `Object` | The object to set as modal, or null to clear the current modal |

### `isFocused() → bool`

Checks if this object currently has focus

**Returns:** `bool` — True if this object has focus, false otherwise

### `setHover()`

Sets the hover state for an object

### `setTextContent(text: string)`

Sets the text content of the object

| Parameter | Type | Description |
|-----------|------|-------------|
| `text` | `string` | The text content to display |

### `setSourceFile(path: string)`

Sets the source file path of the object

| Parameter | Type | Description |
|-----------|------|-------------|
| `path` | `string` | The file path to associate with the object |

### `setClassName(classID: string)`

Sets the class name of the object

| Parameter | Type | Description |
|-----------|------|-------------|
| `classID` | `string` | The class name to assign to the object |

### `findParentOfClass(classID: uint) → Object`

Finds the nearest parent object of a specific class

| Parameter | Type | Description |
|-----------|------|-------------|
| `classID` | `uint` | The hashed identifier of the class to search for |

**Returns:** `Object` — Pointer to the matching parent object, or null if not found

### `findChildOfClass(classID: uint) → Object`

Finds a child object of a specific class

| Parameter | Type | Description |
|-----------|------|-------------|
| `classID` | `uint` | The hashed identifier of the class to search for |

**Returns:** `Object` — Pointer to the matching child object, or null if not found

### `isPrefabView() → bool`

Checks if this object is a prefab view container

**Returns:** `bool` — True if this object is a PrefabView, false otherwise

### `addAlias(name: string, path: string)`

Registers an alias for a child object path

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `string` | The alias name (used as #name in references) |
| `path` | `string` | The full path to the object being aliased |

### `assignAliases(pathToObject: string)`

Resolves and assigns all registered aliases for an object

| Parameter | Type | Description |
|-----------|------|-------------|
| `pathToObject` | `string` | The path to the object being processed for alias resolution |

### `parseClassAttribute(classString: string)`

Parses and applies multiple class names from a class attribute string

| Parameter | Type | Description |
|-----------|------|-------------|
| `classString` | `string` | Space-separated class names, as from a class="..." attribute |

### `setAnimation(name: string)`

Sets the active animation by name

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `string` | The name of the animation to activate |

### `getAnimation() → KeyframeAnim`

Gets the currently active animation

**Returns:** `KeyframeAnim` — Pointer to the active keyframe animation, or null if none is set

### `addAnimation(animation: KeyframeAnim)`

Adds a keyframe animation to the object's animation library

| Parameter | Type | Description |
|-----------|------|-------------|
| `animation` | `KeyframeAnim` | The keyframe animation to add to the library |

### `getInteger(ident: uint, fallback: int) → int`

Gets an integer property value by identifier

| Parameter | Type | Description |
|-----------|------|-------------|
| `ident` | `uint` | The hashed long identifier "ObjectClass.PropertyName" of the property to retrieve |
| `fallback` | `int` | The value to return if the property is not found or incompatible |

**Returns:** `int` — The property value if found and compatible, otherwise the fallback value

### `getProperties() → Property`

Gets the properties collection

**Returns:** `Property` — Pointer to properties collection

### `findImplicitProperty(name: string) → PropertyType`

Looks up a property by context-driven syntax, like "Column" instead of "Grid.Column"

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `string` | The name of the property to find |

**Returns:** `PropertyType` — Pointer to a property type, null if not found

### `findExplicitProperty(name: string) → PropertyType`

Looks up a property by full syntax, like "Grid.Column" instead of "Column"

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `string` | The full name of the property to find |

**Returns:** `PropertyType` — Pointer to a property type, null if not found

### `attachPropertyProgram(name: string, program: string, attribute: PropertyAttribute, mode: BindingMode, enabled: bool) → bool`

Attaches a property program to the specified property

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `string` | The full name of the property to attach the program to |
| `program` | `string` | The program or expression to attach to the property |
| `attribute` | `PropertyAttribute` | The attribute of the property to attach the program to (e.g., Value, Color, etc.) |
| `mode` | `BindingMode` | The binding mode for the property program (e.g., OneWay, TwoWay, OneWayToSource) |
| `enabled` | `bool` | Whether the property program is enabled |

**Returns:** `bool` — True if the property program was successfully attached, false otherwise

### `findPropertyByPath(path: string) → Property`

Finds a property by navigating a hierarchical path

| Parameter | Type | Description |
|-----------|------|-------------|
| `path` | `string` | Slash-separated path to the property (e.g., "Child1/Child2/PropertyName") |

**Returns:** `Property` — Pointer to the property if found, or null if the path is invalid

### `rebuild()`

Rebuilds the object's body content asynchronously

### `getDomain() → lua_State`

Gets the domain of the object

**Returns:** `lua_State` — The domain of the object

