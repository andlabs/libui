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
};
```

`uiControlOSVtable` describes the set of functions that control implementations on Windows need to implement. When registering your control type, you pass this in as a parameter to `uiRegisterControlType()`. Each method here is required.

You are responsible for allocating and initializing this struct. To do so, you simply zero the memory for this struct and set its `Size` field to `sizeof (uiControlOSVtable)`. (TODO put this in a common place)

Each method takes at least two parameters. The first, `c`, is the `uiControl` itself. The second, `implData`, is the implementation data pointer; it is the same as the pointer returned by `uiControlImplData(c)`, and is provided here as a convenience.

Each method is named for the `uiWindowsControl` function that it implements. As such, details on how to implement these methods are documented alongside those functions. For instance, instructions on implementing `TODO()` are given under the documentation for `TODO()`.
