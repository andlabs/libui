// 29 may 2016

#include "uipriv_unix.h"
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gdk/gdkx.h>
#include <pthread.h>
#include <stdlib.h>

#define openGLAreaWidgetType (openGLAreaWidget_get_type())
#define openGLAreaWidget(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), openGLAreaWidgetType, openGLAreaWidget))
#define isOpenGLAreaWidget(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), openGLAreaWidgetType, openGLAreaWidgetClass))
#define openGLAreaWidgetClass(class) (G_TYPE_CHECK_CLASS_CAST((class), openGLAreaWidgetType, openGLAreaWidgetClass))
#define isOpenGLAreaWidgetClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), openGLAreaWidget))
#define getOpenGLAreaWidgetClass(class) (G_TYPE_INSTANCE_GET_CLASS((class), openGLAreaWidgetType, openGLAreaWidgetClass))

#define GLX_ATTRIBUTE_LIST_SIZE	256

typedef struct openGLAreaWidget openGLAreaWidget;
typedef struct openGLAreaWidgetClass openGLAreaWidgetClass;

typedef void (*glXSwapIntervalEXTFn)(Display *, GLXDrawable, int);
typedef GLXContext (*glXCreateContextAttribsARBFn)(Display *, GLXFBConfig, GLXContext, Bool, const int *);

static glXSwapIntervalEXTFn uiGLXSwapIntervalEXT = NULL;
static glXCreateContextAttribsARBFn uiGLXCreateContextAttribsARB = NULL;

static int GLXExtensionSupported(Display *display, int screen_number, const char *extension_name)
{
	if (strstr(glXQueryExtensionsString(display, screen_number), extension_name) == NULL)
		return 0;

	return 1;
}

static pthread_once_t loaded_extensions = PTHREAD_ONCE_INIT;
void load_extensions()
{
	uiGLXSwapIntervalEXT = (glXSwapIntervalEXTFn)glXGetProcAddress((const GLubyte *)"glXSwapIntervalEXT");
	uiGLXCreateContextAttribsARB = (glXCreateContextAttribsARBFn)glXGetProcAddress((const GLubyte *)"glXCreateContextAttribsARB");
}

struct openGLAreaWidget {
	GtkDrawingArea parent_instance;
	uiOpenGLArea *a;
	uiprivClickCounter cc;
};

struct openGLAreaWidgetClass {
	GtkDrawingAreaClass parent_class;
};

struct uiOpenGLArea {
	uiUnixControl c;
	uiOpenGLAreaHandler *ah;
	openGLAreaWidget *area;
	GtkWidget *widget;
	uiOpenGLAttributes *attribs;
	GdkDisplay *gdkDisplay;
	Display *display;
	XVisualInfo *visual;
	GLXContext ctx;
	int initialized;
	uiprivClickCounter *cc;
	GdkEventButton *dragevent;
	int supportsSwapInterval;
};

G_DEFINE_TYPE(openGLAreaWidget, openGLAreaWidget, GTK_TYPE_DRAWING_AREA)

// This function guards against potential buffer overflows in the GLX attribute stack.
static void assign_next_glx_attribute(int *glx_attribs, unsigned *glx_attrib_index, int value)
{
	if (*glx_attrib_index >= GLX_ATTRIBUTE_LIST_SIZE) {
		uiprivImplBug("Out of GLX attribute list space!");
		return;
	}
	glx_attribs[*glx_attrib_index] = value;
	(*glx_attrib_index)++;
}

static void openGLAreaWidget_init(openGLAreaWidget *aw)
{
	// for events
	gtk_widget_add_events(GTK_WIDGET(aw),
		GDK_POINTER_MOTION_MASK |
		GDK_BUTTON_MOTION_MASK |
		GDK_BUTTON_PRESS_MASK |
		GDK_BUTTON_RELEASE_MASK |
		GDK_KEY_PRESS_MASK |
		GDK_KEY_RELEASE_MASK |
		GDK_ENTER_NOTIFY_MASK |
		GDK_LEAVE_NOTIFY_MASK);

	gtk_widget_set_can_focus(GTK_WIDGET(aw), TRUE);

	uiprivClickCounterReset(&(aw->cc));
}

static void openGLAreaWidget_dispose(GObject *obj)
{
	G_OBJECT_CLASS(openGLAreaWidget_parent_class)->dispose(obj);
}

static void openGLAreaWidget_finalize(GObject *obj)
{
	G_OBJECT_CLASS(openGLAreaWidget_parent_class)->finalize(obj);
}

static void loadAreaSize(uiOpenGLArea *a, double *width, double *height)
{
	GtkAllocation allocation;

	gtk_widget_get_allocation(a->widget, &allocation);
	// these are already in drawing space coordinates
	// for drawing, the size of drawing space has the same value as the widget allocation
	// thanks to tristan in irc.gimp.net/#gtk+
	*width = allocation.width;
	*height = allocation.height;
}

static gboolean openGLAreaWidget_draw(GtkWidget *w, cairo_t *cr)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;

	double width, height;
	loadAreaSize(a, &width, &height);
	uiOpenGLAreaMakeCurrent(a);

	if(!a->initialized){
		(*(a->ah->InitGL))(a->ah, a);
		a->initialized = 1;
	}

	(*(a->ah->DrawGL))(a->ah, a, width, height);
	return FALSE;
}

static guint translateModifiers(guint state, GdkWindow *window)
{
	GdkModifierType statetype;

	// GDK doesn't initialize the modifier flags fully; we have to explicitly tell it to (thanks to Daniel_S and daniels (two different people) in irc.gimp.net/#gtk+)
	statetype = state;
	gdk_keymap_add_virtual_modifiers(
		gdk_keymap_get_for_display(gdk_window_get_display(window)),
		&statetype);
	return statetype;
}

static uiModifiers toModifiers(guint state)
{
	uiModifiers m;

	m = 0;
	if ((state & GDK_CONTROL_MASK) != 0)
		m |= uiModifierCtrl;
	if ((state & GDK_META_MASK) != 0)
		m |= uiModifierAlt;
	if ((state & GDK_MOD1_MASK) != 0)		// GTK+ itself requires this to be Alt (just read through gtkaccelgroup.c)
		m |= uiModifierAlt;
	if ((state & GDK_SHIFT_MASK) != 0)
		m |= uiModifierShift;
	if ((state & GDK_SUPER_MASK) != 0)
		m |= uiModifierSuper;
	return m;
}

// capture on drag is done automatically on GTK+
static void finishMouseEvent(uiOpenGLArea *a, uiAreaMouseEvent *me, guint mb, gdouble x, gdouble y, guint state, GdkWindow *window)
{
	// on GTK+, mouse buttons 4-7 are for scrolling; if we got here, that's a mistake
	if (mb >= 4 && mb <= 7)
		return;
	// if the button ID >= 8, continue counting from 4, as in the MouseEvent spec
	if (me->Down >= 8)
		me->Down -= 4;
	if (me->Up >= 8)
		me->Up -= 4;

	state = translateModifiers(state, window);
	me->Modifiers = toModifiers(state);

	// the mb != # checks exclude the Up/Down button from Held
	me->Held1To64 = 0;
	if (mb != 1 && (state & GDK_BUTTON1_MASK) != 0)
		me->Held1To64 |= 1 << 0;
	if (mb != 2 && (state & GDK_BUTTON2_MASK) != 0)
		me->Held1To64 |= 1 << 1;
	if (mb != 3 && (state & GDK_BUTTON3_MASK) != 0)
		me->Held1To64 |= 1 << 2;
	// don't check GDK_BUTTON4_MASK or GDK_BUTTON5_MASK because those are for the scrolling buttons mentioned above
	// GDK expressly does not support any more buttons in the GdkModifierType; see https://git.gnome.org/browse/gtk+/tree/gdk/x11/gdkdevice-xi2.c#n763 (thanks mclasen in irc.gimp.net/#gtk+)

	// these are already in drawing space coordinates
	// the size of drawing space has the same value as the widget allocation
	// thanks to tristan in irc.gimp.net/#gtk+
	me->X = x;
	me->Y = y;

	loadAreaSize(a, &(me->AreaWidth), &(me->AreaHeight));

	(*(a->ah->MouseEvent))(a->ah, a, me);
}

static gboolean openGLAreaWidget_button_press_event(GtkWidget *w, GdkEventButton *e)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;
	gint maxTime, maxDistance;
	GtkSettings *settings;
	uiAreaMouseEvent me;

	// clicking doesn't automatically transfer keyboard focus; we must do so manually (thanks tristan in irc.gimp.net/#gtk+)
	gtk_widget_grab_focus(w);

	// we handle multiple clicks ourselves here, in the same way as we do on Windows
	if (e->type != GDK_BUTTON_PRESS)
		// ignore GDK's generated double-clicks and beyond
		return GDK_EVENT_PROPAGATE;
	settings = gtk_widget_get_settings(w);
	g_object_get(settings,
		"gtk-double-click-time", &maxTime,
		"gtk-double-click-distance", &maxDistance,
		NULL);
	// don't unref settings; it's transfer-none (thanks gregier in irc.gimp.net/#gtk+)
	// e->time is guint32
	// e->x and e->y are floating-point; just make them 32-bit integers
	// maxTime and maxDistance... are gint, which *should* fit, hopefully...
	me.Count = uiprivClickCounterClick(a->cc, me.Down,
		e->x, e->y,
		e->time, maxTime,
		maxDistance, maxDistance);

	me.Down = e->button;
	me.Up = 0;

	// and set things up for window drags
	a->dragevent = e;
	finishMouseEvent(a, &me, e->button, e->x, e->y, e->state, e->window);
	a->dragevent = NULL;
	return GDK_EVENT_PROPAGATE;
}

static gboolean openGLAreaWidget_button_release_event(GtkWidget *w, GdkEventButton *e)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;
	uiAreaMouseEvent me;

	me.Down = 0;
	me.Up = e->button;
	me.Count = 0;
	finishMouseEvent(a, &me, e->button, e->x, e->y, e->state, e->window);
	return GDK_EVENT_PROPAGATE;
}

static gboolean openGLAreaWidget_motion_notify_event(GtkWidget *w, GdkEventMotion *e)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;
	uiAreaMouseEvent me;

	me.Down = 0;
	me.Up = 0;
	me.Count = 0;
	finishMouseEvent(a, &me, 0, e->x, e->y, e->state, e->window);
	return GDK_EVENT_PROPAGATE;
}

// we want switching away from the control to reset the double-click counter, like with WM_ACTIVATE on Windows
// according to tristan in irc.gimp.net/#gtk+, doing this on both enter-notify-event and leave-notify-event is correct (and it seems to be true in my own tests; plus the events DO get sent when switching programs with the keyboard (just pointing that out))
static gboolean onCrossing(openGLAreaWidget *aw, int left)
{
	uiOpenGLArea *a = aw->a;

	(*(a->ah->MouseCrossed))(a->ah, a, left);
	uiprivClickCounterReset(a->cc);
	return GDK_EVENT_PROPAGATE;
}

static gboolean openGLAreaWidget_enter_notify_event(GtkWidget *w, GdkEventCrossing *e)
{
	return onCrossing(openGLAreaWidget(w), 0);
}

static gboolean openGLAreaWidget_leave_notify_event(GtkWidget *w, GdkEventCrossing *e)
{
	return onCrossing(openGLAreaWidget(w), 1);
}

// note: there is no equivalent to WM_CAPTURECHANGED on GTK+; there literally is no way to break a grab like that (at least not on X11 and Wayland)
// even if I invoke the task switcher and switch processes, the mouse grab will still be held until I let go of all buttons
// therefore, no DragBroken()

// we use GDK_KEY_Print as a sentinel because libui will never support the print screen key; that key belongs to the user

static const struct {
	guint keyval;
	uiExtKey extkey;
} extKeys[] = {
	{ GDK_KEY_Escape, uiExtKeyEscape },
	{ GDK_KEY_Insert, uiExtKeyInsert },
	{ GDK_KEY_Delete, uiExtKeyDelete },
	{ GDK_KEY_Home, uiExtKeyHome },
	{ GDK_KEY_End, uiExtKeyEnd },
	{ GDK_KEY_Page_Up, uiExtKeyPageUp },
	{ GDK_KEY_Page_Down, uiExtKeyPageDown },
	{ GDK_KEY_Up, uiExtKeyUp },
	{ GDK_KEY_Down, uiExtKeyDown },
	{ GDK_KEY_Left, uiExtKeyLeft },
	{ GDK_KEY_Right, uiExtKeyRight },
	{ GDK_KEY_F1, uiExtKeyF1 },
	{ GDK_KEY_F2, uiExtKeyF2 },
	{ GDK_KEY_F3, uiExtKeyF3 },
	{ GDK_KEY_F4, uiExtKeyF4 },
	{ GDK_KEY_F5, uiExtKeyF5 },
	{ GDK_KEY_F6, uiExtKeyF6 },
	{ GDK_KEY_F7, uiExtKeyF7 },
	{ GDK_KEY_F8, uiExtKeyF8 },
	{ GDK_KEY_F9, uiExtKeyF9 },
	{ GDK_KEY_F10, uiExtKeyF10 },
	{ GDK_KEY_F11, uiExtKeyF11 },
	{ GDK_KEY_F12, uiExtKeyF12 },
	// numpad numeric keys and . are handled in events.c
	{ GDK_KEY_KP_Enter, uiExtKeyNEnter },
	{ GDK_KEY_KP_Add, uiExtKeyNAdd },
	{ GDK_KEY_KP_Subtract, uiExtKeyNSubtract },
	{ GDK_KEY_KP_Multiply, uiExtKeyNMultiply },
	{ GDK_KEY_KP_Divide, uiExtKeyNDivide },
	{ GDK_KEY_Print, 0 },
};

static const struct {
	guint keyval;
	uiModifiers mod;
} modKeys[] = {
	{ GDK_KEY_Control_L, uiModifierCtrl },
	{ GDK_KEY_Control_R, uiModifierCtrl },
	{ GDK_KEY_Alt_L, uiModifierAlt },
	{ GDK_KEY_Alt_R, uiModifierAlt },
	{ GDK_KEY_Meta_L, uiModifierAlt },
	{ GDK_KEY_Meta_R, uiModifierAlt },
	{ GDK_KEY_Shift_L, uiModifierShift },
	{ GDK_KEY_Shift_R, uiModifierShift },
	{ GDK_KEY_Super_L, uiModifierSuper },
	{ GDK_KEY_Super_R, uiModifierSuper },
	{ GDK_KEY_Print, 0 },
};

static int openGLAreaKeyEvent(uiOpenGLArea *a, int up, GdkEventKey *e)
{
	uiAreaKeyEvent ke;
	guint state;
	int i;

	ke.Key = 0;
	ke.ExtKey = 0;
	ke.Modifier = 0;

	state = translateModifiers(e->state, e->window);
	ke.Modifiers = toModifiers(state);

	ke.Up = up;

	for (i = 0; extKeys[i].keyval != GDK_KEY_Print; i++)
		if (extKeys[i].keyval == e->keyval) {
			ke.ExtKey = extKeys[i].extkey;
			goto keyFound;
		}

	for (i = 0; modKeys[i].keyval != GDK_KEY_Print; i++)
		if (modKeys[i].keyval == e->keyval) {
			ke.Modifier = modKeys[i].mod;
			// don't include the modifier in ke.Modifiers
			ke.Modifiers &= ~ke.Modifier;
			goto keyFound;
		}

	if (uiprivFromScancode(e->hardware_keycode - 8, &ke))
		goto keyFound;

	// no supported key found; treat as unhandled
	return 0;

keyFound:
	return (*(a->ah->KeyEvent))(a->ah, a, &ke);
}

static gboolean openGLAreaWidget_key_press_event(GtkWidget *w, GdkEventKey *e)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;

	if (openGLAreaKeyEvent(a, 0, e))
		return GDK_EVENT_STOP;
	return GDK_EVENT_PROPAGATE;
}

static gboolean openGLAreaWidget_key_release_event(GtkWidget *w, GdkEventKey *e)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;

	if (openGLAreaKeyEvent(a, 1, e))
		return GDK_EVENT_STOP;
	return GDK_EVENT_PROPAGATE;
}

enum {
	pOpenGLArea = 1,
	nProps,
};

static GParamSpec *pspecOpenGLArea;

static void openGLAreaWidget_set_property(GObject *obj, guint prop, const GValue *value, GParamSpec *pspec)
{
	openGLAreaWidget *aw = openGLAreaWidget(obj);

	switch (prop) {
	case pOpenGLArea:
		aw->a = (uiOpenGLArea *)g_value_get_pointer(value);
		aw->a->cc = &(aw->cc);
		return;
	}
	G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop, pspec);
}

static void openGLAreaWidget_get_property(GObject *obj, guint prop, GValue *value, GParamSpec *pspec)
{
	G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop, pspec);
}

static void openGLAreaWidget_class_init(openGLAreaWidgetClass *class)
{
	G_OBJECT_CLASS(class)->dispose = openGLAreaWidget_dispose;
	G_OBJECT_CLASS(class)->finalize = openGLAreaWidget_finalize;
	G_OBJECT_CLASS(class)->set_property = openGLAreaWidget_set_property;
	G_OBJECT_CLASS(class)->get_property = openGLAreaWidget_get_property;

	GTK_WIDGET_CLASS(class)->draw = openGLAreaWidget_draw;
	GTK_WIDGET_CLASS(class)->button_press_event = openGLAreaWidget_button_press_event;
	GTK_WIDGET_CLASS(class)->button_release_event = openGLAreaWidget_button_release_event;
	GTK_WIDGET_CLASS(class)->motion_notify_event = openGLAreaWidget_motion_notify_event;
	GTK_WIDGET_CLASS(class)->enter_notify_event = openGLAreaWidget_enter_notify_event;
	GTK_WIDGET_CLASS(class)->leave_notify_event = openGLAreaWidget_leave_notify_event;
	GTK_WIDGET_CLASS(class)->key_press_event = openGLAreaWidget_key_press_event;
	GTK_WIDGET_CLASS(class)->key_release_event = openGLAreaWidget_key_release_event;

	pspecOpenGLArea = g_param_spec_pointer("libui-opengl-area",
		"libui-opengl-area",
		"uiOpenGLArea.",
		G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(G_OBJECT_CLASS(class), pOpenGLArea, pspecOpenGLArea);
}

uiUnixControlAllDefaultsExceptDestroy(uiOpenGLArea)

static void uiOpenGLAreaDestroy(uiControl *c) {
	uiOpenGLArea *a = uiOpenGLArea(c);
	glXMakeCurrent(a->display, None, NULL);

	uiprivFree(a->attribs);
	XFree(a->visual);
	g_object_unref(a->widget);
	uiFreeControl(uiControl(a));
}

void uiOpenGLAreaQueueRedrawAll(uiOpenGLArea *a)
{
	gtk_widget_queue_draw(a->widget);
}

void uiOpenGLAreaSetVSync(uiOpenGLArea *a, int v)
{
	uiOpenGLAreaMakeCurrent(a);
	if(uiGLXSwapIntervalEXT != NULL && a->supportsSwapInterval)
		uiGLXSwapIntervalEXT(a->display, gdk_x11_window_get_xid(gtk_widget_get_window(a->widget)), v);
	else {
		// TODO handle missing extension
	}
}

void uiOpenGLAreaMakeCurrent(uiOpenGLArea *a)
{
	// TODO glXMakeContextCurrent?
	glXMakeCurrent(a->display, gdk_x11_window_get_xid(gtk_widget_get_window(a->widget)), a->ctx);
}

void uiOpenGLAreaSwapBuffers(uiOpenGLArea *a)
{
	glXSwapBuffers(a->display, gdk_x11_window_get_xid(gtk_widget_get_window(a->widget)));
}

void uiOpenGLAreaBeginUserWindowMove(uiOpenGLArea *a)
{
	GtkWidget *toplevel;

	if (a->dragevent == NULL)
		uiprivUserBug("cannot call uiAreaBeginUserWindowMove() outside of a Mouse() with Down != 0");
	// TODO don't we have a libui function for this? did I scrap it?
	// TODO widget or areaWidget?
	toplevel = gtk_widget_get_toplevel(a->widget);
	if (toplevel == NULL) {
		// TODO
		return;
	}
	// the docs say to do this
	if (!gtk_widget_is_toplevel(toplevel)) {
		// TODO
		return;
	}
	if (!GTK_IS_WINDOW(toplevel)) {
		// TODO
		return;
	}
	gtk_window_begin_move_drag(GTK_WINDOW(toplevel),
		a->dragevent->button,
		a->dragevent->x_root,		// TODO are these correct?
		a->dragevent->y_root,
		a->dragevent->time);
}

static const GdkWindowEdge edges[] = {
	[uiWindowResizeEdgeLeft] = GDK_WINDOW_EDGE_WEST,
	[uiWindowResizeEdgeTop] = GDK_WINDOW_EDGE_NORTH,
	[uiWindowResizeEdgeRight] = GDK_WINDOW_EDGE_EAST,
	[uiWindowResizeEdgeBottom] = GDK_WINDOW_EDGE_SOUTH,
	[uiWindowResizeEdgeTopLeft] = GDK_WINDOW_EDGE_NORTH_WEST,
	[uiWindowResizeEdgeTopRight] = GDK_WINDOW_EDGE_NORTH_EAST,
	[uiWindowResizeEdgeBottomLeft] = GDK_WINDOW_EDGE_SOUTH_WEST,
	[uiWindowResizeEdgeBottomRight] = GDK_WINDOW_EDGE_SOUTH_EAST,
};

void uiOpenGLAreaBeginUserWindowResize(uiOpenGLArea *a, uiWindowResizeEdge edge)
{
	GtkWidget *toplevel;

	if (a->dragevent == NULL)
		uiprivUserBug("cannot call uiAreaBeginUserWindowResize() outside of a Mouse() with Down != 0");
	// TODO don't we have a libui function for this? did I scrap it?
	// TODO widget or areaWidget?
	toplevel = gtk_widget_get_toplevel(a->widget);
	if (toplevel == NULL) {
		// TODO
		return;
	}
	// the docs say to do this
	if (!gtk_widget_is_toplevel(toplevel)) {
		// TODO
		return;
	}
	if (!GTK_IS_WINDOW(toplevel)) {
		// TODO
		return;
	}
	gtk_window_begin_resize_drag(GTK_WINDOW(toplevel),
		edges[edge],
		a->dragevent->button,
		a->dragevent->x_root,		// TODO are these correct?
		a->dragevent->y_root,
		a->dragevent->time);
}

static int ctxErrorOccurred = 0;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev) {
	ctxErrorOccurred = 1;
	return 0;
}

uiOpenGLArea *uiNewOpenGLArea(uiOpenGLAreaHandler *ah, uiOpenGLAttributes *attribs)
{
	uiOpenGLArea *a;

	uiUnixNewControl(uiOpenGLArea, a);

	a->ah = ah;
	a->initialized = 0;

	a->attribs = uiprivAlloc(sizeof(*a->attribs), "uiOpenGLAttributes[]");
	memcpy(a->attribs, attribs, sizeof(*a->attribs));

	a->area = openGLAreaWidget(g_object_new(openGLAreaWidgetType,
		"libui-opengl-area", a,
		NULL));
	a->widget = GTK_WIDGET(a->area);

	gtk_widget_set_double_buffered(a->widget, FALSE);

	a->gdkDisplay = gtk_widget_get_display(a->widget);
	a->display = gdk_x11_display_get_xdisplay(a->gdkDisplay);
	int screen_number = gdk_x11_screen_get_screen_number(gdk_display_get_default_screen(a->gdkDisplay));

	pthread_once(&loaded_extensions, load_extensions);

	//   Different versions of GLX API use rather different attributes lists, see
	//   the following URLs:
	//
	//   - <= 1.2: http://www.opengl.org/sdk/docs/man/xhtml/glXChooseVisual.xml
	//   - >= 1.3: http://www.opengl.org/sdk/docs/man/xhtml/glXChooseFBConfig.xml
	//
	//   Notice in particular that
	//   - GLX_RGBA is boolean attribute in the old version of the API but a
	//     value of GLX_RENDER_TYPE in the new one
	//   - Boolean attributes such as GLX_DOUBLEBUFFER don't take values in the
	//     old version but must be followed by True or False in the new one.

	// /Users/niklas/development/cmake/hugin/mac/ExternalPrograms/repository/wxWidgets-3.0.3/src/unix/glx11.cpp

	// /Users/niklas/development/cmake/hugin/mac/ExternalPrograms/repository/wxWidgets-3.0.3/src/unix/glx11.cpp
	// GLXContext glXCreateContextAttribsARB(Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list);
	// GLX_ARB_create_context &
	// GLX_ARB_create_context_profile

	// GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB (in GLX_ARB_create_context_robustness)


	// www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)


	// GLX Version 1.3 introduces several sweeping changes, starting with the new
	// GLXFBConfig data structure, which describes the GLX framebuffer configuration
	// (including the depth of the color buffer components, and the types, quantities,
	// and sizes of the depth, stencil, accumulation, and auxiliary buffers). The
	// GLXFBConfig structure describes these framebuffer attributes for a GLXDrawable
	// rendering surface. (In X, a rendering surface is called a Drawable.)

	// Install a X error handler, so as to the app doesn't exit (without
	// even a warning) if GL >= 3.0 context creation fails
	ctxErrorOccurred = 0;
	int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

	int glx_major, glx_minor;

	if (!glXQueryVersion(a->display, &glx_major, &glx_minor)) {
		//TODO continue?
		uiprivUserBug("Couldn't query GLX version");
	}

	int isGLX13OrNewer = glx_major >= 1 && glx_minor >= 3;

	GLXFBConfig *fbconfig = NULL;
	if (isGLX13OrNewer) {
		int glx_attribs[GLX_ATTRIBUTE_LIST_SIZE];
		unsigned glx_attrib_index = 0;
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_LEVEL);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, 0);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_RENDER_TYPE);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_RGBA_BIT);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_DOUBLEBUFFER);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->DoubleBuffer ? True : False);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_STEREO);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->Stereo ? True : False);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_RED_SIZE);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->RedBits);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_GREEN_SIZE);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->GreenBits);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_BLUE_SIZE);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->BlueBits);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_ALPHA_SIZE);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->AlphaBits);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_DEPTH_SIZE);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->DepthBits);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_STENCIL_SIZE);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->StencilBits);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, None);

		int num;
		fbconfig = glXChooseFBConfig(a->display, screen_number, glx_attribs, &num);

		if (fbconfig == NULL)
			uiprivUserBug("Couldn't choose a GLX frame buffer configuration!");

		a->visual = glXGetVisualFromFBConfig(a->display, *fbconfig);
		if (a->visual == NULL)
			uiprivUserBug("Couldn't choose a GLX visual!");

	} else {
		int glx_attribs[GLX_ATTRIBUTE_LIST_SIZE];
		unsigned glx_attrib_index = 0;
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_LEVEL);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, 0);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_RGBA);
		if (a->attribs->DoubleBuffer)
			assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_DOUBLEBUFFER);
		if (a->attribs->Stereo)
			assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_STEREO);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_RED_SIZE);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->RedBits);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_GREEN_SIZE);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->GreenBits);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_BLUE_SIZE);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->BlueBits);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_ALPHA_SIZE);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->AlphaBits);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_DEPTH_SIZE);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->DepthBits);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, GLX_STENCIL_SIZE);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, a->attribs->StencilBits);
		assign_next_glx_attribute(glx_attribs, &glx_attrib_index, None);

		a->visual = glXChooseVisual(a->display, 0, glx_attribs);
		if (a->visual == NULL)
			uiprivUserBug("Couldn't choose a GLX visual!");
	}

	if(isGLX13OrNewer && fbconfig != NULL){
		if(GLXExtensionSupported(a->display, screen_number, "GLX_ARB_create_context") &&
		   GLXExtensionSupported(a->display, screen_number, "GLX_ARB_create_context_profile")) {
			int context_attribs[] = {
				GLX_CONTEXT_MAJOR_VERSION_ARB, a->attribs->MajorVersion,
				GLX_CONTEXT_MINOR_VERSION_ARB, a->attribs->MinorVersion,
				GLX_CONTEXT_FLAGS_ARB,
					a->attribs->DebugContext ?  GLX_CONTEXT_DEBUG_BIT_ARB : 0 |
					a->attribs->ForwardCompat ?  GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB : 0,
				GLX_CONTEXT_PROFILE_MASK_ARB,
					a->attribs->CompatProfile == uiOpenGLDontCare ? 0 :
					(
						a->attribs->CompatProfile ? 
							GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB :
							GLX_CONTEXT_CORE_PROFILE_BIT_ARB
					),
				None
			};

			// OpenGL 3 is only availble using this function
			a->ctx = uiGLXCreateContextAttribsARB(a->display, *fbconfig, 0, True, context_attribs);
			XFree(fbconfig);

			XSync(a->display, False);

			if (ctxErrorOccurred || !a->ctx) {
				//TODO how to handle error ? retry with lower version (1.0)
				uiprivUserBug("Couldn't create a GLX (maybe your specified version isn't supported)!");
			}
		} else {
			a->ctx = glXCreateNewContext(a->display, *fbconfig, GLX_RGBA_TYPE, 0, True);
		}
	} else {
		a->ctx = glXCreateContext(a->display, a->visual, NULL, GL_TRUE);
	}

	if (a->ctx == NULL)
		uiprivUserBug("Couldn't create a GLX context!");

	XSetErrorHandler(oldHandler);

	a->supportsSwapInterval = GLXExtensionSupported(a->display, screen_number, "GLX_EXT_swap_control");

	return a;
}
