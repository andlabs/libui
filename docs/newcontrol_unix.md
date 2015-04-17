# Writing new controls for the Unix backend

The Unix backend uses GTK+ to do its UI work. GtkWidget provides the necessary functionality that uiControl needs, so if you have a valid GtkWidget, you're good to go.

For this document, we will write a basic wrapper around [GtkSwitch](https://developer.gnome.org/gtk3/3.4/GtkSwitch.html). It will conform to the following simple interface:

```c
typedef struct mySwitch mySwitch;
struct mySwitch {
	uiControl base;
	int (*On)(mySwitch *);
};
#define mySwitch(x) ((uiSwitch *) (x))
#define mySwitchOn(s) ((*((s)->On))((x)))
```

To be able to create Unix backend controls, we need to include `ui_unix.h`, the file that contains the Unix backend function declarations. It requires that you include both `ui.h` and `<gtk/gtk.h>` beforehand:

```c
#include <gtk/gtk.h>
#include "ui.h"
#include "ui_unix.h"
```

The first thing we need to do is define the data structure used for our new control. The first field of this data structure must be a mySwitch (not a pointer to a mySwitch). This allows us to use our data structure *as* the mySwitch:

```c
struct xswitch {
	mySwitch s;
```

We'll also store copies of the pointer to our switch's GtkWidget as both a GtkWidget and a GtkSwitch, to make things easier for us:

```c
	GtkWidget *widget;
	GtkSwitch *xswitch;
};
```

The good news is that all the work of implementing uiControl is done for you by libui. You only need to worry about destroying your data structure along witht he GtkWidget and implementing the other methods of your interface.

Let's start by writing the code to destroy the switch data structure when the GtkSwitch is destroyed. This will be the GtkSwitch's `::destroy` signal handler implementation. Our `struct xswitch` is passed as the user data:

```c
static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct xswitch *s = (struct xswitch *) data;

	g_free(data);
}
```

We'll connect this when we write our constructor.

Next, let's write the implementation of our `mySwitchOn()` method. Remember that a `struct xswitch` *is* a `mySwitch`, so we simply use a pointer cast to go from the this pointer (the first argument of the method) to our `struct xswitch`:

```c
static int switchOn(mySwitch *ss)
{
	struct xswitch *s = (struct xswitch *) ss;

	return gtk_switch_get_active(s->xswitch) != FALSE;
}
```

Now we need to write the function that creates a new mySwitch. Start by creating a new `struct xswitch`.

```c
mySwitch *newSwitch(void)
{
	struct xswitch *s;

	s = g_new0(struct xswitch, 1);
```

Remember that a `mySwitch` is a `uiControl`, and that a `struct xswitch` is a `mySwitch`. Therefore, a `struct xswitch` is also a `uiControl`. This will be important shortly.

The next step is to actually create the widget. All the work is done for you by the libui function `uiUnixNewControl()`.

The first argument to `uiUnixNewControl()` is a pointer to the `uiControl` that will be filled in with internal data structures and the various uiControl methods. All we need to do is pass in our new `struct xswitch` converted to a `uiControl` with our `uiControl()` conversion macro.

The second argument is the GType that represents the GtkWidget that we want to create. In this case, it's `GTK_TYPE_SWITCH`:

```c
	uiUnixNewControl(uiControl(s), GTK_TYPE_SWITCH,
```

The next argument is a `gboolean` that, if `TRUE`, means the widget should be given a set of scrollbars. You don't have to worry about accessing the GtkScrolledWindow itself or whether you need a viewport; libui will take care of that for you. If you do specify `TRUE`, the next argument is another `gboolean` that, if `TRUE`, adds a border to the GtkScrolledWindow.

For our purposes, however, we don't need scrolling:

```c
		FALSE, FALSE,
```

Finally, we pass in a `NULL`-terminated list of GObject construct properties to give to the new widget. This is exactly like `g_object_new()`. For our purposes, we don't need to pass any, so just pass `NULL`:

```c
		NULL);
```

We now have a fully usable uiControl, but we still need a fully usable mySwitch and a fully usable `struct xswitch`. So let's fill in the rest of the structure.

First, we'll get a handle to the GtkSwitch itself and stuff that into our structure:

```c
	s->widget = GTK_WIDGET(uiControlHandle(uiControl(s)));
	s->xswitch = GTK_SWITCH(s->widget);
```

You may choose to use macros to hide the nested call that gets the value of `s->widget`; the implementation of libui itself uses a macro called `WIDGET()` for this.

Next, let's connect our `onDestroy()` handler from earlier:

```c
	g_signal_connect(s->widget, "destroy", G_CALLBACK(onDestroy), s);
```

Now we need to connect our mySwitch vtable functions. We do this by converting our `struct xswitch` to a `mySwitch` with the `mySwitch()` conversion macro and writing to the vtable entries:

```c
	mySwitch(s)->On = switchOn;
```

And finally we're done, so let's just return our mySwitch:

```c
	return mySwitch(s);
}
```

And that's it! You now have everything you need to build a complete Unix backend uiControl! You can chooose and use the members of your control's backing data structure to your heart's content. All the controls in libui follow the standard set above.

Here's a complete working example showing our `mySwitch` control:

TODO
