// draw.c
extern uiDrawContext *newContext(cairo_t *cr, GtkStyleContext *style);
extern void freeContext(uiDrawContext *);

// image.c
/*TODO remove this*/typedef struct uiImage uiImage;
extern cairo_surface_t *imageAppropriateSurface(uiImage *i, GtkWidget *w);

// cellrendererbutton.c
extern GtkCellRenderer *newCellRendererButton(void);

// future.c
extern void loadFutures(void);
extern PangoAttribute *FUTURE_pango_attr_font_features_new(const gchar *features);
extern PangoAttribute *FUTURE_pango_attr_foreground_alpha_new(guint16 alpha);
extern PangoAttribute *FUTURE_pango_attr_background_alpha_new(guint16 alpha);
extern gboolean FUTURE_gtk_widget_path_iter_set_object_name(GtkWidgetPath *path, gint pos, const char *name);
