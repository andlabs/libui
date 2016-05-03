// 4 september 2015
#include "dtp.h"

// TODO imitate gnome-calendar's day/month/year entries
// TODO connect to ::output to add a leading 0 to minutes and seconds

struct dtpPrivate {
	GtkWidget *calendar;
	GtkWidget *timebox;
	GtkWidget *hours;
	GtkWidget *minutes;
	GtkWidget *seconds;
	GtkWidget *ampm;
};

G_DEFINE_TYPE(dateTimePickerWidget, dateTimePickerWidget, GTK_TYPE_BOX)

static gboolean zeroPadSpinbox(GtkSpinButton *sb, gpointer data)
{
	gchar *text;
	int value;

	value = (int) gtk_spin_button_get_value(sb);
	text = g_strdup_printf("%02d", value);
	gtk_entry_set_text(GTK_ENTRY(sb), text);
	g_free(text);
	return TRUE;
}

static GtkWidget *newSpinbox(int min, int max, gboolean zeroPad)
{
	GtkWidget *sb;

	sb = gtk_spin_button_new_with_range(min, max, 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(sb), 0);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(sb), TRUE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(sb), GTK_ORIENTATION_VERTICAL);
	if (zeroPad)
		g_signal_connect(sb, "output", G_CALLBACK(zeroPadSpinbox), NULL);
	return sb;
}

static void dateTimePickerWidget_init(dateTimePickerWidget *d)
{
	d->priv = G_TYPE_INSTANCE_GET_PRIVATE(d, dateTimePickerWidgetType, struct dtpPrivate);

	gtk_orientable_set_orientation(GTK_ORIENTABLE(d), GTK_ORIENTATION_VERTICAL);
	gtk_box_set_spacing(GTK_BOX(d), 6);

	d->priv->calendar = gtk_calendar_new();
	gtk_container_add(GTK_CONTAINER(d), d->priv->calendar);

	d->priv->timebox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_widget_set_valign(d->priv->timebox, GTK_ALIGN_CENTER);
	gtk_container_add(GTK_CONTAINER(d), d->priv->timebox);

	d->priv->hours = newSpinbox(1, 12, FALSE);
	gtk_container_add(GTK_CONTAINER(d->priv->timebox), d->priv->hours);

	gtk_container_add(GTK_CONTAINER(d->priv->timebox),
		gtk_label_new(":"));

	d->priv->minutes = newSpinbox(0, 59, TRUE);
	gtk_container_add(GTK_CONTAINER(d->priv->timebox), d->priv->minutes);

	gtk_container_add(GTK_CONTAINER(d->priv->timebox),
		gtk_label_new(":"));

	d->priv->seconds = newSpinbox(0, 59, TRUE);
	gtk_container_add(GTK_CONTAINER(d->priv->timebox), d->priv->seconds);

	d->priv->ampm = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(d->priv->ampm), NULL, "AM");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(d->priv->ampm), NULL, "PM");
	gtk_widget_set_valign(d->priv->ampm, GTK_ALIGN_CENTER);
	gtk_container_add(GTK_CONTAINER(d->priv->timebox), d->priv->ampm);

	gtk_widget_show(d->priv->calendar);
	gtk_widget_show_all(d->priv->timebox);
}

static void dateTimePickerWidget_dispose(GObject *obj)
{
	G_OBJECT_CLASS(dateTimePickerWidget_parent_class)->dispose(obj);
}

static void dateTimePickerWidget_finalize(GObject *obj)
{
	G_OBJECT_CLASS(dateTimePickerWidget_parent_class)->finalize(obj);
}

static void dateTimePickerWidget_class_init(dateTimePickerWidgetClass *class)
{
	G_OBJECT_CLASS(class)->dispose = dateTimePickerWidget_dispose;
	G_OBJECT_CLASS(class)->finalize = dateTimePickerWidget_finalize;

	g_type_class_add_private(G_OBJECT_CLASS(class), sizeof (struct dtpPrivate));
}
