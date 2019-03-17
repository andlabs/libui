// 25 june 2018
#include <gtk/gtk.h>

GtkWidget *mainwin;
GtkWidget *vbox;
GtkWidget *hbox;
GtkWidget *startProgress;
GtkWidget *startTable;
GtkWidget *progressbar;
GtkWidget *scrolledWindow;
GtkListStore *model;
GtkWidget *treeview;
GtkWidget *hbox2;

static gboolean pulseProgress(gpointer data)
{
	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progressbar));
	return TRUE;
}

static void onStartProgressClicked(GtkButton *button, gpointer data)
{
	gtk_widget_set_sensitive(startProgress, FALSE);
	g_timeout_add(100, pulseProgress, NULL);
}

gboolean pbarStarted = FALSE;
gint pbarValue;

static void pbarDataFunc(GtkTreeViewColumn *col, GtkCellRenderer *r, GtkTreeModel *m, GtkTreeIter *iter, gpointer data)
{
	if (!pbarStarted) {
		g_object_set(r,
			"pulse", -1,
			"value", 0,
			NULL);
		return;
	}
	pbarValue++;
	if (pbarValue == G_MAXINT)
		pbarValue = 1;
	g_object_set(r, "pulse", pbarValue, NULL);
}

static gboolean pulseTable(gpointer data)
{
	GtkTreePath *path;
	GtkTreeIter iter;

	path = gtk_tree_path_new_from_indices(0, -1);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, path);
	gtk_tree_model_row_changed(GTK_TREE_MODEL(model), path, &iter);
	gtk_tree_path_free(path);
	return TRUE;
}

static void onStartTableClicked(GtkButton *button, gpointer data)
{
	pbarStarted = TRUE;
	pbarValue = 0;

	gtk_widget_set_sensitive(startTable, FALSE);
	g_timeout_add(100, pulseTable, NULL);
}

static gboolean onClosing(GtkWidget *win, GdkEvent *e, gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

int main(void)
{
	GtkTreeIter iter;
	GtkTreeViewColumn *col;
	GtkCellRenderer *r;

	gtk_init(NULL, NULL);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(mainwin, "delete-event", G_CALLBACK(onClosing), NULL);

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);
	gtk_container_add(GTK_CONTAINER(mainwin), vbox);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_widget_set_halign(hbox, GTK_ALIGN_CENTER);
	gtk_container_add(GTK_CONTAINER(vbox), hbox);

	startProgress = gtk_button_new_with_label("Start Progress Bar");
	g_signal_connect(startProgress, "clicked", G_CALLBACK(onStartProgressClicked), NULL);
	gtk_container_add(GTK_CONTAINER(hbox), startProgress);

	startTable = gtk_button_new_with_label("Start Table Cell Renderer");
	g_signal_connect(startTable, "clicked", G_CALLBACK(onStartTableClicked), NULL);
	gtk_container_add(GTK_CONTAINER(hbox), startTable);

	progressbar = gtk_progress_bar_new();
	gtk_container_add(GTK_CONTAINER(vbox), progressbar);

	scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledWindow), GTK_SHADOW_IN);
	gtk_widget_set_vexpand(scrolledWindow, TRUE);
	gtk_container_add(GTK_CONTAINER(vbox), scrolledWindow);

	model = gtk_list_store_new(1, G_TYPE_INT);
	gtk_list_store_append(model, &iter);
	gtk_list_store_set(model, &iter,
		0, 0,
		-1);

	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	gtk_container_add(GTK_CONTAINER(scrolledWindow), treeview);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_resizable(col, TRUE);
	gtk_tree_view_column_set_title(col, "Column");
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);

	r = gtk_cell_renderer_progress_new();
	gtk_tree_view_column_pack_start(col, r, TRUE);
	gtk_tree_view_column_set_cell_data_func(col, r, pbarDataFunc, NULL, NULL);

	hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_widget_set_halign(hbox2, GTK_ALIGN_CENTER);
	gtk_container_add(GTK_CONTAINER(vbox), hbox2);

	gtk_container_add(GTK_CONTAINER(hbox2), gtk_button_new_with_label("These buttons"));
	gtk_container_add(GTK_CONTAINER(hbox2), gtk_button_new_with_label("do nothing"));
	gtk_container_add(GTK_CONTAINER(hbox2), gtk_button_new_with_label("when clicked"));

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
