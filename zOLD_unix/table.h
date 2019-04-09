// 4 june 2018
#include "../common/table.h"

// tablemodel.c
#define uiTableModelType (uiTableModel_get_type())
#define uiTableModel(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), uiTableModelType, uiTableModel))
#define isuiTableModel(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), uiTableModelType))
#define uiTableModelClass(class) (G_TYPE_CHECK_CLASS_CAST((class), uiTableModelType, uiTableModelClass))
#define isuiTableModelClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), uiTableModel))
#define getuiTableModelClass(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), uiTableModelType, uiTableModelClass))
typedef struct uiTableModelClass uiTableModelClass;
struct uiTableModel {
	GObject parent_instance;
	uiTableModelHandler *mh;
};
struct uiTableModelClass {
	GObjectClass parent_class;
};
extern GType uiTableModel_get_type(void);
