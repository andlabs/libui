// 26 june 2016
#include "uipriv_unix.h"

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
	int colorColumn;
	GtkCellRenderer *r;
	uiTable *tv;			// for pixbufs and background color
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
	uiTableModel *model;
	int backgroundColumn;
};

// use the same size as GtkFileChooserWidget's treeview
// TODO refresh when icon theme changes
// TODO doesn't work when scaled
// TODO is this even necessary?
static void setImageSize(GtkCellRenderer *r)
{
	gint size;
	gint width, height;
	gint xpad, ypad;

	size = 16;		// fallback used by GtkFileChooserWidget
	if (gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &width, &height) != FALSE)
		size = MAX(width, height);
	gtk_cell_renderer_get_padding(r, &xpad, &ypad);
	gtk_cell_renderer_set_fixed_size(r,
		2 * xpad + size,
		2 * ypad + size);
}

static void applyColor(GtkTreeModel *mm, GtkTreeIter *iter, int modelColumn, GtkCellRenderer *r, const char *prop, const char *propSet)
{
	GValue value = G_VALUE_INIT;
	GdkRGBA *rgba;

	gtk_tree_model_get_value(mm, iter, modelColumn, &value);
	rgba = (GdkRGBA *) g_value_get_boxed(&value);
	if (rgba != NULL)
		g_object_set(r, prop, rgba, NULL);
	else
		g_object_set(r, propSet, FALSE, NULL);
	g_value_unset(&value);
}

static void dataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *mm, GtkTreeIter *iter, gpointer data)
{
	struct tablePart *part = (struct tablePart *) data;
	GValue value = G_VALUE_INIT;
	const gchar *str;
	uiImage *img;
	int pval;

	switch (part->type) {
	case partText:
		gtk_tree_model_get_value(mm, iter, part->textColumn, &value);
		str = g_value_get_string(&value);
		g_object_set(r, "text", str, NULL);
		if (part->colorColumn != -1)
			applyColor(mm, iter,
				part->colorColumn,
				r, "foreground-rgba", "foreground-set");
		break;
	case partImage:
//TODO		setImageSize(r);
		gtk_tree_model_get_value(mm, iter, part->imageColumn, &value);
		img = (uiImage *) g_value_get_pointer(&value);
		g_object_set(r, "surface",
			uiprivImageAppropriateSurface(img, part->tv->treeWidget),
			NULL);
		break;
	case partButton:
		gtk_tree_model_get_value(mm, iter, part->textColumn, &value);
		str = g_value_get_string(&value);
		g_object_set(r, "text", str, NULL);
		break;
	case partCheckbox:
		gtk_tree_model_get_value(mm, iter, part->valueColumn, &value);
		g_object_set(r, "active", g_value_get_int(&value) != 0, NULL);
		break;
	case partProgressBar:
		gtk_tree_model_get_value(mm, iter, part->valueColumn, &value);
		pval = g_value_get_int(&value);
		if (pval == -1) {
			// TODO
		} else
			g_object_set(r,
				"pulse", -1,
				"value", pval,
				NULL);
		break;
	}
	g_value_unset(&value);

	if (part->tv->backgroundColumn != -1)
		applyColor(mm, iter,
			part->tv->backgroundColumn,
			r, "cell-background-rgba", "cell-background-set");
}

static void onEdited(struct tablePart *part, int column, const char *pathstr, const void *data)
{
	GtkTreePath *path;
	int row;
	uiTableModel *m;

	path = gtk_tree_path_new_from_string(pathstr);
	row = gtk_tree_path_get_indices(path)[0];
	gtk_tree_path_free(path);
	m = part->tv->model;
	(*(m->mh->SetCellValue))(m->mh, m, row, column, data);
	// and update
	uiTableModelRowChanged(m, row);
}

static void appendPart(uiTableColumn *c, struct tablePart *part, GtkCellRenderer *r, int expand)
{
	part->r = r;
	gtk_tree_view_column_pack_start(c->c, part->r, expand != 0);
	gtk_tree_view_column_set_cell_data_func(c->c, part->r, dataFunc, part, NULL);
	g_ptr_array_add(c->parts, part);
}

static void textEdited(GtkCellRendererText *renderer, gchar *path, gchar *newText, gpointer data)
{
	struct tablePart *part = (struct tablePart *) data;

	onEdited(part, part->textColumn, path, newText);
}

void uiTableColumnAppendTextPart(uiTableColumn *c, int modelColumn, int expand)
{
	struct tablePart *part;
	GtkCellRenderer *r;

	part = uiprivNew(struct tablePart);
	part->type = partText;
	part->textColumn = modelColumn;
	part->tv = c->tv;
	part->colorColumn = -1;

	r = gtk_cell_renderer_text_new();
	g_object_set(r, "editable", FALSE, NULL);
	g_signal_connect(r, "edited", G_CALLBACK(textEdited), part);

	appendPart(c, part, r, expand);
}

void uiTableColumnAppendImagePart(uiTableColumn *c, int modelColumn, int expand)
{
	struct tablePart *part;

	part = uiprivNew(struct tablePart);
	part->type = partImage;
	part->imageColumn = modelColumn;
	part->tv = c->tv;
	appendPart(c, part,
		gtk_cell_renderer_pixbuf_new(),
		expand);
}

// TODO wrong type here
static void buttonClicked(GtkCellRenderer *r, gchar *pathstr, gpointer data)
{
	struct tablePart *part = (struct tablePart *) data;

	onEdited(part, part->textColumn, pathstr, NULL);
}

void uiTableColumnAppendButtonPart(uiTableColumn *c, int modelColumn, int expand)
{
	struct tablePart *part;
	GtkCellRenderer *r;

	part = uiprivNew(struct tablePart);
	part->type = partButton;
	part->textColumn = modelColumn;
	part->tv = c->tv;

	r = uiprivNewCellRendererButton();
	g_object_set(r, "sensitive", TRUE, NULL);		// editable by default
	g_signal_connect(r, "clicked", G_CALLBACK(buttonClicked), part);

	appendPart(c, part, r, expand);
}

// yes, we need to do all this twice :|
static void checkboxToggled(GtkCellRendererToggle *r, gchar *pathstr, gpointer data)
{
	struct tablePart *part = (struct tablePart *) data;
	GtkTreePath *path;
	int row;
	uiTableModel *m;
	void *value;
	int intval;

	path = gtk_tree_path_new_from_string(pathstr);
	row = gtk_tree_path_get_indices(path)[0];
	gtk_tree_path_free(path);
	m = part->tv->model;
	value = (*(m->mh->CellValue))(m->mh, m, row, part->valueColumn);
	intval = !uiTableModelTakeInt(value);
	onEdited(part, part->valueColumn, pathstr, uiTableModelGiveInt(intval));
}

void uiTableColumnAppendCheckboxPart(uiTableColumn *c, int modelColumn, int expand)
{
	struct tablePart *part;
	GtkCellRenderer *r;

	part = uiprivNew(struct tablePart);
	part->type = partCheckbox;
	part->valueColumn = modelColumn;
	part->tv = c->tv;

	r = gtk_cell_renderer_toggle_new();
	g_object_set(r, "sensitive", TRUE, NULL);		// editable by default
	g_signal_connect(r, "toggled", G_CALLBACK(checkboxToggled), part);

	appendPart(c, part, r, expand);
}

void uiTableColumnAppendProgressBarPart(uiTableColumn *c, int modelColumn, int expand)
{
	struct tablePart *part;

	part = uiprivNew(struct tablePart);
	part->type = partProgressBar;
	part->valueColumn = modelColumn;
	part->tv = c->tv;
	appendPart(c, part,
		gtk_cell_renderer_progress_new(),
		expand);
}

void uiTableColumnPartSetEditable(uiTableColumn *c, int part, int editable)
{
	struct tablePart *p;

	p = (struct tablePart *) g_ptr_array_index(c->parts, part);
	switch (p->type) {
	case partImage:
	case partProgressBar:
		return;
	case partButton:
	case partCheckbox:
		g_object_set(p->r, "sensitive", editable != 0, NULL);
		return;
	}
	g_object_set(p->r, "editable", editable != 0, NULL);
}

void uiTableColumnPartSetTextColor(uiTableColumn *c, int part, int modelColumn)
{
	struct tablePart *p;

	p = (struct tablePart *) g_ptr_array_index(c->parts, part);
	p->colorColumn = modelColumn;
	// TODO refresh table
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

	c = uiprivNew(uiTableColumn);
	c->c = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(c->c, TRUE);
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

	t->model = model;
	t->backgroundColumn = -1;

	t->widget = gtk_scrolled_window_new(NULL, NULL);
	t->scontainer = GTK_CONTAINER(t->widget);
	t->sw = GTK_SCROLLED_WINDOW(t->widget);
	gtk_scrolled_window_set_shadow_type(t->sw, GTK_SHADOW_IN);

	t->treeWidget = gtk_tree_view_new_with_model(GTK_TREE_MODEL(t->model));
	t->tv = GTK_TREE_VIEW(t->treeWidget);
	// TODO set up t->tv

	gtk_container_add(t->scontainer, t->treeWidget);
	// and make the tree view visible; only the scrolled window's visibility is controlled by libui
	gtk_widget_show(t->treeWidget);

	return t;
}
