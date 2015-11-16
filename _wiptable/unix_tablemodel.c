// 18 october 2015
#include "uipriv_unix.h"

// On GTK+, uiTableModel is a GtkTreeModel.

#define uiTableModelType (uiTableModel_get_type())
#define uiTableModel(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), uiTableModelType, uiTableModel))
#define isAreaWidget(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), uiTableModelType))
#define uiTableModelClass(class) (G_TYPE_CHECK_CLASS_CAST((class), uiTableModelType, uiTableModelClass))
#define isAreaWidgetClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), uiTableModel))
#define getAreaWidgetClass(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), uiTableModelType, uiTableModelClass))

typedef struct uiTableModel uiTableModel;
typedef struct uiTableModelClass uiTableModelClass;

struct uiTableModel {
	GObject parent_instance;
	uiTableModelSpec *spec;
	void *mData;
	intmax_t nColumns;
	GType *coltypes;
};

struct uiTableModelClass {
	GObjectClass parent_class;
};

static void uiTableModel_treeModel_init(GtkTreeModelIface *);

G_DEFINE_TYPE_WITH_CODE(uiTableModel, uiTableModel, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(GTK_TYPE_TREE_MODEL, uiTableModel_treeModel_init))

static void uiTableModel_init(uiTableModel *m)
{
	// do nothing
}

static void uiTableModel_dispose(GObject *obj)
{
	G_OBJECT_CLASS(uiTableModel_parent_class)->dispose(obj);
}

static void uiTableModel_finalize(GObject *obj)
{
	uiTableModel *m = uiTableModel(obj);

	uiFree(m->coltypes);
	G_OBJECT_CLASS(uiTableModel_parent_class)->finalize(obj);
}

static GtkTreeModelFlags uiTableModel_get_flags(GtkTreeModel *mb)
{
	return GTK_TREE_MODEL_LIST_ONLY;
}

static gint uiTableModel_get_n_columns(GtkTreeModel *mb)
{
	uiTableModel *m = uiTableModel(mb);

	return m->nColumns;
}

static GType uiTableModel_get_column_type(GtkTreeModel *mb, gint index)
{
	uiTableModel *m = uiTableModel(mb);

	return m->coltypes[index];
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

#define numRows(m) ((*((m)->spec->NumRows))((m), (m)->mData))
#define cellValue(m, row, col) ((*((m)->spec->CellValue))((m), (m)->mData, row, column))

static gboolean uiTableModel_get_iter(GtkTreeModel *mb, GtkTreeIter *iter, GtkTreePath *path)
{
	uiTableModel *m = uiTableModel(mb);
	gint index;

	if (gtk_tree_path_get_depth(path) != 1)
		goto bad;
	index = gtk_tree_path_get_indices(path)[0];
	if (index < 0)
		goto bad;
	if (index >= numRows(m))
		goto bad;
	iter->stamp = GOOD_STAMP;
	iter->user_data = TO(index);
	return TRUE;
bad:
	iter->stamp = BAD_STAMP;
	return FALSE;
}

static GtkTreePath *uiTableModel_get_path(GtkTreeModel *mb, GtkTreeIter *iter)
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

#define toBool(v) ((int) ((intptr_t) (v)))
#define toStr(v) ((char *) (v))

static void uiTableModel_get_value(GtkTreeModel *mb, GtkTreeIter *iter, gint column, GValue *value)
{
	uiTableModel *m = uiTableModel(mb);
	void *v;
	GType type;

	if (iter->stamp != GOOD_STAMP)
		return;				// this is what both GtkListStore and GtkTreeStore do
	v = cellValue(m, FROM(iter->user_data), column);
	type = m->coltypes[column];
	g_value_init(value, type);
	if (type == G_TYPE_STRING)
		g_value_take_string(value, toStr(v));
		// the GValue now manages the memory of the string that was g_strdup()'d before
	// TODO image
	else if (type == G_TYPE_BOOLEAN)
		g_value_set_boolean(value, toBool(v));
	else
		complain("unknown GType in uiTableModel_get_value()");
}

static gboolean uiTableModel_iter_next(GtkTreeModel *mb, GtkTreeIter *iter)
{
	uiTableModel *m = uiTableModel(mb);
	gint index;

	if (iter->stamp != GOOD_STAMP)
		return FALSE;				// this is what both GtkListStore and GtkTreeStore do
	index = FROM(iter->user_data);
	index++;
	if (index >= numRows(m)) {
		iter->stamp = BAD_STAMP;
		return FALSE;
	}
	iter->user_data = TO(index);
	return TRUE;
}

static gboolean uiTableModel_iter_previous(GtkTreeModel *mb, GtkTreeIter *iter)
{
	uiTableModel *m = uiTableModel(mb);
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

static gboolean uiTableModel_iter_children(GtkTreeModel *mb, GtkTreeIter *iter, GtkTreeIter *parent)
{
	uiTableModel *m = uiTableModel(mb);

	if (parent == NULL && numRows(m) > 0) {
		iter->stamp = GOOD_STAMP;
		iter->user_data = 0;
		return TRUE;
	}
	iter->stamp = BAD_STAMP;
	return FALSE;
}

static gboolean uiTableModel_iter_has_child(GtkTreeModel *mb, GtkTreeIter *iter)
{
	return FALSE;
}

static gint uiTableModel_iter_n_children(GtkTreeModel *mb, GtkTreeIter *iter)
{
	uiTableModel *m = uiTableModel(mb);

	if (iter == NULL)
		return numRows(m);
	return 0;
}

static gboolean uiTableModel_iter_nth_child(GtkTreeModel *mb, GtkTreeIter *iter, GtkTreeIter *parent, gint n)
{
	uiTableModel *m = uiTableModel(mb);

	if (parent == NULL && n >= 0 && n < numRows(m)) {
		iter->stamp = GOOD_STAMP;
		iter->user_data = TO(n);
		return TRUE;
	}
	iter->stamp = BAD_STAMP;
	return FALSE;
}

static gboolean uiTableModel_iter_parent(GtkTreeModel *mb, GtkTreeIter *iter, GtkTreeIter *child)
{
	iter->stamp = BAD_STAMP;
	return FALSE;
}

static void uiTableModel_class_init(uiTableModelClass *class)
{
	G_OBJECT_CLASS(class)->dispose = uiTableModel_dispose;
	G_OBJECT_CLASS(class)->finalize = uiTableModel_finalize;
}

static void uiTableModel_treeModel_init(GtkTreeModelIface *iface)
{
	iface->get_flags = uiTableModel_get_flags;
	iface->get_n_columns = uiTableModel_get_n_columns;
	iface->get_column_type = uiTableModel_get_column_type;
	iface->get_iter = uiTableModel_get_iter;
	iface->get_path = uiTableModel_get_path;
	iface->get_value = uiTableModel_get_value;
	iface->iter_next = uiTableModel_iter_next;
	iface->iter_previous = uiTableModel_iter_previous;
	iface->iter_children = uiTableModel_iter_children;
	iface->iter_has_child = uiTableModel_iter_has_child;
	iface->iter_n_children = uiTableModel_iter_n_children;
	iface->iter_nth_child = uiTableModel_iter_nth_child;
	iface->iter_parent = uiTableModel_iter_parent;
	// no need for ref_node or unref_node
}

uiTableModel *uiNewTableModel(uintmax_t nCols, uiTableColumnType *types, uiTableModelSpec *spec, void *mData)
{
	uiTableModel *m;
	intmax_t i;

	m = uiTableModel(g_object_new(uiTableModelType, NULL));
	m->spec = spec;
	m->mData = mData;
	m->nColumns = nCols;
	m->coltypes = (GType *) uiAlloc(m->nColumns * sizeof (GType), "GType[]");
	for (i = 0; i < m->nColumns; i++)
		switch (types[i]) {
		case uiTableColumnText:
			m->coltypes[i] = G_TYPE_STRING;
			break;
//TODO		case uiTableColumnImage:
			// TODO
		case uiTableColumnCheckbox:
			m->coltypes[i] = G_TYPE_BOOLEAN;
			break;
		default:
			complain("unknown column type %d in uiNewTableModel()", types[i]);
		}
	return m;
}

// TODO ensure no tables are subscribed
void uiFreeTableModel(uiTableModel *m)
{
	g_object_unref(m);
}

void uiTableModelNotify(uiTableModel *m, uiTableNotification notification, intmax_t row, intmax_t column)
{
	GtkTreeModel *model = GTK_TREE_MODEL(m);
	GtkTreePath *path;
	GtkTreeIter iter;

	path = gtk_tree_path_new_from_indices(row, -1);
	switch (notification) {
	case uiTableRowInserted:
		if (gtk_tree_model_get_iter(model, &iter, path) == FALSE)
			complain("invalid row given to row inserted in uiTableModelNotify()");
		gtk_tree_model_row_inserted(model, path, &iter);
		break;
	case uiTableRowDeleted:
		gtk_tree_model_row_deleted(model, path);
		break;
	case uiTableCellChanged:
		if (gtk_tree_model_get_iter(model, &iter, path) == FALSE)
			complain("invalid row given to row changed in uiTableModelNotify()");
		gtk_tree_model_row_changed(model, path, &iter);
		break;
	default:
		complain("unknown uiTable notification %d in uiTableModelNotify()", notification);
	}
	gtk_tree_path_free(path);
}
