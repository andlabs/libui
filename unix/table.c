// 18 october 2015
#include "uipriv_unix.h"

#define tableGTKModelType (tableGTKModel_get_type())
#define tableGTKModel(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), tableGTKModelType, tableGTKModel))
#define isAreaWidget(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), tableGTKModelType))
#define tableGTKModelClass(class) (G_TYPE_CHECK_CLASS_CAST((class), tableGTKModelType, tableGTKModelClass))
#define isAreaWidgetClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), tableGTKModel))
#define getAreaWidgetClass(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), tableGTKModelType, tableGTKModelClass))

typedef struct tableGTKModel tableGTKModel;
typedef struct tableGTKModelClass tableGTKModelClass;

struct uiTable {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *container;
	GtkScrolledWindow *sw;
	GtkWidget *treeWidget;
	GtkTreeView *treeview;
	GtkTreeSelection *selection;
	tableGTKModel *model;
};

struct tableGTKModel {
	GObject parent_instance;
	uiTableModel *model;
};

struct tableGTKModelClass {
	GObjectClass parent_class;
};

static void tableGTKModel_treeModel_init(GtkTreeModel *);

G_DEFINE_TYPE_WITH_CODE(tableGTKModel, tableGTKModel, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(GTK_TYPE_TREE_MODEL, tableGTKModel_treeModel_init))

static void tableGTKModel_init(tableGTKModel *m)
{
	// do nothing
}

static void tableGTKModel_dispose(GObject *obj)
{
	G_OBJECT_CLASS(tableGTKModel_parent_class)->dispose(obj);
}

static void tableGTKModel_finalize(GObject *obj)
{
	G_OBJECT_CLASS(tableGTKModel_parent_class)->finalize(obj);
}

static GtkTreeModelFlags tableGTKModel_get_flags(GtkTreeModel *mb)
{
	return GTK_TREE_MODEL_LIST_ONLY;
}

static gint tableGTKModel_get_n_columns(GtkTreeModel *mb)
{
	tableGTKModel *m = tableGTKModel(mb);

	return (*(m->model->NumColumns))(m->model);
}

static GType tableGTKModel_get_column_type(GtkTreeModel *mb, gint index)
{
	tableGTKModel *m = tableGTKModel(mb);
	uiTableColumnType type;

	type = (*(m->model->ColumnType))(m->model, index);
	switch (type) {
	case uiTableColumnText:
		return G_TYPE_STRING;
//TODO	case uiTableColumnImage:
//TODO		return GDK_TYPE_PIXBUF;
	case uiTableColumnCheckbox:
		return G_TYPE_BOOLEAN;
	}
	complain("unknown column type %d in tableGTKModel_get_column_type()", type);
	return G_TYPE_INVALID;			// make compiler happy
}

/*
how our GtkTreeIters are stored:
	stamp: either GOOD_STAMP or BAD_STAMP
	user_data: row index
Thanks to Company in irc.gimp.net/#gtk+ for suggesting the GSIZE_TO_POINTER() t
rick.
*/
#define GOOD_STAMP 0x1234
#define BAD_STAMP 0x5678
#define FROM(x) ((gint) GPOINTER_TO_SIZE((x)))
#define TO(x) GSIZE_TO_POINTER((gsize) (x))

static gboolean tableGTKModel_get_iter(GtkTreeModel *mb, GtkTreeIter *iter, GtkTreePath *path)
{
	tableGTKModel *m = tableGTKModel(mb);
	gint index;

	if (gtk_tree_path_get_depth(path) != 1)
		goto bad;
	index = gtk_tree_path_get_indices(path)[0];
	if (index < 0)
		goto bad;
	if (index >= (*(m->model->NumRows))(m->model))
		goto bad;
	iter->stamp = GOOD_STAMP;
	iter->user_data = TO(index);
	return TRUE;
bad:
	iter->stamp = BAD_STAMP;
	return FALSE;
}

static GtkTreePath *tableGTKModel_get_path(GtkTreeModel *mb, GtkTreeIter *iter)
{
	// note: from this point forward, the GOOD_STAMP checks ensure that the index stored in iter is nonnegative
	if (iter->stamp != GOOD_STAMP)
		return NULL;			// this is what both GtkListStore and GtkTreeStore do
	return gtk_tree_path_new_from_indices(FROM(iter->user_data), -1);
}

void *uiTableModelFromString(const char *str)
{
	return g_strdup(str);
}

#define toBool(v) ((int) (v))
#define toStr(v) ((const char *) (v))

static void tableGTKModel_get_value(GtkTreeModel *mb, GtkTreeIter *iter, gint column, GValue *value)
{
	tableGTKModel *m = tableGTKModel(mb);
	void *v;
	uiTableColumnType type;

	if (iter->stamp != GOOD_STAMP)
		return;				// this is what both GtkListStore and GtkTreeStore do
	v = (*(m->model->CellValue))(m->model, FROM(iter->user_data), column);
	type = (*(m->model->ColumnType))(m->model, column);
	switch (type) {
	case uiTableColumnText:
		g_value_init(value, G_TYPE_STRING);
		g_value_take_string(value, toStr(v));
		// the GValue now manages the memory of the string that was g_strdup()'d before
		return;
//TODO	case uiTableColumnImage:
		// TODO
	case uiTableColumnCheckbox:
		g_value_init(value, G_TYPE_BOOLEAN);
		g_value_set_boolean(value, toBool(v));
		return;
	}
	complain("unknown column type %d in tableGTKModel_get_value()", type);
}

static gboolean tableGTKModel_iter_next(GtkTreeModel *mb, GtkTreeIter *iter)
{
	tableGTKModel *m = tableGTKModel(mb);
	gint index;

	if (iter->stamp != GOOD_STAMP)
		return FALSE;				// this is what both GtkListStore and GtkTreeStore do
	index = FROM(iter->user_data);
	index++;
	if (index >= (*(m->model->NumRows))(m->model)) {
		iter->stamp = BAD_STAMP;
		return FALSE;
	}
	iter->user_data = TO(index);
	return TRUE;
}

static gboolean tableGTKModel_iter_previous(GtkTreeModel *mb, GtkTreeIter *iter)
{
	tableGTKModel *m = tableGTKModel(mb);
	gint index;

	if (iter->stamp != GOOD_STAMP)
		return FALSE;			// this is what both GtkListStore and GtkTreeStore do
	index = FROM(iter->user_data);
	if (index <= 0) {
		iter->stamp = BAD_STAMP;
		return FALSE;
	}
	index--;
	iter->user_data = TO(index);
	return TRUE;
}

static gboolean tableGTKModel_iter_children(GtkTreeModel *mb, GtkTreeIter *iter, GtkTreeIter *parent)
{
	tableGTKModel *m = tableGTKModel(mb);

	if (parent == NULL && (*(m->model->NumRows))(m->model) > 0) {
		child->stamp = GOOD_STAMP;
		child->user_data = 0;
		return TRUE;
	}
	child->stamp = BAD_STAMP;
	return FALSE;
}

static gboolean tableGTKModel_iter_has_child(GtkTreeModel *mb, GtkTreeIter *iter)
{
	return FALSE;
}

static gint tableGTKModel_iter_n_children(GtkTreeModel *mb, GtkTreeIter *iter)
{
	tableGTKModel *m = tableGTKModel(mb);

	if (iter == NULL)
		return (*(m->model->NumRows))(m->model);
	return 0;
}

static gboolean tableGTKModel_iter_nth_child(GtkTreeModel *mb, GtkTreeIter *iter, GtkTreeIter *parent, gint n)
{
	tableGTKModel *m = tableGTKModel(mb);

	if (parent == NULL && n >= 0 && n < (*(m->model->NumRows))(m->model)) {
		child->stamp = GOOD_STAMP;
		child->user_data = TO(n);
		return TRUE;
	}
	child->stamp = BAD_STAMP;
	return FALSE;
}

static gboolean tableGTKModel_iter_parent(GtkTreeModel *mb, GtkTreeIter *iter, GtkTreeIter *child)
{
	parent->stamp = BAD_STAMP;
	return FALSE;
}

static void tableGTKModel_class_init(tableGTKModelClass *class)
{
	G_OBJECT_CLASS(class)->dispose = tableGTKModel_dispose;
	G_OBJECT_CLASS(class)->finalize = tableGTKModel_finalize;
}

static void tableGTKModel_treeModel_init(GtkTreeModel *iface)
{
	iface->get_flags = tableGTKModel_get_flags;
	iface->get_n_columns = tableGTKModel_get_n_columns;
	iface->get_column_type = tableGTKModel_get_column_type;
	iface->get_iter = tableGTKModel_get_iter;
	iface->get_path = tableGTKModel_get_path;
	iface->get_value = tableGTKModel_get_value;
	iface->iter_next = tableGTKModel_iter_next;
	iface->iter_previous = tableGTKModel_iter_previous;
	iface->iter_children = tableGTKModel_iter_children;
	iface->iter_has_child = tableGTKModel_iter_has_child;
	iface->iter_n_children = tableGTKModel_iter_n_children;
	iface->iter_nth_child = tableGTKModel_iter_nth_child;
	iface->iter_parent = tableGTKModel_iter_parent;
	// no need for ref_node or unref_node
}

static void newGTKModel(uiTableModel *model)
{
	tableGTKModel *m;

	m = tableGTKModel(g_object_new(tableGTKModelType, NULL));
	m->model = model;
	return m;
}

void tableNotify(uiTable *t, uiTableNotification n, intmax_t row, intmax_t column)
{
	// TODO
}

void uiTableSetModel(uiTable *t, uiTableModel *m)
{
	if (t->model != NULL)
		(*(t->model->model->Unsubscribe))(t->model->model, t);
	t->model = newGTKModel(m);
	(*(t->model->model->Subscribe))(t->model->model, t);
	// TODO will this free the old model?
	gtk_tree_view_set_model(t->treeview, GTK_TREE_MODEL(t->moel));
}

void uiTableAppendColumn(uiTable *t, uiTableColumnParams *p)
{
	// TODO
}

uiTable *uiNewTable(void)
{
	uiTable *t;
}
