<!-- 22 may 2020 -->

# Window controls

## Overview

TODO

## Reference

### `uiWindow`

```c
typedef uiControl uiWindow;
uiprivExtern uint32_t uiWindowType(void);
#define uiWindow(obj) ((uiWindow *) uiCheckControlType((obj), uiWindowType()))
```

`uiWindow` is a `uiControl` that represents a window.

Windows are the highest level of a control hierarchy that is visibile on screen. All controls that are current visible are contained within a `uiWindow`, and it is the size and position of the window that ultimately decides their size and position (though a control's minimum size may contribute to this). Windows also have titles, which are used to identify the window to the user.

Windows are themselves controls, but behave slightly differently from other controls: they cannot be added as children of other controls (it is a programmer error to do so), they are hidden by default, and OS-level resources are created alongside the `uiWindow` object and survive for the duration of that object (see the OS-specific `uiControl` implementation notes for details).

`uiWindowType()` is the type identifier of a `uiWindow` as passed to `uiCheckControlType()`. You rarely need to call this directly; the `uiWindow()` conversion macro does this for you.

### `uiNewWindow()`

```c
uiWindow *uiNewWindow(void);
```

`uiWindow` creates a new window. The window will have the empty string as its title and will not have a child.

TODO other parameters?

### `uiWindowTitle()`

```c
const char *uiWindowTitle(uiWindow *w);
```

`uiWindowTitle()` returns `w`'s current title as a UTF-8 string.

The memory storing the title is owned by libui and should not be modified. The returned pointer is valid until the title is changed or `w` is destroyed; in general, you should not store the returned string pointer directly for later use.

It is a programmer error to pass `NULL` for `w`. TODO for this and all other functions: either don't bother doing this check or do a redundant uiControl type check as well...

### `uiWindowSetTitle()`

```c
void uiWindowSetTitle(uiWindow *w, const char *title);
```

`uiWindowSetTitle()` changes `w`'s title to `title`.

It is a programmer error to pass `NULL` for `title`. If `title` is not valid UTF-8, `U+FFFD` characters will be used to sanitize the string.

It is a programmer error to pass `NULL` for `w`.

### `uiWindowChild()`

```c
uiprivExtern uiControl *uiWindowChild(uiWindow *w);
```

`uiWindowChild()` returns the current child of `w`, or `NULL` if there is none.

It is a programmer error to pass `NULL` for `w`.

### `uiWindowSetChild()`

```c
uiprivExtern void uiWindowSetChild(uiWindow *w, uiControl *child);
```

`uiWindowSetChild()` sets the child control of `w` to `child`. If `child` is `NULL`, `w` will have its child removed, and the window will be empty. If `w` already has a child, then the child is seamlessly swapped out, and the current child will be free to add to another parent.

A window can only have one child control at a time, and that child will be given the entire area of the window. Multiple controls within a window are handled using the other container control types.

It is a programmer error to pass `NULL` for `w`. It is also a programmer error to pass a control for `child` that is already the child of some other control.
