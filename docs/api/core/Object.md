# Object

**Interface** | `orca.core`

Core engine host object.

## Overview


The `Object` type is the root of the runtime type system. Every element that can appear in a scene, be manipulated through scripts, or participate in rendering and logic derives its basic functionality from Object. It establishes the fundamental contract that all other types extend, providing common lifecycle methods, hierarchy management, and component composition.

An `Object` is not tied to a specific visual or logical role by itself. Instead, it serves as a container for components, and these components define the actual capabilities of the instance. Higher-level types such as `Node`, `Node2D`, or `ImageView` are constructed by layering specialized components on top of the base `Object`.


## Methods

### `clear`

Clear all children of the object.

### `awake`

Initializes the core component when it is loaded, an essential lifecycle method.

### `animate`

Runs object animations.

### `loadPrefabs`

Loads and instantiates prefabs.

### `emitPropertyChangedEvents`

Emits onPropertyChanged events by comparing to previous values.

### `updateProperties`

Updates object properties.

### `updateLayout`

Updates object layout.

**Parameters:** `width`: `int`, `height`: `int`

### `addChild`

Add a child object.

**Parameters:** `child`: `Object`, `is_template`: `bool`

### `removeFromParent`

Destroys an object.

**Parameters:** `dropModal`: `bool`

### `findChild`

Find a child object by name.

**Parameters:** `name`: `string`, `recursive`: `bool`

**Returns:** `Object`

### `dispatchEvent`

Dispatch an event starting from this object and bubbling up parents.

**Parameters:** `event`: `string`

**Returns:** `Object`

### `postMessage`

Posts a message to the global message queue.

**Parameters:** `message`: `string`

### `play`

Play an animation or resource on the object.

**Parameters:** `animation`: `string`

### `setFocus`

Set focus on the object.

### `doTween`

Tween an object property over time.

### `addStyleSheet`

Add a stylesheet to the object.

**Parameters:** `source`: `string`

### `setTimer`

Set a timer on the object.

**Returns:** `nresults`

### `getName`

Retrieves the object's name identifier

**Returns:** `string`

### `setName`

Sets the object's name identifier

**Parameters:** `name`: `string`

### `getClassName`

Returns the object's class type name

**Returns:** `string`

### `checkName`

Checks if object has a specific name

**Parameters:** `name`: `string`

**Returns:** `bool`

### `findByPath`

Finds child object by hierarchical path

**Parameters:** `name`: `string`

**Returns:** `Object`

### `getStyle`

Retrieves object style flags

**Returns:** `uint`

### `setStyle`

Sets object style flags

**Parameters:** `style`: `uint`

### `findCallbackForID`

Retrieves callback function name for event ID

**Parameters:** `event`: `uint`

**Returns:** `string`

### `applyStyles`

Applies style changes to object hierarchy

**Parameters:** `recursive`: `bool`

### `setDirty`

Sets object dirty and queues it for recalculation

### `getParent`

Gets the parent object in the hierarchy

**Returns:** `Object`

### `getFirstChild`

Gets the first child object

**Returns:** `Object`

### `getNext`

Gets the next sibling object

**Returns:** `Object`

### `getRoot`

Gets the root object of the hierarchy

**Returns:** `Object`

### `findChildByID`

Finds a child object by its unique identifier

**Parameters:** `id`: `uint`

**Returns:** `Object`

### `findChildByAlias`

Finds a child object by its alias identifier

**Parameters:** `id`: `uint`

**Returns:** `Object`

### `getSourceFile`

Gets the source file path

**Returns:** `string`

### `getTextContent`

Gets the text content of the object

**Returns:** `string`

### `getTimestamp`

Gets the last modified timestamp

**Returns:** `long`

### `getLuaObject`

Gets the Lua object reference

**Returns:** `uint`

### `getAlias`

Gets the alias identifier

**Returns:** `uint`

### `getFlags`

Gets the object flags

**Returns:** `uint`

### `setFlags`

Sets the object flags

**Parameters:** `flags`: `uint`

### `getIdentifier`

Gets the identifier of the object

**Returns:** `uint`

### `getModal`

Gets the modal child object

**Returns:** `Object`

### `setModal`

Sets or clears the modal child object

**Parameters:** `modal`: `Object`

### `isFocused`

Checks if this object currently has focus

**Returns:** `bool`

### `setHover`

Sets the hover state for an object

### `setTextContent`

Sets the text content of the object

**Parameters:** `text`: `string`

### `setSourceFile`

Sets the source file path of the object

**Parameters:** `path`: `string`

### `setClassName`

Sets the class name of the object

**Parameters:** `classID`: `string`

### `findParentOfClass`

Finds the nearest parent object of a specific class

**Parameters:** `classID`: `uint`

**Returns:** `Object`

### `findChildOfClass`

Finds a child object of a specific class

**Parameters:** `classID`: `uint`

**Returns:** `Object`

### `isPrefabView`

Checks if this object is a prefab view container

**Returns:** `bool`

### `addAlias`

Registers an alias for a child object path

**Parameters:** `name`: `string`, `path`: `string`

### `assignAliases`

Resolves and assigns all registered aliases for an object

**Parameters:** `pathToObject`: `string`

### `parseClassAttribute`

Parses and applies multiple class names from a class attribute string

**Parameters:** `classString`: `string`

### `setAnimation`

Sets the active animation by name

**Parameters:** `name`: `string`

### `getAnimation`

Gets the currently active animation

**Returns:** `KeyframeAnim`

### `addAnimation`

Adds a keyframe animation to the object's animation library

**Parameters:** `animation`: `KeyframeAnim`

### `getInteger`

Gets an integer property value by identifier

**Parameters:** `ident`: `uint`, `fallback`: `int`

**Returns:** `int`

### `getProperties`

Gets the properties collection

**Returns:** `Property`

### `findImplicitProperty`

Looks up a property by context-driven syntax, like "Column" instead of "Grid.Column"

**Parameters:** `name`: `string`

**Returns:** `PropertyType`

### `findExplicitProperty`

Looks up a property by full syntax, like "Grid.Column" instead of "Column"

**Parameters:** `name`: `string`

**Returns:** `PropertyType`

### `attachPropertyProgram`

Attaches a property program to the specified property

**Parameters:** `name`: `string`, `program`: `string`, `attribute`: `PropertyAttribute`, `mode`: `BindingMode`, `enabled`: `bool`

**Returns:** `bool`

### `findPropertyByPath`

Finds a property by navigating a hierarchical path

**Parameters:** `path`: `string`

**Returns:** `Property`

### `rebuild`

Rebuilds the object's body content asynchronously

### `getDomain`

Gets the domain of the object

**Returns:** `lua_State`

