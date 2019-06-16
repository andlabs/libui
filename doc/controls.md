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

`uiControlType()` is the type identifier of a `uiControl` as passed to `uiCheckControlType()`. You rarely need to call this directly; the `uiControl()` conversion macro does this for you.

### `uiControlVtable`

```c
typedef struct uiControlVtable uiControlVtable;
struct uiControlVtable {
	size_t Size;
	bool (*Init)(uiControl *c, void *implData, void *initData);
	void (*Free)(uiControl *c, void *implData);
};
```

`uiControlVtable` describes the set of functions that control implementations need to implement. When registering your control type, you pass this in as a parameter to `uiRegisterControlType()`. Each method here is required.

You are responsible for allocating and initializing this struct. To do so, you simply zero the memory for this struct and set its `Size` field to `sizeof (uiControlVtable)`. (TODO put this in a common place)

Each method takes at least two parameters. The first, `c`, is the `uiControl` itself. The second, `implData`, is the implementation data pointer; it is the same as the pointer returned by `uiControlImplData(c)`, and is provided here as a convenience.

Each method is named for the `uiControl` function that it implements. As such, details on how to implement these methods are documented alongside those functions. For instance, instructions on implementing `Free()` are given under the documentation for `uiControlFree()`. The only exception is `Init()`, which is discussed under `uiNewControl()` below.

### `uiRegisterControlType()`

```c
uint32_t uiRegisterControlType(const char *name, const uiControlVtable *vtable, const uiControlOSVtable *osVtable, size_t implDataSize);
```

`uiRegisterControlType()` registers a new `uiControl` type with the given vtables and returns its ID as passed to `uiNewControl()`. `implDataSize` is the size of the implementation data struct that is created by `uiNewControl()`.

Each type has a name, passed in the `name` parameter. The type name is only used for debugging and error reporting purposes. The type name is copied into libui-internal memory; the `name` pointer passed to `uiRegisterControlType()` is not used after it returns.

`uiControlVtable` describes the functions of a `uiControl` common between platforms, and is discussed on this page. `uiControlOSVtable` describes functionst hat vary from OS to OS, and are described in the respective OS-specific uiControl implementation pages. The two vtables are copied into libui-internal memory; the vtable pointers passed to `uiRegisterControlType()` are not used after it returns.

It is a programmer error to specify `NULL` for either vtable. It is also a programmer error to specify `NULL` for any of the methods in either vtable — that is, all methods are required. It is also a programmer error to pass the wrong value to the `Size` field of either vtable.

An `implDataSize` of 0 is legal; the implementation data pointer will be `NULL`. This is not particularly useful, however.

### `uiCheckControlType()`

```c
void *uiCheckControlType(void *c, uint32_t type);
```

`uiCheckControlType()` checks whether `c` is a `uiControl`, and if so, whether it is of the type specified by `type`. If `c` is `NULL`, or if either of the above conditions is false, a programmer error is raised. If the conditions are met, the function returns `c` unchanged.

This function is intended to be used to implement a macro that converts an arbitrary `uiControl` pointer into a specific type. For instance, `uiButton` exposes its type ID as a function `uiButtonType()`, and provides the macro `uiButton()` that does the actual conversion as so:

```c
#define uiButton(c) ((uiButton *) uiCheckControlType((c), uiButtonType()))
```

### `uiNewControl()`

```c
uiControl *uiNewControl(uint32_t type, void *initData);
```

`uiNewControl()` creates a new `uiControl` of the given type.

This function is meant for control implementations to use in the implementation of dedicated creation functions; for instance, `uiNewButton()` calls `uiNewControl()`, passing in the appropriate values for `initData`. `initData` is, in turn, passed to the control's `Init()` method, and its format is generally internal to the control. Normal users should not call this function.

It is a programmer error to pass an invalid value for either `type` or `initData`.

**For control implementations**: This function allocates both the `uiControl` and the memory for the implementation data, and then passes both of these allocations as well as the value of `initData` into your `Init()` method. Return `false` from the `Init()` method if `initData` is invalid; if it is valid, initialize the control and return `true`. To discourage direct use of `uiNewControl()`, you should generally not allow `initData` to be `NULL`, even if there are no parameters. Do **not** return `false` for any other reason, including other forms of initialization failures; see [Error handling](error-handling.md) for details on what to do instead.

### `uiControlFree()`

```c
void uiControlFree(uiControl *c);
```

`uiControlFree()` frees the given control.

If `c` has children, those children are also freed. It is a programmer error to free a control that is itself a child of another control.

If `c` has any registered events, those event handlers will be set to be no longer run via `uiEventInvalidateSender()`. The registered handlers themselves will not be removed, to avoid the scenario of another `uiControl` being created with the same pointer value later triggering your handler unexpectedly.

It is a programmer error to specify `NULL` for `c`.

**For control implementations**: This function calls your vtable's `Free()` method. Parameter validity checks are already performed, `uiControlOnFree()` handlers have been called, and `uiControl`-specific events have been invalidated. Your `Free()` should invalidate any events that are specific to your controls, call `uiControlFree()` on all the children of this control, and free dynamically allocated memory that is part of your implementation data. Once your `Free()` method returns, libui will take care of freeing the implementation data memory block itself.

## `uiControlImplData()`

```c
void *uiControlImplData(uiControl *c);
```

`uiControlImplData()` returns the pointer to the implementation data for `c`. The returned pointer is valid for the lifetime of `c`.

This function is meant to be used by control implementations only. There is in general no guarantee as to the size or format of this pointer. Normal users should not call `uiControlImplData()`.

It is a programmer error to pass `NULL` or a non-`uiControl` for `c`.

## `uiControlOnFree()`

```c
uiEvent *uiControlOnFree(void);
```

`uiControlOnFree()` returns a `uiEvent` that is fired by `uiControlFree()` to indicate that a control is about to be freed. In your handler, `sender` is the control in question and `args` is `NULL`.
