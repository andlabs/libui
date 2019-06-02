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
