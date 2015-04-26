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

I'm also not sure how uiTab or uiGroup will be implemented this way. It works for GTK+ because it needs to do everything on its own...

With this idea, uiControl would have only one parenting method:

```c
void uiControlSetParent(uiControl *c, T t);
```

But what is `T`?

Let's assume it's `uiControl`.

```c
void uiControlSetParent(uiControl *c, uiControl *parent);
```

Then the traditional newcontrol implementation would be of the form

```c
void uiControlSetParent(uiControl *c, uiControl *parent)
{
	struct single *s = (struct single *) (c->Internal);

	s->parent = parent;
	if (s->parent != NULL)
		OSSetParent(s->OSHandle, (OSHandle) uiControlHandle(s->parent));
}
```

and the `uiControlDestroy()` would begin with

```c
	if (s->parent != NULL)
		complain("attempt to destroy uiControl %p while it has a parent", c);
```

The only problem is that when the control is hidden, we need to notify its parent to re-layout itself. A normal control has no need for such an update function.

Let's assume that this isn't a problem for now and get to the implementation of uiBox and uiGrid. These will be derived from a new type, uiContainer:

```
interface Container from Control {
	func ContainerResize([same arguments as Control.Resize()]);
	func Update(void);
};
```

`ContainerResize()` is implemented by `uiBox` and `uiGrid` to do the actual resizing work. It also uses `uiControlPreferredSize()`. `Update()` also performs a dummy resize, and is called by things like `uiBoxAppend()` and `uiBoxSetStretchy()`.

`uiNewContainer()` is the function to build a uiContainer subclass. It takes a pointer to where the `uiContainer()` should be placed. (TODO rename this and the OS-specific ones to uiMakeXxxx()?)

As an alternative, we could require chaining up:

```c
void boxResize(...)
{
	(*(b->containerResize))(...);
	...
}
```

Now let's think about uiWindow, uiTab, and uiGroup.

Let's make uiWindow a uiControl! It won't be a uiContainer, but it will have a uiControl as an immediate child. It can't be a child itself; calling `SetParent()` will complain.

uiGroup works the same way.

But neither of these have an `Update()` method! Okay fine, we'll make `Update()` part of `uiControl` too. For most widgets, `uiWidgetUpdate()` is a no-op; it only takes effect when the widge contains others.

uiTab is the problem.

Remember that on Windows we are manually responsible for showing and hiding tabs. But we want to get rid of `ContainerShow()` and `ContainerHide()`! And we can't just use `Show()` and `Hide()` because those are user settings, not libui settings.

In this case, we would need an intermediary uiContainer of sorts that only contains the control we're interested in. GTK+ calls this a "bin"; we can probably create an internal bin control for our use.

Note that this design means that there is a minimum of 2<i>n</i>+1 controls for each uiTab, with <i>n</i> being the number of tabs. If <i>n</i> is 10, we don't have much of a problem, though, so...

GTK+ and OS X are fine; we can just use the control itself as the tab page's GtkWidget/NSView, respectively.

BUT WAIT! All three of these need margins!

On Windows, there's no issue because we're in control of the sizing of all our controls, including parents like uiWindow. We just apply the margins in our parents's `uiControlResize()`.

But on GTK+ and OS X, the child controls get their size allocations and have to make use of them. *They* have to know about the margins, not the parents.

The solution is simple: have *everything* be in a bin! In fact we can have uiWindow and uiGroup be a uiBin too!

...I've just recreated GTK+...
