<!-- 29 may 2019 -->

# Controls

## Overview

TODO

## Reference

### `uiControl`

```c
typedef struct uiControl uiControl;
uint32_t uiControlType(void);
#define uiControl(obj) ((uiControl *) uiCheckControlType((obj), uiControlType()))
```

`uiControl` is an opaque type that describes a control.

`uiControl()` is a conversion macro for casting a pointer of a specific control type (or a `void *`) to `uiControl` while also checking for errors.

`uiControlType()` is the type identifier of a `uiControl` as passed to `uiControlCheckType()`. You rarely need to call this directly; the `uiControl()` conversion macro does this for you.

### `uiControlVtable`

```c
typedef struct uiControlVtable uiControlVtable;
struct uiControlVtable {
	void (*Free)(uiControl *c, void *implData);
};
```

`uiControlVtable` describes the set of functions that control implementations need to implement. When registering your control type, you pass this in as part of the OS-specific counterpart vtable. Each method here is required.

Each method takes at least two parameters. The first, `c`, is the `uiControl` itself. The second, `implData`, is the implementation data pointer; it is the same as the pointer returned by `uiControlImplData(c)`, and is provided here as a convenience.

Each method is named for the `uiControl` function that it implements. As such, details on how to implement these methods are documented alongside those functions. For instance, instructions on implementing `Free()` are given under the documentation for `uiControlFree()`.

### `uiControlFree()`

```c
void uiControlFree(uiControl *c);
```

`uiControlFree()` frees the given control.

If `c` has children, those children are also freed. It is a programmer error to free a control that is itself a child of another control.

If `c` has any registered events, those event handlers will be set to be no longer run via `uiEventInvalidateSender()`. The registered handlers themselves will not be removed, to avoid the scenario of another `uiControl` being created with the same pointer value later triggering your handler unexpectedly.

It is a programmer error to specify `NULL` for `c`.

**For control implementations**: This function calls your vtable's `Free()` method. Parameter validity checks are already performed, `uiControlEventOnFree()` handlers have been called, and `uiControl`-specific events have been invalidated. Your `Free()` should invalidate any events that are specific to your controls, call `uiControlFree()` on all the children of this control, and free dynamically allocated memory that is part of your implementation data. Once your `Free()` method returns, libui will take care of freeing the implementation data memory block itself.
