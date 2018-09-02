#include "uipriv_unix.h"
#include "areacommon.h"

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

static int ctxErrorOccurred = 0;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev) {
	ctxErrorOccurred = 1;
	return 0;
}

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

typedef struct openGLAreaWidget openGLAreaWidget;
typedef struct openGLAreaWidgetClass openGLAreaWidgetClass;

struct openGLAreaWidget {
	GtkDrawingArea parent_instance;
	uiOpenGLArea *a;
	// construct-only parameters aare not set until after the init() function has returned
	// we need this particular object available during init(), so put it here instead of in uiArea
	// keep a pointer in uiArea for convenience, though
	uiprivClickCounter cc;
};

struct openGLAreaWidgetClass {
	GtkDrawingAreaClass parent_class;
};

struct uiOpenGLArea {
	uiUnixControl c;
	GtkWidget *widget;		// either swidget or areaWidget depending on whether it is scrolling

	GtkWidget *swidget;
	GtkContainer *scontainer;
	GtkScrolledWindow *sw;

	GtkWidget *areaWidget;
	GtkDrawingArea *drawingArea;
	openGLAreaWidget *area;

	uiOpenGLAreaHandler *ah;

	gboolean scrolling;
	int scrollWidth;
	int scrollHeight;

	// note that this is a pointer; see above
	uiprivClickCounter *cc;

	// for user window drags
	GdkEventButton *dragevent;

	// ABOVE IS EQUIVALENT TO uiArea

	uiOpenGLAttributes *attribs;
	GdkDisplay *gdkDisplay;
	Display *display;
	XVisualInfo *visual;
	GLXContext ctx;
	int initialized;
	int supportsSwapInterval;
};

G_DEFINE_TYPE(openGLAreaWidget, openGLAreaWidget, GTK_TYPE_DRAWING_AREA)

static gboolean openGLAreaWidget_draw(GtkWidget *w, cairo_t *cr)
{
	uiOpenGLArea *a = openGLAreaWidget(w)->a;

	double width, height;
	uiprivLoadAreaSize((uiArea*) a, &width, &height);
	uiOpenGLAreaMakeCurrent(a);

	if(!a->initialized){
		(*(a->ah->InitGL))(a->ah, a);
		a->initialized = 1;
	}

	(*(a->ah->DrawGL))(a->ah, a, width, height);
	return FALSE;
}

uiprivUnixAreaAllDefaultsExceptDraw(openGLAreaWidget)

enum {
	pOpenGLArea = 1,
	nProps,
};

static GParamSpec *pspecOpenGLArea;

static void openGLAreaWidget_class_init(openGLAreaWidgetClass *class)
{
	G_OBJECT_CLASS(class)->dispose = openGLAreaWidget_dispose;
	G_OBJECT_CLASS(class)->finalize = openGLAreaWidget_finalize;
	G_OBJECT_CLASS(class)->set_property = openGLAreaWidget_set_property;
	G_OBJECT_CLASS(class)->get_property = openGLAreaWidget_get_property;

	GTK_WIDGET_CLASS(class)->size_allocate = openGLAreaWidget_size_allocate;
	GTK_WIDGET_CLASS(class)->draw = openGLAreaWidget_draw;
	GTK_WIDGET_CLASS(class)->get_preferred_height = openGLAreaWidget_get_preferred_height;
	GTK_WIDGET_CLASS(class)->get_preferred_width = openGLAreaWidget_get_preferred_width;
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
	uiAreaBeginUserWindowMove((uiArea*) a);
}

void uiOpenGLAreaBeginUserWindowResize(uiOpenGLArea *a, uiWindowResizeEdge edge)
{
	uiAreaBeginUserWindowResize((uiArea*) a, edge);
}

static void initContext(uiOpenGLArea *a) {
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
			uiprivUserBug("Couldn't choose a GLX visual (glXGetVisualFromFBConfig)!");

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
			uiprivUserBug("Couldn't choose a GLX visual (glXChooseVisual)!");
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
			if (a->ctx == NULL)
				uiprivUserBug("Couldn't create a GLX context (glXCreateNewContext)!");
		}
	} else {
		a->ctx = glXCreateContext(a->display, a->visual, NULL, GL_TRUE);
		if (a->ctx == NULL)
			uiprivUserBug("Couldn't create a GLX context (glXCreateContext)!");
	}


	XSetErrorHandler(oldHandler);

	a->supportsSwapInterval = GLXExtensionSupported(a->display, screen_number, "GLX_EXT_swap_control");
}

uiOpenGLArea *uiNewOpenGLArea(uiOpenGLAreaHandler *ah, uiOpenGLAttributes *attribs)
{
	uiOpenGLArea *a;

	uiUnixNewControl(uiOpenGLArea, a);

	a->ah = ah;
	a->initialized = 0;
	a->scrolling = FALSE;

	a->attribs = uiprivAlloc(sizeof(*a->attribs), "uiOpenGLAttributes[]");
	memcpy(a->attribs, attribs, sizeof(*a->attribs));

	a->areaWidget = GTK_WIDGET(g_object_new(openGLAreaWidgetType,
		"libui-opengl-area", a,
		NULL));
	a->drawingArea = GTK_DRAWING_AREA(a->areaWidget);
	a->area = openGLAreaWidget(a->areaWidget);

	a->widget = a->areaWidget;

	gtk_widget_set_double_buffered(a->areaWidget, FALSE);

	initContext(a);

	return a;
}
