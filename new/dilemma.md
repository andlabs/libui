There are five public types that can hold uiControls:

- uiWindow - a window. This is the only one that is not itself a uiControl. This can hold one uiControl.
- uiTab - pages of tabs. This can hold many uiControls, but each uiControl is placed into a separate tab.
- uiGruop - a group box. This can hold one uiControl.
- uiBox - a horizontal or vertical row of controls. This can hold many uiControls.
- uiGrid - a rectangular grid of controls. This can hold many uiControls.

Out of these, only uiBox and uiGrid do not need to be backed by an OS control.

On the implementation side,

- uiWindow
	- Windows - no issues, we can just pack the controls directly in
	- GTK+ - a GtkWindow can only have one child control. We would need a custom container *anyway* in order to be able to pack our various uiControls within.
	- OS X - no issues, but a custom NSView will be needed to handle resizes
- uiTab
	- Windows - we have to do all tab management oursselves; that means we have to watch for when tabs are changed and hide and show the content respectively; otherwise, we can handle children normally. Subclassing necessary for resizes.
	- GTK+ - once again, one GtkWidget per GtkNotebook tab. We need our custom container here too.
	- OS X - once again, custom NSView needed for resizes, but otherwise everything fine
- uiGroup
	- Windows - no issues; subclassing necessary for resizes.
	- GTK+ - same as usual: GtkFrame can only have one child so a custom container is necessary
	- OS X - same as usual: NSBox is fine but we would need a custom NSView for handling resizes

Lifetimes are also an issue. We have to synchronize a foreign OS control with our own allocated data structures.

- When a control is removed from a parent, it IS NOT destroyed.
- When a control is destroyed, its children ARE destroyed.
- A control cannot be destroyed while it is a child of another control.
- If 1 is returned from the uiWindow's OnClosing() handler, it is destroyed.
- If the OS destroys a control underfoot, then we have a bug in the library.

The second and third rules ccombined cause some of the inelegance and potential bugs; in order to enforce the third rule, places where the second rule has to first mark that the control has no parent, *then* destroy it.

The fifth rule exists at all because on OS X, we can't know for sure whether a control is destroyed or not. Relying on reference counts and object lifetimes is discouraged because of potential internal (or even external) bookkeeping. That leaves two options:

- handle `viewDidMoveToSuperview` and, since `nil` is a valid superview that NSTabView uses to hide tabs that aren't currently shown, have a special "deleted controls view" to do automatic destruction like on Windows and GTK+, or
- don't allow the OS to automatically destroy controls at all and do everything manually

------------------------

One potential solution is to imitate GTK+: have uiControl be the superclass of all other controls, then have uiContainer be a general-purpose container. This has a few advantages:

- it removes the distinction between a uiControl as a parent and the parent of a control from the OS side
- it removes the need for `ContainerShow/Hide/Enable/Disable()`

But it has a major disadvantage:

- on Windows, windows are valuable objects and we can only have about 10,000 or so in a single process

The current system (and every other solution I can think of, including past systems) have all been based around this weakness.

This weakness does not exist on either GTK+ or OS X.

I'm personally presently edging toward throwing my hands up and implementing this solution on Windows itself. It wouldn't get rid of the inelegance of having to unparent children before destruction, but it *would* make things consistent, which would lead into elegance.

I don't know what this will mean for uiWindow.
