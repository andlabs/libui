// 26 june 2016
#include "uipriv_unix.h"
#include "table.h"

// TODO with GDK_SCALE set to 2 the 32x32 images are scaled up to 64x64?

struct uiTable {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *scontainer;
	GtkScrolledWindow *sw;
	GtkWidget *treeWidget;
	GtkTreeView *tv;
	uiTableModel *model;
	GPtrArray *columnParams;
	int backgroundColumn;
	// keys are struct rowcol, values are gint
	// TODO document this properly
	GHashTable *indeterminatePositions;
	guint indeterminateTimer;
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
	GtkTreePath *path;
	int row;
	gboolean editable;

	// TODO avoid the need for this
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(m), iter);
	row = gtk_tree_path_get_indices(path)[0];
	gtk_tree_path_free(path);
	editable = uiprivTableModelCellEditable(m, row, modelColumn) != 0;
	g_object_set(r, prop, editable, NULL);
}

static void applyBackgroundColor(uiTable *t, GtkTreeModel *m, GtkTreeIter *iter, GtkCellRenderer *r)
{
	if (t->backgroundColumn != -1)
		applyColor(m, iter, t->backgroundColumn,
			r, "cell-background-rgba", "cell-background-set");
}

static void onEdited(uiTableModel *m, int column, const char *pathstr, const uiTableValue *tvalue, GtkTreeIter *iter)
{
	GtkTreePath *path;
	int row;

	path = gtk_tree_path_new_from_string(pathstr);
	row = gtk_tree_path_get_indices(path)[0];
	if (iter != NULL)
		gtk_tree_model_get_iter(GTK_TREE_MODEL(m), iter, path);
	gtk_tree_path_free(path);
	uiprivTableModelSetCellValue(m, row, column, tvalue);
}

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

	gtk_tree_model_get_value(m, iter, p->modelColumn, &value);
	str = g_value_get_string(&value);
	g_object_set(r, "text", str, NULL);
	g_value_unset(&value);

	setEditable(p->m, iter, p->editableColumn, r, "editable");

	if (p->params.ColorModelColumn != -1)
		applyColor(m, iter, p->params.ColorModelColumn,
			r, "foreground-rgba", "foreground-set");

	applyBackgroundColor(p->t, m, iter, r);
}

static void textColumnEdited(GtkCellRendererText *r, gchar *path, gchar *newText, gpointer data)
{
	struct textColumnParams *p = (struct textColumnParams *) data;
	uiTableValue *tvalue;
	GtkTreeIter iter;

	tvalue = uiNewTableValueString(newText);
	onEdited(p->m, p->modelColumn, path, tvalue, &iter);
	uiFreeTableValue(tvalue);
	// and update the column TODO copy comment here
	textColumnDataFunc(NULL, GTK_CELL_RENDERER(r), GTK_TREE_MODEL(p->m), &iter, data);
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

	setEditable(p->m, iter, p->editableColumn, r, "activatable");

	applyBackgroundColor(p->t, m, iter, r);
}

static void checkboxColumnToggled(GtkCellRendererToggle *r, gchar *pathstr, gpointer data)
{
	struct checkboxColumnParams *p = (struct checkboxColumnParams *) data;
	GValue value = G_VALUE_INIT;
	int v;
	uiTableValue *tvalue;
	GtkTreePath *path;
	GtkTreeIter iter;

	path = gtk_tree_path_new_from_string(pathstr);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(p->m), &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get_value(GTK_TREE_MODEL(p->m), &iter, p->modelColumn, &value);
	v = g_value_get_int(&value);
	g_value_unset(&value);
	tvalue = uiNewTableValueInt(!v);
	onEdited(p->m, p->modelColumn, pathstr, tvalue, NULL);
	uiFreeTableValue(tvalue);
	// and update the column TODO copy comment here
	// TODO avoid fetching the model data twice
	checkboxColumnDataFunc(NULL, GTK_CELL_RENDERER(r), GTK_TREE_MODEL(p->m), &iter, data);
}

struct progressBarColumnParams {
	uiTable *t;
	int modelColumn;
};

struct rowcol {
	int row;
	int col;
};

static guint rowcolHash(gconstpointer key)
{
	const struct rowcol *rc = (const struct rowcol *) key;
	guint row, col;

	row = (guint) (rc->row);
	col = (guint) (rc->col);
	return row ^ col;
}

static gboolean rowcolEqual(gconstpointer a, gconstpointer b)
{
	const struct rowcol *ra = (const struct rowcol *) a;
	const struct rowcol *rb = (const struct rowcol *) b;

	return (ra->row == rb->row) && (ra->col == rb->col);
}

static void pulseOne(gpointer key, gpointer value, gpointer data)
{
	uiTable *t = uiTable(data);
	struct rowcol *rc = (struct rowcol *) key;

	// TODO this is bad: it produces changed handlers for every table because that's how GtkTreeModel works, yet this is per-table because that's how it works
	// however, a proper fix would require decoupling progress from normal integers, which we could do...
	uiTableModelRowChanged(t->model, rc->row);
}

static gboolean indeterminatePulse(gpointer data)
{
	uiTable *t = uiTable(data);

	g_hash_table_foreach(t->indeterminatePositions, pulseOne, t);
	return TRUE;
}

static void progressBarColumnDataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *m, GtkTreeIter *iter, gpointer data)
{
	struct progressBarColumnParams *p = (struct progressBarColumnParams *) data;
	GValue value = G_VALUE_INIT;
	int pval;
	struct rowcol *rc;
	gint *val;
	GtkTreePath *path;

	gtk_tree_model_get_value(m, iter, p->modelColumn, &value);
	pval = g_value_get_int(&value);
	rc = uiprivNew(struct rowcol);
	// TODO avoid the need for this
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(m), iter);
	rc->row = gtk_tree_path_get_indices(path)[0];
	gtk_tree_path_free(path);
	rc->col = p->modelColumn;
	val = (gint *) g_hash_table_lookup(p->t->indeterminatePositions, rc);
	if (pval == -1) {
		if (val == NULL) {
			val = uiprivNew(gint);
			*val = 1;
			g_hash_table_insert(p->t->indeterminatePositions, rc, val);
		} else {
			uiprivFree(rc);
			(*val)++;
			if (*val == G_MAXINT)
				*val = 1;
		}
		g_object_set(r,
			"pulse", *val,
			NULL);
		if (p->t->indeterminateTimer == 0)
			// TODO verify the timeout
			p->t->indeterminateTimer = g_timeout_add(100, indeterminatePulse, p->t);
	} else {
		if (val != NULL) {
			g_hash_table_remove(p->t->indeterminatePositions, rc);
			if (g_hash_table_size(p->t->indeterminatePositions) == 0) {
				g_source_remove(p->t->indeterminateTimer);
				p->t->indeterminateTimer = 0;
			}
		}
		uiprivFree(rc);
		g_object_set(r,
			"pulse", -1,
			"value", pval,
			NULL);
	}
	g_value_unset(&value);

	applyBackgroundColor(p->t, m, iter, r);
}

struct buttonColumnParams {
	uiTable *t;
	uiTableModel *m;
	int modelColumn;
	int clickableColumn;
};

static void buttonColumnDataFunc(GtkTreeViewColumn *c, GtkCellRenderer *r, GtkTreeModel *m, GtkTreeIter *iter, gpointer data)
{
	struct buttonColumnParams *p = (struct buttonColumnParams *) data;
	GValue value = G_VALUE_INIT;
	const gchar *str;

	gtk_tree_model_get_value(m, iter, p->modelColumn, &value);
	str = g_value_get_string(&value);
	g_object_set(r, "text", str, NULL);
	g_value_unset(&value);

	setEditable(p->m, iter, p->clickableColumn, r, "sensitive");

	applyBackgroundColor(p->t, m, iter, r);
}

// TODO wrong type here
static void buttonColumnClicked(GtkCellRenderer *r, gchar *pathstr, gpointer data)
{
	struct buttonColumnParams *p = (struct buttonColumnParams *) data;

	onEdited(p->m, p->modelColumn, pathstr, NULL, NULL);
}

static GtkTreeViewColumn *addColumn(uiTable *t, const char *name)
{
	GtkTreeViewColumn *c;

	c = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(c, TRUE);
	gtk_tree_view_column_set_title(c, name);
	gtk_tree_view_append_column(t->tv, c);
	return c;
}

static void addTextColumn(uiTable *t, GtkTreeViewColumn *c, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	struct textColumnParams *p;
	GtkCellRenderer *r;

	p = uiprivNew(struct textColumnParams);
	p->t = t;
	// TODO get rid of these fields AND rename t->model in favor of t->m
	p->m = t->model;
	p->modelColumn = textModelColumn;
	p->editableColumn = textEditableModelColumn;
	if (textParams != NULL)
		p->params = *textParams;
	else
		p->params = uiprivDefaultTextColumnOptionalParams;

	r = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(c, r, TRUE);
	gtk_tree_view_column_set_cell_data_func(c, r, textColumnDataFunc, p, NULL);
	g_signal_connect(r, "edited", G_CALLBACK(textColumnEdited), p);
	g_ptr_array_add(t->columnParams, p);
}

// TODO rename modelCOlumn and params everywhere
void uiTableAppendTextColumn(uiTable *t, const char *name, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	GtkTreeViewColumn *c;

	c = addColumn(t, name);
	addTextColumn(t, c, textModelColumn, textEditableModelColumn, textParams);
}

static void addImageColumn(uiTable *t, GtkTreeViewColumn *c, int imageModelColumn)
{
	struct imageColumnParams *p;
	GtkCellRenderer *r;

	p = uiprivNew(struct imageColumnParams);
	p->t = t;
	p->modelColumn = imageModelColumn;

	r = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(c, r, FALSE);
	gtk_tree_view_column_set_cell_data_func(c, r, imageColumnDataFunc, p, NULL);
	g_ptr_array_add(t->columnParams, p);
}

void uiTableAppendImageColumn(uiTable *t, const char *name, int imageModelColumn)
{
	GtkTreeViewColumn *c;

	c = addColumn(t, name);
	addImageColumn(t, c, imageModelColumn);
}

void uiTableAppendImageTextColumn(uiTable *t, const char *name, int imageModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	GtkTreeViewColumn *c;

	c = addColumn(t, name);
	addImageColumn(t, c, imageModelColumn);
	addTextColumn(t, c, textModelColumn, textEditableModelColumn, textParams);
}

static void addCheckboxColumn(uiTable *t, GtkTreeViewColumn *c, int checkboxModelColumn, int checkboxEditableModelColumn)
{
	struct checkboxColumnParams *p;
	GtkCellRenderer *r;

	p = uiprivNew(struct checkboxColumnParams);
	p->t = t;
	p->m = t->model;
	p->modelColumn = checkboxModelColumn;
	p->editableColumn = checkboxEditableModelColumn;

	r = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(c, r, FALSE);
	gtk_tree_view_column_set_cell_data_func(c, r, checkboxColumnDataFunc, p, NULL);
	g_signal_connect(r, "toggled", G_CALLBACK(checkboxColumnToggled), p);
	g_ptr_array_add(t->columnParams, p);
}

void uiTableAppendCheckboxColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn)
{
	GtkTreeViewColumn *c;

	c = addColumn(t, name);
	addCheckboxColumn(t, c, checkboxModelColumn, checkboxEditableModelColumn);
}

void uiTableAppendCheckboxTextColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	GtkTreeViewColumn *c;

	c = addColumn(t, name);
	addCheckboxColumn(t, c, checkboxModelColumn, checkboxEditableModelColumn);
	addTextColumn(t, c, textModelColumn, textEditableModelColumn, textParams);
}

void uiTableAppendProgressBarColumn(uiTable *t, const char *name, int progressModelColumn)
{
	GtkTreeViewColumn *c;
	struct progressBarColumnParams *p;
	GtkCellRenderer *r;

	c = addColumn(t, name);

	p = uiprivNew(struct progressBarColumnParams);
	p->t = t;
	// TODO make progress and progressBar consistent everywhere
	p->modelColumn = progressModelColumn;

	r = gtk_cell_renderer_progress_new();
	gtk_tree_view_column_pack_start(c, r, TRUE);
	gtk_tree_view_column_set_cell_data_func(c, r, progressBarColumnDataFunc, p, NULL);
	g_ptr_array_add(t->columnParams, p);
}

void uiTableAppendButtonColumn(uiTable *t, const char *name, int buttonModelColumn, int buttonClickableModelColumn)
{
	GtkTreeViewColumn *c;
	struct buttonColumnParams *p;
	GtkCellRenderer *r;

	c = addColumn(t, name);

	p = uiprivNew(struct buttonColumnParams);
	p->t = t;
	p->m = t->model;
	p->modelColumn = buttonModelColumn;
	p->clickableColumn = buttonClickableModelColumn;

	r = uiprivNewCellRendererButton();
	gtk_tree_view_column_pack_start(c, r, TRUE);
	gtk_tree_view_column_set_cell_data_func(c, r, buttonColumnDataFunc, p, NULL);
	g_signal_connect(r, "clicked", G_CALLBACK(buttonColumnClicked), p);
	g_ptr_array_add(t->columnParams, p);
}

uiUnixControlAllDefaultsExceptDestroy(uiTable)

static void uiTableDestroy(uiControl *c)
{
	uiTable *t = uiTable(c);
	guint i;

	for (i = 0; i < t->columnParams->len; i++)
		uiprivFree(g_ptr_array_index(t->columnParams, i));
	g_ptr_array_free(t->columnParams, TRUE);
	if (g_hash_table_size(t->indeterminatePositions) != 0)
		g_source_remove(t->indeterminateTimer);
	g_hash_table_destroy(t->indeterminatePositions);
	g_object_unref(t->widget);
	uiFreeControl(uiControl(t));
}

uiTable *uiNewTable(uiTableParams *p)
{
	uiTable *t;

	uiUnixNewControl(uiTable, t);

	t->model = p->Model;
	t->columnParams = g_ptr_array_new();
	t->backgroundColumn = p->RowBackgroundColorModelColumn;

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

	t->indeterminatePositions = g_hash_table_new_full(rowcolHash, rowcolEqual,
		uiprivFree, uiprivFree);

	return t;
}
