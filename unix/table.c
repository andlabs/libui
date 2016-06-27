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
	void *data;

	if (iter->stamp != STAMP_GOOD)
		return;
	row = GPOINTER_TO_INT(iter->user_data);
	data = (*(m->mh->CellValue))(m->mh, m, row, column);
	switch ((*(m->mh->ColumnType))(m->mh, m, column)) {
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

static gboolean uiTableModel_iter_next(GtkTreeModel *mm, GtkTreeIter *iter)
{
	uiTableModel *m = uiTableModel(mm);
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

enum {
	partText,
	partImage,
	partButton,
	partCheckbox,
	partProgressBar,
};

struct tablePart {
	int type;
	int textColumn;
	int imageColumn;
	int valueColumn;
	uiTable *tv;			// for pixbufs and background color
	int editable;
};

struct uiTableColumn {
	GtkTreeViewColumn *c;
	uiTable *tv;			// for pixbufs and background color
	GPtrArray *parts;
};

struct uiTable {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *scontainer;
	GtkScrolledWindow *sw;
	GtkWidget *treeWidget;
	GtkTreeView *tv;
	GPtrArray *columns;
	int backgroundColumn;
};

static void dataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *mm, GtkTreeIter *iter, gpointer data)
{
	struct tablePart *part = (struct tablePart *) data;
	GValue value = G_VALUE_INIT;
	const gchar *str;
	uiImage *img;

	switch (part->type) {
	case partText:
		gtk_tree_model_get_value(mm, iter, part->textColumn, &value);
		str = g_value_get_string(&value);
		g_object_set(r, "text", str, NULL);
		if (part->editable)
			g_object_set(r, "mode", GTK_CELL_RENDERER_MODE_EDITABLE, NULL);
		else
			g_object_set(r, "mode", GTK_CELL_RENDERER_MODE_INERT, NULL);
		break;
	case partImage:
		gtk_tree_model_get_value(mm, iter, part->imageColumn, &value);
		img = (uiImage *) g_value_get_pointer(&value);
		// TODO
		g_object_set(r, "mode", GTK_CELL_RENDERER_MODE_INERT, NULL);
		break;
	case partButton:
		gtk_tree_model_get_value(mm, iter, part->textColumn, &value);
		// TODO
		if (part->editable)
			g_object_set(r, "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE, NULL);
		else
			g_object_set(r, "mode", GTK_CELL_RENDERER_MODE_INERT, NULL);
		break;
	case partCheckbox:
		gtk_tree_model_get_value(mm, iter, part->valueColumn, &value);
		g_object_set(r, "active", g_value_get_int(&value) != 0, NULL);
		if (part->editable)
			g_object_set(r, "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE, NULL);
		else
			g_object_set(r, "mode", GTK_CELL_RENDERER_MODE_INERT, NULL);
		break;
	case partProgressBar:
		gtk_tree_model_get_value(mm, iter, part->valueColumn, &value);
		// TODO
		g_object_set(r, "mode", GTK_CELL_RENDERER_MODE_INERT, NULL);
		break;
	}
	g_value_unset(&value);

	if (part->tv->backgroundColumn != -1) {
		GdkRGBA *rgba;

		gtk_tree_model_get_value(mm, iter, part->tv->backgroundColumn, &value);
		rgba = (GdkRGBA *) g_value_get_boxed(&value);
		if (rgba != NULL)
			g_object_set(r, "cell-background-rgba", rgba, NULL);
		g_value_unset(&value);
	}
}

void uiTableColumnAppendTextPart(uiTableColumn *c, int modelColumn, int expand)
{
	struct tablePart *part;
	GtkCellRenderer *r;

	part = uiNew(struct tablePart);
	part->type = partText;
	part->textColumn = modelColumn;
	part->tv = c->tv;
	part->editable = 0;

	r = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(c->c, r, expand != 0);
	gtk_tree_view_column_set_cell_data_func(c->c, r, dataFunc, part, NULL);
	// TODO editing signal

	g_ptr_array_add(c->parts, part);
}

void uiTableColumnAppendImagePart(uiTableColumn *c, int modelColumn, int expand)
{
	struct tablePart *part;
	GtkCellRenderer *r;

	part = uiNew(struct tablePart);
	part->type = partImage;
	part->textColumn = modelColumn;
	part->tv = c->tv;
	part->editable = 0;

	r = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(c->c, r, expand != 0);
	gtk_tree_view_column_set_cell_data_func(c->c, r, dataFunc, part, NULL);

	g_ptr_array_add(c->parts, part);
}

void uiTableColumnAppendButtonPart(uiTableColumn *c, int modelColumn, int expand)
{
	// TODO
}

void uiTableColumnAppendCheckboxPart(uiTableColumn *c, int modelColumn, int expand)
{
	struct tablePart *part;
	GtkCellRenderer *r;

	part = uiNew(struct tablePart);
	part->type = partCheckbox;
	part->valueColumn = modelColumn;
	part->tv = c->tv;
	part->editable = 1;		// editable by default

	r = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(c->c, r, expand != 0);
	gtk_tree_view_column_set_cell_data_func(c->c, r, dataFunc, part, NULL);
	// TODO editing signal

	g_ptr_array_add(c->parts, part);
}

void uiTableColumnAppendProgressBarPart(uiTableColumn *c, int modelColumn, int expand)
{
	struct tablePart *part;
	GtkCellRenderer *r;

	part = uiNew(struct tablePart);
	part->type = partProgressBar;
	part->valueColumn = modelColumn;
	part->tv = c->tv;
	part->editable = 0;

	r = gtk_cell_renderer_progress_new();
	gtk_tree_view_column_pack_start(c->c, r, expand != 0);
	gtk_tree_view_column_set_cell_data_func(c->c, r, dataFunc, part, NULL);

	g_ptr_array_add(c->parts, part);
}

void uiTableColumnPartSetEditable(uiTableColumn *c, int part, int editable)
{
	// TODO
}

void uiTableColumnPartSetTextColor(uiTableColumn *c, int part, int modelColumn)
{
	// TODO
}

uiUnixControlAllDefaultsExceptDestroy(uiTable)

static void uiTableDestroy(uiControl *c)
{
	uiTable *t = uiTable(c);

	// TODO
	g_object_unref(t->widget);
	uiFreeControl(uiControl(t));
}

uiTableColumn *uiTableAppendColumn(uiTable *t, const char *name)
{
	uiTableColumn *c;

	c = uiNew(uiTableColumn);
	c->c = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(c->c, name);
	gtk_tree_view_append_column(t->tv, c->c);
	c->tv = t;		// TODO rename field to t, cascade
	c->parts = g_ptr_array_new();
	return c;
}

void uiTableSetRowBackgroundColorModelColumn(uiTable *t, int modelColumn)
{
	t->backgroundColumn = modelColumn;
	// TODO refresh table
}

uiTable *uiNewTable(uiTableModel *model)
{
	uiTable *t;

	uiUnixNewControl(uiTable, t);

	t->backgroundColumn = -1;

	t->widget = gtk_scrolled_window_new(NULL, NULL);
	t->scontainer = GTK_CONTAINER(t->widget);
	t->sw = GTK_SCROLLED_WINDOW(t->widget);
	gtk_scrolled_window_set_shadow_type(t->sw, GTK_SHADOW_IN);

	t->treeWidget = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	t->tv = GTK_TREE_VIEW(t->treeWidget);
	// TODO set up t->tv

	gtk_container_add(t->scontainer, t->treeWidget);
	// and make the tree view visible; only the scrolled window's visibility is controlled by libui
	gtk_widget_show(t->treeWidget);

	return t;
}
