// 26 june 2016
#include "uipriv_unix.h"
#include "table.h"

struct uiTable {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *scontainer;
	GtkScrolledWindow *sw;
	GtkWidget *treeWidget;
	GtkTreeView *tv;
	uiTableModel *model;
	int backgroundColumn;
};

// use the same size as GtkFileChooserWidget's treeview
// TODO refresh when icon theme changes
// TODO doesn't work when scaled?
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

static void applyColor(GtkTreeModel *m, GtkTreeIter *iter, int modelColumn, GtkCellRenderer *r, const char *prop, const char *propSet)
{
	GValue value = G_VALUE_INIT;
	GdkRGBA *rgba;

	gtk_tree_model_get_value(m, iter, modelColumn, &value);
	rgba = (GdkRGBA *) g_value_get_boxed(&value);
	if (rgba != NULL)
		g_object_set(r, prop, rgba, NULL);
	else
		g_object_set(r, propSet, FALSE, NULL);
	g_value_unset(&value);
}

static void setEditable(uiTableModel *m, GtkTreeIter *iter, int modelColumn, GtkCellRenderer *r, const char *prop)
{
	uiTableData *data;
	GValue value = G_VALUE_INIT;
	int value;
	gboolean editable;

	switch (modelColumn) {
	case uiTableModelColumnNeverEditable:
		editable = FALSE;
		break;
	case uiTableModelColumnAlwaysEditable:
		editable = TRUE;
		break;
	default:
		gtk_tree_model_get_value(m, iter, p->editableColumn, &value);
		editable = gtk_value_get_int(&value) != 0;
		g_value_unset(&value);
	}
	g_object_set(r, "editable", editable, NULL);
}

static void applyBackgroundColor(uiTable *t, GtkTreeModel *m, GtkTreeIter *iter, GtkCellRenderer *r)
{
	if (t->backgroundColumn != -1)
		applyColor(m, iter, t->backgroundColumn,
			r, "cell-background-rgba", "cell-background-set");
}

static void onEdited(uiTableModel *m, int column, const char *pathstr, const uiTableData *data, GtkTreeIter *iter)
{
	GtkTreePath *path;
	int row;

	path = gtk_tree_path_new_from_string(pathstr);
	row = gtk_tree_path_get_indices(path)[0];
	if (iter != NULL)
		gtk_tree_model_convert_path_to_iter(m, path, iter);
	gtk_tree_path_free(path);
	(*(m->mh->SetCellValue))(m->mh, m, row, column, data);
}

// TODO deduplicate this between platforms
static uiTableTextColumnOptionalParams defaultTextColumnOptionalParams = {
	.ColorModelColumn = -1,
};

struct textColumnParams {
	uiTable *t;
	uiTableModel *m;
	int modelColumn;
	int editableColumn;
	uiTableTextColumnOptionalParams params;
};

static void textColumnDataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *m, GtkTreeIter *iter, gpointer data)
{
	struct textColumnParams *p = (struct textColumnParams *) data;
	GValue value = G_VALUE_INIT;
	const gchar *str;
	gboolean editable;

	gtk_tree_model_get_value(m, iter, p->modelColumn, &value);
	str = g_value_get_string(&value);
	g_object_set(r, "text", str, NULL);
	g_value_unset(&value);

	setEditable(m, iter, p->editableColumn, r, "editable");

	if (p->params.ColorModelColumn != -1)
		applyColor(m, iter, p->params.ColorModelColumn,
			r, "foreground-rgba", "foreground-set");

	applyBackgroundColor(p->t, m, iter, r);
}

static void textColumnEdited(GtkCellRendererText *renderer, gchar *path, gchar *newText, gpointer data)
{
	struct textColumnParams *p = (struct textColumnParams *) data;
	uiTableData *data;
	GtkTreeIter iter;

	data = uiNewTableDataString(newText);
	onEdited(p->m, p->textColumn, path, data, &iter);
	uiFreeData(data);
	// and update the column TODO copy comment here
	textColumnDataFunc(NULL, r, GTK_TREE_MODEL(p->m), &iter, data);
}

struct imageColumnParams {
	uiTable *t;
	int modelColumn;
};

static void imageColumnDataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *m, GtkTreeIter *iter, gpointer data)
{
	struct imageColumnParams *p = (struct imageColumnParams *) data;
	GValue value = G_VALUE_INIT;
	uiImage *img;

//TODO	setImageSize(r);
	gtk_tree_model_get_value(m, iter, p->modelColumn, &value);
	img = (uiImage *) g_value_get_pointer(&value);
	g_object_set(r, "surface",
		uiprivImageAppropriateSurface(img, p->t->treeWidget),
		NULL);
	g_value_unset(&value);

	applyBackgroundColor(p->t, m, iter, r);
}

struct checkboxColumnParams {
	uiTable *t;
	uiTableModel *m;
	int modelColumn;
	int editableColumn;
};

static void checkboxColumnDataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *m, GtkTreeIter *iter, gpointer data)
{
	struct checkboxColumnParams *p = (struct checkboxColumnParams *) data;
	GValue value = G_VALUE_INIT;
	gboolean active;

	gtk_tree_model_get_value(m, iter, p->modelColumn, &value);
	active = g_value_get_int(&value) != 0;
	g_object_set(r, "active", active, NULL);
	g_value_unset(&value);

	setEditable(m, iter, p->editableColumn, r, "activatable");

	applyBackgroundColor(p->t, m, iter, r);
}

static void checkboxColumnToggled(GtkCellRendererToggle *r, gchar *pathstr, gpointer data)
{
	struct checkboxColumnParams *p = (struct checkboxColumnParams *) data;
	GValue value = G_VALUE_INIT;
	int v;
	uiTableData *data;
	GtkTreeIter iter;

	gtk_tree_model_get_value(p->m, iter, p->modelColumn, &value);
	v = g_value_get_int(&value);
	g_value_unset(&value);
	data = uiNewTableDataInt(!v);
	onEdited(p->m, p->modelColumn, path, data, &iter);
	uiFreeData(data);
	// and update the column TODO copy comment here
	// TODO avoid fetching the model data twice
	checkboxColumnDataFunc(NULL, r, GTK_TREE_MODEL(p->m), &iter, data);
}

struct progressBarColumnParams {
	uiTable *t;
	int modelColumn;
};

static void progressBarDataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *m, GtkTreeIter *iter, gpointer data)
{
	struct progressBarColumnParams *p = (struct progressBarColumnParams *) data;
	GValue value = G_VALUE_INIT;
	int pval;

	gtk_tree_model_get_value(m, iter, p->modelColumn, &value);
	pval = g_value_get_int(&value);
	if (pval == -1) {
		// TODO
	} else
		g_object_set(r,
			"pulse", -1,
			"value", pval,
			NULL);
	g_value_unset(&value);

	applyBackgroundColor(p->t, m, iter, r);
}

struct buttonColumnParams {
	uiTable *t;
	uiTableModel *m;
	int modelColumn;
	int clickableColumn;
};

static void buttonDataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *m, GtkTreeIter *iter, gpointer data)
{
	struct buttonColumnParams *p = (struct buttonColumnParams *) data;
	GValue value = G_VALUE_INIT;
	const gchar *str;
	gboolean clickable;

	gtk_tree_model_get_value(m, iter, p->modelColumn, &value);
	str = g_value_get_string(&value);
	g_object_set(r, "text", str, NULL);
	g_value_unset(&value);

	setEditable(m, iter, p->clickableColumn, r, "clickable");

	applyBackgroundColor(p->t, m, iter, r);
}

static void buttonColumnClicked(uiprivCellRendererButton *r, gchar *pathstr, gpointer data)
{
	struct buttonColumnParams *p = (struct buttonColumnParams *) data;

	onEdited(p->m, p->modelColumn, path, NULL, NULL);
}

=================== TODOTODO

static void appendPart(uiTableColumn *c, struct tablePart *part, GtkCellRenderer *r, int expand)
{
	part->r = r;
	gtk_tree_view_column_pack_start(c->c, part->r, expand != 0);
	gtk_tree_view_column_set_cell_data_func(c->c, part->r, dataFunc, part, NULL);
	g_ptr_array_add(c->parts, part);
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
