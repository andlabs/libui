typedef struct areaWidget areaWidget;
typedef struct areaWidgetClass areaWidgetClass;

struct areaWidget {
	GtkDrawingArea parent_instance;
	uiArea *a;
	// construct-only parameters aare not set until after the init() function has returned
	// we need this particular object available during init(), so put it here instead of in uiArea
	// keep a pointer in uiArea for convenience, though
	uiprivClickCounter cc;
};

struct areaWidgetClass {
	GtkDrawingAreaClass parent_class;
};

struct uiArea {
	uiUnixControl c;
	GtkWidget *widget;		// either swidget or areaWidget depending on whether it is scrolling

	GtkWidget *swidget;
	GtkContainer *scontainer;
	GtkScrolledWindow *sw;

	GtkWidget *areaWidget;
	GtkDrawingArea *drawingArea;
	areaWidget *area;

	uiAreaHandler *ah;

	gboolean scrolling;
	int scrollWidth;
	int scrollHeight;

	// note that this is a pointer; see above
	uiprivClickCounter *cc;

	// for user window drags
	GdkEventButton *dragevent;
};


extern void uiprivAreaCommonInit(areaWidget *aw);
extern void uiprivAreaCommonDispose(gpointer parentClass, GObject *obj);
extern void uiprivAreaCommonFinalize(gpointer parentClass, GObject *obj);
extern void uiprivAreaCommonSizeAllocate(uiArea *a, gpointer parentClass, GtkWidget *w, GtkAllocation *allocation);
extern void uiprivLoadAreaSize(uiArea *a, double *width, double *height);
extern void uiprivAreaCommonGetPreferredHeight(uiArea *a, gpointer parentClass, GtkWidget *w, gint *min, gint *nat);
extern void uiprivAreaCommonGetPreferredWidth(uiArea *a, gpointer parentClass, GtkWidget *w, gint *min, gint *nat);
extern gboolean uiprivAreaCommonButtonPressEvent(uiArea *a, GtkWidget *w, GdkEventButton *e);
extern gboolean uiprivAreaCommonButtonReleaseEvent(uiArea *a, GdkEventButton *e);
extern gboolean uiprivAreaCommonMotionNotifyEvent(uiArea *a, GdkEventMotion *e);
extern gboolean uiprivAreaCommonEnterNotifyEvent(areaWidget *aw);
extern gboolean uiprivAreaCommonLeaveNotifyEvent(areaWidget *aw);
extern gboolean uiprivAreaCommonKeyPressEvent(uiArea *a, GdkEventKey *e);
extern gboolean uiprivAreaCommonKeyReleaseEvent(uiArea *a, GdkEventKey *e);
extern void uiprivAreaCommonSetProperty(areaWidget *aw, GObject *obj, guint prop, const GValue *value, GParamSpec *pspec);
extern void uiprivAreaCommonGetProperty(GObject *obj, guint prop, GValue *value, GParamSpec *pspec);

#define uiprivUnixAreaAllDefaultsExceptDraw(type) \
	static void type ## _init(type *aw) \
	{ \
		uiprivAreaCommonInit((areaWidget*) aw); \
	} \
	static void type ## _dispose(GObject *obj) \
	{ \
		uiprivAreaCommonDispose(type ## _parent_class, obj); \
	} \
	static void type ## _finalize(GObject *obj) \
	{ \
		uiprivAreaCommonFinalize(type ## _parent_class, obj); \
	} \
	static void type ## _size_allocate(GtkWidget *w, GtkAllocation *allocation) \
	{ \
		uiprivAreaCommonSizeAllocate((uiArea*) type(w)->a, type ## _parent_class, w, allocation); \
	} \
	static void type ## _get_preferred_height(GtkWidget *w, gint *min, gint *nat) \
	{ \
		uiprivAreaCommonGetPreferredHeight((uiArea*) type(w)->a, type ## _parent_class, w, min, nat); \
	} \
	static void type ## _get_preferred_width(GtkWidget *w, gint *min, gint *nat) \
	{ \
		uiprivAreaCommonGetPreferredWidth((uiArea*) type(w)->a, type ## _parent_class, w, min, nat); \
	} \
	static gboolean type ## _button_press_event(GtkWidget *w, GdkEventButton *e) \
	{ \
		return uiprivAreaCommonButtonPressEvent((uiArea*) type(w)->a, w, e); \
	} \
	static gboolean type ## _button_release_event(GtkWidget *w, GdkEventButton *e) \
	{ \
		return uiprivAreaCommonButtonReleaseEvent((uiArea*) type(w)->a, e); \
	} \
	static gboolean type ## _motion_notify_event(GtkWidget *w, GdkEventMotion *e) \
	{ \
		return uiprivAreaCommonMotionNotifyEvent((uiArea*) type(w)->a, e); \
	} \
	static gboolean type ## _enter_notify_event(GtkWidget *w, GdkEventCrossing *e) \
	{ \
		return uiprivAreaCommonEnterNotifyEvent((areaWidget*) type(w)); \
	} \
	static gboolean type ## _leave_notify_event(GtkWidget *w, GdkEventCrossing *e) \
	{ \
		return uiprivAreaCommonLeaveNotifyEvent((areaWidget*) type(w)); \
	} \
	static gboolean type ## _key_press_event(GtkWidget *w, GdkEventKey *e) \
	{ \
		return uiprivAreaCommonKeyPressEvent((uiArea*) type(w)->a, e); \
	} \
	static gboolean type ## _key_release_event(GtkWidget *w, GdkEventKey *e) \
	{ \
		return uiprivAreaCommonKeyReleaseEvent((uiArea*) type(w)->a, e); \
	} \
	static void type ## _set_property(GObject *obj, guint prop, const GValue *value, GParamSpec *pspec) \
	{ \
		uiprivAreaCommonSetProperty((areaWidget*) type(obj), obj, prop, value, pspec); \
	} \
	static void type ## _get_property(GObject *obj, guint prop, GValue *value, GParamSpec *pspec) \
	{ \
		uiprivAreaCommonGetProperty(obj, prop, value, pspec); \
	}
