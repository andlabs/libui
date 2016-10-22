struct uiWindow {
//	void *onClosingData;
	void (*onPositionChanged)(uiWindow *, void *);
	void *onPositionChangedData;
	gboolean changingPosition;
//	void (*onContentSizeChanged)(uiWindow *, void *);
};

// static gboolean onClosing(GtkWidget *win, GdkEvent *e, gpointer data)

static gboolean onConfigure(GtkWidget *win, GdkEvent *e, gpointer data)
{
	uiWindow *w = uiWindow(data);

	// there doesn't seem to be a way to determine if only moving or only resizing is happening :/
	if (w->changingPosition)
		w->changingPosition = FALSE;
	else
		(*(w->onPositionChanged))(w, w->onPositionChangedData);
	// always continue handling
	return FALSE;
}

// static void onSizeAllocate(GtkWidget *widget, GdkRectangle *allocation, gpointer data)

// static int defaultOnClosing(uiWindow *w, void *data)

static void defaultOnPositionContentSizeChanged(uiWindow *w, void *data)
{
	// do nothing
}

// static void uiWindowDestroy(uiControl *c)

// void uiWindowSetTitle(uiWindow *w, const char *title)

// TODO allow specifying either as NULL on all platforms
void uiWindowPosition(uiWindow *w, int *x, int *y)
{
	gint rx, ry;

	gtk_window_get_position(w->window, &rx, &ry);
	*x = rx;
	*y = ry;
}

void uiWindowSetPosition(uiWindow *w, int x, int y)
{
	w->changingPosition = TRUE;
	gtk_window_move(w->window, x, y);
	// gtk_window_move() is asynchronous
	// we need to wait for a configure-event
	// thanks to hergertme in irc.gimp.net/#gtk+
	while (w->changingPosition)
		if (!uiMainStep(1))
			break;		// stop early if uiQuit() called
}

void uiWindowCenter(uiWindow *w)
{
	gint x, y;
	GtkAllocation winalloc;
	GdkWindow *gdkwin;
	GdkScreen *screen;
	GdkRectangle workarea;

	gtk_widget_get_allocation(w->widget, &winalloc);
	gdkwin = gtk_widget_get_window(w->widget);
	screen = gdk_window_get_screen(gdkwin);
	gdk_screen_get_monitor_workarea(screen,
		gdk_screen_get_monitor_at_window(screen, gdkwin),
		&workarea);

	x = (workarea.width - winalloc.width) / 2;
	y = (workarea.height - winalloc.height) / 2;
	// TODO move up slightly? see what Mutter or GNOME Shell or GNOME Terminal do(es)?
	uiWindowSetPosition(w, x, y);
}

// TODO this and size changed get set during uiWindowDestroy
void uiWindowOnPositionChanged(uiWindow *w, void (*f)(uiWindow *, void *), void *data)
{
	w->onPositionChanged = f;
	w->onPositionChangedData = data;
}

// void uiWindowContentSize(uiWindow *w, int *width, int *height)

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
//	g_signal_connect(w->widget, "delete-event", G_CALLBACK(onClosing), w);
	g_signal_connect(w->widget, "configure-event", G_CALLBACK(onConfigure), w);
//	g_signal_connect(w->childHolderWidget, "size-allocate", G_CALLBACK(onSizeAllocate), w);
//	uiWindowOnClosing(w, defaultOnClosing, NULL);
	uiWindowOnPositionChanged(w, defaultOnPositionContentSizeChanged, NULL);
//	uiWindowOnContentSizeChanged(w, defaultOnPositionContentSizeChanged, NULL);
}
