<!-- 10 june 2019 -->

# Controls on Unix systems other than macOS

## Overview

TODO

## Reference

### `uiControlOSVtable`

```c
typedef struct uiControlOSVtable uiControlOSVtable;
struct uiControlOSVtable {
	size_t Size;
	GtkWidget *(*Handle)(uiControl *c, void *implData);
};
```

`uiControlOSVtable` describes the set of functions that control implementations on Unix need to implement. When registering your control type, you pass this in as a parameter to `uiRegisterControlType()`. Each method here is required.

You are responsible for allocating and initializing this struct. To do so, you simply zero the memory for this struct and set its `Size` field to `sizeof (uiControlOSVtable)`. (TODO put this in a common place)

Each method takes at least two parameters. The first, `c`, is the `uiControl` itself. The second, `implData`, is the implementation data pointer; it is the same as the pointer returned by `uiControlImplData(c)`, and is provided here as a convenience.

Each method is named for the `uiUnixControl` function that it implements. As such, details on how to implement these methods are documented alongside those functions. For instance, instructions on implementing `Handle()` are given under the documentation for `uiUnixControlHandle()`.

### `uiUnixControlHandle()`

```objective-c
uiprivExtern GtkWidget *uiUnixControlHandle(uiControl *c);
```

`uiUnixControlHandle()` returns the GtkWidget that underpins `c`, or `NULL` if `c` does not have any underlying widget associated with it when called.

The widget returned by `uiUnixControlHandle()` is owned by `c`; you do not receive a reference to it at all. The widget is valid until either `c` is destroyed or until `c` decides to destroy the widget; you can handle the latter by catching TODO. In general, you should not store the returned widget pointer directly for later use, nor should you attempt to acquire a reference to the returned object. Instead, use the returned handle immediately if you have to, or follow TODO if you need to adjust properties of the handle that should persist across handle destruction/creation.

`uiWindow`s have a single handle of type `GtkWindow` that is created when the `uiWindow` is created and destroyed when the `uiWindow` is destroyed. Despite this, you should still follow the best practices described above.

For all other `uiControl`s defined by libui, the returned object is of the appropriate `GtkWidget` subclass:

* TODO

It is a programmer error to pass `NULL` for `c`. TODO a non-`uiControl`?
