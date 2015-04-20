# Object lifetimes

Know them.

One of the more annoying parts of libui is that it has to juggle an OS control handle and at least one C data structure for a single uiControl, uiParent, uiWindow, etc. To that end, this document exists to lay the ground rules for object lifetimes.

## To end programmers

TODO complain loudly if rules broken
TODO complain loudly if Destroying a control with a parent

The rules for lifetimes are simple:

- Removing a control from a container (for instance, with `uiBoxDelete()` or `uiTabDeletePage()`) does NOT destroy the control.
- Destroying a container (`uiWindowDestroy()`, `uiControlDestroy()` on a  uiBox or uiTab, etc.) WILL destroy children.
- Otherwise, a control stays alive.
- It is an error to destroy a control while it is part of a container.
- It is an error to add a control to more than one container at once.

Currently, breaking these rules results in undefined behavior. Error checks will be added soon.

## Implementation

Note that control implementations generally don't need to worry about backend-specific information; these are handled by the backends's convenience functions. If you bypass those, however, you *will* need to worry.

### General

### Containers (uiBox, uiGrid)

### Windows

Windows has no reference counting for window handles. The only situations where a window handle can be destroyed are

- with a call to `DestroyWindow()`, or
- by `DefWindowProc()` if it receives a `WM_CLOSE` message (when the user clicks the Close button on the titlebar of a toplevel window, for instance)

The destruction process is simple:

1. Any owned windows are destroyed. ([Source](http://stackoverflow.com/questions/29419291/is-my-subclassing-common-controls-tooltip-destroying-itself-in-wm-destroy-before))
2. The window receives `WM_DESTROY`.
3. Any child windows are destroyed.
4. The window receives `WM_NCDESTROY`.

TODO write this

### Unix

GtkWidgets are destroyed with the function `gtk_widget_destroy()`. For a container, which is also a widget, all children of the container at destruction time are also destroyed.

GtkWidgets also have a reference count. Initially, a GtkWidget starts out as *floating* (referenceless but unowned). Adding a widget to a container removes the floating status and increments the refcount. Removing a widget from a container decreases the reference count (but does NOT destroy the widget in and of itself). Once the referencce count drops back to zero, `gtk_widget_destroy()` is called.

What does this mean? In the normal case, you create a widget, add it to a container, and add the container to a GtkWindow (also a GtkWidget). Then, when you're done, you simply destroy the GtkWindow, which will destroy the widgets inside.

Note that removing a widget from a container does not call `gtk_widget_destroy()`; it merely decrements the reference count of the widget.

As with Windows, a `::delete-event` signal (which is sent when the user clicks a toplevel window's Close button) that is allowed to propagate will result in the window being destroyed.

TODO describe what we need to do
TODO remove lifetimes.c?

### Mac OS X

While Cocoa is a reference-counted object environment, simple reference counting of AppKit objects does not really work, and monitoring reference counts for lifetimes is dicouraged. Why? Strong references that we may not know about (think internal bookkeeping or third-party tools). However, if we pretend these additional references don't exist, the lifetime view beccomes simple.

When a view is created, it is created in a state where it is initially unowned; the documentation says we have to add it to the view hierarchy before we can use it. When a view is added to a superview, its reference count goes up (*retained*). When a view is removed from its superview, its reference count goes down (*released*). Once this virtual reference count goes to zero, the view is no longer safe to use.

In other words, when we create a view, add it to a superview, then remove it again, we should consider the view destroyed.

In the case of NSWindow, there's a property `releasedWhenClosed` that determines if a NSWindow is released when it is closed. (TODO before or after `windowWillClose`?)

do the whole "keep a reference around" thing
TODO drop the deleted controls view
