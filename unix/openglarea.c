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

struct openGLAreaWidget {
	GtkDrawingArea parent_instance;
	uiOpenGLArea *a;
};

struct openGLAreaWidgetClass {
	GtkDrawingAreaClass parent_class;
};

struct uiOpenGLArea {
	uiUnixControl c;
	uiOpenGLAreaHandler *ah;
	areaEventHandler eh;
	openGLAreaWidget *area;
	GtkWidget *widget;
	uiOpenGLAttributes *attribs;
	GdkDisplay *gdkDisplay;
	Display *display;
	XVisualInfo *visual;
	Colormap colormap;
	GLXContext ctx;
};

static glXSwapIntervalEXTFn uiGLXSwapIntervalEXT = NULL;

G_DEFINE_TYPE(openGLAreaWidget, openGLAreaWidget, GTK_TYPE_DRAWING_AREA)

enum {
	pOpenGLArea = 1,
	nProps,
};

static GParamSpec *pspecOpenGLArea;

// This function guards against potential buffer overflows in the GLX attribute stack.j
static void assign_next_glx_attribute(int *glx_attribs, unsigned *glx_attrib_index, int value)
{
	if (*glx_attrib_index >= GLX_ATTRIBUTE_LIST_SIZE) {
		implbug("Out of GLX attribute list space!");
		return;
	}
	glx_attribs[*glx_attrib_index] = value;
	(*glx_attrib_index)++;
}

static void openGLAreaWidget_init(openGLAreaWidget *aw)
{
}

static void openGLAreaWidget_dispose(GObject *obj)
{
	G_OBJECT_CLASS(openGLAreaWidget_parent_class)->dispose(obj);
}

static void openGLAreaWidget_finalize(GObject *obj)
{
	G_OBJECT_CLASS(openGLAreaWidget_parent_class)->finalize(obj);
}

static void openGLAreaWidget_set_property(GObject *obj, guint prop, const GValue *value, GParamSpec *pspec)
{
	openGLAreaWidget *aw = openGLAreaWidget(obj);

	switch (prop) {
	case pOpenGLArea:
		aw->a = (uiOpenGLArea *)g_value_get_pointer(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop, pspec);
	}
}

static void openGLAreaWidget_get_property(GObject *obj, guint prop, GValue *value, GParamSpec *pspec)
{
	G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop, pspec);
}

static gboolean openGLAreaWidget_draw(GtkWidget *w, cairo_t *cr)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;
	uiOpenGLAreaMakeCurrent(a);
	(*(a->ah->DrawGL))(a->ah, a);
	return FALSE;
}

static gboolean openGLAreaWidget_button_press_event(GtkWidget *w, GdkEventButton *e)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;
	return areaEventHandler_button_press_event(&a->eh, uiControl(a), w, e, 1);
}

static gboolean openGLAreaWidget_button_release_event(GtkWidget *w, GdkEventButton *e)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;
	return areaEventHandler_button_release_event(&a->eh, uiControl(a), w, e, 1);
}

static gboolean openGLAreaWidget_motion_notify_event(GtkWidget *w, GdkEventMotion *e)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;
	return areaEventHandler_motion_notify_event(&a->eh, uiControl(a), w, e, 1);
}

static gboolean openGLAreaWidget_enter_notify_event(GtkWidget *w, GdkEventCrossing *e)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;
	return areaEventHandler_enter_notify_event(&a->eh, uiControl(a), w, e);
}

static gboolean openGLAreaWidget_leave_notify_event(GtkWidget *w, GdkEventCrossing *e)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;
	return areaEventHandler_leave_notify_event(&a->eh, uiControl(a), w, e);
}

static gboolean openGLAreaWidget_key_press_event(GtkWidget *w, GdkEventKey *e)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;
	return areaEventHandler_key_press_event(&a->eh, uiControl(a), w, e);
}

static gboolean openGLAreaWidget_key_release_event(GtkWidget *w, GdkEventKey *e)
{
	openGLAreaWidget *aw = openGLAreaWidget(w);
	uiOpenGLArea *a = aw->a;
	return areaEventHandler_key_release_event(&a->eh, uiControl(a), w, e);
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

	pspecOpenGLArea = g_param_spec_pointer("libui-opengl-area", "libui-opengl-area", "uiOpenGLArea.", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(G_OBJECT_CLASS(class), pOpenGLArea, pspecOpenGLArea);
}

uiUnixControlAllDefaults(uiOpenGLArea)

static pthread_once_t loaded_extensions = PTHREAD_ONCE_INIT;

void load_extensions()
{
	uiGLXSwapIntervalEXT = (glXSwapIntervalEXTFn)glXGetProcAddress((const GLubyte *)"glXSwapIntervalEXT");
}

void uiOpenGLAreaGetSize(uiOpenGLArea *a, int *width, int *height)
{
	GtkAllocation alloc;
	gtk_widget_get_allocation(a->widget, &alloc);
	*width = alloc.width;
	*height = alloc.height;
}

void uiOpenGLAreaQueueRedrawAll(uiOpenGLArea *a)
{
	gtk_widget_queue_draw(a->widget);
}

void uiOpenGLAreaSetSwapInterval(uiOpenGLArea *a, int si)
{
	pthread_once(&loaded_extensions, load_extensions);

	uiOpenGLAreaMakeCurrent(a);
	uiGLXSwapIntervalEXT(a->display, gdk_x11_window_get_xid(gtk_widget_get_window(a->widget)), si);
}

void uiOpenGLAreaMakeCurrent(uiOpenGLArea *a)
{
	glXMakeCurrent(a->display, gdk_x11_window_get_xid(gtk_widget_get_window(a->widget)), a->ctx);
}

void uiOpenGLAreaSwapBuffers(uiOpenGLArea *a)
{
	glXSwapBuffers(a->display, gdk_x11_window_get_xid(gtk_widget_get_window(a->widget)));
}

uiOpenGLArea *uiNewOpenGLArea(uiOpenGLAreaHandler *ah, uiOpenGLAttributes *attribs)
{
	uiOpenGLArea *a;

	uiUnixNewControl(uiOpenGLArea, a);

	a->ah = ah;
	a->eh.eh = &ah->eh;

	a->attribs = malloc(sizeof(*a->attribs));
	memcpy(a->attribs, attribs, sizeof(*a->attribs));

	a->area = openGLAreaWidget(g_object_new(openGLAreaWidgetType, "libui-opengl-area", a, NULL));
	a->widget = GTK_WIDGET(a->area);

	gtk_widget_add_events(a->widget,
		GDK_POINTER_MOTION_MASK |
		GDK_BUTTON_MOTION_MASK |
		GDK_BUTTON_PRESS_MASK |
		GDK_BUTTON_RELEASE_MASK |
		GDK_KEY_PRESS_MASK |
		GDK_KEY_RELEASE_MASK |
		GDK_ENTER_NOTIFY_MASK |
		GDK_LEAVE_NOTIFY_MASK);

	gtk_widget_set_double_buffered(a->widget, FALSE);

	a->gdkDisplay = gtk_widget_get_display(a->widget);
	a->display = gdk_x11_display_get_xdisplay(a->gdkDisplay);
	Window rootWindow = gdk_x11_get_default_root_xwindow();

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
		userbug("Couldn't choose a GLX visual!");

	a->colormap = XCreateColormap(a->display, rootWindow, a->visual->visual, AllocNone);
	if (a->colormap == 0)
		userbug("Couldn't create an X colormap for this OpenGL view!");
	a->ctx = glXCreateContext(a->display, a->visual, NULL, GL_TRUE);
	if (a->ctx == NULL)
		userbug("Couldn't create a GLX context!");

	return a;
}

