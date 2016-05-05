// 18 october 2015
#include "uipriv_unix.h"

struct uiTable {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *scontainer;
	GtkScrolledWindow *sw;
	GtkWidget *treeWidget;
	GtkTreeView *treeview;
	GtkTreeSelection *selection;
	uiTableModel *model;
};

uiUnixDefineControl(
	uiTable								// type name
)

void uiTableSetModel(uiTable *t, uiTableModel *m)
{
	t->model = m;
	gtk_tree_view_set_model(t->treeview, GTK_TREE_MODEL(t->model));
}

void uiTableAppendColumn(uiTable *t, uiTableColumnParams *p)
{
	GtkTreeViewColumn *col;
	GtkCellRenderer *r;
	const char *attribute;
	const char *mutableAttr;
	gboolean mutable;

	switch (p->Type) {
	case uiTableColumnText:
		r = gtk_cell_renderer_text_new();
		attribute = "text";
		mutableAttr = "editable";
		break;
//TODO	case uiTableColumnImage:
		// TODO
	case uiTableColumnCheckbox:
		r = gtk_cell_renderer_toggle_new();
		attribute = "active";
		mutableAttr = "activatable";
		break;
	default:
		complain("unknown table column type %d in uiTableAppendColumn()", p->Type);
	}
	mutable = FALSE;
	if (p->Mutable)
		mutable = TRUE;
	g_object_set(r,
		mutableAttr, mutable,
		NULL);
	col = gtk_tree_view_column_new_with_attributes(p->Name, r,
		attribute, p->ValueColumn,
		NULL);
	// allow columns to be resized
	gtk_tree_view_column_set_resizable(col, TRUE);
	gtk_tree_view_append_column(t->treeview, col);
}

uiTable *uiNewTable(void)
{
	uiTable *t;

	t = (uiTable *) uiNewControl(uiTableType());

	t->widget = gtk_scrolled_window_new(NULL, NULL);
	t->scontainer = GTK_CONTAINER(t->widget);
	t->sw = GTK_SCROLLED_WINDOW(t->widget);

	t->treeWidget = gtk_tree_view_new();
	t->treeview = GTK_TREE_VIEW(t->treeWidget);

	t->selection = gtk_tree_view_get_selection(t->treeview);

	// give a border and add the table
	gtk_scrolled_window_set_shadow_type(t->sw, GTK_SHADOW_IN);
	gtk_container_add(t->scontainer, t->treeWidget);
	// and make the table visible; only the scrolled window's visibility is controlled by libui
	gtk_widget_show(t->treeWidget);

	uiUnixFinishNewControl(t, uiTable);

	return t;
}
