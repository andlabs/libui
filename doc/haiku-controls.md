<!-- 10 june 2019 -->

# Controls on Haiku

## Overview

TODO

## Reference

### `uiControlOSVtable`

```objective-c
typedef struct uiControlOSVtable uiControlOSVtable;
struct uiControlOSVtable {
	size_t Size;
	void *(*Handle)(uiControl *c, void *implData);
};
```

`uiControlOSVtable` describes the set of functions that control implementations on Haiku need to implement. When registering your control type, you pass this in as a parameter to `uiRegisterControlType()`. Each method here is required.

You are responsible for allocating and initializing this struct. To do so, you simply zero the memory for this struct and set its `Size` field to `sizeof (uiControlOSVtable)`. (TODO put this in a common place)

Each method takes at least two parameters. The first, `c`, is the `uiControl` itself. The second, `implData`, is the implementation data pointer; it is the same as the pointer returned by `uiControlImplData(c)`, and is provided here as a convenience.

Each method is named for the `uiHaikuControl` function that it implements. As such, details on how to implement these methods are documented alongside those functions. For instance, instructions on implementing `Handle()` are given under the documentation for `uiHaikuControlHandle()`.

### `uiHaikuControlHandle()`

```c
uiprivExtern void *uiHaikuControlHandle(uiControl *c);
```

`uiHaikuControlHandle()` returns the Objective-C object that underpins `c`, or `NULL` if `c` does not have any underlying object associated with it when called.

The object returned by `uiHaikuControlHandle()` is owned by `c`; you do not receive a reference to it at all. The object is valid until either `c` is destroyed or until `c` decides to destroy the object; you can handle the latter by catching TODO. In general, you should not store the returned object pointer directly for later use. Instead, use the returned handle immediately if you have to, or follow TODO if you need to adjust properties of the handle that should persist across handle destruction/creation.

`uiWindow`s have a single handle of type `BWindow` that is created when the `uiWindow` is created and destroyed when the `uiWindow` is destroyed. Despite this, you should still follow the best practices described above.

For all other `uiControl`s defined by libui, the returned object is of the appropriate class:

* TODO

It is a programmer error to pass `NULL` for `c`. TODO a non-`uiControl`?
