<!-- 10 june 2019 -->

# Controls on Windows

## Overview

TODO

## Reference

### `uiControlOSVtable`

```c
typedef struct uiControlOSVtable uiControlOSVtable;
struct uiControlOSVtable {
	size_t Size;
	HWND (*Handle)(uiControl *c, void *implData);
	HWND (*ParentHandleForChild)(uiControl *c, void *implData, uiControl *child);
	HRESULT (*SetControlPos)(uiControl *c, void *implData, const RECT *r);
};
```

`uiControlOSVtable` describes the set of functions that control implementations on Windows need to implement. When registering your control type, you pass this in as a parameter to `uiRegisterControlType()`. Each method here is required.

You are responsible for allocating and initializing this struct. To do so, you simply zero the memory for this struct and set its `Size` field to `sizeof (uiControlOSVtable)`. (TODO put this in a common place)

Each method takes at least two parameters. The first, `c`, is the `uiControl` itself. The second, `implData`, is the implementation data pointer; it is the same as the pointer returned by `uiControlImplData(c)`, and is provided here as a convenience.

Each method is named for the `uiWindowsControl` function that it implements. As such, details on how to implement these methods are documented alongside those functions. For instance, instructions on implementing `Handle()` are given under the documentation for `uiWindowsControlHandle()`.

### `uiWindowsControlHandle()`

```c
uiprivExtern HWND uiWindowsControlHandle(uiControl *c);
```

`uiWindowsControlHandle()` returns the window handle that underpins `c`, or `NULL` if `c` does not have any underlying window handle associated with it when called.

The window returned by `uiWindowsControlHandle()` is owned by `c`; you do not receive a reference to it at all. The window is valid until either `c` is destroyed or until `c` decides to destroy the window; you can handle the latter by catching TODO. In general, you should not store the returned window handle directly for later use. Instead, use the returned handle immediately if you have to, or follow TODO if you need to adjust properties of the handle that should persist across handle destruction/creation.

`uiWindow`s have a single window that is created when the `uiWindow` is created and destroyed when the `uiWindow` is destroyed. Despite this, you should still follow the best practices described above. The window is of a special libui-internal window class.

For all other `uiControl`s defined by libui, the returned window is of the appropriate window class:

* TODO

It is a programmer error to pass `NULL` for `c`. TODO a non-`uiControl`?

**For control implementations**: This function does the above programmer error checks and then calls your `Handle()` method. You do not need to repeat the check yourself.

### `uiWindowsControlParentHandle()`

```c
uiprivExtern HWND uiWindowsControlParentHandle(uiControl *c);
```

`uiWindowsControlParentHandle()` returns the parent handle for `c`, or `NULL` if there currently is no such handle (either because `c` has no parent control or because none of its parent controls have a handle).

This is the parent from the point of view of the Windows API. When creating the window handle for a uiControl, this is the handle to use as the `hwndParent`.

The value returned by this function is valid until the control's parent changes for any reason. TODO should not be stored anyway TODO use `ParentChanging()`/`ParentChanged()` to destroy/create (respectively) window handles TODO refer to the top of this page for the control model

It is a programmer error to pass `NULL` for `c`. TODO a non-`uiControl`?

**For control implementations**: This function does the above programmer error checks; you do not need to repeat the checks yourself.

Unlike the other functions that operate on a `uiControl`, this function actually calls the `ParentHandleForChild()` method of the *parent control* of `c`, passing `c` as the `child` argument. If your parent control is a container that has window handles to use as the parent handles of its children, you should return the approprpiate window handle. (As an example of a case where knowing the child is important, `uiTab` has a separate parent handle for each of its tab pages.)

If your parent control is a container that does not have window handles of its own, you should call `uiWindowsControlParentHandle()` on the parent control itself, which will cause libui to chain up until it has reached the top level:

```c
static HWND controlParentHandleForChild(uiControl *c, void *implData, uiControl *child)
{
	return uiWindowsControlParentHandle(c);
}
```

If your parent control is not a container, return `NULL`. TODO programmer error?

As libui ensures that the arguments to `ParentHandleForChild()` are actually related, you do not need to check that `child` is actually your child yourself.

### `uiWindowsControlSetControlPos()`

```c
uiprivExtern HRESULT uiWindowsControlSetControlPos(uiControl *c, const RECT *r);
```

`uiWindowsControlSetControlPos()` causes `c` to be moved and resized to fill `r`. `r` must be in the *client* coordinates of `c`'s parent handle.

This function should be called by container implementations to reposition its children, either in response to a window being resized or when children need to be laid out due to some change (such as visibility). Users should not call this function directly.

It returns `S_OK` if the resize succeeded or some error if the resize failed *from the perspective of the OS*. It will not return an error in the event of a libui-specific programmer or internal error of some other sort. This error return is only intended for libui-internal use; see the control implementation details below.

It is a programmer error to pass `NULL` for `c` or `r`. It is also a programmer error to call `uiWindowsControlSetControlPos()` on a `uiWindow`.

**For control implementations**: This function calls your `SetControlPos()` method. For a simple control with a single window handle, the method should do nothing but call `uiWindowsSetControlHandlePos()` and return its return value:

```c
static HWND controlSetControlPos(uiControl *c, void *implData, RECT *r)
{
	controlImplData *ci = (controlImplData *) implData;

	return uiWindowsSetControlHandlePos(ci->hwnd, r);
}
```

If your control is a container, then your method should call this function recursively, passing an appropriate `RECT` for each of its children, returning the first error it finds (if any). If those children have a different parent handle from your own, then you will need ot make sure those `RECT`s are in the *client* coordinates of those parent handles. Return any error as soon as possible, to minimize ill effects (and to prepare for a potential switch to using `DeferWindowPos()` and friends instead).

You can also use this function to initiate a reposition of any children in the even that their layout has changed. In this case, TODO how to handle errors TODO something here about saving r

### `uiWindowsSetControlHandlePos()`

```c
uiprivExtern HRESULT uiWindowsSetControlHandlePos(HWND hwnd, const RECT *r);
```

TODO
