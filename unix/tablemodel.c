// 26 june 2016
#include "uipriv_unix.h"
#include "table.h"

static void uiTableModel_gtk_tree_model_interface_init(GtkTreeModelIface *iface);

G_DEFINE_TYPE_WITH_CODE(uiTableModel, uiTableModel, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(GTK_TYPE_TREE_MODEL, uiTableModel_gtk_tree_model_interface_init))

static void uiTableModel_init(uiTableModel *m)
{
	// nothing to do
}

static void uiTableModel_dispose(GObject *obj)
{
	G_OBJECT_CLASS(uiTableModel_parent_class)->dispose(obj);
}

static void uiTableModel_finalize(GObject *obj)
{
	G_OBJECT_CLASS(uiTableModel_parent_class)->finalize(obj);
}

static GtkTreeModelFlags uiTableModel_get_flags(GtkTreeModel *mm)
{
	return GTK_TREE_MODEL_LIST_ONLY;
}

static gint uiTableModel_get_n_columns(GtkTreeModel *mm)
{
	uiTableModel *m = uiTableModel(mm);

	return uiprivTableModelNumColumns(m);
}

static GType uiTableModel_get_column_type(GtkTreeModel *mm, gint index)
{
	uiTableModel *m = uiTableModel(mm);

	switch (uiprivTableModelColumnType(m, index)) {
	case uiTableValueTypeString:
		return G_TYPE_STRING;
	case uiTableValueTypeImage:
		return G_TYPE_POINTER;
	case uiTableValueTypeInt:
		return G_TYPE_INT;
	case uiTableValueTypeColor:
		return GDK_TYPE_RGBA;
	}
	// TODO
	return G_TYPE_INVALID;
}

#define STAMP_GOOD 0x1234
#define STAMP_BAD 0x5678

static gboolean uiTableModel_get_iter(GtkTreeModel *mm, GtkTreeIter *iter, GtkTreePath *path)
{
	uiTableModel *m = uiTableModel(mm);
	gint row;

	if (gtk_tree_path_get_depth(path) != 1)
		goto bad;
	row = gtk_tree_path_get_indices(path)[0];
	if (row < 0)
		goto bad;
	if (row >= uiprivTableModelNumRows(m))
		goto bad;
	iter->stamp = STAMP_GOOD;
	iter->user_data = GINT_TO_POINTER(row);
	return TRUE;
bad:
	iter->stamp = STAMP_BAD;
	return FALSE;
}

// GtkListStore returns NULL on error; let's do that too
static GtkTreePath *uiTableModel_get_path(GtkTreeModel *mm, GtkTreeIter  *iter)
{
	gint row;

	if (iter->stamp != STAMP_GOOD)
		return NULL;
	row = GPOINTER_TO_INT(iter->user_data);
	return gtk_tree_path_new_from_indices(row, -1);
}

// GtkListStore leaves value empty on failure; let's do the same
static void uiTableModel_get_value(GtkTreeModel *mm, GtkTreeIter *iter, gint column, GValue *value)
{
	uiTableModel *m = uiTableModel(mm);
	gint row;
	uiTableValue *tvalue;
	double r, g, b, a;
	GdkRGBA rgba;

	if (iter->stamp != STAMP_GOOD)
		return;
	row = GPOINTER_TO_INT(iter->user_data);
	tvalue = uiprivTableModelCellValue(m, row, column);
	switch (uiprivTableModelColumnType(m, column)) {
	case uiTableValueTypeString:
		g_value_init(value, G_TYPE_STRING);
		g_value_set_string(value, uiTableValueString(tvalue));
		uiFreeTableValue(tvalue);
		return;
	case uiTableValueTypeImage:
		g_value_init(value, G_TYPE_POINTER);
		g_value_set_pointer(value, uiTableValueImage(tvalue));
		uiFreeTableValue(tvalue);
		return;
	case uiTableValueTypeInt:
		g_value_init(value, G_TYPE_INT);
		g_value_set_int(value, uiTableValueInt(tvalue));
		uiFreeTableValue(tvalue);
		return;
	case uiTableValueTypeColor:
		g_value_init(value, GDK_TYPE_RGBA);
		if (tvalue == NULL) {
			g_value_set_boxed(value, NULL);
			return;
		}
		uiTableValueColor(tvalue, &r, &g, &b, &a);
		uiFreeTableValue(tvalue);
		rgba.red = r;
		rgba.green = g;
		rgba.blue = b;
		rgba.alpha = a;
		g_value_set_boxed(value, &rgba);
		return;
	}
	// TODO
}

static gboolean uiTableModel_iter_next(GtkTreeModel *mm, GtkTreeIter *iter)
{
	uiTableModel *m = uiTableModel(mm);
	gint row;

	if (iter->stamp != STAMP_GOOD)
		return FALSE;
	row = GPOINTER_TO_INT(iter->user_data);
	row++;
	if (row >= uiprivTableModelNumRows(m)) {
		iter->stamp = STAMP_BAD;
		return FALSE;
	}
	iter->user_data = GINT_TO_POINTER(row);
	return TRUE;
}

static gboolean uiTableModel_iter_previous(GtkTreeModel *mm, GtkTreeIter *iter)
{
	gint row;

	if (iter->stamp != STAMP_GOOD)
		return FALSE;
	row = GPOINTER_TO_INT(iter->user_data);
	row--;
	if (row < 0) {
		iter->stamp = STAMP_BAD;
		return FALSE;
	}
	iter->user_data = GINT_TO_POINTER(row);
	return TRUE;
}

static gboolean uiTableModel_iter_children(GtkTreeModel *mm, GtkTreeIter *iter, GtkTreeIter *parent)
{
	return gtk_tree_model_iter_nth_child(mm, iter, parent, 0);
}

static gboolean uiTableModel_iter_has_child(GtkTreeModel *mm, GtkTreeIter *iter)
{
	return FALSE;
}

static gint uiTableModel_iter_n_children(GtkTreeModel *mm, GtkTreeIter *iter)
{
	uiTableModel *m = uiTableModel(mm);

	if (iter != NULL)
		return 0;
	return uiprivTableModelNumRows(m);
}

static gboolean uiTableModel_iter_nth_child(GtkTreeModel *mm, GtkTreeIter *iter, GtkTreeIter *parent, gint n)
{
	uiTableModel *m = uiTableModel(mm);

	if (iter->stamp != STAMP_GOOD)
		return FALSE;
	if (parent != NULL)
		goto bad;
	if (n < 0)
		goto bad;
	if (n >= uiprivTableModelNumRows(m))
		goto bad;
	iter->stamp = STAMP_GOOD;
	iter->user_data = GINT_TO_POINTER(n);
	return TRUE;
bad:
	iter->stamp = STAMP_BAD;
	return FALSE;
}

gboolean uiTableModel_iter_parent(GtkTreeModel *mm, GtkTreeIter *iter, GtkTreeIter *child)
{
	iter->stamp = STAMP_BAD;
	return FALSE;
}

static void uiTableModel_class_init(uiTableModelClass *class)
{
	G_OBJECT_CLASS(class)->dispose = uiTableModel_dispose;
	G_OBJECT_CLASS(class)->finalize = uiTableModel_finalize;
}

static void uiTableModel_gtk_tree_model_interface_init(GtkTreeModelIface *iface)
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
	// don't specify ref_node() or unref_node()
}

uiTableModel *uiNewTableModel(uiTableModelHandler *mh)
{
	uiTableModel *m;

	m = uiTableModel(g_object_new(uiTableModelType, NULL));
	m->mh = mh;
	return m;
}

void uiFreeTableModel(uiTableModel *m)
{
	g_object_unref(m);
}

void uiTableModelRowInserted(uiTableModel *m, int newIndex)
{
	GtkTreePath *path;
	GtkTreeIter iter;

	path = gtk_tree_path_new_from_indices(newIndex, -1);
	iter.stamp = STAMP_GOOD;
	iter.user_data = GINT_TO_POINTER(newIndex);
	gtk_tree_model_row_inserted(GTK_TREE_MODEL(m), path, &iter);
	gtk_tree_path_free(path);
}

void uiTableModelRowChanged(uiTableModel *m, int index)
{
	GtkTreePath *path;
	GtkTreeIter iter;

	path = gtk_tree_path_new_from_indices(index, -1);
	iter.stamp = STAMP_GOOD;
	iter.user_data = GINT_TO_POINTER(index);
	gtk_tree_model_row_changed(GTK_TREE_MODEL(m), path, &iter);
	gtk_tree_path_free(path);
}

void uiTableModelRowDeleted(uiTableModel *m, int oldIndex)
{
	GtkTreePath *path;

	path = gtk_tree_path_new_from_indices(oldIndex, -1);
	gtk_tree_model_row_deleted(GTK_TREE_MODEL(m), path);
	gtk_tree_path_free(path);
}

uiTableModelHandler *uiprivTableModelHandler(uiTableModel *m)
{
	return m->mh;
}
