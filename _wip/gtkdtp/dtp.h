// 4 september 2015
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_32
#define GLIB_VERSION_MAX_ALLOWED GLIB_VERSION_2_32
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_4
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_4
#include <gtk/gtk.h>

#define dateTimePickerWidgetType (dateTimePickerWidget_get_type())
#define dateTimePickerWidget(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), dateTimePickerWidgetType, dateTimePickerWidget))
#define isDateTimePickerWidget(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), dateTimePickerWidgetType))
#define dateTimePickerWidgetClass(class) (G_TYPE_CHECK_CLASS_CAST((class), dateTimePickerWidgetType, dateTimePickerWidgetClass))
#define isDateTimePickerWidgetClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), dateTimePickerWidget))
#define getDateTimePickerWidgetClass(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), dateTimePickerWidgetType, dateTimePickerWidgetClass))

typedef struct dateTimePickerWidget dateTimePickerWidget;
typedef struct dateTimePickerWidgetClass dateTimePickerWidgetClass;

struct dateTimePickerWidget {
	GtkBox parent_instance;
	struct dtpPrivate *priv;
};

struct dateTimePickerWidgetClass {
	GtkBoxClass parent_class;
};

extern GType dateTimePickerWidget_get_type(void);
