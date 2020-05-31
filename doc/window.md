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