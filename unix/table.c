// 26 june 2016
#include "uipriv_unix.h"

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

static void uiTableModel_gtk_tree_model_interface_init(GtkTreeModelInterface *iface);

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

	return (*(m->mh->NumColumns))(m->mh, m);
}

static GType uiTableModel_get_column_type(GtkTreeModel *mm, gint index)
{
	uiTableModel *m = uiTableModel(mm);

	switch ((*(m->mh->ColumnType))(m->mh, m, index)) {
	case uiTableModelColumnString:
		return G_TYPE_STRING;
	case uiTableModelColumnImage:
		// TODO
	case uiTableModelColumnInt:
		return G_TYPE_INT;
	case uiTableModelColumnColor:
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
	if (row >= (*(m->mh->NumRows))(m->mh, m))
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
	uiTableModel *m = uiTableModel(mm);
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

	if (iter->stamp != STAMP_GOOD)
		return;
	row = GPOINTER_TO_INT(iter->user_data);
	data = (*(m->mh->CellValue))(m->mh, m, row, column);
	switch ((*(m->mh->ColumnType))(m->mh, m, index)) {
	case uiTableModelColumnString:
		g_value_init(value, G_TYPE_STRING);
		g_value_take_string(value, (char *) data);
		return;
	case uiTableModelColumnImage:
		// TODO
		return;
	case uiTableModelColumnInt:
		g_value_init(value, G_TYPE_INT);
		g_value_set_int(value, uiTableModelTakeInt(data));
		return;
	case uiTableModelColumnColor:
		g_value_init(value, GDK_TYPE_RGBA);
		g_value_take_boxed(value, data);
		return;
	}
	// TODO
}

static gboolean uiTreeModel_iter_next(GtkTreeModel *mm, GtkTreeIter *iter)
{
	uiTableModel *m = uiTableModel(mm);
	gint row;

	gint row;

	if (iter->stamp != STAMP_GOOD)
		return FALSE;
	row = GPOINTER_TO_INT(iter->user_data);
	row++;
	if (row >= (*(m->mh->NumRows))(m->mh, m)) {
		iter->stamp = STAMP_BAD;
		return FALSE;
	}
	iter->user_data = GINT_TO_POINTER(row);
	return TRUE;
}

static gboolean uiTableModel_iter_previous(GtkTreeModel *mm, GtkTreeIter *iter)
{
	uiTableModel *m = uiTableModel(mm);
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
	return gtk_tree_model_nth_child(mm, iter, parent, 0);
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
	return (*(m->mh->NumRows))(m->mh, m);
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
	if (n >= (*(m->mh->NumRows))(m->mh, m))
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
	// nothing to do
}

static void uiTableModel_gtk_tree_model_interface_init(GtkTreeModelInterface *iface)
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

void *uiTableModelStrdup(const char *str)
{
	return g_strdup(str);
}

void *uiTableModelGiveColor(double r, double g, double b, double a)
{
	GdkRGBA rgba;

	rgba.red = r;
	rgba.green = g;
	rgba.blue = b;
	rgba.alpha = a;
	return gdk_rgba_copy(&rgba);
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
	gtk_tree_model_row_inserted(GTK_TREE_MODEL(m), path, iter);
	gtk_tree_path_free(path);
}

void uiTableModelRowChanged(uiTableModel *m, int index)
{
	GtkTreePath *path;
	GtkTreeIter iter;

	path = gtk_tree_path_new_from_indices(newIndex, -1);
	iter.stamp = STAMP_GOOD;
	iter.user_data = GINT_TO_POINTER(newIndex);
	gtk_tree_model_row_changed(GTK_TREE_MODEL(m), path, iter);
	gtk_tree_path_free(path);
}

void uiTableModelRowDeleted(uiTableModel *m, int oldIndex)
{
	GtkTreePath *path;

	path = gtk_tree_path_new_from_indices(newIndex, -1);
	gtk_tree_model_row_removed(GTK_TREE_MODEL(m), path);
	gtk_tree_path_free(path);
}

struct uiTableColumn {
	GtkTreeViewColumn *c;
	GtkTreeView *tv;		// for pixbufs
};

void uiTableColumnAppendTextPart(uiTableColumn *c, int modelColumn, int expand)
{
	GtkCellRenderer *r;

	r = gtk_cell_renderer_text_new();
	// TODO make uneditable
	gtk_table_column_pack_start(c->c, r, expand != 0);
	gtk_table_column_add_attribute(c->c, r, "text", modelColumn);
	// TODO editing signal
}

struct pixbufData {
	GtkTreeView *tv;
	int modelColumn;
};

static void pixbufDataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *mm, GtkTreeIter *iter, gpointer data)
{
	struct pixbufData *d = (struct pixbufData *) data;
	GValue value = G_VALUE_INIT;
	uiImage *img;

	gtk_tree_model_get_value(mm, iter, d->modelColumn, &value);
	img = (uiImage *) g_value_get_pointer(&value);
	// TODO
}

void uiTableColumnAppendImagePart(uiTableColumn *c, int modelColumn, int expand)
{
	GtkCellRenderer *r;
	struct pixbufData *d;

	r = gtk_cell_renderer_pixbuf_new();
	// TODO make uneditable
	gtk_table_column_pack_start(c->c, r, expand != 0);
	// TODO use uiNew and uiFree (need to wrap the latter)
	d = g_new0(1, struct pixbufData);
	d->tv = c->tv;
	d->modelColumn = modelColumn;
	gtk_tree_view_column_set_cell_data_func(c->c, r,
		pixbufDataFunc, d, g_free);
}

void uiTableColumnAppendButtonPart(uiTableColumn *c, int modelColumn, int expand)
{
	// TODO
}

void uiTableColumnAppendCheckboxPart(uiTableColumn *c, int modelColumn, int expand)
{
	GtkCellRenderer *r;

	r = gtk_cell_renderer_toggle_new();
	// TODO make editable
	gtk_table_column_pack_start(c->c, r, expand != 0);
	gtk_table_column_add_attribute(c->c, r, "active", modelColumn);
	// TODO editing signal
}

void uiTableColumnAppendProgressBarPart(uiTableColumn *c, int modelColumn, int expand)
{
	// TODO
}

void uiTableColumnPartSetEditable(uiTableColumn *c, int part, int editable)
{
	// TODO
}

void uiTableColumnPartSetTextColor(uiTableColumn *c, int part, int modelColumn)
{
	// TODO
}

uiTableColumn *uiTableAppendColumn(uiTable *t, const char *name)
{
	// TODO
}

void uiTableSetRowBackgroundColorModelColumn(uiTable *t, int modelColumn)
{
	// TODO
}

uiTable *uiNewTable(uiTableModel *model)
{
	// TODO
}
