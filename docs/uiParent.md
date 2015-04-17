# uiParent
uiParent is an interface that is used by uiWindow, uiTab, and uiGroup to hold all the OS control handles of a tree of uiControls. That is, when you add a uiControl to a uiWindow, uiTab, or uiGroup, its OS control handle (if any) and the OS control handles of its children (when applicable) are added to the OS control handle of the uiParent. This is what allows uiWindow, uiTab, and uiGroup to hold more than one uiControl from the point of view of the OS.

Note that the control hierarchy is flattened; that is, there is no unique uiParent allocated to every single uiBox, uiGrid, or uiStack. This is to deal with systems where OS control handles are valuable.

## field Internal
```c
void *Internal;
```
This pointer is used internally by libui. Do not attempt to alter it or access/alter its data.

## func Destroy()
```c
void uiParentDestroy(uiParent *p);
```
Destroys the uiParent. This also destroys the main child, if any. This is called by `uiWindowDestroy()` and uiTab's/uiGroup's `uiControlDestroy()`.

## func Handle()
```c
uintptr_t uiParentHandle(uiParent *p);
```
Returns the OS control handle that is backed by this uiParent. On Windows, this is a HWND. On Unix, this is a GtkContainer. On Mac OS X, this is a NSView. The actual class types of this handle is internal to libui; you should only attempt to use this handle as if it was an instance of the specified classes or of their superclasses (GtkWidget, etc.).

## func SetMainControl()
```c
void uiParentSetMainControl(uiParent *p, uiControl *c);
```
Sets the main control of a uiParent. The *main control* is the uiControl that represnets the root of the control hierarchy that a uiParent is managing. This is the only ontrol that the uiParent knows about directly; all children of this uiControl are handled transparently through the uiControl. For example, when the uiWindow, uiTab, or uiGroup holding this uiParent is resized, the uiParent resizes the main control, which in turn resizes its children.

If the main control is already set, `SetMainControl()` replaces the main control. The old main control is NOT destroyed; its parent is merely set to `NULL` (and from there, all of the old main control's children's parents are also set to `NULL`).

If `c` is `NULL`, the main control is unset. Otherwise, `SetMainControl()` sets the parent of `c` to itself, triggering a `uiControlSetParent()` of all of `c`'s children.

`SetMainControl()` does not automatically update the uiParent.

## func SetMargins()
```c
void uiParentSetMargins(uiParent *p, intmax_t left, intmax_t top, intmax_t right, intmax_t bottom);
```
Sets the margins that the uiParent will give to the main control when it is resized. The units of the margins are defined by the backend (for the Windows backend, these are in dialog units; for the Unix and OS X backends, these are in pixels).

The initial margins are all zero.

`SetMargins()` does not automatically update the uiParent.

## func Update()
```c
void uiParentUpdate(uiParent *p);
```
Tells the uiParent to update itself in place. The main control is resized according to the uiParent's size.
